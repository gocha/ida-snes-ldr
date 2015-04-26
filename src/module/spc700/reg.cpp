/*
 *      Interactive disassembler (IDA).
 *      Version 3.05
 *      Copyright (c) 1990-95 by Ilfak Guilfanov.
 *      ALL RIGHTS RESERVED.
 *                              FIDO:   2:5020/209
 *                              E-mail: ig@estar.msk.su
 *
 */

#include "spc700.hpp"

//--------------------------------------------------------------------------
static const char *const RegNames[] =
{
  "a",
  "x",
  "y",
  "ya",
  "sp",
  "Cs",
  "Ds",

  "psw",
  "C",
};

//----------------------------------------------------------------------
static int idaapi notify(processor_t::idp_notify msgid, ...) // Various messages:
{
  va_list va;
  va_start(va, msgid);

// A well behaved processor module should call invoke_callbacks()
// in his notify() function. If this function returns 0, then
// the processor module should process the notification itself
// Otherwise the code should be returned to the caller:

  int code = invoke_callbacks(HT_IDP, msgid, va);
  if ( code ) return code;

  switch ( msgid )
  {
    case processor_t::newfile:
      break;

    default:
      break;
  }
  va_end(va);

  return(1);
}


//-----------------------------------------------------------------------
//      XLA-DX assembler definition
//-----------------------------------------------------------------------
static const asm_t xladxasm =
{
  AS_COLON | ASH_HEXF4,
  UAS_NOSEG,
  "XLA-DX Assembler by Ville Helin",
  0,
  NULL,         // headers
  NULL,
  ".org",
  ".end",

  ";",          // comment string
  '"',          // string delimiter
  '\'',         // char delimiter
  "\\'",        // special symbols in char and string constants

  ".db",        // ascii string directive
  ".db",        // byte directive
  ".dw",        // word directive
};

static const asm_t *const asms[] = { &xladxasm, NULL };
//-----------------------------------------------------------------------
static const char *const shnames[] = { "spc700", NULL };
static const char *const lnames[] = { "Sony SPC700", NULL };

//--------------------------------------------------------------------------
static const uchar retcode_1[] = { 0x6f }; // ret
static const uchar retcode_2[] = { 0x7f }; // reti

static const bytes_t retcodes[] =
{
 { sizeof(retcode_1), retcode_1 },
 { sizeof(retcode_2), retcode_2 },
 { 0, NULL }
};

//-----------------------------------------------------------------------
//      Processor Definition
//-----------------------------------------------------------------------

// TODO: PLFM ID?
#define PLFM_SPC700	PLFM_6502

processor_t LPH =
{
  IDP_INTERFACE_VERSION,// version
  PLFM_SPC700,          // id
  0,                    // flags
  8,                    // 8 bits in a byte for code segments
  8,                    // 8 bits in a byte for other segments

  shnames,
  lnames,

  asms,

  notify,

  header,
  footer,

  segstart,
  std_gen_segm_footer,

  NULL,                 // assumes,

  ana,
  emu,

  out,
  outop,
  intel_data,
  NULL,                 // compare operands
  NULL,                 // can have type

  qnumber(RegNames),            // Number of registers
  RegNames,                     // Register names
  NULL,                         // get abstract register

  0,                            // Number of register files
  NULL,                         // Register file names
  NULL,                         // Register descriptions
  NULL,                         // Pointer to CPU registers

  rCs,                          // first segreg
  rDs,                          // last  segreg
  0,                            // size of a segment register
  rCs,                          // number of CS register
  rDs,                          // number of DS register

  NULL,                         // No known code start sequences
  retcodes,

  0,SPC_last,
  Instructions
};
