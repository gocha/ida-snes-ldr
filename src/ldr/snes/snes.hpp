
#ifndef __SNES_HPP__
#define __SNES_HPP__

#pragma pack(push, 1)  // 1-byte alignments, to read rom.

struct rom_info_t
{
  uint8 rom_name[21];
  uint8 rom_speed;
  uint8 rom_type;
  uint8 rom_size;
  uint8 sram_size;
  uint8 country;
  uint8 license;
  uint8 version;
  uint16 checksum_c;
  uint16 checksum;
};

#pragma pack(pop)

#endif //__SNES_HPP__
