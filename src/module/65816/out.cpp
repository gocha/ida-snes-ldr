/*
 *      Interactive disassembler (IDA).
 *      Version 3.05
 *      Copyright (c) 1990-95 by Ilfak Guilfanov.
 *      ALL RIGHTS RESERVED.
 *                              FIDO:   2:5020/209
 *                              E-mail: ig@estar.msk.su
 *
 */

#include "m65816.hpp"
#include "bt.hpp"

//----------------------------------------------------------------------
static void out_dp(op_t &x)
{
  sel_t dp = get_segreg(cmd.ea, rD);
  if ( dp != BADSEL )
  {
    ea_t orig_ea = dp + x.addr;
    ea_t ea = xlat(orig_ea);

    //if ( dp != 0 )
    //  out_symbol('(');

    if ( !out_name_expr(x, ea, BADADDR) )
    {
      out_tagon(COLOR_ERROR);
      OutValue(x,OOF_ADDR|OOFS_NOSIGN|OOFW_8);
      out_tagoff(COLOR_ERROR);
      QueueSet(Q_noName, cmd.ea);
    }

    //if ( dp != 0 )
    //{
    //  out_symbol(' ');
    //  out_symbol('-');
    //  out_symbol(' ');
    //  OutLong(dp << 8, 16);
    //  out_symbol(')');
    //}
  }
  else
  {
    OutValue(x,OOF_ADDR|OOFS_NOSIGN|OOFW_8);
  }
}

//----------------------------------------------------------------------
static void out_addr_near(op_t &x)
{
  sel_t db = get_segreg(cmd.ea, rB);
  if ( db != BADSEL )
  {
    ea_t orig_ea = (db << 16) + x.addr;
    ea_t ea = xlat(orig_ea);

    if ( !out_name_expr(x, ea, BADADDR) )
    {
      out_tagon(COLOR_ERROR);
      OutValue(x,OOF_ADDR|OOFS_NOSIGN|OOFW_16);
      out_tagoff(COLOR_ERROR);
      QueueSet(Q_noName, cmd.ea);
    }
  }
  else
  {
    OutValue(x,OOF_ADDR|OOFS_NOSIGN|OOFW_16);
  }
}

//----------------------------------------------------------------------
static void out_addr_far(op_t &x)
{
  ea_t orig_ea = x.addr;
  ea_t ea = xlat(orig_ea);

  if ( !out_name_expr(x, ea, BADADDR) )
  {
    out_tagon(COLOR_ERROR);
    OutValue(x,OOF_ADDR|OOFS_NOSIGN|OOFW_24);
    out_tagoff(COLOR_ERROR);
    QueueSet(Q_noName, cmd.ea);
  }
}

//----------------------------------------------------------------------
void print_orig_ea(const op_t &x)
{
  if ( !has_cmt(get_flags_novalue(cmd.ea)) )
  {
    char buf[64];
    qsnprintf(buf, sizeof(buf),
              COLSTR(" %s orig=0x%0*a", SCOLOR_AUTOCMT),
              ash.cmnt,
              (x.type == o_far || x.type == o_mem_far) ? 6 : 4,
              x.addr);
    OutLine(buf);
  }
}

//----------------------------------------------------------------------
ea_t calc_addr(const op_t &x, ea_t *orig_ea)
{
  ea_t ea;
  switch ( x.type )
  {
    case o_near:
      ea = toEA(codeSeg(x.addr, x.n), x.addr);
      goto XLAT_ADDR;
    case o_far:
    case o_mem_far:
      ea = x.addr;
      goto XLAT_ADDR;
    case o_mem:
      ea = toEA(dataSeg_op(x.n), x.addr);
XLAT_ADDR:
      if ( orig_ea != NULL )
        *orig_ea = ea;
      return xlat(ea);
    default:
      INTERR(559);
  }
}

