_CFLAGS=-Ipe
include ../allmake.mak

ALLDIRS=w32run aout elf hex nlm aif aof lx pe omf coff dbg ne dex \
        pilot dos pef qnx javaldr rt11 exp os9 n64 psx epoc amiga \
        hpsom geos mach-o dump xbe tmobj intelomf mas is5x sbn osk \
        bochsrc windmp snes snes_spc script_ldrs $(ADDITIONAL_LOADERS)

# remove any missing loaders from the list
ALLDIRS:=$(wildcard $(ALLDIRS))

# remove loaders that depend on pc source code
ifeq ($(wildcard ../module/pc),)
  ALLDIRS:=$(filter-out coff elf windmp,$(ALLDIRS))
endif

# remove loaders that depend on arm source code
ifeq ($(wildcard ../module/arm),)
  ALLDIRS:=$(filter-out coff elf,$(ALLDIRS))
endif

# remove loaders that depend on java source code
ifeq ($(wildcard ../module/java),)
  ALLDIRS:=$(filter-out javaldr,$(ALLDIRS))
endif

# remove loaders that depend on pdp11 source code
ifeq ($(wildcard ../module/pdp11),)
  ALLDIRS:=$(filter-out rt11,$(ALLDIRS))
endif

# remove loaders that depend on debugger source code
ifeq ($(wildcard ../dbg),)
  ALLDIRS:=$(filter-out bochsrc windmp,$(ALLDIRS))
endif

ifdef __X64__
  ALLDIRS:=
endif

.PHONY: $(ALLDIRS)
all:    $(ALLDIRS)

$(ALLDIRS):
	@cd $@; $(MAKE)
