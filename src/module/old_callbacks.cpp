
// This file should be used for the processor modules that have PR_TINFO set
// It adds support for old plugins (including the decompiler)

#include "idaidp.hpp"
#undef NO_OBSOLETE_FUNCS
#include <typeinf.hpp>
#include <allow_deprecated.hpp>

//---------------------------------------------------------------------
static int regs_req(int msgid, va_list va, callregs_t *callregs)
{
  if ( callregs->gpregs.empty() )
  {
    int rc = ph.notify((processor_t::idp_notify)msgid, callregs);
    if ( rc < 2 )
      return rc - 2;
    callregs->gpregs.push_back(-1);
  }
  int rc;
  const int **retregs = va_arg(va, const int **);
  if ( callregs->policy == ARGREGS_POLICY_UNDEFINED )
  {
    rc = 0;
    *retregs = NULL;
  }
  else
  {
    rc = callregs->gpregs.size() - 1;
    *retregs = callregs->gpregs.begin();
  }
  return rc;
}

//---------------------------------------------------------------------
int handle_old_type_callbacks(int msgid, va_list va)
{
  struct ida_local funcargvec_creator_t
  {
    funcargvec_t args;
    funcargvec_creator_t(
      const type_t * const *types,
      const char * const *names,
      const uint32 *regs,
      int n)
    {
      args.resize(n);
      for ( int i=0; i < n; i++ )
      {
        funcarg_t &fa = args[i];
        fa.name = names[i];
        fa.argloc.set_reg1(regs[i]);
        const type_t *tptr = types[i];
        fa.type.deserialize(idati, &tptr);
      }
    }
  };

  struct ida_local use_arg_caller_t
  {
    use_arg_caller_t(
      ea_t ea,
      const type_t * const *types,
      const char * const *names,
      const varloc_t *arglocs,
      int n,
      const type_t **rtypes,
      const char **rnames,
      uint32 *regs,
      int *p_rn)
    {
      func_type_data_t fti;
      fti.resize(n);
      for ( int i=0; i < n; i++ )
      {
        funcarg_t &fa = fti[i];
        fa.name = names[i];
        convert_varloc_to_argloc(&fa.argloc, arglocs[i]);
        const type_t *tptr = types[i];
        fa.type.deserialize(idati, &tptr);
      }
      funcargvec_creator_t fc(rtypes, rnames, regs, *p_rn);
      ph.notify(ph.use_arg_types3, ea, &fti, &fc.args);
      *p_rn = fc.args.size();
      for ( int i=0; i < *p_rn; i++ )
        regs[i] = fc.args[i].argloc.reg1();
    }
  };

  switch ( msgid )
  {
    case processor_t::obsolete_use_regarg_type:
      {
        ea_t ea                     = va_arg(va, ea_t);
        const type_t * const *types = va_arg(va, const type_t * const *);
        const char * const *names   = va_arg(va, const char * const *);
        const uint32 *regs          = va_arg(va, const uint32 *);
        int n                       = va_arg(va, int);
        funcargvec_creator_t fc(types, names, regs, n);
        int idx = -1;
        ph.notify(ph.use_regarg_type3, &idx, ea, &fc.args);
        return idx;
      }

    case processor_t::obsolete_use_arg_types:
      {
        ea_t ea                     = va_arg(va, ea_t);
        const type_t * const *types = va_arg(va, const type_t * const *);
        const char * const *names   = va_arg(va, const char * const *);
        const varloc_t *arglocs     = va_arg(va, const varloc_t *);
        int n                       = va_arg(va, int);
        const type_t **rtypes       = va_arg(va, const type_t **);
        const char **rnames         = va_arg(va, const char **);
        uint32 *regs                = va_arg(va, uint32 *);
        int rn                      = va_arg(va, int);
        use_arg_caller_t uac(ea, types, names, arglocs, n, rtypes, rnames, regs, &rn);
        return rn;
      }

    case processor_t::obsolete_get_fastcall_regs:
    case processor_t::obsolete_get_fastcall_regs2:
      {
        static callregs_t callregs;
        int rc = regs_req(ph.get_fastcall_regs3, va, &callregs);
        return msgid == processor_t::obsolete_get_fastcall_regs2 ? (rc+2) : rc;
      }

    case processor_t::obsolete_get_varcall_regs:
    case processor_t::obsolete_get_varcall_regs2:
      {
        static callregs_t callregs;
        int rc = regs_req(ph.get_varcall_regs3, va, &callregs);
        return msgid == processor_t::obsolete_get_varcall_regs2 ? (rc+2) : rc;
      }

    case processor_t::obsolete_get_thiscall_regs:
    case processor_t::obsolete_get_thiscall_regs2:
      {
        static callregs_t callregs;
        int rc = regs_req(ph.get_thiscall_regs3, va, &callregs);
        return msgid == processor_t::obsolete_get_thiscall_regs2 ? (rc+2) : rc;
      }

    case processor_t::obsolete_calc_cdecl_purged_bytes:
      {
        ea_t ea = va_arg(va, ea_t);
        return ph.notify(ph.calc_cdecl_purged_bytes2, ea) - 2;
      }

    case processor_t::obsolete_get_stkarg_offset:
                                // get offset from SP to the first stack argument
                                // args: none
                                // returns: the offset
      return ph.notify(ph.get_stkarg_offset2) - 2;

    case processor_t::obsolete_decorate_name:
      {
        const til_t *ti    = va_arg(va, const til_t *);
        const char *name   = va_arg(va, const char *);
        const type_t *type = va_arg(va, const type_t *);
        char *outbuf       = va_arg(va, char *);
        size_t bufsize     = va_arg(va, size_t);
        bool mangle        = va_argi(va, bool);
        cm_t real_cc       = va_argi(va, cm_t);
        return gen_decorate_name(ti, name, type, outbuf, bufsize, mangle, real_cc);
      }

    case processor_t::obsolete_calc_purged_bytes:
      {
        const type_t *type = va_arg(va, const type_t *);
        tinfo_t tif;
        if ( tif.deserialize(idati, &type) )
        {
          func_type_data_t fti;
          if ( tif.get_func_details(&fti) )
          {
            int npurged;
            if ( ph.notify(ph.calc_purged_bytes3, &npurged, &fti) == 2 )
              return npurged + 1;
          }
        }
      }
      break;

    case processor_t::obsolete_calc_arglocs2:
      {
        const type_t *type = va_arg(va, const type_t *);
        cm_t cc            = va_argi(va, cm_t);
        varloc_t *arglocs  = va_arg(va, varloc_t *);
        qtype functype;
        functype.append(BT_FUNC);
        functype.append(cc);
        functype.append(type);
        tinfo_t tif;
        tif.deserialize(idati, &functype);
        func_type_data_t fti;
        if ( !tif.get_func_details(&fti, GTD_NO_ARGLOCS) )
          return -1; // failed
        if ( ph.notify(ph.calc_arglocs3, &fti) != 2 )
          return -1; // failed
        int n = fti.size();
        for ( int i=0; i < n; i++ )
          if ( !convert_argloc_to_varloc(&arglocs[i], fti[i].argloc) )
            return -1; // failed
        arglocs[n].set_stkoff(fti.stkargs);
        return 2;
      }

    case processor_t::obsolete_calc_varglocs:
      {
        const func_type_info_t *fti   = va_arg(va, const func_type_info_t *);
        int nargs                     = va_arg(va, int);
        const type_t *const *argtypes = va_arg(va, const type_t *const *);
        varloc_t *arglocs             = va_arg(va, varloc_t *);
        regobjs_t *regargs            = va_arg(va, regobjs_t *);

        // convert func_type_info_t => func_type_data_t
        func_type_data_t ftd;
        ftd.rettype.deserialize(idati, &fti->rettype);
        convert_varloc_to_argloc(&ftd.retloc, fti->retloc);
        ftd.flags  = fti->flags;
        ftd.cc     = fti->cc;
        int nfixed = fti->size();
        ftd.resize(nargs);
        for ( int i=0; i < nargs; i++ )
        {
          funcarg_t &fa = ftd[i];
          const type_t *type;
          if ( i < nfixed )
          {
            const funcarg_info_t &fi = fti->at(i);
            convert_varloc_to_argloc(&fa.argloc, fi.argloc);
            type = fi.type.begin();
          }
          else
          {
            convert_varloc_to_argloc(&fa.argloc, arglocs[i-nfixed]);
            type = argtypes[i-nfixed];
          }
          fa.type.deserialize(idati, &type);
        }
        if ( ph.notify(ph.calc_varglocs3, &ftd, regargs, NULL, nfixed) != 2 )
          return -1;
        for ( int i=nfixed; i < nargs; i++ )
          if ( !convert_argloc_to_varloc(&arglocs[i-nfixed], ftd[i].argloc) )
            return false;
        arglocs[nargs-nfixed].set_stkoff(ftd.stkargs);
        return 2;
      }

    case processor_t::obsolete_calc_retloc:
      {
        const type_t *type = va_arg(va, const type_t *);
        cm_t cc            = va_argi(va, cm_t);
        varloc_t *retloc   = va_arg(va, varloc_t *);
        tinfo_t tif;
        tif.deserialize(idati, &type);
        argloc_t al;
        int code = ph.notify(ph.calc_retloc3, &tif, cc, &al);
        if ( code == 2 && retloc != NULL )
        {
          if ( !convert_argloc_to_varloc(retloc, al) )
            code = -1;
        }
        return code;
      }

    case processor_t::obsolete_use_stkarg_type:
      {
        ea_t ea            = va_arg(va, ea_t);
        const type_t *type = va_arg(va, const type_t *);
        const char *name   = va_arg(va, const char *);
        funcarg_t fa;
        fa.type.deserialize(idati, &type);
        fa.name = name;
        return ph.notify(ph.use_stkarg_type3, ea, &fa);
      }

    case processor_t::obsolete_use_regarg_type2:
      {
        int *retidx                 = va_arg(va, int *);
        ea_t ea                     = va_arg(va, ea_t);
        const type_t *const *types  = va_arg(va, const type_t *const *);
        const char *const *names    = va_arg(va, const char *const *);
        const uint32 *regs          = va_arg(va, const uint32 *);
        int n                       = va_arg(va, int);
        funcargvec_creator_t fc(types, names, regs, n);
        return ph.notify(ph.use_regarg_type3, retidx, ea, &fc.args);
      }

    case processor_t::obsolete_use_arg_types2:
      {
        ea_t ea                     = va_arg(va, ea_t);
        const type_t *const *types  = va_arg(va, const type_t *const *);
        const char *const *names    = va_arg(va, const char *const *);
        const varloc_t *arglocs     = va_arg(va, const varloc_t *);
        int n                       = va_arg(va, int);
        const type_t **rtypes       = va_arg(va, const type_t **);
        const char **rnames         = va_arg(va, const char **);
        uint32 *regs                = va_arg(va, uint32 *);
        int *rn                     = va_arg(va, int *);
        use_arg_caller_t uac(ea, types, names, arglocs, n, rtypes, rnames, regs, rn);
        return 2;
      }
    case processor_t::obsolete_get_reg_info:
      {
        const char *name = va_arg(va, const char *);
        const char **res = va_arg(va, const char **);
        uint64 *mask     = va_arg(va, uint64 *);
        bitrange_t bitrange;
        int code = ph.notify(ph.get_reg_info2, name, res, &bitrange);
        if ( code != 2 )
          return 1; // unimplemented
        if ( mask != NULL )
          *mask = bitrange.mask64();
      }
      return 0;
  }
  return 1;
}
