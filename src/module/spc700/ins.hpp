/*
 *      Interactive disassembler (IDA).
 *      Copyright (c) 1990-2008 Hex-Rays
 *      ALL RIGHTS RESERVED.
 *
 */

#ifndef __INSTRS_HPP
#define __INSTRS_HPP

extern instruc_t Instructions[];

enum spc_itype_t
{
  SPC_adc,
  SPC_addw,
  SPC_and,
  SPC_and1,
  SPC_asl,
  SPC_bbc0,
  SPC_bbc1,
  SPC_bbc2,
  SPC_bbc3,
  SPC_bbc4,
  SPC_bbc5,
  SPC_bbc6,
  SPC_bbc7,
  SPC_bbs0,
  SPC_bbs1,
  SPC_bbs2,
  SPC_bbs3,
  SPC_bbs4,
  SPC_bbs5,
  SPC_bbs6,
  SPC_bbs7,
  SPC_bcc,
  SPC_bcs,
  SPC_beq,
  SPC_bmi,
  SPC_bne,
  SPC_bpl,
  SPC_bra,
  SPC_brk,
  SPC_bvc,
  SPC_bvs,
  SPC_call,
  SPC_cbne,
  SPC_clr0,
  SPC_clr1,
  SPC_clr2,
  SPC_clr3,
  SPC_clr4,
  SPC_clr5,
  SPC_clr6,
  SPC_clr7,
  SPC_clrc,
  SPC_clrp,
  SPC_clrv,
  SPC_cmp,
  SPC_cpx, // cmp x
  SPC_cpy, // cmp y
  SPC_cmpw,
  SPC_daa,
  SPC_das,
  SPC_dbnz,
  SPC_dec,
  SPC_dex, // dec x
  SPC_dey, // dec y
  SPC_decw,
  SPC_di,
  SPC_div,
  SPC_ei,
  SPC_eor,
  SPC_eor1,
  SPC_inc,
  SPC_inx,
  SPC_iny,
  SPC_incw,
  SPC_jmp,
  SPC_lda, // mov a, *
  SPC_ldx, // mov x, *
  SPC_ldy, // mov y, *
  SPC_lsr,
  SPC_mov,
  SPC_movw,
  SPC_mov1,
  SPC_mul,
  SPC_nop,
  SPC_not1,
  SPC_notc,
  SPC_or,
  SPC_or1,
  SPC_pcall,
  SPC_pha, // push a
  SPC_php, // push psw
  SPC_phx, // push x
  SPC_phy, // push y
  SPC_pla, // pop a
  SPC_plp, // pop psw
  SPC_plx, // pop x
  SPC_ply, // pop y
  SPC_ret,
  SPC_reti,
  SPC_rol,
  SPC_ror,
  SPC_sbc,
  SPC_set0,
  SPC_set1,
  SPC_set2,
  SPC_set3,
  SPC_set4,
  SPC_set5,
  SPC_set6,
  SPC_set7,
  SPC_setc,
  SPC_setp,
  SPC_sleep,
  SPC_stop,
  SPC_sta, // mov
  SPC_stx, // mov
  SPC_sty, // mov
  SPC_subw,
  SPC_tax, // mov x, a
  SPC_tay, // mov y, a
  SPC_tsx, // mov x, sp
  SPC_txa, // mov a, x
  SPC_txs, // mov sp, x
  SPC_tya, // mov a, y
  SPC_tcall,
  SPC_tclr1,
  SPC_tset1,
  SPC_xcn,
  SPC_last
};

#endif
