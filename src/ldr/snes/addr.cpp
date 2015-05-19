
// This file is included from the loader module and the processor module

#include "super-famicom.hpp"

static SuperFamicomCartridge g_cartridge;

//----------------------------------------------------------------------------
// rom name=program.rom size=hex(rom_size)
// ram name=save.ram size=hex(ram_size)
// map id=rom address=00-7f,80-ff:8000-ffff mask=0x8000
// map id=ram address=70-7f,f0-ff:[0000-7fff|0000-ffff]
static ea_t xlat_lorom(ea_t address)
{
  uint16 addr = address & 0xffff;
  uint8 bank = (address >> 16) & 0xff;

  // WRAM
  if ( bank >= 0x7e && bank <= 0x7f )
    return address;

  // SRAM
  if ( g_cartridge.ram_size != 0 )
  {
    bool preserve_rom_mirror = (g_cartridge.rom_size > 0x200000) || (g_cartridge.ram_size > 32 * 1024);

    if ( ( bank >= 0x70 && bank <= 0x7d ) || ( bank >= 0xf0 && bank <= 0xff ) )
    {
      if ( addr <= 0x7fff || !preserve_rom_mirror )
      {
        uint32 ram_mask = g_cartridge.ram_size - 1;
        uint32 ram_offset = (((bank & 0xf) << 15) + (addr & 0x7fff)) & ram_mask;

        ea_t ea = ((0x70 + (ram_offset >> 15)) << 16) + (ram_offset & 0x7fff);
        if ( bank >= 0xfe )
          ea += 0x800000;
        return ea;
      }
    }
  }

  // mirror 00-7d => 80-fd (excluding SRAM)
  if ( bank <= 0x7d )
    return xlat_lorom(((0x80 + bank) << 16) + addr);

  if ( addr <= 0x7fff )
  {
    if ( addr <= 0x1fff ) // Low RAM
      return 0x7e0000 + addr;
    else if ( addr >= 0x2100 && addr <= 0x213f ) // PPU registers
      return addr;
    else if ( addr >= 0x2140 && addr <= 0x2183 ) // CPU registers
      return addr;
    else if ( addr >= 0x4016 && addr <= 0x4017 ) // CPU registers
      return addr;
    else if ( addr >= 0x4200 && addr <= 0x421f ) // CPU registers
      return addr;
    else if ( addr >= 0x4300 && addr <= 0x437f ) // CPU registers
      return addr;
  }
  else
  {
    // ROM
    return address;
  }

  return address;
}

//----------------------------------------------------------------------------
// rom name=program.rom size=hex(rom_size)
// ram name=save.ram size=hex(ram_size)
// map id=rom address=00-3f,80-bf:8000-ffff
// map id=rom address=40-7f,c0-ff:0000-ffff
// map id=ram address=10-3f,90-bf:6000-7fff mask=0xe000
static ea_t xlat_hirom(ea_t address)
{
  uint16 addr = address & 0xffff;
  uint8 bank = (address >> 16) & 0xff;

  // WRAM
  if ( bank >= 0x7e && bank <= 0x7f )
    return address;

  // SRAM
  if ( g_cartridge.ram_size != 0 )
  {
    if ( ( bank >= 0x10 && bank <= 0x3f ) || ( bank >= 0x90 && bank <= 0xbf ) )
    {
      if ( addr >= 0x6000 && addr <= 0x7fff )
      {
        // Typically, HiROM SRAM starts from $20:0000, but there are exceptions.
        // Example: Donkey Kong Country 2 (reads $B0:6000 for 2 kilobytes SRAM)
        uint32 ram_mask = g_cartridge.ram_size - 1;
        uint32 ram_offset = (((bank & 0x1f) << 13) + (addr - 0x6000)) & ram_mask;
        return ((0x20 + (ram_offset >> 13)) << 16) + (0x6000 + (ram_offset & 0x1fff));
      }
    }
  }

  // mirror 00-7d => 80-fd (excluding SRAM)
  if ( bank <= 0x7d )
    return xlat_hirom(((0x80 + bank) << 16) + addr);

  if ( bank <= 0xbf && addr <= 0x7fff )
  {
    if ( addr <= 0x1fff ) // Low RAM
      return 0x7e0000 + addr;
    else if ( addr >= 0x2100 && addr <= 0x213f ) // PPU registers
      return addr;
    else if ( addr >= 0x2140 && addr <= 0x2183 ) // CPU registers
      return addr;
    else if ( addr >= 0x4016 && addr <= 0x4017 ) // CPU registers
      return addr;
    else if ( addr >= 0x4200 && addr <= 0x421f ) // CPU registers
      return addr;
    else if ( addr >= 0x4300 && addr <= 0x437f ) // CPU registers
      return addr;
  }
  else
  {
    // ROM
    return ((0xc0 + (bank & 0x3f)) << 16) + addr;
  }

  return address;
}

