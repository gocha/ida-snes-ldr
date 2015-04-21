_CFLAGS=$(__CFLAGS) -D__IDP__

ifndef BASE
  BASE=0x14000000
endif

__IDP__=1
ifndef O
  include ../../allmake.mak
endif

ifdef O1
  OBJ1=$(F)$(O1)$(O)
endif
ifdef O2
  OBJ2=$(F)$(O2)$(O)
endif
ifdef O3
  OBJ3=$(F)$(O3)$(O)
endif
ifdef O4
  OBJ4=$(F)$(O4)$(O)
endif
ifdef O5
  OBJ5=$(F)$(O5)$(O)
endif
ifdef O6
  OBJ6=$(F)$(O6)$(O)
endif

OBJS=$(F)$(PROC)$(O) $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4) $(OBJ5) $(OBJ6) \
     $(OBJ7) $(ADDITIONAL_OBJS)

# Info 785 Too few initializers for aggregate
LINTFLAGS="-e785" $(_LINTFLAGS)

LDR_MODULE=$(R)loaders/$(PROC)$(LDR)

all:	objdir $(LDR_MODULE) $(ADDITIONAL_GOALS)
include ../../objdir.mak

ifdef __UNIX__
  ifdef __LINUX__
    PLUGIN_SCRIPT=-Wl,--version-script=../ldr.script
  endif
  ifdef __MAC__
    PLUGIN_SCRIPT=-Wl,-install_name,$(PROC)$(LDR)
  endif

$(LDR_MODULE): ../ldr.script $(OBJS) makefile
	$(CCL) $(OUTDLL) $(OUTSW)$@ $(OBJS) -L$(R) $(LINKIDA) $(PLUGIN_SCRIPT) $(ADDITIONAL_LIBS) $(STDLIBS)

else # windows

  ifdef __X64__
    DEFFILE=../ldr64.def
  else
    DEFFILE=../ldr.def
  endif

$(LDR_MODULE): $(DEFFILE) $(OBJS) $(IDALIB) makefile
	$(LINKER) $(LINKOPTS) /STUB:../stub /INCREMENTAL:NO /OUT:$@ $(OBJS) $(IDALIB) user32.lib
	@$(RM) $(@:$(LDR)=.exp) $(@:$(LDR)=.lib)
endif
