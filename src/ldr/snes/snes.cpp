
#include "../idaldr.h"
#include "snes.hpp"
#include "addr.cpp"

//----------------------------------------------------------------------------
static bool rom_has_header(linput_t *li)
{
  int32 rom_size = qlsize(li);
  bool has_header = (rom_size % 0x2000) == 0x200;
  return has_header;
}

//----------------------------------------------------------------------------
static void map_io_seg(ea_t start, ea_t end, const char *const name)
{
  segment_t s;
  s.startEA = start;
  s.endEA   = end;
  s.type    = SEG_IMEM;
  s.sel     = allocate_selector(start >> 4);
  add_segm_ex(&s, name, NULL, ADDSEG_NOSREG);
}


//----------------------------------------------------------------------------
static void map_hwregs()
{
  map_io_seg(0x2100, 0x2144, "gpuapu");
  map_io_seg(0x2180, 0x2184, "wramrw");
  map_io_seg(0x4200, 0x4220, "misc");
  map_io_seg(0x4300, 0x4380, "dma");
}


//----------------------------------------------------------------------------
static void map_wram()
{
  segment_t s;
  s.startEA = 0x7e0000;
  s.endEA   = 0x800000;
  s.type    = SEG_IMEM;
  s.sel     = allocate_selector(s.startEA >> 4);
  add_segm_ex(&s, "wram", NULL, ADDSEG_NOSREG);
}

//----------------------------------------------------------------------------
// http://git.redump.net/cgit.cgi/mess/tree/src/mame/machine/snes.c
static sel_t map_mode_20(linput_t *li, uint32 rom_start_in_file, uint32 rom_size)
{
  // 32KB chunks count
  uint32 chunks = rom_size / 0x8000;

  // Banks 80 -> ff
  sel_t start_sel = 0;
  for ( uint32 mapped = 0, bank = 0x80; mapped < chunks; bank++, mapped++ )
  {
    ea_t start         = (bank << 16) + 0x8000;
    ea_t end           = start + 0x8000;
    uint32 off_in_file = rom_start_in_file + (mapped << 15);

    if ( !file2base(li, off_in_file, start, end, FILEREG_PATCHABLE) )
      loader_failure("Failed mapping 0x%x -> [0x%a, 0x%a)\n", off_in_file, start, end);

    char seg_name[0x10];
    sel_t selector = allocate_selector((start - 0x8000) >> 4);
    qsnprintf(seg_name, sizeof(seg_name), ".%02X", bank);
    if ( !add_segm(selector, start, end, seg_name, "BANK_ROM") )
      loader_failure("Failed adding .BANK segment\n");

    if ( bank == 0x80 )
      start_sel = selector;
  }

  return start_sel;
}

//----------------------------------------------------------------------------
static sel_t map_mode_21(linput_t *li, uint32 rom_start_in_file, uint32 rom_size)
{
  sel_t start_sel = 0;

  // Banks c0 -> ff
  uint32 chunks = rom_size / 0x10000;
  for (uint32 mapped = 0, bank = 0xc0; mapped < chunks; bank++, mapped++ )
  {
    ea_t start         = bank << 16;
    ea_t end           = start + 0x10000;
    uint32 off_in_file = rom_start_in_file + (mapped << 16);
    if ( !file2base(li, off_in_file, start, end, FILEREG_PATCHABLE) )
      loader_failure("Failed mapping 0x%x -> [0x%a, 0x%a)\n", off_in_file, start, end);

    char seg_name[0x10];
    sel_t selector = allocate_selector((start) >> 4);
    qsnprintf(seg_name, sizeof(seg_name), ".%02X", bank);
    if ( !add_segm(selector, start, end, seg_name, "BANK_ROM") )
      loader_failure("Failed adding .BANK segment\n");

    if ( bank == 0xC0 )
      start_sel = selector;
  }

  return start_sel;
}

//----------------------------------------------------------------------------
static bool read_info_at(rom_info_t *rinfo, linput_t *li, int pos)
{
  if ( qlseek(li, pos) != pos )
    return false;

  if ( qlread(li, rinfo, sizeof(rom_info_t)) != sizeof(rom_info_t) )
    return false;

  return true;
}

//----------------------------------------------------------------------------
static int score_info(const rom_info_t &sinfo, int32 rom_size)
{
  uint32 score = 0;
  if ( (uint16)(sinfo.checksum | sinfo.checksum_c) == 0xffff )
    score += 10;

  if ( sinfo.rom_type < 7 )
    score += 4;

  if ( sinfo.rom_size >= 0x8 && sinfo.rom_size <= 0xc )
    score += 4;

  for ( int i = 0; i < sizeof(sinfo.rom_name) && sinfo.rom_name[i] != '\0'; i++ )
    if ( isascii(sinfo.rom_name[i]) )
      score++;

  // Rom size not a multiple of 0x2000? Bad.
  if ( rom_size % 0x2000 )
    score /= 2;
  // Rom size is a multiple of 0x8000? Good.
  else if ( !(rom_size % 0x8000) )
    score += 6;

  return score;
}

//----------------------------------------------------------------------------
static bool score_rom(linput_t *li, uint32 *out_score_lo, uint32 *out_score_hi)
{
  // Snes ROMS must be 'scored', in order to
  // determine whether they are HI-, or LO-ROM.
  uint32 header_off = 0;
  if ( rom_has_header(li) )
    header_off = 0x200;

  int32 rom_size = qlsize(li) - header_off;

  rom_info_t rom_info;
  uint32 off = 0x7fc0 + header_off;
  if ( !read_info_at(&rom_info, li, off) )
    return false;
  *out_score_lo = score_info(rom_info, rom_size);

  off = 0xffc0 + header_off;
  if ( !read_info_at(&rom_info, li, off) )
    *out_score_hi = 0;  // 32k ROM.
  else
    *out_score_hi = score_info(rom_info, rom_size);

  return true;
}