//----------------------------------------------------------------------
bool idaapi outop(op_t &x)
{
  ea_t ea, orig_ea;
  switch ( x.type )
  {
    case o_reg:
      out_register(ph.regNames[x.reg]);
      break;
    case o_imm:
      out_symbol('#');
      OutValue(x, 0);
      break;
    case o_near:
    case o_far:
      if ( cmd.indirect )
        out_symbol('(');
      ea = calc_addr(x, &orig_ea);
      if ( !out_name_expr(x, ea, BADADDR) )
      {
        uint32 v = x.addr;
        if ( x.type == o_far )
          v &= 0xFFFFFF;
        else
          v &= 0xFFFF;
        out_tagon(COLOR_ERROR);
        OutLong(v, 16);
        out_tagoff(COLOR_ERROR);
        QueueSet(Q_noName, cmd.ea);
      }
      if ( cmd.indirect )
        out_symbol(')');
      if ( orig_ea != ea )
        print_orig_ea(x);
      break;
    case o_mem:
    case o_mem_far:
      {
        if ( cmd.indirect )
          out_symbol('(');

        if ( x.type == o_mem_far )
        {
          ea = calc_addr(x, &orig_ea);
          out_addr_far(x);
        }
        else
        {
          sel_t db = get_segreg(cmd.ea, rB);
          if ( db == BADSEL )
            ea = orig_ea = x.addr;
          else
            ea = calc_addr(x, &orig_ea);
          out_addr_near(x);
        }

        if ( cmd.indirect )
          out_symbol(')');

        if ( orig_ea != ea )
          print_orig_ea(x);
      }
      break;
    case o_displ:
      switch ( x.phrase )
      {
        case rS:
          out_register(ph.regNames[x.phrase]);
          out_symbol(',');
          OutChar(' ');
          OutValue(x,OOF_ADDR|OOFS_NOSIGN|OOFW_8);
          break;
        case rD:
          out_register(ph.regNames[x.phrase]);
          out_symbol(',');
          OutChar(' ');
          out_dp(x);
          break;
        case rSiY:
          out_symbol('(');
          out_register("S");
          out_symbol(',');
          OutChar(' ');
          OutValue(x,OOF_ADDR|OOFS_NOSIGN|OOFW_8);
          out_symbol(',');
          OutChar(' ');
          out_register("Y");
          out_symbol(')');
          break;
        case rDi:
        case rSDi:
          out_symbol('(');
          out_register("D");
          out_symbol(',');
          OutChar(' ');
          out_dp(x);
          out_symbol(')');
          break;
        case rDiL:
          out_symbol('[');
          out_register("D");
          out_symbol(',');
          OutChar(' ');
          out_dp(x);
          out_symbol(']');
          break;
        case rDX:
        case rDY:
          out_register("D");
          out_symbol(',');
          OutChar(' ');
          out_dp(x);
          out_symbol(',');
          OutChar(' ');
          out_register((x.phrase == rDX) ? "X" : "Y");
          break;
        case riDX:
          out_symbol('(');
          out_register("D");
          out_symbol(',');
          OutChar(' ');
          out_dp(x);
          out_symbol(',');
          OutChar(' ');
          out_register("X");
          out_symbol(')');
          break;
        case rDiY:
        case rDiLY:
          out_symbol(x.phrase == rDiLY ? '[' : '(');
          out_register("D");
          out_symbol(',');
          OutChar(' ');
          out_dp(x);
          out_symbol(x.phrase == rDiLY ? ']' : ')');
          out_symbol(',');
          OutChar(' ');
          out_register("Y");
          break;
        case rAbsi:
          out_symbol('(');
          out_addr_near(x);
          out_symbol(')');
          break;
        case rAbsiL:
          out_symbol('[');
          out_addr_near(x);
          out_symbol(']');
          break;
        case rAbsX:
        case rAbsY:
          out_addr_near(x);
          out_symbol(',');
          OutChar(' ');
          out_register(x.phrase == rAbsY ? "Y" : "X");
          break;
        case rAbsLX:
          {
            ea_t orig_ea = x.addr;
            ea_t ea = xlat(orig_ea);

            out_addr_far(x);
            out_symbol(',');
            OutChar(' ');
            out_register("X");

            if ( orig_ea != ea )
            {
              print_orig_ea(x);
            }
          }
          break;
        case rAbsXi:
          out_symbol('(');
          out_addr_near(x);
          out_symbol(',');
          OutChar(' ');
          out_register("X");
          out_symbol(')');
          break;
        default:
          goto err;
      }
      break;
    case o_void:
      return 0;
    default:
    err:
      warning("out: %a: bad optype %d", cmd.ea, x.type);
      break;
  }
  return 1;
}


//----------------------------------------------------------------------
inline bool forced_print(ea_t ea, int reg)
{
  return (reg == rFm || reg == rFx) && isFunc(get_flags_novalue(ea));
}

