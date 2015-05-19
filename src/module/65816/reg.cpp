
#include <idp.hpp>
#include <srarea.hpp>
#include <diskio.hpp>

#include "m65816.hpp"

#include "../../ldr/snes/addr.cpp"

//lint -esym(714,device)       is not referenced
//--------------------------------------------------------------------------
static const char *const RegNames[] =
{
  "A",  // Accumulator
  "X",  // Index
  "Y",  // Index
  "S",  // Stack register (used?)
  "cs",
  "ds",

  "PB", // Program bank
  "B",  // Data bank
  "D",  // Direct page register (used?)

  "m", // Holds accumulator-is-8-bits flag
  "x", // Holds indices-are-8-bits flag
  "e"  // Holds emulation mode flag
};


//----------------------------------------------------------------------
//       Prepare global variables & defines for ../iocommon.cpp
//----------------------------------------------------------------------
static netnode helper;
char device[MAXSTR];
static size_t numports = 0;
static ioport_t *ports = NULL;
SuperFamicomCartridge cartridge;

//--------------------------------------------------------------------------
//lint -esym(528,ioresp_ok) is not referenced
static bool ioresp_ok(void)
{
  if ( inf.like_binary() )
    return true;
  else
    return get_segm_by_name("gpuapu") != NULL;
}
#define CHECK_IORESP      ioresp_ok()
#include "../iocommon.cpp"

// ---------------------------------------------------------------------------
// Handler for: get_autocmt.
// Will possibly store a comment in 'buf',
// depending on whether an autocmt is deemed necessary
// for the current line.
//
// For the moment this will just print, in a user-friendly
// way, information about the addressing mode, if needed.
static bool make_insn_cmt(char *buf, size_t bufsize)
{
  uint8 opcode = get_byte(cmd.ea);
  const struct opcode_info_t &opcode_info = get_opcode_info(opcode);
  static const bool addressing_info_required[] =
  {
    false, // ABS
    false, // ABS_IX,
    false, // ABS_IY,
    false, // ABS_IX_INDIR,
    false, // ABS_INDIR,
    false, // ABS_INDIR_LONG,
    false, // ABS_LONG,
    false, // ABS_LONG_IX,
    false, // ACC,
    true , // BLK_MOV,
    false, // DP,
    false, // DP_IX,
    false, // DP_IY,
    false, // DP_IX_INDIR,
    false, // DP_INDIR,
    false, // DP_INDIR_LONG,
    false, // DP_INDIR_IY,
    false, // DP_INDIR_LONG_IY,
    false, // IMM,
    false, // IMPLIED,
    true , // PC_REL,
    true , // PC_REL_LONG,
    false, // STACK_ABS,
    false, // STACK_DP_INDIR,
    false, // STACK_INT,
    false, // STACK_PC_REL,
    false, // STACK_PULL,
    false, // STACK_PUSH,
    false, // STACK_RTI,
    false, // STACK_RTL,
    false, // STACK_RTS,
    false, // STACK_REL,
    false  // STACK_REL_INDIR_IY,
  };

  if ( !addressing_info_required[opcode_info.addr] )
    return false;

  const struct addrmode_info_t &addrmode_info = AddressingModes[opcode_info.addr];
  qstrncpy(buf, addrmode_info.name, bufsize);
  return true;
}

