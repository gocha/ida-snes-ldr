/*
 *      Interactive disassembler (IDA).
 *      Version 3.05
 *      Copyright (c) 1990-95 by Ilfak Guilfanov.
 *      ALL RIGHTS RESERVED.
 *                              FIDO:   2:5020/209
 *                              E-mail: ig@estar.msk.su
 *
 */

#include <idp.hpp>
#include <srarea.hpp>
#include <diskio.hpp>
#include <enum.hpp>

#include "spc700.hpp"

//--------------------------------------------------------------------------
static const char *const RegNames[] =
{
  "a",
  "x",
  "y",
  "ya",
  "sp",
  "psw",

  "C",

  "Cs",
  "Ds",

  "P",
};

CASSERT(qnumber(RegNames) == rLast);


//----------------------------------------------------------------------
//       Prepare global variables & defines for ../iocommon.cpp
//----------------------------------------------------------------------
static netnode helper;
char device[MAXSTR];
static size_t numports = 0;
static ioport_t *ports = NULL;

//--------------------------------------------------------------------------
//lint -esym(528,ioresp_ok) is not referenced
static bool ioresp_ok(void)
{
  if ( inf.like_binary() )
    return true;
  else
    return get_segm_by_name("RAM") != NULL;
}
#define CHECK_IORESP      ioresp_ok()
#include "../iocommon.cpp"

