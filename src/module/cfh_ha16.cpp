
// This file contains a custom fixup handler for the HA16 fixup type.
// It is used by PPC, MIPS, and similar processors.
// Also thie file contains a custom refinfo handler for the HA16 type.

#include <fixup.hpp>

int cfh_ha16_id;  // id of fixup handler
int ref_ha16_id;  // id of refinfo handler

//--------------------------------------------------------------------------
// 'apply' a fixup: take it into account while analyzing the file
// usually it consists of converting the operand into an offset expression
static bool idaapi ha16_apply(
        ea_t ea,
        const fixup_data_t *fdp,
        ea_t item_start,
        int opnum,
        bool is_macro)
{
  if ( isUnknown(get_flags_novalue(item_start)) )
    do16bit(ea, 2);

  refinfo_t ri;
  ri.base   = get_fixup_base(ea, fdp);
  ri.target = ri.base + fdp->off;
  ri.tdelta = fdp->displacement;
  ri.flags = REFINFO_CUSTOM;

  if ( is_macro )
  { // it is a macro instruction, check for the second fixup within it
    fixup_data_t low;
    ea_t end = get_item_end(item_start);
    while ( true )
    {
      ea = get_next_fixup_ea(ea);
      if ( ea >=  end )
        return false; // could not find the fixup for the low part
      get_fixup(ea, &low);
      if ( low.get_type() == FIXUP_LOW16 )
        break; // found the pair!
    }
//    if ( int16(low.off + low.displacement) < 0 )
//      ri->tdelta -= 0x10000;
    ri.flags = REF_OFF32;
    ri.target = BADADDR;
  }
  else
  {
    ri.set_type(ref_ha16_id);
    ri.flags |= REFINFO_CUSTOM;
  }
  op_offset_ex(item_start, opnum, &ri);
  return true;
}

//--------------------------------------------------------------------------
// move the fixup from one location to another (called when the program is rebased)
static void idaapi ha16_move(ea_t ea, fixup_data_t *fdp, adiff_t delta)
{
  fdp->off += delta;
  put_word(ea, (fdp->off>>16)+1);
  set_fixup(ea, fdp);
}

//--------------------------------------------------------------------------
static const custom_fixup_handler_t cfh_ha16 =
{
  sizeof(custom_fixup_handler_t),
  "HIGHA16",                    // Format name, must be unique
  0,                            // properties (currently 0)
  2,                            // size in bytes
  NULL,                         // get_base, use standard formula
  NULL,                         // get_desc, use the format name
  ha16_apply,
  ha16_move,
};


//--------------------------------------------------------------------------
inline bool was_displacement_generated(const char *buf)
{
  const char *ptr = strchr(buf, COLOR_SYMBOL);    // is there a displacement?
  return ptr != NULL && (ptr[1] == '+' || ptr[1] == '-');
}

//--------------------------------------------------------------------------
// generate offset expression for HA16 refinfo
static int idaapi ha16_gen_expr(
        ea_t /*ea*/,
        int n,
        refinfo_t * /*ri*/,
        ea_t from,
        adiff_t * /*opval*/,
        char *buf,
        size_t bufsize,
        char * /*format*/,
        size_t /*formatsize*/,
        ea_t * /*target*/,
        ea_t *fullvalue,
        int getn_flags)
{
  ea_t ref = *fullvalue;

#ifdef _TRICORE_HPP
#define HIS_FUN COLSTR("@HIS", SCOLOR_KEYWORD) "("
  size_t fsz = sizeof(HIS_FUN) - 1;
  qstrncpy(buf, HIS_FUN, bufsize);
  buf += fsz;
  bufsize -= fsz;
  ssize_t len = get_name_expr(from, n, ref, BADADDR, buf, bufsize, getn_flags);
  if ( len == 0 )
    btoa(buf, bufsize, ref, 16);
  qstrncat(buf, ")", bufsize);
#undef HIS_FUN
#else
  ssize_t len = get_name_expr(from, n, ref, BADADDR, buf, bufsize, getn_flags);
  if ( len > 0 )
  { // if complex expression has been created, add parenthesis
    if ( was_displacement_generated(buf) )
    {
      qstring expr;
      expr.append(COLSTR("(", SCOLOR_SYMBOL));
      expr.append(buf);
      expr.append(COLSTR(")", SCOLOR_SYMBOL));
      qstrncpy(buf, expr.begin(), bufsize);
    }
  }
  else
  {
    btoa(buf, bufsize, ref, 16);
  }
  qstrncat(buf, COLSTR("@ha", SCOLOR_KEYWORD), bufsize);
#endif
  // always return 'simple expression' to avoid parenthesis around the
  // generated expression
  return 1;
}

//--------------------------------------------------------------------------
static const custom_refinfo_handler_t ref_ha16 =
{
  sizeof(custom_refinfo_handler_t),
  "HIGHA16",
  "high adjusted 16 bits of 32-bit offset",
  0,                    // properties (currently 0)
  NULL,                 // calc_basevalue, use ::calc_refinfo_basevalue
  NULL,                 // calc_target, use ::calc_refinfo_target
  ha16_gen_expr,
};
