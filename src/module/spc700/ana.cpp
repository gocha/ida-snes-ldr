/*
 *      Interactive disassembler (IDA).
 *      Copyright (c) 1990-95 by Ilfak Guilfanov.
 *      ALL RIGHTS RESERVED.
 *                              FIDO:   2:5020/209
 *                              E-mail: ig@estar.msk.su
 *
 */

#include "spc700.hpp"

#define DI(itype, len, addr_mode, dreg, sreg) {(itype), (addr_mode), ((SPC_registers)(dreg)), ((SPC_registers)(sreg))},

static const struct opcode_info_t opinfos[] =
{
  // 0x00
  DI(SPC_nop,   1, IMPLIED,      -1,   -1)
  DI(SPC_tcall, 1, IMPLIED,      -1,   -1)
  DI(SPC_set0,  2, DP,           -1,   -1)
  DI(SPC_bbs0,  3, DP_PC_REL,    -1,   -1)
  DI(SPC_or,    2, DP,           rA,   -1)
  DI(SPC_or,    3, ABS,          rA,   -1)
  DI(SPC_or,    1, INDIR_IX,     rA,   -1)
  DI(SPC_or,    2, DP_IX_INDIR,  rA,   -1)

  // 0x08
  DI(SPC_or,    2, IMM,          rA,   -1)
  DI(SPC_or,    3, DP_DP,        -1,   -1)
  DI(SPC_or1,   3, BIT_OP,       rFc,  -1)
  DI(SPC_asl,   2, DP,           -1,   -1)
  DI(SPC_asl,   3, ABS,          -1,   -1)
  DI(SPC_php,   1, IMPLIED,      rPSW, -1)
  DI(SPC_tset1, 3, ABS,          -1,   -1)
  DI(SPC_brk,   1, IMPLIED,      -1,   -1)

  // 0x10
  DI(SPC_bpl,   2, PC_REL,       -1,   -1)
  DI(SPC_tcall, 1, IMPLIED,      -1,   -1)
  DI(SPC_clr0,  2, DP,           -1,   -1)
  DI(SPC_bbc0,  3, DP_PC_REL,    -1,   -1)
  DI(SPC_or,    2, DP_IX,        rA,   -1)
  DI(SPC_or,    3, ABS_IX,       rA,   -1)
  DI(SPC_or,    3, ABS_IY,       rA,   -1)
  DI(SPC_or,    2, DP_INDIR_IY,  rA,   -1)

  // 0x18
  DI(SPC_or,    3, IMM_DP,       -1,   -1)
  DI(SPC_or,    1, INDIR_IX_IY,  -1,   -1)
  DI(SPC_decw,  2, DP,           -1,   -1)
  DI(SPC_asl,   2, DP_IX,        -1,   -1)
  DI(SPC_asl,   1, IMPLIED,      rA,   -1)
  DI(SPC_dex,   1, IMPLIED,      rX,   -1)
  DI(SPC_cpx,   3, ABS,          rX,   -1)
  DI(SPC_jmp,   3, ABS_IX_INDIR, -1,   -1)

  // 0x20
  DI(SPC_clrp,  1, IMPLIED,      -1,   -1)
  DI(SPC_tcall, 1, IMPLIED,      -1,   -1)
  DI(SPC_set1,  2, DP,           -1,   -1)
  DI(SPC_bbs1,  3, DP_PC_REL,    -1,   -1)
  DI(SPC_and,   2, DP,           rA,   -1)
  DI(SPC_and,   3, ABS,          rA,   -1)
  DI(SPC_and,   1, INDIR_IX,     rA,   -1)
  DI(SPC_and,   2, DP_IX_INDIR,  rA,   -1)

  // 0x28
  DI(SPC_and,   2, IMM,          rA,   -1)
  DI(SPC_and,   3, DP_DP,        -1,   -1)
  DI(SPC_or1,   3, BIT_OP,       rFc,  -1)
  DI(SPC_rol,   2, DP,           -1,   -1)
  DI(SPC_rol,   3, ABS,          -1,   -1)
  DI(SPC_pha,   1, IMPLIED,      rA,   -1)
  DI(SPC_cbne,  3, DP_PC_REL,    -1,   -1)
  DI(SPC_bra,   2, PC_REL,       -1,   -1)

  // 0x30
  DI(SPC_bmi,   2, PC_REL,       -1,   -1)
  DI(SPC_tcall, 1, IMPLIED,      -1,   -1)
  DI(SPC_clr1,  2, DP,           -1,   -1)
  DI(SPC_bbc1,  3, DP_PC_REL,    -1,   -1)
  DI(SPC_and,   2, DP_IX,        rA,   -1)
  DI(SPC_and,   3, ABS_IX,       rA,   -1)
  DI(SPC_and,   3, ABS_IY,       rA,   -1)
  DI(SPC_and,   2, DP_INDIR_IY,  rA,   -1)

  // 0x38
  DI(SPC_and,   3, IMM_DP,       -1,   -1)
  DI(SPC_and,   1, INDIR_IX_IY,  -1,   -1)
  DI(SPC_incw,  2, DP,           -1,   -1)
  DI(SPC_rol,   2, DP_IX,        -1,   -1)
  DI(SPC_rol,   1, IMPLIED,      rA,   -1)
  DI(SPC_inx,   1, IMPLIED,      rX,   -1)
  DI(SPC_cpx,   2, DP,           rX,   -1)
  DI(SPC_call,  3, ABS,          -1,   -1)

  // 0x40
  DI(SPC_setp,  1, IMPLIED,      -1,   -1)
  DI(SPC_tcall, 1, IMPLIED,      -1,   -1)
  DI(SPC_set2,  2, DP,           -1,   -1)
  DI(SPC_bbs2,  3, DP_PC_REL,    -1,   -1)
  DI(SPC_eor,   2, DP,           rA,   -1)
  DI(SPC_eor,   3, ABS,          rA,   -1)
  DI(SPC_eor,   1, INDIR_IX,     rA,   -1)
  DI(SPC_eor,   2, DP_IX_INDIR,  rA,   -1)

  // 0x48
  DI(SPC_eor,   2, IMM,          rA,   -1)
  DI(SPC_eor,   3, DP_DP,        -1,   -1)
  DI(SPC_and1,  3, BIT_OP,       rFc,  -1)
  DI(SPC_lsr,   2, DP,           -1,   -1)
  DI(SPC_lsr,   3, ABS,          -1,   -1)
  DI(SPC_phx,   1, IMPLIED,      rX,   -1)
  DI(SPC_tclr1, 3, ABS,          -1,   -1)
  DI(SPC_pcall, 2, DP,           -1,   -1)

  // 0x50
  DI(SPC_bvc,   2, PC_REL,       -1,   -1)
  DI(SPC_tcall, 1, IMPLIED,      -1,   -1)
  DI(SPC_clr2,  2, DP,           -1,   -1)
  DI(SPC_bbc2,  3, DP_PC_REL,    -1,   -1)
  DI(SPC_eor,   2, DP_IX,        rA,   -1)
  DI(SPC_eor,   3, ABS_IX,       rA,   -1)
  DI(SPC_eor,   3, ABS_IY,       rA,   -1)
  DI(SPC_eor,   2, DP_INDIR_IY,  rA,   -1)

  // 0x58
  DI(SPC_eor,   3, IMM_DP,       -1,   -1)
  DI(SPC_eor,   1, INDIR_IX_IY,  -1,   -1)
  DI(SPC_cmpw,  2, DP,           rYA,  -1)
  DI(SPC_lsr,   2, DP_IX,        -1,   -1)
  DI(SPC_lsr,   1, IMPLIED,      rA,   -1)
  DI(SPC_tax,   1, IMPLIED,      rX,   rA)
  DI(SPC_cpy,   3, ABS,          rY,   -1)
  DI(SPC_jmp,   3, ABS,          -1,   -1)

  // 0x60
  DI(SPC_clrc,  1, IMPLIED,      -1,   -1)
  DI(SPC_tcall, 1, IMPLIED,      -1,   -1)
  DI(SPC_set3,  2, DP,           -1,   -1)
  DI(SPC_bbs3,  3, DP_PC_REL,    -1,   -1)
  DI(SPC_cmp,   2, DP,           rA,   -1)
  DI(SPC_cmp,   3, ABS,          rA,   -1)
  DI(SPC_cmp,   1, INDIR_IX,     rA,   -1)
  DI(SPC_cmp,   2, DP_IX_INDIR,  rA,   -1)

  // 0x68
  DI(SPC_cmp,   2, IMM,          rA,   -1)
  DI(SPC_cmp,   3, DP_DP,        -1,   -1)
  DI(SPC_and1,  3, BIT_OP,       rFc,  -1)
  DI(SPC_ror,   2, DP,           -1,   -1)
  DI(SPC_ror,   3, ABS,          -1,   -1)
  DI(SPC_phy,   1, IMPLIED,      rY,   -1)
  DI(SPC_dbnz,  3, DP_PC_REL,    -1,   -1)
  DI(SPC_ret,   1, IMPLIED,      -1,   -1)

  // 0x70
  DI(SPC_bvs,   2, PC_REL,       -1,   -1)
  DI(SPC_tcall, 1, IMPLIED,      -1,   -1)
  DI(SPC_clr3,  2, DP,           -1,   -1)
  DI(SPC_bbc3,  3, DP_PC_REL,    -1,   -1)
  DI(SPC_cmp,   2, DP_IX,        rA,   -1)
  DI(SPC_cmp,   3, ABS_IX,       rA,   -1)
  DI(SPC_cmp,   3, ABS_IY,       rA,   -1)
  DI(SPC_cmp,   2, DP_INDIR_IY,  rA,   -1)

  // 0x78
  DI(SPC_cmp,   3, IMM_DP,       -1,   -1)
  DI(SPC_cmp,   1, INDIR_IX_IY,  -1,   -1)
  DI(SPC_addw,  2, DP,           rYA,  -1)
  DI(SPC_ror,   2, DP_IX,        -1,   -1)
  DI(SPC_ror,   1, IMPLIED,      rA,   -1)
  DI(SPC_txa,   1, IMPLIED,      rA,   rX)
  DI(SPC_cpy,   2, DP,           rY,   -1)
  DI(SPC_reti,  1, IMPLIED,      -1,   -1)

  // 0x80
  DI(SPC_setc,  1, IMPLIED,      -1,   -1)
  DI(SPC_tcall, 1, IMPLIED,      -1,   -1)
  DI(SPC_set4,  2, DP,           -1,   -1)
  DI(SPC_bbs4,  3, DP_PC_REL,    -1,   -1)
  DI(SPC_adc,   2, DP,           rA,   -1)
  DI(SPC_adc,   3, ABS,          rA,   -1)
  DI(SPC_adc,   1, INDIR_IX,     rA,   -1)
  DI(SPC_adc,   2, DP_IX_INDIR,  rA,   -1)

  // 0x88
  DI(SPC_adc,   2, IMM,          rA,   -1)
  DI(SPC_adc,   3, DP_DP,        -1,   -1)
  DI(SPC_eor1,  3, BIT_OP,       rFc,  -1)
  DI(SPC_dec,   2, DP,           -1,   -1)
  DI(SPC_dec,   3, ABS,          -1,   -1)
  DI(SPC_ldy,   2, IMM,          rY,   -1)
  DI(SPC_plp,   1, IMPLIED,      rPSW, -1)
  DI(SPC_mov,   3, IMM_DP,       -1,   -1)

  // 0x90
  DI(SPC_bcc,   2, PC_REL,       -1,   -1)
  DI(SPC_tcall, 1, IMPLIED,      -1,   -1)
  DI(SPC_clr4,  2, DP,           -1,   -1)
  DI(SPC_bbc4,  3, DP_PC_REL,    -1,   -1)
  DI(SPC_adc,   2, DP_IX,        rA,   -1)
  DI(SPC_adc,   3, ABS_IX,       rA,   -1)
  DI(SPC_adc,   3, ABS_IY,       rA,   -1)
  DI(SPC_adc,   2, DP_INDIR_IY,  rA,   -1)

  // 0x98
  DI(SPC_adc,   3, IMM_DP,       -1,   -1)
  DI(SPC_adc,   1, INDIR_IX_IY,  -1,   -1)
  DI(SPC_subw,  2, DP,           rYA,  -1)
  DI(SPC_dec,   2, DP_IX,        -1,   -1)
  DI(SPC_dec,   1, IMPLIED,      rA,   -1)
  DI(SPC_tsx,   1, IMPLIED,      rX,   rSP)
  DI(SPC_div,   1, IMPLIED,      rYA,  rX)
  DI(SPC_xcn,   1, IMPLIED,      -1,   -1)

  // 0xa0
  DI(SPC_ei,    1, IMPLIED,      -1,   -1)
  DI(SPC_tcall, 1, IMPLIED,      -1,   -1)
  DI(SPC_set5,  2, DP,           -1,   -1)
  DI(SPC_bbs5,  3, DP_PC_REL,    -1,   -1)
  DI(SPC_sbc,   2, DP,           rA,   -1)
  DI(SPC_sbc,   3, ABS,          rA,   -1)
  DI(SPC_sbc,   1, INDIR_IX,     rA,   -1)
  DI(SPC_sbc,   2, DP_IX_INDIR,  rA,   -1)

  // 0xa8
  DI(SPC_sbc,   2, IMM,          rA,   -1)
  DI(SPC_sbc,   3, DP_DP,        -1,   -1)
  DI(SPC_mov1,  3, BIT_OP,       rFc,  -1)
  DI(SPC_inc,   2, DP,           -1,   -1)
  DI(SPC_inc,   3, ABS,          -1,   -1)
  DI(SPC_cpy,   2, IMM,          rY,   -1)
  DI(SPC_pla,   1, IMPLIED,      rA,   -1)
  DI(SPC_sta,   1, INDIR_IX_INC, -1,   rA)

  // 0xb0
  DI(SPC_bcs,   2, PC_REL,       -1,   -1)
  DI(SPC_tcall, 1, IMPLIED,      -1,   -1)
  DI(SPC_clr5,  2, DP,           -1,   -1)
  DI(SPC_bbc5,  3, DP_PC_REL,    -1,   -1)
  DI(SPC_sbc,   2, DP_IX,        rA,   -1)
  DI(SPC_sbc,   3, ABS_IX,       rA,   -1)
  DI(SPC_sbc,   3, ABS_IY,       rA,   -1)
  DI(SPC_sbc,   2, DP_INDIR_IY,  rA,   -1)

  // 0xb8
  DI(SPC_sbc,   3, IMM_DP,       -1,   -1)
  DI(SPC_sbc,   1, INDIR_IX_IY,  -1,   -1)
  DI(SPC_movw,  2, DP,           rYA,  -1)
  DI(SPC_inc,   2, DP_IX,        -1,   -1)
  DI(SPC_inc,   1, IMPLIED,      rA,   -1)
  DI(SPC_txs,   1, IMPLIED,      rSP,  rX)
  DI(SPC_das,   1, IMPLIED,      rA,   -1)
  DI(SPC_lda,   1, INDIR_IX_INC, rA,   -1)

  // 0xc0
  DI(SPC_di,    1, IMPLIED,      -1,   -1)
  DI(SPC_tcall, 1, IMPLIED,      -1,   -1)
  DI(SPC_set6,  2, DP,           -1,   -1)
  DI(SPC_bbs6,  3, DP_PC_REL,    -1,   -1)
  DI(SPC_sta,   2, DP,           -1,   rA)
  DI(SPC_sta,   3, ABS,          -1,   rA)
  DI(SPC_sta,   1, INDIR_IX,     -1,   rA)
  DI(SPC_sta,   2, DP_IX_INDIR,  -1,   rX)

  // 0xc8
  DI(SPC_cpx,   2, IMM,          rX,   -1)
  DI(SPC_stx,   3, ABS,          -1,   rX)
  DI(SPC_and1,  3, BIT_OP,       -1,   rFc)
  DI(SPC_sty,   2, DP,           -1,   rY)
  DI(SPC_sty,   3, ABS,          -1,   rY)
  DI(SPC_ldx,   2, IMM,          rX,   -1)
  DI(SPC_plx,   1, IMPLIED,      rX,   -1)
  DI(SPC_mul,   1, IMPLIED,      rYA,  -1)

  // 0xd0
  DI(SPC_bne,   2, PC_REL,       -1,   -1)
  DI(SPC_tcall, 1, IMPLIED,      -1,   -1)
  DI(SPC_clr6,  2, DP,           -1,   -1)
  DI(SPC_bbc6,  3, DP_PC_REL,    -1,   -1)
  DI(SPC_sta,   2, DP_IX,        -1,   rA)
  DI(SPC_sta,   3, ABS_IX,       -1,   rA)
  DI(SPC_sta,   3, ABS_IY,       -1,   rA)
  DI(SPC_sta,   2, DP_INDIR_IY,  -1,   rA)

  // 0xd8
  DI(SPC_stx,   2, DP,           -1,   rA)
  DI(SPC_stx,   2, DP_IY,        -1,   rX)
  DI(SPC_movw,  2, DP,           -1,   rYA)
  DI(SPC_sty,   2, DP_IX,        -1,   rY)
  DI(SPC_dey,   1, IMPLIED,      rY,   -1)
  DI(SPC_tya,   1, IMPLIED,      rA,   rY)
  DI(SPC_cbne,  3, DP_IX_PC_REL, -1,   -1)
  DI(SPC_daa,   1, IMPLIED,      rA,   -1)

  // 0xe0
  DI(SPC_clrv,  1, IMPLIED,      -1,   -1)
  DI(SPC_tcall, 1, IMPLIED,      -1,   -1)
  DI(SPC_set7,  2, DP,           -1,   -1)
  DI(SPC_bbs7,  3, DP_PC_REL,    -1,   -1)
  DI(SPC_lda,   2, DP,           rA,   -1)
  DI(SPC_lda,   3, ABS,          rA,   -1)
  DI(SPC_lda,   1, INDIR_IX,     rA,   -1)
  DI(SPC_lda,   2, DP_IX_INDIR,  rA,   -1)

  // 0xe8
  DI(SPC_lda,   2, IMM,          rA,   -1)
  DI(SPC_ldx,   3, ABS,          rX,   -1)
  DI(SPC_not1,  3, BIT_OP,       -1,   -1)
  DI(SPC_ldy,   2, DP,           rY,   -1)
  DI(SPC_ldy,   3, ABS,          rY,   -1)
  DI(SPC_notc,  1, IMPLIED,      -1,   -1)
  DI(SPC_ply,   1, IMPLIED,      rY,   -1)
  DI(SPC_sleep, 1, IMPLIED,      -1,   -1)

  // 0xf0
  DI(SPC_beq,   2, PC_REL,       -1,   -1)
  DI(SPC_tcall, 1, IMPLIED,      -1,   -1)
  DI(SPC_clr7,  2, DP,           -1,   -1)
  DI(SPC_bbc7,  3, DP_PC_REL,    -1,   -1)
  DI(SPC_lda,   2, DP_IX,        rA,   -1)
  DI(SPC_lda,   3, ABS_IX,       rA,   -1)
  DI(SPC_lda,   3, ABS_IY,       rA,   -1)
  DI(SPC_lda,   2, DP_INDIR_IY,  rA,   -1)

  // 0xf8
  DI(SPC_ldx,   2, DP,           rX,   -1)
  DI(SPC_ldx,   2, DP_IY,        rX,   -1)
  DI(SPC_mov,   3, DP_DP,        -1,   -1)
  DI(SPC_ldy,   2, DP_IX,        rY,   -1)
  DI(SPC_iny,   1, IMPLIED,      rY,   -1)
  DI(SPC_tay,   1, IMPLIED,      rY,   rA)
  DI(SPC_dbnz,  2, PC_REL,       rY,   -1)
  DI(SPC_stop,  1, IMPLIED,      -1,   -1)
  };

