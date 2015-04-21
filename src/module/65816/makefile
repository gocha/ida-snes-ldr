PROC=m65816
ADDITIONAL_GOALS=cfg
O1=bt

include ../module.mak

cfg: $(C)m65816.cfg
$(C)m65816.cfg: m65816.cfg
	$(CP) $? $@

# MAKEDEP dependency list ------------------
$(F)ana$(O)     : $(I)area.hpp $(I)auto.hpp $(I)bitrange.hpp $(I)bytes.hpp   \
	          $(I)fpro.h $(I)funcs.hpp $(I)ida.hpp $(I)idp.hpp          \
	          $(I)kernwin.hpp $(I)lines.hpp $(I)llong.hpp               \
	          $(I)loader.hpp $(I)nalt.hpp $(I)name.hpp $(I)netnode.hpp  \
	          $(I)offset.hpp $(I)pro.h $(I)queue.hpp $(I)segment.hpp    \
	          $(I)srarea.hpp $(I)ua.hpp $(I)xref.hpp                    \
	          ../../module/idaidp.hpp ana.cpp ins.hpp m65816.hpp
$(F)bt$(O)      : $(I)area.hpp $(I)auto.hpp $(I)bitrange.hpp $(I)bytes.hpp   \
	          $(I)fpro.h $(I)funcs.hpp $(I)ida.hpp $(I)idp.hpp          \
	          $(I)kernwin.hpp $(I)lines.hpp $(I)llong.hpp               \
	          $(I)loader.hpp $(I)nalt.hpp $(I)name.hpp $(I)netnode.hpp  \
	          $(I)offset.hpp $(I)pro.h $(I)queue.hpp $(I)segment.hpp    \
	          $(I)srarea.hpp $(I)ua.hpp $(I)xref.hpp                    \
	          ../../module/idaidp.hpp bt.cpp bt.hpp ins.hpp m65816.hpp
$(F)emu$(O)     : $(I)area.hpp $(I)auto.hpp $(I)bitrange.hpp $(I)bytes.hpp   \
	          $(I)fpro.h $(I)funcs.hpp $(I)ida.hpp $(I)idp.hpp          \
	          $(I)kernwin.hpp $(I)lines.hpp $(I)llong.hpp               \
	          $(I)loader.hpp $(I)nalt.hpp $(I)name.hpp $(I)netnode.hpp  \
	          $(I)offset.hpp $(I)pro.h $(I)queue.hpp $(I)segment.hpp    \
	          $(I)srarea.hpp $(I)ua.hpp $(I)xref.hpp                    \
	          ../../module/idaidp.hpp bt.hpp emu.cpp ins.hpp            \
	          m65816.hpp
$(F)ins$(O)     : $(I)area.hpp $(I)auto.hpp $(I)bitrange.hpp $(I)bytes.hpp   \
	          $(I)fpro.h $(I)funcs.hpp $(I)ida.hpp $(I)idp.hpp          \
	          $(I)kernwin.hpp $(I)lines.hpp $(I)llong.hpp               \
	          $(I)loader.hpp $(I)nalt.hpp $(I)name.hpp $(I)netnode.hpp  \
	          $(I)offset.hpp $(I)pro.h $(I)queue.hpp $(I)segment.hpp    \
	          $(I)srarea.hpp $(I)ua.hpp $(I)xref.hpp                    \
	          ../../module/idaidp.hpp ins.cpp ins.hpp m65816.hpp
$(F)out$(O)     : $(I)area.hpp $(I)auto.hpp $(I)bitrange.hpp $(I)bytes.hpp   \
	          $(I)fpro.h $(I)funcs.hpp $(I)ida.hpp $(I)idp.hpp          \
	          $(I)kernwin.hpp $(I)lines.hpp $(I)llong.hpp               \
	          $(I)loader.hpp $(I)nalt.hpp $(I)name.hpp $(I)netnode.hpp  \
	          $(I)offset.hpp $(I)pro.h $(I)queue.hpp $(I)segment.hpp    \
	          $(I)srarea.hpp $(I)ua.hpp $(I)xref.hpp                    \
	          ../../module/idaidp.hpp ins.hpp m65816.hpp out.cpp
$(F)reg$(O)     : $(I)area.hpp $(I)auto.hpp $(I)bitrange.hpp $(I)bytes.hpp   \
	          $(I)diskio.hpp $(I)entry.hpp $(I)fpro.h $(I)funcs.hpp     \
	          $(I)ida.hpp $(I)idp.hpp $(I)kernwin.hpp $(I)lines.hpp     \
	          $(I)llong.hpp $(I)loader.hpp $(I)nalt.hpp $(I)name.hpp    \
	          $(I)netnode.hpp $(I)offset.hpp $(I)pro.h $(I)queue.hpp    \
	          $(I)segment.hpp $(I)srarea.hpp $(I)ua.hpp $(I)xref.hpp    \
	          ../../ldr/snes/addr.cpp ../../module/idaidp.hpp           \
	          ../iocommon.cpp ins.hpp m65816.hpp reg.cpp