//----------------------------------------------------------------------
static tid_t set_dsp_regs_enum()
{
  static const char enum_name[] = "DSPRegisters";
  enum_t id = get_enum(enum_name);
  if ( id != BADNODE )
    return id;
  id = add_enum(-1, enum_name, hexflag());

  add_enum_member(id, "V0VOLL"  , 0x00);
  add_enum_member(id, "V0VOLR"  , 0x01);
  add_enum_member(id, "V0PL"    , 0x02);
  add_enum_member(id, "V0PH"    , 0x03);
  add_enum_member(id, "V0SRCN"  , 0x04);
  add_enum_member(id, "V0ADSR1" , 0x05);
  add_enum_member(id, "V0ADSR2" , 0x06);
  add_enum_member(id, "V0GAIN"  , 0x07);
  add_enum_member(id, "V0ENVX"  , 0x08);
  add_enum_member(id, "V0OUTX"  , 0x09);

  add_enum_member(id, "V1VOLL"  , 0x10);
  add_enum_member(id, "V1VOLR"  , 0x11);
  add_enum_member(id, "V1PL"    , 0x12);
  add_enum_member(id, "V1PH"    , 0x13);
  add_enum_member(id, "V1SRCN"  , 0x14);
  add_enum_member(id, "V1ADSR1" , 0x15);
  add_enum_member(id, "V1ADSR2" , 0x16);
  add_enum_member(id, "V1GAIN"  , 0x17);
  add_enum_member(id, "V1ENVX"  , 0x18);
  add_enum_member(id, "V1OUTX"  , 0x19);

  add_enum_member(id, "V2VOLL"  , 0x20);
  add_enum_member(id, "V2VOLR"  , 0x21);
  add_enum_member(id, "V2PL"    , 0x22);
  add_enum_member(id, "V2PH"    , 0x23);
  add_enum_member(id, "V2SRCN"  , 0x24);
  add_enum_member(id, "V2ADSR1" , 0x25);
  add_enum_member(id, "V2ADSR2" , 0x26);
  add_enum_member(id, "V2GAIN"  , 0x27);
  add_enum_member(id, "V2ENVX"  , 0x28);
  add_enum_member(id, "V2OUTX"  , 0x29);

  add_enum_member(id, "V3VOLL"  , 0x30);
  add_enum_member(id, "V3VOLR"  , 0x31);
  add_enum_member(id, "V3PL"    , 0x32);
  add_enum_member(id, "V3PH"    , 0x33);
  add_enum_member(id, "V3SRCN"  , 0x34);
  add_enum_member(id, "V3ADSR1" , 0x35);
  add_enum_member(id, "V3ADSR2" , 0x36);
  add_enum_member(id, "V3GAIN"  , 0x37);
  add_enum_member(id, "V3ENVX"  , 0x38);
  add_enum_member(id, "V3OUTX"  , 0x39);

  add_enum_member(id, "V4VOLL"  , 0x40);
  add_enum_member(id, "V4VOLR"  , 0x41);
  add_enum_member(id, "V4PL"    , 0x42);
  add_enum_member(id, "V4PH"    , 0x43);
  add_enum_member(id, "V4SRCN"  , 0x44);
  add_enum_member(id, "V4ADSR1" , 0x45);
  add_enum_member(id, "V4ADSR2" , 0x46);
  add_enum_member(id, "V4GAIN"  , 0x47);
  add_enum_member(id, "V4ENVX"  , 0x48);
  add_enum_member(id, "V4OUTX"  , 0x49);

  add_enum_member(id, "V5VOLL"  , 0x50);
  add_enum_member(id, "V5VOLR"  , 0x51);
  add_enum_member(id, "V5PL"    , 0x52);
  add_enum_member(id, "V5PH"    , 0x53);
  add_enum_member(id, "V5SRCN"  , 0x54);
  add_enum_member(id, "V5ADSR1" , 0x55);
  add_enum_member(id, "V5ADSR2" , 0x56);
  add_enum_member(id, "V5GAIN"  , 0x57);
  add_enum_member(id, "V5ENVX"  , 0x58);
  add_enum_member(id, "V5OUTX"  , 0x59);

  add_enum_member(id, "V6VOLL"  , 0x60);
  add_enum_member(id, "V6VOLR"  , 0x61);
  add_enum_member(id, "V6PL"    , 0x62);
  add_enum_member(id, "V6PH"    , 0x63);
  add_enum_member(id, "V6SRCN"  , 0x64);
  add_enum_member(id, "V6ADSR1" , 0x65);
  add_enum_member(id, "V6ADSR2" , 0x66);
  add_enum_member(id, "V6GAIN"  , 0x67);
  add_enum_member(id, "V6ENVX"  , 0x68);
  add_enum_member(id, "V6OUTX"  , 0x69);

  add_enum_member(id, "V7VOLL"  , 0x70);
  add_enum_member(id, "V7VOLR"  , 0x71);
  add_enum_member(id, "V7PL"    , 0x72);
  add_enum_member(id, "V7PH"    , 0x73);
  add_enum_member(id, "V7SRCN"  , 0x74);
  add_enum_member(id, "V7ADSR1" , 0x75);
  add_enum_member(id, "V7ADSR2" , 0x76);
  add_enum_member(id, "V7GAIN"  , 0x77);
  add_enum_member(id, "V7ENVX"  , 0x78);
  add_enum_member(id, "V7OUTX"  , 0x79);

  add_enum_member(id, "MVOLL"   , 0x0c);
  add_enum_member(id, "MVOLR"   , 0x1c);
  add_enum_member(id, "EVOLL"   , 0x2c);
  add_enum_member(id, "EVOLR"   , 0x3c);
  add_enum_member(id, "KON"     , 0x4c);
  add_enum_member(id, "KOF"     , 0x5c);
  add_enum_member(id, "FLG"     , 0x6c);
  add_enum_member(id, "ENDX"    , 0x7c);
  add_enum_member(id, "EFB"     , 0x0d);
  add_enum_member(id, "PMON"    , 0x2d);
  add_enum_member(id, "NON"     , 0x3d);
  add_enum_member(id, "EON"     , 0x4d);
  add_enum_member(id, "DIR"     , 0x5d);
  add_enum_member(id, "ESA"     , 0x6d);
  add_enum_member(id, "EDL"     , 0x7d);
  add_enum_member(id, "FIR0"    , 0x0f);
  add_enum_member(id, "FIR1"    , 0x1f);
  add_enum_member(id, "FIR2"    , 0x2f);
  add_enum_member(id, "FIR3"    , 0x3f);
  add_enum_member(id, "FIR4"    , 0x4f);
  add_enum_member(id, "FIR5"    , 0x5f);
  add_enum_member(id, "FIR6"    , 0x6f);
  add_enum_member(id, "FIR7"    , 0x7f);

  return id;
}