//----------------------------------------------------------------------
static int idaapi notify(processor_t::idp_notify msgid, ...)
{
  va_list va;
  va_start(va, msgid);

  // A well behaved processor module should call invoke_callbacks()
  // in his notify() function. If this function returns 0, then
  // the processor module should process the notification itself
  // Otherwise the code should be returned to the caller:
  int code = invoke_callbacks(HT_IDP, msgid, va);
  if ( code )
    return code;

  int retcode = 1;
  switch ( msgid )
  {
    case processor_t::init:
      helper.create("$ m65816");
      break;
    case processor_t::term:
      free_ioports(ports, numports);
      break;
    case processor_t::newprc:
      break;
    case processor_t::newseg:
      {
        segment_t *sptr = va_arg(va, segment_t *);

        // default DS is equal to CS
        sptr->defsr[rDs - ph.regFirstSreg] = sptr->sel;

        // detect SNES bank 0
        if (xlat(0) == (sptr->startEA & 0xff0000))
        {
          // initial bank must be $00 (especially important on HiROM)
          // Example: Donkey Kong Country 2 - Emulation_mode_RESET
          sptr->defsr[rB  - ph.regFirstSreg] = 0;
          sptr->defsr[rPB - ph.regFirstSreg] = 0;
        }
        else
        {
          // otherwise, set the default bank number from EA
          uint8 pb = sptr->startEA >> 16;
          sptr->defsr[rB  - ph.regFirstSreg] = pb;
          sptr->defsr[rPB - ph.regFirstSreg] = pb;
        }
      }
      break;
    case processor_t::oldfile:
    case processor_t::newfile:
      {
        cartridge.read_hash(helper);
        //cartridge.print();

        // read rommode_t for backward compatibility
        nodeidx_t mode = helper.hashval_long("rommode_t");
        switch ( mode )
        {
          case 0x20:
            cartridge.mapper = SuperFamicomCartridge::LoROM;
            break;

          case 0x21:
            cartridge.mapper = SuperFamicomCartridge::HiROM;
            break;
        }

        if ( !addr_init(cartridge) )
        {
          warning("Unsupported mapper: %s", cartridge.mapper_string());
        }

        char buf[MAXSTR];
        const char *device_ptr = buf;
        ssize_t len = helper.hashstr("device", buf, sizeof(buf));
        if ( len <= 0 )
          device_ptr = "65816";

        if ( msgid == processor_t::newfile )
        {
          set_device_name(device_ptr, IORESP_ALL);

          set_default_segreg_value(NULL, rFm, 1);
          set_default_segreg_value(NULL, rFx, 1);
          set_default_segreg_value(NULL, rFe, 1);
          set_default_segreg_value(NULL, rD,  0);

          // see processor_t::newseg for the following registers
          //set_default_segreg_value(NULL, rPB, 0);
          //set_default_segreg_value(NULL, rB,  0);
          //set_default_segreg_value(NULL, rDs, 0);

          if ( inf.startIP != BADADDR )
          {
            ea_t reset_ea = xlat(inf.startIP);
            split_srarea(reset_ea, rFm,  get_segreg(cmd.ea, rFm),  SR_auto);
            split_srarea(reset_ea, rFx,  get_segreg(cmd.ea, rFx),  SR_auto);
            split_srarea(reset_ea, rFe,  get_segreg(cmd.ea, rFe),  SR_auto);
            split_srarea(reset_ea, rPB,  0,                        SR_auto);
            split_srarea(reset_ea, rB,   0,                        SR_auto);
            split_srarea(reset_ea, rD,   get_segreg(cmd.ea, rD),   SR_auto);
          }
        }
      }
      break;
    case processor_t::get_autocmt:
      {
        char *buf      = va_arg(va, char *);
        size_t bufsize = va_arg(va, size_t);
        if ( make_insn_cmt(buf, bufsize) )
          ++retcode; // = 2
      }
      break;
    case processor_t::setsgr:
      {
        ea_t startEA = va_arg(va, ea_t);
        ea_t dummy   = va_arg(va, ea_t); qnotused(dummy);
        int regnum   = va_arg(va, int);
        sel_t value  = va_arg(va, sel_t);
        if ( regnum == rB )
        {
//        sel_t d2 = va_arg(va, sel_t); qnotused(d2);
          split_srarea(startEA, rDs, value << 12, SR_auto);
        }
        else if ( regnum == rPB )
        {
          uint16 offset = startEA & 0xffff;
          ea_t newEA = xlat((value << 16) + offset);
          if ( startEA != newEA )
            warning("Inconsistent program bank number ($%02X:%04X != $%02X:%04X)", startEA >> 16, offset, value, offset);
        }
      }
      break;
    case processor_t::may_be_func:
      retcode = 0;
      ea_t cref_addr;
      for( cref_addr = get_first_cref_to(cmd.ea);
           cref_addr != BADADDR;
           cref_addr = get_next_cref_to(cmd.ea, cref_addr) )
      {
        uint8 opcode = get_byte(cref_addr);
        const struct opcode_info_t &opinfo = get_opcode_info(opcode);
        if ( opinfo.itype == M65816_jsl
          || opinfo.itype == M65816_jsr
          || opinfo.itype == M65816_jml )
        {
          retcode = 100;
          break;
        }
      }
      break;
    case processor_t::is_call_insn:
      {
        const struct opcode_info_t &opinfo = get_opcode_info(get_byte(va_arg(va, ea_t)));
        if ( opinfo.itype == M65816_jsr
          || opinfo.itype == M65816_jsl )
          retcode = 2;
        else
          retcode = 0;
      }
      break;
    case processor_t::is_ret_insn:
      {
        const struct opcode_info_t &opinfo = get_opcode_info(get_byte(va_arg(va, ea_t)));
        if ( opinfo.itype == M65816_rti
          || opinfo.itype == M65816_rtl
          || opinfo.itype == M65816_rts )
          retcode = 2;
        else
          retcode = 0;
      }
      break;
    case processor_t::is_indirect_jump:
      {
        const struct opcode_info_t &opinfo = get_opcode_info(get_byte(va_arg(va, ea_t)));
        if ( opinfo.itype == M65816_jmp
          || opinfo.itype == M65816_jml )
        {
          if ( opinfo.addr == ABS_INDIR
            || opinfo.addr == ABS_IX_INDIR
            || opinfo.addr == ABS_INDIR_LONG )
            retcode = 3;
          else
            retcode = 2;
        }
        else
          retcode = 1;
      }
      break;
    default:
      break;
  }
  va_end(va);

  return retcode;
}


