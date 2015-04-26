
#ifndef __SNES_HPP__
#define __SNES_HPP__

#pragma pack(push, 1)  // 1-byte alignments, to read rom.

struct spc_file_t
{
  char  signature [35];
  uint8 has_id666;
  uint8 version;
  uint8 pcl, pch;
  uint8 a;
  uint8 x;
  uint8 y;
  uint8 psw;
  uint8 sp;
  char  text [212];
  uint8 ram [0x10000];
  uint8 dsp [128];
  uint8 unused [0x40];
  uint8 ipl_rom [0x40];
};

#pragma pack(pop)

#endif //__SNES_HPP__