//----------------------------------------------------------------------
void idaapi assumes(ea_t ea)
{
  char buf[MAXSTR];
  char *ptr = buf;
  char *end = buf + sizeof(buf);
  for ( int reg=ph.regFirstSreg; reg <= ph.regLastSreg; reg++ )
  {
    if ( reg == rCs )
      continue;
    segreg_area_t srarea;
    get_srarea2(&srarea, ea, reg);
    segreg_area_t prev;
    bool prev_exists = get_srarea2(&prev, ea - 1, reg);
    // if 'prev' does not exist, force to print because we are at
    // the beginning of the segment
    sel_t curval  = srarea.val;
    sel_t prevval = prev_exists ? prev.val : curval - 1;
    if ( curval != prevval || forced_print(ea, reg) )
    {
      if ( reg == rFm || reg == rFx )
      {
        printf_line(0, ".%c%d", reg == rFm ? 'A' : 'I', curval > 0 ? 8 : 16);
      }
      else
      {
        if ( ptr != buf )
          APPCHAR(ptr, end, ' ');
        if ( curval == BADSEL && ( reg == rD || reg == rB || reg == rPB ) )
          ptr += qsnprintf(ptr, end-ptr, "%s=%d", ph.regNames[reg], curval);
        else
          ptr += qsnprintf(ptr, end-ptr, "%s=%a", ph.regNames[reg], curval);
      }
    }
  }
  if ( ptr != buf )
    gen_cmt_line("%s", buf);
}

//----------------------------------------------------------------------
void idaapi out(void)
{
  char buf[MAXSTR];

  init_output_buffer(buf, sizeof(buf));
  if ( inf.s_showbads
    && cmd.Op1.type == o_displ
    && (cmd.Op1.phrase == rX || cmd.Op1.phrase == rY)
    && cmd.Op1.value == uchar(cmd.Op1.value) )
  {
    OutBadInstruction();
  }

  OutMnem();
  out_one_operand(0);
  if ( cmd.Op2.type != o_void )
  {
    out_symbol(',');
    OutChar(' ');
    out_one_operand(1);
  }

  if ( isVoid(cmd.ea, uFlag, 0) )
    OutImmChar(cmd.Op1);

  term_output_buffer();
  gl_comm = 1;
  MakeLine(buf);
}

//--------------------------------------------------------------------------
void idaapi header(void)
{
  gen_cmt_line("%s Processor:        %s", ash.cmnt, inf.procName);
  gen_cmt_line("%s Target assembler: %s", ash.cmnt, ash.name);
  if ( ash.header != NULL )
    for ( const char *const *ptr=ash.header; *ptr != NULL; ptr++ )
      MakeLine(*ptr,0);
}

//--------------------------------------------------------------------------
void idaapi segstart(ea_t ea)
{
  segment_t *Sarea = getseg(ea);
  char name[MAXNAMELEN];
  get_segm_name(Sarea, name, sizeof(name));
  if ( ash.uflag & UAS_SECT )
  {
    printf_line(0, COLSTR("%s: .section",SCOLOR_ASMDIR), name);
  }
  else
  {
    printf_line(inf.indent, COLSTR("%s.segment %s",SCOLOR_ASMDIR),
                   (ash.uflag & UAS_NOSEG) ? ash.cmnt : "",
                   name);
    if ( ash.uflag & UAS_SELSG )
      MakeLine(name, inf.indent);
    if ( ash.uflag & UAS_CDSEG )
      MakeLine(COLSTR("CSEG",SCOLOR_ASMDIR), inf.indent);  // XSEG - eXternal memory
  }
  if ( inf.s_org )
  {
    ea_t org = ea - get_segm_base(Sarea);
    if ( org != 0 )
    {
      char buf[MAX_NUMBUF];
      btoa(buf, sizeof(buf), org);
      printf_line(inf.indent, COLSTR("%s %s",SCOLOR_ASMDIR), ash.origin, buf);
    }
  }
}

//--------------------------------------------------------------------------
void idaapi footer(void)
{
  char buf[MAXSTR];
  if ( ash.end != NULL )
  {
    MakeNull();
    char *ptr = buf;
    char *end = buf + sizeof(buf);
    APPEND(ptr, end, ash.end);
    qstring name;
    if ( get_colored_name(&name, inf.beginEA) > 0 )
    {
      if ( ash.uflag & UAS_NOENS )
        APPEND(ptr, end, ash.cmnt);
      APPCHAR(ptr, end, ' ');
      APPEND(ptr, end, name.begin());
    }
    MakeLine(buf, inf.indent);
  }
  else
  {
    gen_cmt_line("end of file");
  }
}
