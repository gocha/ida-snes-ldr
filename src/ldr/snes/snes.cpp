
#include "../idaldr.h"
#include "addr.cpp"

//----------------------------------------------------------------------------
static void map_io_seg(ea_t start, ea_t end, const char *const name)
{
  segment_t s;
  s.startEA = start;
  s.endEA   = end;
  s.type    = SEG_IMEM;
  s.sel     = allocate_selector(start >> 4);
  if ( !add_segm_ex(&s, name, NULL, ADDSEG_NOSREG) )
    loader_failure("Failed adding %s segment\n", name);
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

  char seg_name[0x10];
  qsnprintf(seg_name, sizeof(seg_name), "wram");
  if ( !add_segm_ex(&s, seg_name, NULL, ADDSEG_NOSREG) )
    loader_failure("Failed adding %s segment\n", seg_name);
}

//----------------------------------------------------------------------------
static void map_lorom_sram(uint32 ram_size, bool preserve_rom_mirror)
{
  // Usually, the lower half of bank (0x8000 bytes) is SRAM, and the upper half is ROM mirror.
  // However, some cartridges maps the whole of bank (0x10000 bytes) to SRAM.
  // In that case, the upper half is probably mirrored as same as the lower half.

  // create ram banks 70-7d (and fe-ff)
  const uint32 bank_size = 0x8000;
  uint32 ram_chunks = (ram_size + bank_size - 1) / bank_size;
  for ( uint32 mapped = 0, bank = 0x70; mapped < ram_chunks; bank++, mapped++ )
  {
    if (bank == 0x7e)
      bank = 0xfe;

    segment_t s;
    s.startEA = (bank << 16);
    s.endEA   = s.startEA + bank_size - 1;
    s.type    = SEG_IMEM;
    s.sel     = allocate_selector(s.startEA >> 4);

    char seg_name[0x10];
    qsnprintf(seg_name, sizeof(seg_name), ".%02X", bank);
    if ( !add_segm_ex(&s, seg_name, "BANK_RAM", ADDSEG_NOSREG) )
      loader_failure("Failed adding %s segment\n", seg_name);
  }
}

//----------------------------------------------------------------------------
static void map_hirom_sram(uint32 ram_size)
{
  // create ram banks 20-3f
  const uint32 bank_size = 0x2000;
  uint32 ram_chunks = (ram_size + bank_size - 1) / bank_size;
  for ( uint32 mapped = 0, bank = 0x20; mapped < ram_chunks; bank++, mapped++ )
  {
    segment_t s;
    s.startEA = (bank << 16) + 0x6000;
    s.endEA   = s.startEA + bank_size - 1;
    s.type    = SEG_IMEM;
    s.sel     = allocate_selector(s.startEA >> 4);

    char seg_name[0x10];
    qsnprintf(seg_name, sizeof(seg_name), ".%02X", bank);
    if ( !add_segm_ex(&s, seg_name, "BANK_RAM", ADDSEG_NOSREG) )
      loader_failure("Failed adding %s segment\n", seg_name);
  }
}

//----------------------------------------------------------------------------
static void map_sa1rom_bwram(uint32 ram_size)
{
  // create ram banks 40-41
  const uint32 bank_size = 0x10000;
  uint32 ram_chunks = (ram_size + bank_size - 1) / bank_size;
  for ( uint32 mapped = 0, bank = 0x40; mapped < ram_chunks; bank++, mapped++ )
  {
    segment_t s;
    s.startEA = bank << 16;
    s.endEA   = s.startEA + bank_size - 1;
    s.type    = SEG_IMEM;
    s.sel     = allocate_selector(s.startEA >> 4);

    char seg_name[0x10];
    qsnprintf(seg_name, sizeof(seg_name), ".%02X", bank);
    if ( !add_segm_ex(&s, seg_name, "BANK_RAM", ADDSEG_NOSREG) )
      loader_failure("Failed adding %s segment\n", seg_name);
  }
}

