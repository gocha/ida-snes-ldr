
#include "m65816.hpp"
#include "bt.hpp"

static bool flow;

//------------------------------------------------------------------------
inline void doImmdValue(void)
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
      break;

    case o_imm:
      QASSERT(557, read_access);
      dreftype = dr_O;
      goto MAKE_IMMD;

    case o_displ:
      dreftype = read_access ? dr_R : dr_W;
MAKE_IMMD:
      doImmdValue();
      if ( isOff(uFlag, x.n) )
        ua_add_off_drefs(x, dreftype);
      break;

    case o_mem:
      ea = calc_addr(x);
      ua_dodata2(x.offb, ea, x.dtyp);
      if ( !read_access )
        doVar(ea);
      ua_add_dref(x.offb, ea, read_access ? dr_R : dr_W);
      break;

    case o_near:
    case o_far:
      {
        ea_t orig_ea;
        ea = calc_addr(x, &orig_ea);
        if ( cmd.itype == M65816_per )
        {
          ua_add_dref(x.offb, ea, dr_O);
        }
        else
        {
          bool iscall = InstrIsSet(cmd.itype, CF_CALL);
          cref_t creftype = x.type == o_near
                          ? iscall ? fl_CN : fl_JN
                          : iscall ? fl_CF : fl_JF;
          ua_add_cref(x.offb, ea, creftype);
          if ( flow && iscall )
            flow = func_does_return(ea);
        }
      }
      break;

    default:
      INTERR(558);
  }
}

//----------------------------------------------------------------------
/**
 * Get what is known of the status flags register,
 * at address 'ea'.
 *
 * ea      : The effective address.
 *
 * returns : A 9-bit value, composed with what is known of the
 *           status register at the 'ea' effective address. Its
 *           layout is the following:
 * +----------------------------------------------------------------+
 * | 0 | 0 | 0 | 0 | 0 | 0 | 0 | e || n | v | m | x | d | i | z | c |
 * +----------------------------------------------------------------+
 *  15                                7                           0
 *           Note that a 16-bit value is returned, in order to
 *           take the emulation-mode flag into consideration.
 */
static uint16 get_cpu_status(ea_t ea)
{
  return (get_segreg(ea, rFe) << 8) | (get_segreg(ea, rFm) << 5) | (get_segreg(ea, rFx) << 4);
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

  if ( flow )
    ua_add_cref(0,cmd.ea+cmd.size,fl_F);



  switch ( cmd.itype )
  {
    case M65816_sep:
    case M65816_rep:
      {
        // Switching 8 -> 16 bits modes.
        uint8 flag_data = get_byte(cmd.ea + 1);
        uint8 m_flag = flag_data & 0x20;
        uint8 x_flag = flag_data & 0x10;
        uint8 val    = (cmd.itype == M65816_rep) ? 0 : 1;

        if ( m_flag )
          split_srarea(cmd.ea + 2, rFm, val, SR_auto);
        if ( x_flag )
          split_srarea(cmd.ea + 2, rFx, val, SR_auto);
      }
      break;

    case M65816_xce:
      {
        // Switching to native mode?
        uint8 prev = get_byte(cmd.ea - 1);
        const struct opcode_info_t &opinf = get_opcode_info(prev);
        if ( opinf.itype == M65816_clc )
          split_srarea(cmd.ea + 1, rFe, 0, SR_auto);
        else if ( opinf.itype == M65816_sec )
          split_srarea(cmd.ea + 1, rFe, 1, SR_auto);
      }
      break;

    case M65816_jmp:
    case M65816_jml:
    case M65816_jsl:
    case M65816_jsr:
      {
        if ( cmd.Op1.full_target_ea )
        {
          ea_t ftea = cmd.Op1.full_target_ea;
          if ( cmd.itype != M65816_jsl && cmd.itype != M65816_jml )
            ftea = toEA(codeSeg(ftea, 0), ftea);
          else
            ftea = xlat(ftea);

          split_srarea(ftea, rFm,  get_segreg(cmd.ea, rFm),  SR_auto);
          split_srarea(ftea, rFx,  get_segreg(cmd.ea, rFx),  SR_auto);
          split_srarea(ftea, rFe,  get_segreg(cmd.ea, rFe),  SR_auto);
          split_srarea(ftea, rB,   get_segreg(cmd.ea, rB),   SR_auto);
          split_srarea(ftea, rDs,  get_segreg(cmd.ea, rDs),  SR_auto);
          split_srarea(ftea, rD,   get_segreg(cmd.ea, rD),   SR_auto);
        }
      }
      break;

    case M65816_plb:
      {
        int32 val = backtrack_value(cmd.ea, 1, BT_STACK);
        if ( val != -1 )
          split_srarea(cmd.ea + cmd.size, rB, val, SR_auto);
      }
      break;

    case M65816_pld:
      {
        int32 val = backtrack_value(cmd.ea, 2, BT_STACK);
        if ( val != -1 )
          split_srarea(cmd.ea + cmd.size, rD, val, SR_auto);
      }
      break;

    case M65816_plp:
      {
        // Ideally, should pass another parameter, specifying when to stop
        // backtracking.
        // For example, in order to avoid this:
        //     PHP
        //     PLP <-- this one is causing interference
        //             (dunno if that even happens, though)
        //     PLP
        ea_t ea = backtrack_prev_ins(cmd.ea, M65816_php);
        if ( ea != BADADDR )
        {
          uint16 p = get_cpu_status(ea);
          split_srarea(cmd.ea + cmd.size, rFm, (p >> 5) & 0x1, SR_auto);
          split_srarea(cmd.ea + cmd.size, rFx, (p >> 4) & 0x1, SR_auto);
        }
      }
      break;
  }

  return 1;
}

