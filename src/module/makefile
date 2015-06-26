include ../allmake.mak

DIRS64=alpha ia64
DIRS32=pc                                                                      \
       6502 65816 78k0 78k0s 80196                                             \
       ad218x arc avr                                                          \
       c166 c39 cli cr16                                                       \
       dalvik dsp56k                                                           \
       f2mc                                                                    \
       fr h8 h8500 hppa                                                        \
       i51 i860 i960                                                           \
       java                                                                    \
       kr1878                                                                  \
       m16c m32r m740 m7700 m7900 mc6812 mc6816 mc68k mc68xx mips mn102 mcore  \
       oakdsp                                                                  \
       pdp11 pic ppc                                                           \
       sam8 sh3 sparc spc700 st20 st7 st9                                      \
       tlcs900                                                                 \
       tms320c1 tms320c3 tms320c5 tms320c54 tms320c55 tms320c6 tricore trimedia\
       xa                                                                      \
       script unsp                                                             \
       z8 z80 nec850

# We continue to compile 32-bit versions of the modules for 64-bit processors
# at least for MS Windows because of the existing old databases.

ALLDIRS:=$(DIRS32) $(DIRS64) $(ADDITIONAL_MODULES)

# remove any missing modules from the list
ALLDIRS:=$(wildcard $(ALLDIRS))

# remove plugins that depend on elf loader source code
ifeq ($(wildcard ../ldr/elf),)
  ALLDIRS:=$(filter-out arm pc ppc,$(ALLDIRS))
endif

# remove plugins that depend on aof loader source code
ifeq ($(wildcard ../ldr/aof),)
  ALLDIRS:=$(filter-out avr,$(ALLDIRS))
endif

# remove plugins that depend on pe loader source code
ifeq ($(wildcard ../ldr/pe),)
  ALLDIRS:=$(filter-out cli,$(ALLDIRS))
endif

# remove plugins that depend on dex loader source code
ifeq ($(wildcard ../ldr/dex),)
  ALLDIRS:=$(filter-out dalvik,$(ALLDIRS))
endif

ifdef __X64__
  ALLDIRS:=
endif

.PHONY: $(ALLDIRS)
all:    $(ALLDIRS)

$(ALLDIRS):
	@cd $@; $(MAKE)

