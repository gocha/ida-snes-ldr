/*
 *      Interactive disassembler (IDA).
 *      Version 3.05
 *      Copyright (c) 1990-95 by Ilfak Guilfanov.
 *      ALL RIGHTS RESERVED.
 *                              FIDO:   2:5020/209
 *                              E-mail: ig@estar.msk.su
 *
 */

#include "spc700.hpp"

static bool flow;
//------------------------------------------------------------------------
static void doImmdValue(void)
{
  doImmd(cmd.ea);
}

//----------------------------------------------------------------------
static void handle_operand(op_t &x, bool read_access)
{
  ea_t ea;
  dref_t dreftype;
  switch ( x.type )
  {
    case o_void:
    case o_reg:
    case o_phrase:
      break;

    case o_imm:
      QASSERT(557, read_access);
      dreftype = dr_O;
MAKE_IMMD:
      doImmdValue();
      if ( isOff(uFlag, x.n) )
        ua_add_off_drefs(x, dreftype);
      break;

    case o_displ:
      dreftype = read_access ? dr_R : dr_W;
      switch ( x.phrase )
      {
        case rD:        // "dp"
        case rDX:       // "dp, X"
        case rDY:       // "dp, Y"
        case riDX:      // "(dp, X)"
        case rDiY:      // "(dp,n), Y"
          {
            sel_t dp = get_segreg(cmd.ea, rFp);
            if ( dp != BADSEL )
            {
              ea = toEA(dataSeg_op(x.n), (dp << 8) | x.addr);
              goto MAKE_DREF;
            }
            else
            {
              goto MAKE_IMMD;
            }
          }

        case rAbsX:     // "abs, X"
        case rAbsY:     // "abs, Y"
        case rAbsXi:    // "(abs,X)"
        case rDbit:     // "abs.n"
        case rDbitnot:  // "/abs.n"
          ea = toEA(dataSeg_op(x.n), x.addr);
          goto MAKE_DREF;

        case rTCall:    // "tcall n"
        case rPCall:    // "pcall n"
          ea = x.addr;
          goto MAKE_CREF;

        default:
          goto MAKE_IMMD;
      }
      break;

    case o_mem:
      ea = toEA(dataSeg_op(x.n), x.addr);
MAKE_DREF:
      ua_dodata2(x.offb, ea, x.dtyp);
      if ( !read_access )
        doVar(ea);
      ua_add_dref(x.offb, ea, read_access ? dr_R : dr_W);
      break;

    case o_near:
    case o_far:
      {
        if ( x.type == o_near )
        {
          ea_t segbase = codeSeg(x.addr, x.n);
          ea = toEA(segbase, x.addr);
        }
        else
        {
          ea = x.addr;
        }

MAKE_CREF:
        bool iscall = InstrIsSet(cmd.itype, CF_CALL);
        cref_t creftype = x.type == o_near
                        ? iscall ? fl_CN : fl_JN
                        : iscall ? fl_CF : fl_JF;
        ua_add_cref(x.offb, ea, creftype);
        if ( flow && iscall )
          flow = func_does_return(ea);
      }
      break;

    default:
      INTERR(558);
  }
}

//----------------------------------------------------------------------
// Mark the jump table content as a offset, and mark the destination address as code
// Note: sometimes the real table is located at another address (e.g. Super Mario World)
static void handle_jump_table(ea_t jtable_addr)
{
  asize_t jtable_size = 0;

  // guess jump table size
  // TODO: more elegant jump table handling?
  bool is_first_elem = true;
  ea_t jtable_dest_min;
  ea_t jtable_dest_max;
  ea_t jtable_elem_boundary = 0x10000;
  for (asize_t elem_n = 0; elem_n < 128; elem_n++)
  {
    ea_t elem_addr = jtable_addr + elem_n * 2;
    if ( elem_addr >= 0x10000 )
      break;

    ea_t dest = get_word(elem_addr);
    if ( is_first_elem )
    {
      jtable_dest_min = dest;
      jtable_dest_max = dest;

      if ( dest >= jtable_addr )
        jtable_elem_boundary = dest;

      is_first_elem = false;
    }
    else
    {
      // address is already pointed by the table - out of bounds
      if ( elem_addr >= jtable_elem_boundary )
        break;

      // assumption: addresses must be close each other
      const uint16 distance_threshold = 0x800;
      if ( ( jtable_dest_min >= distance_threshold && dest < jtable_dest_min - distance_threshold )
        || dest > jtable_dest_max + distance_threshold )
        break;

      // assumption: code must not be in page 0
      if ( dest <= 0xff )
      {
        jtable_size = 0;
        break;
      }

      // update boundaries
      if ( jtable_dest_min > dest )
        jtable_dest_min = dest;
      if ( jtable_dest_max < dest )
        jtable_dest_max = dest;
      if ( dest >= jtable_addr && jtable_elem_boundary > dest )
        jtable_elem_boundary = dest;
    }

    jtable_size++;
  }

  // check table size (very short tables could be a false-positive)
  if ( jtable_size >= 3 )
  {
    // mark each offsets
    doWord(jtable_addr, 2 * jtable_size);
    for (asize_t elem_n = 0; elem_n < jtable_size; elem_n++)
    {
      ea_t elem_addr = jtable_addr + elem_n * 2;
      ea_t dest = get_word(elem_addr);

      auto_make_code(dest);
      op_offset(elem_addr, 0, REF_OFF16);
    }
  }
}

//----------------------------------------------------------------------
int idaapi emu(void)
{
  uint32 Feature = cmd.get_canon_feature();
  flow = ((Feature & CF_STOP) == 0);

  if ( Feature & CF_USE1 ) handle_operand(cmd.Op1, 1);
  if ( Feature & CF_USE2 ) handle_operand(cmd.Op2, 1);
  if ( Feature & CF_CHG1 ) handle_operand(cmd.Op1, 0);
  if ( Feature & CF_CHG2 ) handle_operand(cmd.Op2, 0);
  if ( Feature & CF_JUMP ) QueueSet(Q_jumps,cmd.ea);

  uint8 code = get_byte(cmd.ea);
  const struct opcode_info_t &opinfo = get_opcode_info(code);

  if ( opinfo.itype == SPC_jmp )
  {
    if ( opinfo.addr == ABS_IX_INDIR ) {
      QueueSet(Q_jumps,cmd.ea);

      handle_jump_table(cmd.Op1.addr);
    }
  }

  if ( flow )
    ua_add_cref(0,cmd.ea+cmd.size,fl_F);

  switch ( cmd.itype )
  {
    case SPC_setp:
      split_srarea(cmd.ea + 1, rFp, 1, SR_auto);
      break;

    case SPC_clrp:
      split_srarea(cmd.ea + 1, rFp, 0, SR_auto);
      break;

    case SPC_jmp:
    case SPC_call:
      {
        if ( cmd.Op1.full_target_ea )
        {
          ea_t ftea = cmd.Op1.full_target_ea;

          split_srarea(ftea, rFp,  get_segreg(cmd.ea, rFp),  SR_auto);
        }
      }
      break;

    case SPC_php:
      // TODO: backtrack stack and update rFp
      break;

    case SPC_plp:
      // TODO: backtrack stack and update rFp
      break;
  }

  return 1;
}