//-----------------------------------------------------------------------
//                           CA65 ASSEMBLER
//
// http://www.cc65.org/doc/ca65-4.html#ss4.1
//-----------------------------------------------------------------------
static const asm_t ca65asm =
{
  AS_COLON | ASH_HEXF4, // Assembler features
  0,                    // User-defined flags
  "CA65 ASSEMBLER",     // Name
  0,
  NULL,                 // headers
  NULL,                 // unsupported  instructions
  ".ORG",               // origin directive
  ".END",               // end directive

  ";",          // comment string
  '\'',         // string delimiter
  '\'',         // char delimiter
  "\\'",        // special symbols in char and string constants

  ".BYTE",      // ascii string directive
  ".BYTE",      // byte directive
  ".WORD",      // word directive
};

//-----------------------------------------------------------------------
//      PseudoSam
//-----------------------------------------------------------------------
static const char *const ps_headers[] =
{
  ".code",
  NULL
};

static const asm_t pseudosam =
{
  AS_COLON | ASH_HEXF1 | AS_N2CHR | AS_NOXRF,
  UAS_SELSG,
  "PseudoSam by PseudoCode",
  0,
  ps_headers,
  NULL,
  ".org",
  ".end",

  ";",          // comment string
  '"',          // string delimiter
  '\'',         // char delimiter
  "\\\"'",      // special symbols in char and string constants

  ".db",        // ascii string directive
  ".db",        // byte directive
  ".dw",        // word directive
  NULL,         // dword  (4 bytes)
  NULL,         // qword  (8 bytes)
  NULL,         // oword  (16 bytes)
  NULL,         // float  (4 bytes)
  NULL,         // double (8 bytes)
  NULL,         // tbyte  (10/12 bytes)
  NULL,         // packed decimal real
  NULL,         // arrays (#h,#d,#v,#s(...)
  ".rs %s",     // uninited arrays
  ".equ",       // equ
  NULL,         // seg prefix
  NULL,         // checkarg_preline
  NULL,         // checkarg_atomprefix
  NULL,         // checkarg_operations
  NULL,         // XlatAsciiOutput
  NULL,         // curip
  NULL,         // func_header
  NULL,         // func_footer
  NULL,         // public
  NULL,         // weak
  NULL,         // extrn
  NULL,         // comm
  NULL,         // get_type_name
  NULL,         // align
  '(', ')',     // lbrace, rbrace
  NULL,    // mod
  NULL,    // and
  NULL,    // or
  NULL,    // xor
  NULL,    // not
  NULL,    // shl
  NULL,    // shr
  NULL,    // sizeof
};