//----------------------------------------------------------------------------
// sa1
//   rom name=program.rom size=hex(rom_size)
//   ram id=bitmap name=save.ram size=hex(ram_size)
//   ram id=internal size=0x800
//   map id=io address=00-3f,80-bf:2200-23ff
//   map id=rom address=00-3f,80-bf:8000-ffff
//   map id=rom address=c0-ff:0000-ffff
//   map id=bwram address=00-3f,80-bf:6000-7fff
//   map id=bwram address=40-4f:0000-ffff
//   map id=iram address=00-3f,80-bf:3000-37ff
static ea_t xlat_sa1rom(ea_t address)
{
  uint16 addr = address & 0xffff;
  uint8 bank = (address >> 16) & 0xff;

  // mirror 80-bf => 00-3f
  if ( bank >= 0x80 && bank <= 0xbf )
    return xlat_hirom(((bank - 0x80) << 16) + addr);

  // WRAM
  if ( bank >= 0x7e && bank <= 0x7f )
    return address;

  // SA1 BWRAM (SRAM)
  if ( g_cartridge.ram_size != 0 )
  {
    if ( bank <= 0x3f )
    {
      if ( addr >= 0x6000 && addr <= 0x7fff )
      {
        // 8 kilobytes RAM (shared with 40:0000-1fff)
        uint32 ram_offset = (addr & 0x7fff) - 0x6000;
        return (0x40 << 16) + ram_offset;
      }
    }
    else if ( bank >= 0x40 && bank <= 0x4f )
    {
      // 128 kB address space, redirects to banks 40-41
      return ((bank & ~0xe) << 16) + addr;
    }
  }

  if ( bank >= 0xc0 )
  {
    // ROM (HiROM layout)
    return address;
  }
  else if ( bank <= 0x3f )
  {
    if ( addr <= 0x7fff )
    {
      if ( addr <= 0x1fff ) // Low RAM
        return 0x7e0000 + addr;
      else if ( addr >= 0x2100 && addr <= 0x213f ) // PPU registers
        return addr;
      else if ( addr >= 0x2140 && addr <= 0x2183 ) // CPU registers
        return addr;
      else if ( addr >= 0x4016 && addr <= 0x4017 ) // CPU registers
        return addr;
      else if ( addr >= 0x4200 && addr <= 0x421f ) // CPU registers
        return addr;
      else if ( addr >= 0x4300 && addr <= 0x437f ) // CPU registers
        return addr;
      else if ( addr >= 0x2200 && addr <= 0x23ff ) // SA1 registers
        return addr;
      else if ( addr >= 0x3000 && addr <= 0x37ff ) // SA1 IWRAM
        return addr;
    }
    else
    {
      // ROM (LoROM layout)
      return address;
    }
  }

  // TODO: SA1 Missing Memory Map
  // 00-3f|80-bf:0000-07ff IWRAM (SA1 side)
  // 60-6f:0000-ffff       BWRAM Bitmap (SA1 side)

  return address;
}

//----------------------------------------------------------------------------
static bool addr_init(const SuperFamicomCartridge & cartridge)
{
  g_cartridge = cartridge;

  switch ( g_cartridge.mapper )
  {
    case SuperFamicomCartridge::LoROM:
    case SuperFamicomCartridge::HiROM:
    case SuperFamicomCartridge::SA1ROM:
      return true;
    default:
      return false;
  }
}

//----------------------------------------------------------------------------
ea_t xlat(ea_t address)
{
  switch ( g_cartridge.mapper )
  {
    case SuperFamicomCartridge::LoROM:
      return xlat_lorom(address);
    case SuperFamicomCartridge::HiROM:
      return xlat_hirom(address);
    case SuperFamicomCartridge::SA1ROM:
      return xlat_sa1rom(address);
    default:
      return address;
  }
}
