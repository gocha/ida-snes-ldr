
// common file to handle odd memory mappings

//lint -esym(843,default_mapping_size) could be declared as const
struct mapping_t : public area_t
{
  ea_t target;
};

static areacb_t mapping;
static asize_t default_mapping_size;

//--------------------------------------------------------------------------
// call from ph.init
void init_mapping(asize_t default_size, const char *idpname, const char *mname=NULL)
{
  if ( mname == NULL ) mname = "$ memory mappings";
  default_mapping_size = default_size;
  if ( !mapping.link(mname, mname, sizeof(mapping_t)) )
  {
    mapping.create(mname, mname, sizeof(mapping_t));
    static const char *const mapping_exists =
      "IDA for %s supports memory mapping.\n"
      "If you see references to the memory addresses not\n"
      "belonging to the program, you may want to use it.\n"
      "The memory mapping is available at\n"
      "Options, General, Analysis, Processor specific options\n";
    info(mapping_exists, idpname);
  }
}

//--------------------------------------------------------------------------
// call from ph.savebase
inline void save_mapping(void)
{
  mapping.save();
}

//--------------------------------------------------------------------------
// call from ph.closebase
inline void term_mapping(void)
{
  mapping.terminate();
}

//--------------------------------------------------------------------------
inline void move_mapping(ea_t from, segment_t *s)
{
  mapping.move_areas(from, s->startEA, s->size());
}

//--------------------------------------------------------------------------
ea_t use_mapping(ea_t ea)
{
  mapping_t *m = (mapping_t *)mapping.get_area(ea);
  if ( m != NULL )
    return m->target + (ea-m->startEA);
  return ea;
}

//--------------------------------------------------------------------------
static void print_mappings(char *buf, int bufsize)
{
  char *ptr = buf;
  int n = mapping.get_area_qty();
  for ( int i=0; i < n; i++ )
  {
    mapping_t *m = (mapping_t *)mapping.getn_area(i);
    int len = qsnprintf(ptr, bufsize, "  %08a..%08a -> %08a..%08a\n",
        m->startEA, m->endEA, m->target, m->target + m->size());
    ptr += len;
    bufsize -= len;
  }
  qsnprintf(ptr, bufsize, "\n\n");
}

// ---------------------------------------------------------------------------
static bool idaapi apply_mapping(ea_t startEA, ea_t endEA, ea_t targetEA)
{
  area_t *e;
  mapping_t m;
  m.startEA = startEA;
  m.endEA   = endEA;
  m.target  = targetEA;
  while ( (e=mapping.get_area(m.endEA)) != NULL && e->startEA != m.endEA )
    m.endEA = e->startEA;
  if ( !mapping.create_area(&m) )
    return false;
  noUsed(inf.minEA, inf.maxEA);
  return true;
}

//--------------------------------------------------------------------------
static void idaapi add_mapping(TView *fields[], int)
{
  static const char form[] =
    "Add mapping\n"
    "\n"
    "<~F~rom:N:16:16::> (nonexisting address)\n"
    "<~T~o  :N:16:16::> (existing address)\n"
    "<~S~ize:N:16:16::>\n"
    "\n"
    "\n";
  static ea_t source;
  static ea_t target;
  asize_t size = default_mapping_size;
  while ( AskUsingForm_c(form, &source, &target, &size) )
  {
    if ( apply_mapping(source, source + size, target) )
    {
      close_form(fields, true);
      break;
    }
    else
    {
      warning("Can't add mapping!");
    }
  }
}

//--------------------------------------------------------------------------
static const char *const maphdr[] = { "Start", "End", "Target" };
static const int widths[] = { 8, 8, 8 };
CASSERT(qnumber(widths) == qnumber(maphdr));

static void idaapi getl_mapping(areacb_t *,uint32 n,char * const *arrptr)
{
  if ( n == 0 )
  {
    for ( int i=0; i < qnumber(maphdr); i++ )
      qstrncpy(arrptr[i], maphdr[i], MAXSTR);
    return;
  }
  mapping_t *m = (mapping_t *)mapping.getn_area(n-1);
  qsnprintf(arrptr[0], MAXSTR, "%08a", m->startEA);
  qsnprintf(arrptr[1], MAXSTR, "%08a", m->endEA);
  qsnprintf(arrptr[2], MAXSTR, "%08a", m->target);
}

static void idaapi remove_mapping(TView *fields[], int)
{
  mapping_t *m = (mapping_t *)mapping.choose_area2(true,
                qnumber(widths), widths,
                getl_mapping,
                "Please select mapping to remove",
                -1);
  if ( m != NULL )
  {
    mapping.del_area(m->startEA);
    noUsed(inf.minEA, inf.maxEA);
    close_form(fields, true);
  }
}

