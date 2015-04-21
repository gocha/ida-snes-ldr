_CFLAGS=$(__CFLAGS) -D__IDP__
BASE=0x13000000
__IDP__=1
MAC_NO_DEAD_STRIP=1
# Info 528 Could be declared as const
# Info 785 Too few initializers for aggregate
# Info 843 Could be declared as const
# Info 1764 Reference parameter 'x' could be declared const ref
LINTFLAGS=\
          "-esym(528,respect_info)" \
          "-esym(528,ports)"        \
          "-esym(528,numports)"     \
          "-e785"                   \
          "-esym(843,ports)"        \
          "-esym(843,numports)"     \
          "-esym(1764,x)"

include ../../allmake.mak

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
ifdef O7
  OBJ7=$(F)$(O7)$(O)
endif
ifdef O8
  OBJ8=$(F)$(O8)$(O)
endif
ifdef O9
  OBJ9=$(F)$(O9)$(O)
endif

OBJS=$(F)ins$(O) $(F)ana$(O) $(F)out$(O) $(F)reg$(O) $(F)emu$(O) \
     $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4) $(OBJ5) $(OBJ6) $(OBJ7) $(OBJ8) $(OBJ9) \
     $(ADDITIONAL_OBJS)

IDP_MODULE=$(R)procs/$(PROC)$(IDP)

all:    objdir $(IDP_MODULE) $(ADDITIONAL_GOALS)
include ../../objdir.mak

ifdef __UNIX__
  ifdef __LINUX__
    PLUGIN_SCRIPT=-Wl,--version-script=../idp.script
  endif
  ifdef __MAC__
    PLUGIN_SCRIPT=-Wl,-install_name,$(PROC)$(IDP)
  endif

$(IDP_MODULE): ../idp.script $(OBJS) makefile
	$(CCL) $(OUTDLL) $(OUTSW)$@ $(OBJS) -L$(R) $(LINKIDA) $(PLUGIN_SCRIPT) $(ADDITIONAL_LIBS) $(STDLIBS)

else # Windows

  ifndef __X64__
    DEFFILE:=../idp.def
  else
    DEFFILE:=../idp64.def
  endif

# sometimes /STUB may result in completely missing MZ header if incremental linking is on
# this is why we turn off incremental link
$(IDP_MODULE): $(OBJS) $(IDALIB) $(DEFFILE)
	$(LINKER) $(LINKOPTS) /STUB:../stub /INCREMENTAL:NO /OUT:$@ $(OBJS) $(IDALIB) $(ADDITIONAL_LIBS) user32.lib
	@$(RM) $(@:$(IDP)=.exp) $(@:$(IDP)=.lib)

endif
