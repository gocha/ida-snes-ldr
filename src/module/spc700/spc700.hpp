/*
 *      Interactive disassembler (IDA).
 *      Version 3.05
 *      Copyright (c) 1990-95 by Ilfak Guilfanov.
 *      ALL RIGHTS RESERVED.
 *                              FIDO:   2:5020/209
 *                              E-mail: ig@estar.msk.su
 *
 */

#ifndef _SPC700_HPP
#define _SPC700_HPP

#include "../idaidp.hpp"
#include <srarea.hpp>
#include "ins.hpp"

// If there is an address in 'Op[N].full_target_ea',
// it means the target address of a branch/jump
// is already known. That's there to help the 'emu'
// module propagate flags & status.
#define full_target_ea  specval

// Is indirect memory reference?
#define indirect        segpref

#define UAS_SECT        0x0002          // Segments are named .SECTION
#define UAS_NOSEG       0x0004          // No 'segment' directives
#define UAS_SELSG       0x0010          // Segment should be selected by its name
#define UAS_CDSEG       0x0080          // Only DSEG,CSEG,XSEG
#define UAS_NOENS       0x0200          // don't specify start addr in the .end directive

//------------------------------------------------------------------------
enum SPC_registers {
  rA = 0,
  rX,
  rY,
  rYA,
  rSP,
  rPSW,
  rFc,

  rCs,  // code segment (virtual)
  rDs,  // data segment (virtual)

  // This will be considered segment registers by IDA,
  // but we'll actually use them to keep information
  // about the 'p' flags, determining the direct page address.
  rFp,
};


// Addressing modes
enum spc_addrmode_t
{
  ABS = 0,
  ABS_IX,
  ABS_IY,
  ABS_IX_INDIR,
  BIT_OP,
  DP,
  DP_IX,
  DP_IY,
  DP_IX_INDIR,
  DP_INDIR_IY,
  DP_DP,
  DP_PC_REL,
  DP_IX_PC_REL,
  IMM,
  IMM_DP,
  IMPLIED,
  INDIR_IX,
  INDIR_IY,
  INDIR_IX_INC,
  INDIR_IX_IY,
  PC_REL,
  ADDRMODE_last
};

// The various phrases that can be used in case
// an operand is of type 'o_phrase'.
enum ophrase_phrases_t
{
  riX = 100, // "(X)"                INDIR_IX
  riY,       // "(Y)"                INDIR_IY
  riXinc,    // "(X)+"               INDIR_IX_INC
};

// The various phrases that can be used in case
// an operand is of type 'o_displ'.
enum odispl_phrases_t
{
  rD = 100,  // "dp"                 DP
  rDX,       // "dp, X"              DP_IX
  rDY,       // "dp, Y"              DP_IY
  riDX,      // "(dp, X)"            DP_IX_INDIR
  rAbsX,     // "abs, X"             ABS_IX
  rAbsY,     // "abs, Y"             ABS_IY
  rAbsXi,    // "(abs,X)"            ABS_IX_INDIR
  rDiY,      // "(dp,n), Y"          DP_INDIR_IY
  rDbit,     //                      (for bit tests)
  rDbitnot,  //                      (for bit tests)
  rTCall,    //                      (for tcall)
  rPCall,    //                      (for pcall)
};


// Information about addressing modes.
struct addrmode_info_t
{
  const char *name;
};

extern struct addrmode_info_t AddressingModes[];

// Information about an opcode
struct opcode_info_t
{
  spc_itype_t    itype;
  spc_addrmode_t addr;
  SPC_registers  dreg;
  SPC_registers  sreg;
};

const struct opcode_info_t &get_opcode_info(uint8 opcode);

//------------------------------------------------------------------------
void    idaapi header(void);
void    idaapi footer(void);

void    idaapi segstart(ea_t ea);

int     idaapi ana(void);
int     idaapi emu(void);
void    idaapi out(void);
bool    idaapi outop(op_t &op);
void    idaapi assumes(ea_t ea);

#endif