//----------------------------------------------------------------------------
static void map_sa1rom_iram()
{
  segment_t s;
  s.startEA = 0x3000;
  s.endEA   = 0x37ff;
  s.type    = SEG_IMEM;
  s.sel     = allocate_selector(s.startEA >> 4);

  char seg_name[0x10];
  qsnprintf(seg_name, sizeof(seg_name), "iram");
  if ( !add_segm_ex(&s, seg_name, NULL, ADDSEG_NOSREG) )
    loader_failure("Failed adding %s segment\n", seg_name);
}

//----------------------------------------------------------------------------
static void map_sa1rom_hwregs()
{
  map_io_seg(0x2200, 0x23ff, "sa1");
}

//----------------------------------------------------------------------------
static sel_t map_lorom_offset(linput_t *li, uint32 rom_start_in_file, uint32 rom_size, uint8 start_bank, uint32 offset)
{
  // 32KB chunks count
  uint32 chunks = rom_size / 0x8000;

  // map rom to banks
  sel_t start_sel = 0;
  for ( uint32 mapped = 0, bank = start_bank; mapped < chunks; bank++, mapped++ )
  {
    ea_t start         = (bank << 16) + 0x8000;
    ea_t end           = start + 0x8000;
    uint32 off_in_file = rom_start_in_file + offset + (mapped << 15);

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
static sel_t map_lorom(linput_t *li, uint32 rom_start_in_file, uint32 rom_size)
{
  // map rom to banks 80-ff
  return map_lorom_offset(li, rom_start_in_file, rom_size, 0x80, 0);
}

//----------------------------------------------------------------------------
static sel_t map_hirom_offset(linput_t *li, uint32 rom_start_in_file, uint32 rom_size, uint8 start_bank, uint32 offset)
{
  sel_t start_sel = 0;

  // map rom to banks
  uint32 chunks = rom_size / 0x10000;
  for (uint32 mapped = 0, bank = start_bank; mapped < chunks; bank++, mapped++ )
  {
    ea_t start         = bank << 16;
    ea_t end           = start + 0x10000;
    uint32 off_in_file = rom_start_in_file + offset + (mapped << 16);
    if ( !file2base(li, off_in_file, start, end, FILEREG_PATCHABLE) )
      loader_failure("Failed mapping 0x%x -> [0x%a, 0x%a)\n", off_in_file, start, end);

    char seg_name[0x10];
    sel_t selector = allocate_selector((start) >> 4);
    qsnprintf(seg_name, sizeof(seg_name), ".%02X", bank);
    if ( !add_segm(selector, start, end, seg_name, "BANK_ROM") )
      loader_failure("Failed adding .BANK segment\n");

    if ( bank == 0xc0 )
      start_sel = selector;
  }

  return start_sel;
}

//----------------------------------------------------------------------------
static sel_t map_hirom(linput_t *li, uint32 rom_start_in_file, uint32 rom_size)
{
  // map rom to banks c0-ff
  return map_hirom_offset(li, rom_start_in_file, rom_size, 0xc0, 0);
}

//----------------------------------------------------------------------------
static sel_t map_sa1rom(linput_t *li, uint32 rom_start_in_file, uint32 rom_size)
{
  // map rom to banks 00-3f
  sel_t start_sel = map_lorom_offset(li, rom_start_in_file, qmin(rom_size, 0x200000), 0x00, 0);

  // map > 2MB rom to banks e0-ff
  if ( rom_size > 0x200000 )
    map_hirom_offset(li, rom_start_in_file, rom_size - 0x200000, 0xe0, 0x200000);

  return start_sel;
}

//----------------------------------------------------------------------------
static sel_t map_lorom_cartridge(linput_t *li, uint32 rom_start_in_file, uint32 rom_size, uint32 ram_size)
{
  sel_t start_sel = map_lorom(li, rom_start_in_file, rom_size);

  bool preserve_rom_mirror = (rom_size > 0x200000) || (ram_size > 32 * 1024);
  map_lorom_sram(ram_size, preserve_rom_mirror);

  return start_sel;
}

//----------------------------------------------------------------------------
static sel_t map_hirom_cartridge(linput_t *li, uint32 rom_start_in_file, uint32 rom_size, uint32 ram_size)
{
  sel_t start_sel = map_hirom(li, rom_start_in_file, rom_size);

  map_hirom_sram(ram_size);

  return start_sel;
}

//----------------------------------------------------------------------------
static sel_t map_sa1rom_cartridge(linput_t *li, uint32 rom_start_in_file, uint32 rom_size, uint32 ram_size)
{
  sel_t start_sel = map_sa1rom(li, rom_start_in_file, rom_size);

  map_sa1rom_bwram(ram_size);
  map_sa1rom_iram();
  map_sa1rom_hwregs();

  return start_sel;
}

//----------------------------------------------------------------------------
int idaapi accept_file(
        linput_t *li,
        char fileformatname[MAX_FILE_FORMAT_NAME],
        int n)
{
  if ( n > 0 )
    return 0;

  SuperFamicomCartridge cartridge(li);
  unsigned score = cartridge.score_header(li, cartridge.header_offset);

  const int ACCEPTABLE_SCORE_TRESHOLD = 8;
  if (score >= ACCEPTABLE_SCORE_TRESHOLD &&
      cartridge.type != SuperFamicomCartridge::TypeUnknown)
  {
    qstrncpy(fileformatname, "SNES ROM", MAX_FILE_FORMAT_NAME);
    return 1;
  }

  return 0;
}

//----------------------------------------------------------------------------
static void add_interrupt_vector(uint16 addr, const char *name, bool make_code)
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
    if ( make_code )
    {
      auto_make_code(vector_addr);
    }

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

  SuperFamicomCartridge cartridge(li);
  //cartridge.print();

  // Determine whether ROM has a header
  int32 start = cartridge.has_copier_header ? 512 : 0;

  // Store information for the cpu module
  netnode node;
  node.create("$ m65816");
  node.hashset("device", "snes");
  cartridge.write_hash(node);

  addr_init(cartridge);

  sel_t start_cs;
  switch ( cartridge.mapper )
  {
    case SuperFamicomCartridge::LoROM:
      start_cs = map_lorom_cartridge(li, start, cartridge.rom_size, cartridge.ram_size);
      break;
    case SuperFamicomCartridge::HiROM:
      start_cs = map_hirom_cartridge(li, start, cartridge.rom_size, cartridge.ram_size);
      break;
    case SuperFamicomCartridge::SA1ROM:
      start_cs = map_sa1rom_cartridge(li, start, cartridge.rom_size, cartridge.ram_size);
      break;
    default:
      loader_failure("Unsupported mapper: %s", cartridge.mapper_string());
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
  add_interrupt_vector(0xfffc, "Emulation-mode RESET", true);

  // http://en.wikibooks.org/wiki/Super_NES_Programming/SNES_memory_map
  // ------- Emulation-mode vectors
  add_interrupt_vector(0xfff4, "Emulation-mode COP", false);
  add_interrupt_vector(0xfff8, "Emulation-mode ABORT", false);
  add_interrupt_vector(0xfffa, "Emulation-mode NMI", false);
  add_interrupt_vector(0xfffe, "Emulation-mode IRQ", false);

  // ------- Native-mode vectors --------
  add_interrupt_vector(0xffe4, "Native-mode COP", false);
  add_interrupt_vector(0xffe6, "Native-mode BRK", false);
  add_interrupt_vector(0xffe8, "Native-mode ABORT", false);
  add_interrupt_vector(0xffea, "Native-mode NMI", true);
  add_interrupt_vector(0xffec, "Native-mode RESET", false);
  add_interrupt_vector(0xffee, "Native-mode IRQ", false);

  // Header info
  ea_t header = xlat(0xffc0);
  set_name(header, "snes_header");
  make_ascii_string(header, 21, ASCSTR_C);
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