//----------------------------------------------------------------------------
int idaapi accept_file(
        linput_t *li,
        char fileformatname[MAX_FILE_FORMAT_NAME],
        int n)
{
  if ( n > 0 )
    return 0;

  uint32 lo, hi;
  if ( !score_rom(li, &lo, &hi) )
    return 0;

  const int ACCEPTABLE_SCORE_TRESHOLD = 29;
  if ( lo >= ACCEPTABLE_SCORE_TRESHOLD
    || hi >= ACCEPTABLE_SCORE_TRESHOLD )
  {
    qstrncpy(fileformatname, "SNES ROM", MAX_FILE_FORMAT_NAME);
    return 1;
  }

  return 0;
}

//----------------------------------------------------------------------------
static void add_interrupt_vector(uint16 addr, const char *name)
{
  // Set 'addr' as dword
  ea_t real_ea = xlat(addr);
  doWord(real_ea, 2);

  ea_t orig_vector_addr = get_word(real_ea);
  ea_t vector_addr = xlat(orig_vector_addr);
  if ( orig_vector_addr != 0 && orig_vector_addr != 0xffff )
  {
    // Set 'vector_addr' name to be 'name'
    set_name(vector_addr, name, SN_NOCHECK);

    // Set 'real_ea' as offset
    op_offset(real_ea, OPND_MASK, REF_OFF16);

    // Push the vector_addr into the autoanalysis queue.
    // Do not make use of auto_make_proc(), because some
    // interrupt handler functions are ``overlaid''. Thus,
    // we'd break a procedure w/ inserting another
    // procedure right into the previous procedure's code.
    auto_make_code(vector_addr);

    set_cmt(real_ea, name, false);
  }
}

//----------------------------------------------------------------------------
void idaapi load_file(linput_t *li, ushort /*neflags*/, const char * /*ffn*/)
{
  // One should always set the processor type
  // as early as possible: IDA will draw some
  // informations from it; e.g., the size of segments.
  //
  // Should this instruction be placed after the calls to
  // map_mode_2x(), IDA would create 32-bits segments,
  // because, until the processor type is specified, IDA
  // assumes x86.
  set_processor_type("m65816", SETPROC_ALL|SETPROC_FATAL);

  uint32 lo, hi;
  if ( !score_rom(li, &lo, &hi) )
    INTERR(20017);

  uint32 info_offset;
  bool is_lo_rom = lo > hi;
  if ( is_lo_rom )
    info_offset = 0x7fc0;
  else
    info_offset = 0xffc0;

  rom_info_t rom_info;
  if ( !read_info_at(&rom_info, li, info_offset) )
    loader_failure("Failed loading the info at 0x%x\n", info_offset);

  // Determine whether ROM has a header
  int32 rom_start_in_file = 0;
  int32 rom_size = qlsize(li);
  if ( rom_size < 0 )
    loader_failure("Failed retrieving rom size.\n");

  bool has_header = rom_has_header(li);
  if ( has_header )
  {
    // msg("Rom has header\n");
    rom_size -= 0x200;
    rom_start_in_file = 0x200;
  }
  else
  {
    // msg("Rom has no header\n");
  }

  // Mapping mode
  rommode_t mode = is_lo_rom ? MODE_20 : MODE_21;

  // Store information for the cpu module
  netnode node;
  node.create("$ m65816");
  node.hashset("rommode_t", mode);
  node.hashset("device", "snes");

  addr_init(mode);

  sel_t start_cs;
  switch ( mode )
  {
    case MODE_20:
      start_cs = map_mode_20(li, rom_start_in_file, rom_size);
      break;
    case MODE_21:
      start_cs = map_mode_21(li, rom_start_in_file, rom_size);
      break;
    default:
      loader_failure("Unsupported rom mode: %d", mode);
  }
  inf.start_cs = start_cs;

  // Hardware registers
  map_hwregs();

  // WRAM
  map_wram();

  ea_t reset_vector_loc = xlat(0xfffc);
  uint16 start_pc = get_word(reset_vector_loc);
  ea_t start_address = xlat(start_pc);
  inf.startIP  = start_address & 0xffff;

  // http://en.wikibooks.org/wiki/Super_NES_Programming/SNES_memory_map
  // ------- Emulation-mode vectors
  add_interrupt_vector(0xfff4, "Emulation-mode COP");
  add_interrupt_vector(0xfff8, "Emulation-mode ABORT");
  add_interrupt_vector(0xfffa, "Emulation-mode NMI");
  add_interrupt_vector(0xfffc, "Emulation-mode RESET");
  add_interrupt_vector(0xfffe, "Emulation-mode IRQ");

  // ------- Native-mode vectors --------
  add_interrupt_vector(0xffe4, "Native-mode COP");
  add_interrupt_vector(0xffe6, "Native-mode BRK");
  add_interrupt_vector(0xffe8, "Native-mode ABORT");
  add_interrupt_vector(0xffea, "Native-mode NMI");
  add_interrupt_vector(0xffec, "Native-mode RESET");
  add_interrupt_vector(0xffee, "Native-mode IRQ");

  // Header info
  ea_t header = xlat(0xffc0);
  set_name(header, "snes_header");
  make_ascii_string(header, sizeof(rom_info.rom_name), ASCSTR_C);
}

//----------------------------------------------------------------------------
loader_t LDSC =
{
  IDP_INTERFACE_VERSION,
  LDRF_RELOAD,
  accept_file,
  load_file,
  NULL,
  NULL
};

