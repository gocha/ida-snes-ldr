
#include "../idaldr.h"
#include "snes_spc.hpp"

#define BASE_DSP    0x10000

//----------------------------------------------------------------------------
static sel_t map_dsp()
{
  segment_t s;
  s.startEA = BASE_DSP + 0x00;
  s.endEA   = BASE_DSP + 0x80;
  s.type    = SEG_IMEM;
  s.sel     = allocate_selector(s.startEA >> 4);
  add_segm_ex(&s, "DSP", NULL, ADDSEG_NOSREG);
  return s.sel;
}


//----------------------------------------------------------------------------
static sel_t map_psram(linput_t *li, uint32 psram_start_in_file)
{
  segment_t s;
  s.startEA = 0x00000;
  s.endEA   = 0x10000;
  s.type    = SEG_IMEM;
  s.sel     = allocate_selector(s.startEA >> 4);

  if ( !file2base(li, psram_start_in_file, s.startEA, s.endEA, FILEREG_PATCHABLE) )
    loader_failure("Failed mapping 0x%x -> [0x%a, 0x%a)\n", psram_start_in_file, s.startEA, s.endEA);

  add_segm_ex(&s, "PSRAM", NULL, ADDSEG_NOSREG);
  return s.sel;
}

//----------------------------------------------------------------------------
int idaapi accept_file(
        linput_t *li,
        char fileformatname[MAX_FILE_FORMAT_NAME],
        int n)
{
  if ( n > 0 )
    return 0;

  int32 spc_file_size = qlsize(li);
  if ( spc_file_size < 0x10200 )
    return 0;

  spc_file_t spc_info;
  if ( qlseek(li, 0) != 0 )
    return 0;
  if ( qlread(li, &spc_info, sizeof(spc_file_t)) != sizeof(spc_file_t) )
    return 0;

  if (memcmp(spc_info.signature, "SNES-SPC700 Sound File Data", 27) != 0
    || spc_info.signature[0x21] != 0x1a || spc_info.signature[0x22] != 0x1a)
    return 0;

  qstrncpy(fileformatname, "SNES-SPC700 Sound File Data", MAX_FILE_FORMAT_NAME);
  return 1;
}

//----------------------------------------------------------------------------
void idaapi load_file(linput_t *li, ushort /*neflags*/, const char * /*ffn*/)
{
  // One should always set the processor type
  // as early as possible: IDA will draw some
  // informations from it; e.g., the size of segments.
  set_processor_type("spc700", SETPROC_ALL|SETPROC_FATAL);

  // DSP
  map_dsp();

  // PSRAM
  inf.start_cs = map_psram(li, 0x100);

  set_name(BASE_DSP + 0x5d, "DSP_DIR");

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

