
// Common include files for IDP modules:

#include <ida.hpp>
#include <idp.hpp>
#include <ua.hpp>
#include <name.hpp>
#include <auto.hpp>
#include <bytes.hpp>
#include <queue.hpp>
#include <lines.hpp>
#include <loader.hpp>
#include <offset.hpp>
#include <segment.hpp>
#include <kernwin.hpp>

// Current processor in the module
// It must be exported
idaman processor_t ida_module_data LPH;

int handle_old_type_callbacks(int code, va_list va);
//-------------------------------------------------------------------------
#define GH_PRINT_PROC           1 << 0
#define GH_PRINT_ASM            1 << 1
#define GH_PRINT_BYTESEX        1 << 2
#define GH_PRINT_HEADER         1 << 3
#define GH_BYTESEX_HAS_HIGHBYTE 1 << 4

#define GH_PRINT_PROC_AND_ASM (GH_PRINT_PROC | GH_PRINT_ASM)
#define GH_PRINT_PROC_ASM_AND_BYTESEX (GH_PRINT_PROC_AND_ASM | GH_PRINT_BYTESEX)
#define GH_PRINT_ALL (GH_PRINT_PROC_ASM_AND_BYTESEX | GH_PRINT_HEADER)
#define GH_PRINT_ALL_BUT_BYTESEX (GH_PRINT_PROC_AND_ASM | GH_PRINT_HEADER)

#define SHOULD(f) (flags & (f)) == (f)
#define HAS(f) SHOULD(f)

static inline void gen_header_extra()
{
  if ( ash.header != NULL )
    for ( const char *const *ptr=ash.header; *ptr != NULL; ptr++ )
      printf_line(0,COLSTR("%s",SCOLOR_ASMDIR),*ptr);
}

static inline void gen_header(
        int flags = GH_PRINT_PROC_AND_ASM,
        const char *proc_name = NULL,
        const char *proc_flavour = NULL)
{
  if ( SHOULD(GH_PRINT_PROC) )
  {
    char buf[32];
    if ( proc_name == NULL )
      proc_name = inf.get_proc_name(buf);
    if ( proc_flavour == NULL )
      gen_cmt_line("Processor       : %s", proc_name);
    else
      gen_cmt_line("Processor       : %s [%s]", proc_name, proc_flavour);
  }

  if ( SHOULD(GH_PRINT_ASM) )
  {
    gen_cmt_line("Target assembler: %s", ash.name);
  }

  if ( SHOULD(GH_PRINT_BYTESEX) )
  {
    if ( HAS(GH_BYTESEX_HAS_HIGHBYTE) )
      gen_cmt_line("Byte sex        : %s, %s",
                   inf.mf ? "Big endian" : "Little endian",
                   inf.wide_high_byte_first ? "high_byte_first" : "high_byte_last");
    else
      gen_cmt_line("Byte sex        : %s",
                   inf.mf ? "Big endian" : "Little endian");
  }

  if ( SHOULD(GH_PRINT_HEADER) )
  {
    gen_header_extra();
  }
}

#undef HAS
#undef SHOULD
