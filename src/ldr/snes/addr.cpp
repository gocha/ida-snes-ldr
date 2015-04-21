
// This file is included from the loader module and the processor module

//lint -esym(843,g_mode) Variable could be declared as const
//lint -esym(843,xlator) Variable could be declared as const

enum rommode_t
{
  MODE_UNKNOWN = 0,
  MODE_20 = 20,
  MODE_21 = 21,
  MODE_22 = 22,
  MODE_25 = 25
};

static rommode_t g_mode = MODE_UNKNOWN;

typedef ea_t (*xlator_t)(ea_t address);

static xlator_t xlator = NULL;

//----------------------------------------------------------------------------
// http://git.redump.net/cgit.cgi/mess/tree/src/mame/machine/snes.c
//
// MODE_20
//      banks 0x00      0x20          0x40      0x60       0x70     0x7e  0x80   0xc0   0xff
// address               |             |         |          |        |     |      |      |
// 0xffff  ------------------------------------------------------------------------------|
//              ROM      |     ROM /   |   ROM   |  ROM     |  ROM / |     | 0x00 | 0x40 |
//                       |     DSP     |         |          |  SRAM? |     |  to  |  to  |
// 0x8000  ----------------------------------------------------------|     | 0x3f | 0x7f |
//                    Reserv           |         |          |        |  W  |      |      |
//                                     |         |          |   S    |  R  |  m   |  m   |
// 0x6000  ----------------------------|         |  DSP /   |   R    |  A  |  i   |  i   |
//                      I/O            |  Reserv |          |   A    |  M  |  r   |  r   |
// 0x2000  ----------------------------|         |  Reserv  |   M    |     |  r   |  r   |
//              Low RAM (from 0x7e)    |         |          |        |     |  o   |  o   |
//                                     |         |          |        |     |  r   |  r   |
// 0x0000  -------------------------------------------------------------------------------
static ea_t xlat_20(ea_t address)
{
  return address & 0x7fffff;
}

//----------------------------------------------------------------------------
// MODE_21
//      banks 0x00      0x10          0x30      0x40   0x7e  0x80      0xc0   0xff
// address               |             |         |      |     |         |      |
// 0xffff  --------------------------------------------------------------------|
//                          mirror               | 0xc0 |     | mirror  |      |
//                       upper half ROM          |  to  |     | up half |      |
//                      from 0xc0 to 0xff        | 0xff |     |   ROM   |      |
// 0x8000  --------------------------------------|      |     |---------|      |
//              DSP /    |    Reserv   |  SRAM   |      |  W  |         |      |
//             Reserv    |             |         |  m   |  R  |   0x00  |  R   |
// 0x6000  --------------------------------------|  i   |  A  |    to   |  O   |
//                           I/O                 |  r   |  M  |   0x3f  |  M   |
// 0x2000  --------------------------------------|  r   |     |  mirror |      |
//                    Low RAM (from 0x7e)        |  o   |     |         |      |
//                                               |  r   |     |         |      |
// 0x0000  ---------------------------------------------------------------------
static ea_t xlat_21(ea_t address)
{
  ea_t ret = address;
  uint16 o = address & 0xffff;
  uint8 k  = (address >> 16) & 0xff;
  uint8 uk = (k >> 4) & 0xf;
  switch ( uk )
  {
    case 0x0:
    case 0x1:
    case 0x2:
    case 0x3:
      if ( o < 0x2000 )
        ret = 0x7E0000 + o;
      else if ( o >= 0x8000 )
        ret = ((0xC0 + k) << 16) + o;
      break;
    case 0x4:
    case 0x5:
    case 0x6:
    case 0x7:
      if ( k < 0x7e )
        ret = ((0xC0 + (k - 0x40)) << 16) + o;
      break;
    case 0x8:
    case 0x9:
    case 0xa:
    case 0xb:
      if ( o >= 0x8000 )
        ret = ((0xC0 + (k - 0x80)) << 16) + o;
      else
        ret = ((k - 0x80) << 16) + o;
      break;
    default:
      break;
  }
  return ret;
}

//----------------------------------------------------------------------------
static bool addr_init(rommode_t new_mode)
{
  if ( g_mode != MODE_UNKNOWN )
    return false;

  g_mode = new_mode;
  switch ( g_mode )
  {
    case MODE_20:
      xlator = xlat_20;
      break;
    case MODE_21:
      xlator = xlat_21;
      break;
    case MODE_22:
      xlator = NULL;
      break;
    case MODE_25:
      xlator = NULL;
      break;
    default:
      return false;
  }
  return true;
}

//----------------------------------------------------------------------------
ea_t xlat(ea_t address)
{
  QASSERT(20016, xlator != NULL);
  return xlator(address);
}

