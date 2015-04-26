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

//----------------------------------------------------------------------
bool idaapi outop(op_t &x)
{
  ea_t ea;
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
    case o_mem:
      if ( cmd.indirect )
        out_symbol('(');
      ea = x.addr;
      if ( !out_name_expr(x, ea, BADADDR) )
      {
        out_tagon(COLOR_ERROR);
        OutLong(ea, 16);
        out_tagoff(COLOR_ERROR);
        QueueSet(Q_noName, cmd.ea);
      }
      if ( cmd.indirect )
        out_symbol(')');
      break;
    case o_displ:
      switch ( x.phrase )
      {
        case rD:
          OutValue(x,OOF_ADDR|OOFS_NOSIGN|OOFW_8);
          break;
        case rDX:
        case rDY:
          OutValue(x,OOF_ADDR|OOFS_NOSIGN|OOFW_8);
          out_symbol('+');
          out_register(x.phrase == rDX ? ph.regNames[rX] : ph.regNames[rY]);
          break;
        case riDX:
          out_symbol('(');
          OutValue(x,OOF_ADDR|OOFS_NOSIGN|OOFW_8);
          out_symbol('+');
          out_register(ph.regNames[rX]);
          out_symbol(')');
          break;
        case rDiY:
          out_symbol('(');
          OutValue(x,OOF_ADDR|OOFS_NOSIGN|OOFW_8);
          out_symbol(')');
          out_symbol('+');
          out_register(ph.regNames[rY]);
          break;
        case rAbsX:
        case rAbsY:
          OutValue(x,OOF_ADDR|OOFS_NOSIGN|OOFW_16);
          out_symbol('+');
          out_register(x.phrase == rAbsX ? ph.regNames[rX] : ph.regNames[rY]);
          break;
        case rAbsXi:
          out_symbol('(');
          OutValue(x,OOF_ADDR|OOFS_NOSIGN|OOFW_16);
          out_symbol('+');
          out_register(ph.regNames[rX]);
          out_symbol(')');
          break;
        case riX:
        case riY:
          out_symbol('(');
          out_register(x.phrase == riX ? ph.regNames[rX] : ph.regNames[rY]);
          out_symbol(')');
          break;
        case riXinc:
          out_symbol('(');
          out_register(ph.regNames[rX]);
          out_symbol(')');
          out_symbol('+');
          break;
        case rDbitnot:
          out_symbol('/');
          // fall through
        case rDbit:
          OutValue(x,OOF_ADDR|OOFS_NOSIGN|OOFW_16);
          out_symbol('.');
          OutValue(x, 0);
          break;
        case rTCall:
          OutLong(x.value, 10);
          break;
        case rPCall:
          OutLong(x.value, 16);
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
void idaapi out(void)
{
  char buf[MAXSTR];

  init_output_buffer(buf, sizeof(buf));
  if ( inf.s_showbads && cmd.Op1.type == o_displ &&
       (cmd.Op1.phrase == rX || cmd.Op1.phrase == rY) &&
       cmd.Op1.value == uchar(cmd.Op1.value) ) OutBadInstruction();

  OutMnem();
  out_one_operand(0);
  if ( cmd.Op2.type != o_void ) {
    out_symbol(',');
    OutChar(' ');
    out_one_operand(1);
  }

  if ( isVoid(cmd.ea,uFlag,0) ) OutImmChar(cmd.Op1);

  term_output_buffer();
  gl_comm = 1;
  MakeLine(buf);
}

//--------------------------------------------------------------------------
void idaapi header(void)
{
  gen_cmt_line("%s Processor:        %s",ash.cmnt,inf.procName);
  gen_cmt_line("%s Target assembler: %s",ash.cmnt,ash.name);
  if ( ash.header != NULL )
    for ( const char *const *ptr=ash.header; *ptr != NULL; ptr++ )
      MakeLine(*ptr, 0);
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
    if ( ash.uflag & UAS_SELSG ) MakeLine(name, inf.indent);
    if ( ash.uflag & UAS_CDSEG ) MakeLine(COLSTR("CSEG",SCOLOR_ASMDIR), inf.indent);  // XSEG - eXternal memory
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