//-----------------------------------------------------------------------
//      SVENSON ELECTRONICS 6502/65C02 ASSEMBLER - V.1.0 - MAY, 1988
//-----------------------------------------------------------------------
static const asm_t svasm =
{
  AS_COLON | ASH_HEXF4,
  UAS_NOSEG,
  "SVENSON ELECTRONICS 6502/65C02 ASSEMBLER - V.1.0 - MAY, 1988",
  0,
  NULL,         // headers
  NULL,
  "* = ",
  ".END",

  ";",          // comment string
  '\'',         // string delimiter
  '\'',         // char delimiter
  "\\'",        // special symbols in char and string constants

  ".BYTE",      // ascii string directive
  ".BYTE",      // byte directive
  ".WORD",      // word directive
};

//-----------------------------------------------------------------------
//      TASM assembler definiton
//-----------------------------------------------------------------------
static const asm_t tasm =
{
  AS_COLON | AS_N2CHR | AS_1TEXT,
  UAS_NOENS | UAS_NOSEG,
  "Table Driven Assembler (TASM) by Speech Technology Inc.",
  0,
  NULL,         // headers,
  NULL,
  ".org",
  ".end",

  ";",          // comment string
  '"',          // string delimiter
  '\'',         // char delimiter
  "\\\"'",      // special symbols in char and string constants

  ".text",      // ascii string directive
  ".db",        // byte directive
  ".dw",        // word directive
  NULL,         // dword  (4 bytes)
  NULL,         // qword  (8 bytes)
  NULL,         // oword  (16 bytes)
  NULL,         // float  (4 bytes)
  NULL,         // double (8 bytes)
  NULL,         // tbyte  (10/12 bytes)
  NULL,         // packed decimal real
  NULL,         // arrays (#h,#d,#v,#s(...)
  ".block %s",  // uninited arrays
  ".equ",
  NULL,         // seg prefix
  NULL,         // checkarg_preline
  NULL,         // checkarg_atomprefix
  NULL,         // checkarg_operations
  NULL,         // XlatAsciiOutput
  NULL,         // curip
  NULL,         // func_header
  NULL,         // func_footer
  NULL,         // public
  NULL,         // weak
  NULL,         // extrn
  NULL,         // comm
  NULL,         // get_type_name
  NULL,         // align
  '(', ')',     // lbrace, rbrace
  NULL,     // mod
  "and",    // and
  "or",     // or
  NULL,    // xor
  "not",    // not
  NULL,    // shl
  NULL,    // shr
  NULL,    // sizeof
};


