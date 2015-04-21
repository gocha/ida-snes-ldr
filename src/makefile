include allmake.mak
ifndef __UNIX__
  CFGFILE := makeenv_vc.mak
  ENV := env
endif

ALLDIRS=module ldr plugins

.PHONY: env $(ALLDIRS) all

all: $(ENV) allbin

env:
	@$(MAKE) -f $(CFGFILE)

allbin: $(ENV) $(ALLDIRS)

$(ALLDIRS): $(ENV)
	@cd $@; $(MAKE)
###
