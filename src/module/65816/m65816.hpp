
#ifndef __M65816_HPP__
#define __M65816_HPP__

#include "../../module/idaidp.hpp"
#include <srarea.hpp>
#include "ins.hpp"


// If there is an address in 'Op[N].full_target_ea',
// it means the target address of a branch/jump
// is already known. That's there to help the 'emu'
// module propagate M&X flags & status.
#define full_target_ea  specval


// Is indirect memory reference?
#define indirect        segpref

// These defines are used by some 6502 asm_t descriptors.
// Although this is primarily a 65816 module, they'll
// remain here since at some point, this CPU module /might/
// supersede the 6502 one.
// Should that happen, we wouldn't want to waste the
// set of asm_t's that are defined for 6502.
#define UAS_SECT        0x0002          // Segments are named .SECTION
#define UAS_NOSEG       0x0004          // No 'segment' directives
#define UAS_SELSG       0x0010          // Segment should be selected by its name
#define UAS_CDSEG       0x0080          // Only DSEG,CSEG,XSEG
#define UAS_NOENS       0x0200          // don't specify start addr in the .end directive


enum M65816_registers {
  rA,   // Accumulator
  rX,   // X index
  rY,   // Y index
  rS,   // Stack
  rCs,  // code segment
  rDs,  // data segment

  // program bank register
  rPB,

  // This will hold the value of B, the
  // data bank register. We won't make use of Ds
  // directly, as it is typically used, in computation,
  // as a 16-byte paragraph register, while B is a
  // 64KB pages register. Also, by having a dedicated
  // B, the user will be able to modify it more
  // easily (without having to manually shift the value by
  // 12 bits).
  // Note: Also, we won't have this register ``mapped'' to a sel_t.
  // We'll stuff the B value in there directly, which allows
  // it to be more versatile, and access banks where there's
  // no ROM loaded (such as [S|W]RAM bank(s)).
  rB,

  // Direct page register. Same note as that of rB applies.
  rD,

  // These will be considered segment
  // registers by IDA (just as rCs, rDs, rB and rD),
  // but we'll actually use them to keep information
  // about the 'm', 'x' and 'e' flags, determining
  // what's the accumulator & indices mode, and whether
  // we run in 6502 emulation or 65816 native mode.
  rFm,
  rFx,
  rFe
};


// Addressing modes
enum m65_addrmode_t
{
  ABS = 0,
  ABS_IX,
  ABS_IY,
  ABS_IX_INDIR,
  ABS_INDIR,
  ABS_INDIR_LONG,
  ABS_LONG,
  ABS_LONG_IX,
  ACC,
  BLK_MOV,
  DP,
  DP_IX,
  DP_IY,
  DP_IX_INDIR,
  DP_INDIR,
  DP_INDIR_LONG,
  DP_INDIR_IY,
  DP_INDIR_LONG_IY,
  IMM,
  IMPLIED,
  PC_REL,
  PC_REL_LONG,
  STACK_ABS,
  STACK_DP_INDIR,
  STACK_INT,
  STACK_PC_REL,
  STACK_PULL,
  STACK_PUSH,
  STACK_RTI,
  STACK_RTL,
  STACK_RTS,
  STACK_REL,
  STACK_REL_INDIR_IY,
  ADDRMODE_last
};

// The various phrases that can be used in case
// an operand is of type 'o_displ'.
enum odispl_phrases_t
{
  rDX = 100, // "dp, X"              DP_IX
  rDY,       // "dp, Y"              DP_IY
  riDX,      // "(dp, X)"            DP_IX_INDIR
  rAbsi,     // "(abs)"              ABS_INDIR
  rAbsiL,    // "long(abs)"          ABS_INDIR_LONG
  rAbsX,     // "abs, X"             ABS_IX
  rAbsY,     // "abs, Y"             ABS_IY
  rAbsLX,    // "long abs, X"        ABS_LONG_IX
  rAbsXi,    // "(abs,X)"            ABS_IX_INDIR
  rDi,       // "(dp,n)"             DP_INDIR
  rDiL,      // "long(dp,n)"         DP_INDIR_LONG
  rDiY,      // "(dp,n), Y"          DP_INDIR_IY
  rDiLY,     // "long(dp,n), Y"      DP_INDIR_LONG_IY
  rSiY,      // (s,n),Y              STACK_REL_INDIR_IY
  rSDi       // "(dp,n)"             STACK_DP_INDIR
};


// Information about addressing modes.
struct addrmode_info_t
{
  const char *name;
};

extern struct addrmode_info_t AddressingModes[];


// The type of m65* processors. Used
// to declare availability of certain opcodes depending
// on the processor.
enum m65_variant_t
{
  M6502  = 1,
  M65C02 = 2,
  M65802 = 4,
  M65816 = 8,
  M6X    = 1 | 2 | 4 | 8
};


// Special flags, for certain opcodes
enum opcode_flags_t
{
  // Increment instruction's byte count
  // if accumulator is in 16-bits mode.
  ACC16_INCBC = 1,

  // Increment instruction's byte count
  // if X/Y registers are in 16-bits mode.
  XY16_INCBC  = 2
};

// Information about an opcode
struct opcode_info_t
{
  m65_itype_t    itype;
  m65_addrmode_t addr;
  uint8          cpu_variants; // OR'd m65_variant_t
  uint16         flags;        // OR'd opcode_flags_t
};

inline bool is_acc_16_bits (ea_t ea) { return (get_segreg(ea, rFm) == 0); }
inline bool is_xy_16_bits  (ea_t ea) { return (get_segreg(ea, rFx) == 0); }
inline bool is_acc_16_bits (void)    { return is_acc_16_bits(cmd.ea); }
inline bool is_xy_16_bits  (void)    { return is_xy_16_bits(cmd.ea); }

const struct opcode_info_t &get_opcode_info(uint8 opcode);

// Determines whether an m65_itype_t is of type 'push'
#define M65_ITYPE_PUSH(op) \
       (((op) == M65816_pea) \
     || ((op) == M65816_pei) \
     || ((op) == M65816_per) \
     || ((op) == M65816_pha) \
     || ((op) == M65816_phb) \
     || ((op) == M65816_phd) \
     || ((op) == M65816_phk) \
     || ((op) == M65816_php) \
     || ((op) == M65816_phx) \
     || ((op) == M65816_phy))

// Determines whether an m65_itype_t is of type 'pull'
#define M65_ITYPE_PULL(op) \
       (((op) == M65816_pla) \
     || ((op) == M65816_plb) \
     || ((op) == M65816_pld) \
     || ((op) == M65816_plp) \
     || ((op) == M65816_plx) \
     || ((op) == M65816_ply))

struct SuperFamicomCartridge;
extern SuperFamicomCartridge cartridge;

//------------------------------------------------------------------------
void    idaapi header(void);
void    idaapi footer(void);

void    idaapi segstart(ea_t ea);

int     idaapi ana(void);
int     idaapi emu(void);
void    idaapi out(void);
bool    idaapi outop(op_t &op);
void    idaapi assumes(ea_t ea);


// Translate a (potentially mirrorred) address
// to its actual physical address.
//
// This function is provided from ../../ldr/snes/addr.cpp
ea_t xlat(ea_t address);

ea_t calc_addr(const op_t &x, ea_t *orig_ea=NULL);

#endif