//----------------------------------------------------------------------
static int idaapi notify(processor_t::idp_notify msgid, ...) // Various messages:
{
  va_list va;
  va_start(va, msgid);

// A well behaved processor module should call invoke_callbacks()
// in his notify() function. If this function returns 0, then
// the processor module should process the notification itself
// Otherwise the code should be returned to the caller:

  int code = invoke_callbacks(HT_IDP, msgid, va);
  if ( code ) return code;

  int retcode = 1;
  switch ( msgid )
  {
    case processor_t::init:
      helper.create("$ spc700");
      break;
    case processor_t::term:
      free_ioports(ports, numports);
      break;
    case processor_t::oldfile:
    case processor_t::newfile:
      {
        char buf[MAXSTR];
        const char *device_ptr = buf;
        ssize_t len = helper.hashstr("device", buf, sizeof(buf));
        if ( len <= 0 )
          device_ptr = "spc700";

        if ( msgid == processor_t::newfile )
        {
          set_device_name(device_ptr, IORESP_ALL);
          set_dsp_regs_enum();

          set_default_segreg_value(NULL, rDs, 0);
          set_default_segreg_value(NULL, rFp, 0);
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
        if ( opinfo.itype == SPC_call
          || opinfo.itype == SPC_jmp )
        {
          retcode = 100;
          break;
        }
      }
      break;

    case processor_t::is_call_insn:
      {
        const struct opcode_info_t &opinfo = get_opcode_info(get_byte(va_arg(va, ea_t)));
        if ( opinfo.itype == SPC_call )
          retcode = 2;
        else
          retcode = 0;
      }
      break;

    case processor_t::is_ret_insn:
      {
        const struct opcode_info_t &opinfo = get_opcode_info(get_byte(va_arg(va, ea_t)));
        if ( opinfo.itype == SPC_ret
          || opinfo.itype == SPC_reti )
          retcode = 2;
        else
          retcode = 0;
      }
      break;

    case processor_t::is_indirect_jump:
      {
        const struct opcode_info_t &opinfo = get_opcode_info(get_byte(va_arg(va, ea_t)));
        if ( opinfo.itype == SPC_jmp )
        {
          if ( opinfo.addr == ABS_IX_INDIR )
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
//      XLA-DX assembler definition
//-----------------------------------------------------------------------
static const asm_t xladxasm =
{
  AS_COLON | ASH_HEXF4,
  UAS_NOSEG,
  "XLA-DX Assembler by Ville Helin",
  0,
  NULL,         // headers
  NULL,
  ".org",
  NULL,

  ";",          // comment string
  '"',          // string delimiter
  '\'',         // char delimiter
  "\\'",        // special symbols in char and string constants

  ".db",        // ascii string directive
  ".db",        // byte directive
  ".dw",        // word directive
};

static const asm_t *const asms[] = { &xladxasm, NULL };
//-----------------------------------------------------------------------
static const char *const shnames[] = { "spc700", NULL };
static const char *const lnames[] = { "Sony SPC700", NULL };

//--------------------------------------------------------------------------
static const uchar retcode_1[] = { 0x6f }; // ret
static const uchar retcode_2[] = { 0x7f }; // reti

static const bytes_t retcodes[] =
{
 { sizeof(retcode_1), retcode_1 },
 { sizeof(retcode_2), retcode_2 },
 { 0, NULL }
};

//-----------------------------------------------------------------------
//      Processor Definition
//-----------------------------------------------------------------------

#define PLFM_SPC700	0x816A

processor_t LPH =
{
  IDP_INTERFACE_VERSION,// version
  PLFM_SPC700,          // id
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
  rFp,                          // last  segreg
  0,                            // size of a segment register
  rCs,                          // number of CS register
  rDs,                          // number of DS register

  NULL,                         // No known code start sequences
  retcodes,

  0,
  SPC_last,
  Instructions
};