//-----------------------------------------------------------------------
//      Avocet assembler definiton
//-----------------------------------------------------------------------
static const asm_t avocet =
{
  AS_COLON | AS_N2CHR | AS_1TEXT,
  UAS_NOENS | UAS_NOSEG,
  "Avocet Systems 2500AD 6502 Assembler",
  0,
  NULL,         // headers,
  NULL,
  ".org",
  ".end",

  ";",          // comment string
  '"',          // string delimiter
  '\'',         // char delimiter
  "\\\"'",      // special symbols in char and string constants

  ".fcc",       // ascii string directive
  ".db",        // byte directive
  ".dw",        // word directive
  NULL,         // dword  (4 bytes)
  NULL,         // qword  (8 bytes)
  NULL,         // oword  (16 bytes)
  NULL,         // float  (4 bytes)
  NULL,         // double (8 bytes)
  NULL,         // tbyte  (10/12 bytes)
  NULL,         // packed decimal real
  NULL,         // arrays (#h,#d,#v,#s(...)
  ".ds %s",     // uninited arrays
};

static const asm_t *const asms[] =
{
  &ca65asm,

  // 6502 asm_t; imported from the 6502 CPU module.
  &svasm,
  &tasm,
  &pseudosam,
  &avocet,
  NULL
};

//-----------------------------------------------------------------------
#define FAMILY "MOS Technology 658xx series:"
static const char *const shnames[] = { "m65816", "m65c816", NULL };
static const char *const lnames[] = { FAMILY"MOS Technology 65816", "MOS Technology 65C816", NULL };

static const uchar retcode_1[] = { 0x60 }; // RTS
static const uchar retcode_2[] = { 0x40 }; // RTI
static const uchar retcode_3[] = { 0x6b }; // RTL

static const bytes_t retcodes[] =
{
  { sizeof(retcode_1), retcode_1 },
  { sizeof(retcode_2), retcode_2 },
  { sizeof(retcode_3), retcode_3 },
  { 0, NULL }
};

//-----------------------------------------------------------------------
//      Processor Definition
//-----------------------------------------------------------------------

processor_t LPH =
{
  IDP_INTERFACE_VERSION,// version
  PLFM_65C816,          // id
  PR_SEGS|PR_SEGTRANS,  // flags
  8,                    // 8 bits in a byte for code segments
  8,                    // 8 bits in a byte for other segments

  shnames,
  lnames,

  asms,

  notify,

  header,
  footer,

  segstart,
  std_gen_segm_footer,

  assumes,

  ana,
  emu,

  out,
  outop,
  intel_data,
  NULL,                 // compare operands
  NULL,                 // can have type

  qnumber(RegNames),            // Number of registers
  RegNames,                     // Register names
  NULL,                         // get abstract register

  0,                            // Number of register files
  NULL,                         // Register file names
  NULL,                         // Register descriptions
  NULL,                         // Pointer to CPU registers

  rCs,                          // first segreg
  rFe,                          // last  segreg
  0,                            // size of a segment register
  rCs,                          // number of CS register
  rDs,                          // number of DS register

  NULL,                         // No known code start sequences
  retcodes,

  0,
  M65816_last,
  Instructions,

  //------
  NULL,                 // int  (*is_far_jump)(int icode);
  NULL,                 // Translation function for offsets
  3,                    // int tbyte_size;  -- doesn't exist

  NULL,                 // int (*realcvt)(void *m, ushort *e, ushort swt);
  { 0, 0, 0, 0 },       // char real_width[4];
                            // number of symbols after decimal point
                            // 2byte float (0-does not exist)
                            // normal float
                            // normal double
                            // long double
  NULL,                 // int (*is_switch)(switch_info_t *si);
  NULL,                 // int32 (*gen_map_file)(FILE *fp);
  NULL,                 // ea_t (*extract_address)(ea_t ea,const char *string,int x);
  NULL,                 // int (*is_sp_based)(op_t &x);
  NULL,                 // int (*create_func_frame)(func_t *pfn);
  NULL,                 // int (*get_frame_retsize(func_t *pfn)
  NULL,                 // void (*gen_stkvar_def)(char *buf,const member_t *mptr,long v);
  NULL,                 // Generate text representation of an item in a special segment
  M65816_rts,           // Icode of return instruction. It is ok to give any of possible return instructions
};

