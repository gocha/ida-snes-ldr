
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
static void set_dsp_names()
{
  set_name(BASE_DSP + 0x00, "V0VOLL");
  set_name(BASE_DSP + 0x01, "V0VOLR");
  set_name(BASE_DSP + 0x02, "V0PL");
  set_name(BASE_DSP + 0x03, "V0PH");
  set_name(BASE_DSP + 0x04, "V0SRCN");
  set_name(BASE_DSP + 0x05, "V0ADSR1");
  set_name(BASE_DSP + 0x06, "V0ADSR2");
  set_name(BASE_DSP + 0x07, "V0GAIN");
  set_name(BASE_DSP + 0x08, "V0ENVX");
  set_name(BASE_DSP + 0x09, "V0OUTX");

  set_name(BASE_DSP + 0x10, "V1VOLL");
  set_name(BASE_DSP + 0x11, "V1VOLR");
  set_name(BASE_DSP + 0x12, "V1PL");
  set_name(BASE_DSP + 0x13, "V1PH");
  set_name(BASE_DSP + 0x14, "V1SRCN");
  set_name(BASE_DSP + 0x15, "V1ADSR1");
  set_name(BASE_DSP + 0x16, "V1ADSR2");
  set_name(BASE_DSP + 0x17, "V1GAIN");
  set_name(BASE_DSP + 0x18, "V1ENVX");
  set_name(BASE_DSP + 0x19, "V1OUTX");

  set_name(BASE_DSP + 0x20, "V2VOLL");
  set_name(BASE_DSP + 0x21, "V2VOLR");
  set_name(BASE_DSP + 0x22, "V2PL");
  set_name(BASE_DSP + 0x23, "V2PH");
  set_name(BASE_DSP + 0x24, "V2SRCN");
  set_name(BASE_DSP + 0x25, "V2ADSR1");
  set_name(BASE_DSP + 0x26, "V2ADSR2");
  set_name(BASE_DSP + 0x27, "V2GAIN");
  set_name(BASE_DSP + 0x28, "V2ENVX");
  set_name(BASE_DSP + 0x29, "V2OUTX");

  set_name(BASE_DSP + 0x30, "V3VOLL");
  set_name(BASE_DSP + 0x31, "V3VOLR");
  set_name(BASE_DSP + 0x32, "V3PL");
  set_name(BASE_DSP + 0x33, "V3PH");
  set_name(BASE_DSP + 0x34, "V3SRCN");
  set_name(BASE_DSP + 0x35, "V3ADSR1");
  set_name(BASE_DSP + 0x36, "V3ADSR2");
  set_name(BASE_DSP + 0x37, "V3GAIN");
  set_name(BASE_DSP + 0x38, "V3ENVX");
  set_name(BASE_DSP + 0x39, "V3OUTX");

  set_name(BASE_DSP + 0x40, "V4VOLL");
  set_name(BASE_DSP + 0x41, "V4VOLR");
  set_name(BASE_DSP + 0x42, "V4PL");
  set_name(BASE_DSP + 0x43, "V4PH");
  set_name(BASE_DSP + 0x44, "V4SRCN");
  set_name(BASE_DSP + 0x45, "V4ADSR1");
  set_name(BASE_DSP + 0x46, "V4ADSR2");
  set_name(BASE_DSP + 0x47, "V4GAIN");
  set_name(BASE_DSP + 0x48, "V4ENVX");
  set_name(BASE_DSP + 0x49, "V4OUTX");

  set_name(BASE_DSP + 0x50, "V5VOLL");
  set_name(BASE_DSP + 0x51, "V5VOLR");
  set_name(BASE_DSP + 0x52, "V5PL");
  set_name(BASE_DSP + 0x53, "V5PH");
  set_name(BASE_DSP + 0x54, "V5SRCN");
  set_name(BASE_DSP + 0x55, "V5ADSR1");
  set_name(BASE_DSP + 0x56, "V5ADSR2");
  set_name(BASE_DSP + 0x57, "V5GAIN");
  set_name(BASE_DSP + 0x58, "V5ENVX");
  set_name(BASE_DSP + 0x59, "V5OUTX");

  set_name(BASE_DSP + 0x60, "V6VOLL");
  set_name(BASE_DSP + 0x61, "V6VOLR");
  set_name(BASE_DSP + 0x62, "V6PL");
  set_name(BASE_DSP + 0x63, "V6PH");
  set_name(BASE_DSP + 0x64, "V6SRCN");
  set_name(BASE_DSP + 0x65, "V6ADSR1");
  set_name(BASE_DSP + 0x66, "V6ADSR2");
  set_name(BASE_DSP + 0x67, "V6GAIN");
  set_name(BASE_DSP + 0x68, "V6ENVX");
  set_name(BASE_DSP + 0x69, "V6OUTX");

  set_name(BASE_DSP + 0x70, "V7VOLL");
  set_name(BASE_DSP + 0x71, "V7VOLR");
  set_name(BASE_DSP + 0x72, "V7PL");
  set_name(BASE_DSP + 0x73, "V7PH");
  set_name(BASE_DSP + 0x74, "V7SRCN");
  set_name(BASE_DSP + 0x75, "V7ADSR1");
  set_name(BASE_DSP + 0x76, "V7ADSR2");
  set_name(BASE_DSP + 0x77, "V7GAIN");
  set_name(BASE_DSP + 0x78, "V7ENVX");
  set_name(BASE_DSP + 0x79, "V7OUTX");

  set_name(BASE_DSP + 0x0c, "MVOLL");
  set_name(BASE_DSP + 0x1c, "MVOLR");
  set_name(BASE_DSP + 0x2c, "EVOLL");
  set_name(BASE_DSP + 0x3c, "EVOLR");
  set_name(BASE_DSP + 0x4c, "KON");
  set_name(BASE_DSP + 0x5c, "KOF");
  set_name(BASE_DSP + 0x6c, "FLG");
  set_name(BASE_DSP + 0x7c, "ENDX");
  set_name(BASE_DSP + 0x0d, "EFB");
  set_name(BASE_DSP + 0x2d, "PMON");
  set_name(BASE_DSP + 0x3d, "NON");
  set_name(BASE_DSP + 0x4d, "EON");
  set_name(BASE_DSP + 0x5d, "DIR");
  set_name(BASE_DSP + 0x6d, "ESA");
  set_name(BASE_DSP + 0x7d, "EDL");
  set_name(BASE_DSP + 0x0f, "FIR0");
  set_name(BASE_DSP + 0x1f, "FIR1");
  set_name(BASE_DSP + 0x2f, "FIR2");
  set_name(BASE_DSP + 0x3f, "FIR3");
  set_name(BASE_DSP + 0x4f, "FIR4");
  set_name(BASE_DSP + 0x5f, "FIR5");
  set_name(BASE_DSP + 0x6f, "FIR6");
  set_name(BASE_DSP + 0x7f, "FIR7");
}

//----------------------------------------------------------------------------
static void set_spc700_names()
{
  set_name(0xf0, "TEST");
  set_name(0xf1, "CONTROL");
  set_name(0xf2, "DSPADDR");
  set_name(0xf3, "DSPDATA");
  set_name(0xf4, "CPU0");
  set_name(0xf5, "CPU1");
  set_name(0xf6, "CPU2");
  set_name(0xf7, "CPU3");
  set_name(0xfa, "T0TARGET");
  set_name(0xfb, "T1TARGET");
  set_name(0xfc, "T2TARGET");
  set_name(0xfd, "T0OUT");
  set_name(0xfe, "T1OUT");
  set_name(0xff, "T2OUT");
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

  set_dsp_names();
  set_spc700_names();
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

