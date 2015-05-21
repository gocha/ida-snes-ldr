
#include "../idaldr.h"
#include "snes_spc.hpp"

//----------------------------------------------------------------------------
static bool map_psram(linput_t *li, uint32 psram_start_in_file)
{
  bool succeeded = false;

  segment_t s;
  s.startEA = 0x00000;
  s.endEA   = 0x10000;
  s.sel     = allocate_selector(s.startEA >> 4);

  if ( !file2base(li, psram_start_in_file, s.startEA, s.endEA, FILEREG_PATCHABLE) )
    loader_failure("Failed mapping 0x%x -> [0x%a, 0x%a)\n", psram_start_in_file, s.startEA, s.endEA);

  succeeded = add_segm(s.sel, s.startEA, s.endEA, "RAM", NULL);
  if ( succeeded )
    succeeded = true;
  else
    loader_failure("Failed adding RAM segment\n", psram_start_in_file, s.startEA, s.endEA);

  return succeeded;
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

  // Store information for the cpu module
  netnode node;
  node.create("$ spc700");
  node.hashset("device", "spc700");

  // PSRAM
  inf.start_cs = map_psram(li, 0x100);

  info("You have just loaded a SNES SPC-700 sound file.\n\n"
       "Please move to what you think is an entry point and press 'C' to start the auto analysis.");
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

