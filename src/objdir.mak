
# this makefile creates the output directories for object/library files

objdir $(OBJDIR):
	-@if [ ! -d "$(OBJDIR)" ] ; then mkdir -p 2>/dev/null $(OBJDIR) ; fi