#undef DI


// ---------------------------------------------------------------------------
const struct opcode_info_t &get_opcode_info(uint8 opcode)
{
  return opinfos[opcode];
}

//----------------------------------------------------------------------
int idaapi ana(void)
{
  cmd.Op1.dtyp = dt_byte;
  uint8 code = ua_next_byte();

  // Fetch instruction info
  const struct opcode_info_t &opinfo = get_opcode_info(code);
  cmd.itype = opinfo.itype;

  int op_i = 0;
  if ( opinfo.dreg != -1 )
  {
    cmd.Operands[op_i].type = o_reg;
    cmd.Operands[op_i].phrase = opinfo.dreg;
    op_i++;
  }

  switch ( opinfo.addr )
  {
    case IMPLIED:
      if ( cmd.itype == SPC_tcall )
      {
        cmd.Operands[op_i].type   = o_displ;
        cmd.Operands[op_i].phrase = rTCall;
        cmd.Operands[op_i].value  = code >> 4;
        cmd.Operands[op_i].addr   = 0xffde - (cmd.Operands[op_i].value << 1);
        op_i++;
      }
      break;
    case INDIR_IX:
      cmd.Operands[op_i].type = o_phrase;
      cmd.Operands[op_i].phrase = riX;
      op_i++;
      break;
    case INDIR_IX_INC:
      cmd.Operands[op_i].type = o_phrase;
      cmd.Operands[op_i].phrase = riXinc;
      op_i++;
      break;
    case INDIR_IX_IY:
      cmd.Operands[op_i].type = o_phrase;
      cmd.Operands[op_i].phrase = riX;
      op_i++;
      cmd.Operands[op_i].type = o_phrase;
      cmd.Operands[op_i].phrase = riY;
      op_i++;
      break;
    case IMM:
      cmd.Operands[op_i].type = o_imm;
      cmd.Operands[op_i].value = ua_next_byte();
      cmd.Operands[op_i].dtyp = dt_byte;
      op_i++;
      break;
    case IMM_DP:
      cmd.Operands[op_i + 1].type = o_imm;
      cmd.Operands[op_i + 1].value = ua_next_byte();
      cmd.Operands[op_i + 1].dtyp = dt_byte;

      cmd.Operands[op_i].type   = o_displ;
      cmd.Operands[op_i].phrase = rD;
      cmd.Operands[op_i].addr   = ua_next_byte();
      cmd.Operands[op_i].dtyp   = dt_byte;

      op_i += 2;
      break;
    case ABS:
      cmd.Operands[op_i].type = o_mem;
      cmd.Operands[op_i].addr = ua_next_word();
      if ( cmd.itype == SPC_call || cmd.itype == SPC_jmp )
        cmd.Operands[op_i].type = o_near;
      else
        cmd.Operands[op_i].dtyp = dt_byte;
      op_i++;
      break;
    case ABS_IX:
    case ABS_IY:
      cmd.Operands[op_i].type   = o_displ;
      cmd.Operands[op_i].phrase = opinfo.addr == ABS_IX ? rAbsX : rAbsY;
      cmd.Operands[op_i].addr   = ua_next_word();
      cmd.Operands[op_i].dtyp   = dt_byte;
      op_i++;
      break;
    case ABS_IX_INDIR:
      cmd.Operands[op_i].type   = o_displ;
      cmd.Operands[op_i].phrase = rAbsXi;
      cmd.Operands[op_i].addr   = ua_next_word();
      if ( cmd.itype == SPC_jmp )
        cmd.Operands[op_i].dtyp = dt_word;
      else
        cmd.Operands[op_i].dtyp = dt_byte;
      op_i++;
      break;
    case BIT_OP:
      {
        uint16 v = ua_next_word();
        cmd.Operands[op_i].type   = o_displ;
        if ( code == 0x2a || code == 0x6a )
            cmd.Operands[op_i].phrase = rDbitnot;
        else
            cmd.Operands[op_i].phrase = rDbit;
        cmd.Operands[op_i].addr   = v & 0x1fff;
        cmd.Operands[op_i].value  = v >> 13;
        cmd.Operands[op_i].dtyp   = dt_byte;
        op_i++;
      }
      break;
    case DP:
      if ( cmd.itype == SPC_pcall )
      {
        cmd.Operands[op_i].type = o_displ;
        cmd.Operands[op_i].phrase = rPCall;
        cmd.Operands[op_i].value  = ua_next_byte();
        cmd.Operands[op_i].addr = 0xff00 | cmd.Operands[op_i].value;
        op_i++;
      }
      else
      {
        cmd.Operands[op_i].type   = o_displ;
        cmd.Operands[op_i].phrase = rD;
        cmd.Operands[op_i].addr   = ua_next_byte();
        if ( cmd.itype == SPC_decw || cmd.itype == SPC_incw || cmd.itype == SPC_cmpw
          || cmd.itype == SPC_addw || cmd.itype == SPC_subw || cmd.itype == SPC_movw)
          cmd.Operands[op_i].dtyp   = dt_word;
        else
          cmd.Operands[op_i].dtyp   = dt_byte;
        op_i++;
      }
      break;
    case DP_IY:
    case DP_IX:
      cmd.Operands[op_i].type   = o_displ;
      cmd.Operands[op_i].phrase = opinfo.addr == DP_IX ? rDX : rDY;
      cmd.Operands[op_i].addr   = ua_next_byte();
      cmd.Operands[op_i].dtyp   = dt_byte;
      op_i++;
      break;
    case DP_IX_INDIR:
      cmd.Operands[op_i].type   = o_displ;
      cmd.Operands[op_i].phrase = riDX;
      cmd.Operands[op_i].addr   = ua_next_byte();
      cmd.Operands[op_i].dtyp   = dt_byte;
      op_i++;
      break;
    case DP_INDIR_IY:
      cmd.Operands[op_i].type   = o_displ;
      cmd.Operands[op_i].phrase = rDiY;
      cmd.Operands[op_i].addr   = ua_next_byte();
      cmd.Operands[op_i].dtyp   = dt_byte;
      op_i++;
      break;
    case DP_DP:
      cmd.Operands[op_i + 1].type   = o_displ;
      cmd.Operands[op_i + 1].phrase = rD;
      cmd.Operands[op_i + 1].addr   = ua_next_byte();
      cmd.Operands[op_i + 1].dtyp   = dt_byte;

      cmd.Operands[op_i].type   = o_displ;
      cmd.Operands[op_i].phrase = rD;
      cmd.Operands[op_i].addr   = ua_next_byte();
      cmd.Operands[op_i].dtyp   = dt_byte;

      op_i += 2;
      break;
    case DP_PC_REL:
	case DP_IX_PC_REL:
      cmd.Operands[op_i].type   = o_displ;
      if ( opinfo.addr == DP_IX_PC_REL )
        cmd.Operands[op_i].phrase = rDX;
      else
        cmd.Operands[op_i].phrase = rD;
      cmd.Operands[op_i].addr   = ua_next_byte();
      cmd.Operands[op_i].dtyp   = dt_byte;
      op_i++;

      cmd.Operands[op_i].type = o_near;
      {
        char x = ua_next_byte();
        cmd.Operands[op_i].addr = uint16(cmd.ip + cmd.size + x);
      }
      op_i++;
      break;
    case PC_REL:
      cmd.Operands[op_i].type = o_near;
      {
        char x = ua_next_byte();
        cmd.Operands[op_i].addr = uint16(cmd.ip + cmd.size + x);
      }
      op_i++;
      break;
    default:
      warning("ana: bad code 0x%x, @: 0x%a (IP=%a)", code, cmd.ea, cmd.ip);
      return 0;
  }

  if ( opinfo.sreg != -1 )
  {
    cmd.Operands[op_i].type = o_reg;
    cmd.Operands[op_i].phrase = opinfo.sreg;
    op_i++;
  }

  return cmd.size;
}
