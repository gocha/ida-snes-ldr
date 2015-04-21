
# Create configuration files

include allmake.mak

ifdef USE_VC11
  SDK_INCLUDE1=/I$(MSSDK)Include/um
  SDK_INCLUDE2=/I$(MSSDK)Include/shared
else
  SDK_INCLUDE1=/I$(MSSDK)Include
endif

define CFG0             # Common flags
/nologo
/I$(shell cygpath -m $(call qabspath, $(IDA))include)
/DMAXSTR=1024
/DNO_TV_STREAMS
-DNO_OBSOLETE_FUNCS
/D__NT__
# Generate debug info (old style)
/Z7
# Directory for PDB files
/Fd$(F)
endef

############################################################################
#               Visual Studio for ARM
ifdef __CEARM__
define CFG1
/I\"$(MSVCARMDIR)Include\"
/I\"$(ARMSDK)Include\"
#       Merge duplicate strings
/GF
#       No standard stack frame
/Oy
#       Maximum optimization
/Ox
#       All warnings on
/W2
#       Exception handling (try/catch can handle only C++ exceptions; use __except for SEH)
/EHs
#	Separate functions for linker
/Gy
#       show full paths
/FC
/DARM
/DUNDER_CE
/DUNICODE
/DNDEBUG
/D_WIN32_WCE=0x420
endef
else
############################################################################
#               Visual Studio for Intel
define CFG1
/I$(MSVCDIR)Include
$(SDK_INCLUDE1)
$(SDK_INCLUDE2)
#       Merge duplicate strings
/GF
#       Exception handling (try/catch can handle only C++ exceptions; use __except for SEH)
/EHs
#	Separate functions for linker
/Gy
/D__VC__
#       show full paths
/FC
#       All warnings on
/Wall
#       warning C4018: >= : signed/unsigned mismatch
/wd4018
#       warning C4061: enumerator xxx in switch of enum yyy is not explicitly handled by a case label
/wd4061
#       warning C4121: xxx: alignment of a member was sensitive to packing
/wd4121
#       warning C4127: conditional expression is constant
/wd4127
#       warning C4146: unary minus operator applied to unsigned type, result still unsigned
/wd4146
#       warning C4200: nonstandard extension used: zero-sized array in struct/union
/wd4200
#       warning C4201: nonstandard extension used : nameless struct/union
/wd4201
#       warning C4239: nonstandard extension used : non-const refernece=>lvalue
/wd4239
#       warning C4242: initializing : conversion from a to b, possible loss of data
/wd4242
#       warning C4244: xxx : conversion from a to b, possible loss of data
/wd4244
#       warning C4245: xxx : conversion from a to b, signed/unsigned mismatch
/wd4245
#       warning C4267: xxx : conversion from size_t to b, possible loss of data
/wd4267
#       warning C4310: cast truncates constant value
/wd4310
#       warning C4324: xxx : structure was padded due to __declspec(align())
/wd4324
#       warning C4350: behavior change: '' called instead of ''
#       for some reason vc11 started to complain about it and i do not see
#       how to shut it up
/wd4350
#       warning C4355: this : used in base member initializer list
/wd4355
#       warning C4365: xx : signed/unsigned mismatch (= or return)
/wd4365
endef
define CFG1a
#       warning C4366: The result of the unary & operator may be unaligned
/wd4366
#       warning C4371: xxx : layout of class may have changed from a previous version of the compiler due to better packing of member xxx
/wd4371
#       warning C4389: != : signed/unsigned mismatch
/wd4389
#       warning C4480: != :  nonstandard extension used: ....
/wd4480
#       warning C4512: xxx : assignment operator could not be generated
/wd4512
#       warning C4514: xxx : unreferenced inline function has been removed
/wd4514
#       warning C4548: expression before comma has no effect; expected expression with side-effect
/wd4548
#       warning C4571: Informational: catch(...) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught
/wd4571
#       warning C4611: interaction between _setjmp and C++ object destruction is non-portable
/wd4611
#       warning C4619: pragma warning : there is no warning number xxx
/wd4619
#       warning C4625: xxx : copy constructor could not be generated because a base class copy constructor is inaccessible
/wd4625
#       warning C4626: xxx : assignment operator could not be generated because a base class assignment operator is inaccessible
/wd4626
#       warning C4640: p : construction of local static object is not thread-safe
/wd4640
#       warning C4668: xxx is not defined as a preprocessor macro, replacing with 0 for #if/#elif
/wd4668
#       warning C4686: xxx : possible change in behavior, change in UDT return calling convention
/wd4686
#       warning C4701: xxx : potentially uninitialized local variable
/wd4701
#       warning C4710: xxx : function not inlined
/wd4710
#       warning C4711: xxx : function select for automatic inline expansion
/wd4711
#       warning C4738: storing 32-bit float result in memory, possible loss of performance
/wd4738
#       warning C4820: xxx : x bytes padding added after member xxx
/wd4820
#       warning C4917: xxx : a GUID can only be associated with a class, interface or namespace
/wd4917
#       warning C4986: operator new[]: exception specification does not match previous declaration entered the game.
/wd4986
#       warning C4987: setjmp.h(vc10) nonstandard extension used: throw(...)
/wd4987
#
# treat as errors:
#       warning C4541: 'dynamic_cast' used on polymorphic type 'xxx' with /GR-; unpredictable behavior may result
/we4541
#       warning C4715: 'function' : not all control paths return a value
/we4715
#       warning C4296:  '' : expression is always false
/we4296
#       warning C4315: 'class' : 'this' pointer for member 'class::member' may not be aligned 8 as expected by the constructor
/we4315
#       warning C4805: 'operation' : unsafe mix of type 'type' and type 'type' in operation
/we4805
#       
endef
endif

ifdef USE_VC11          # /analyze related suppressions
define CFG2
#       warning C4435: '' : Object layout under /vd2 will change due to virtual base ''
/wd4435
#       warning C6323: Use of arithmetic operator on Boolean type(s).
/wd6323
#       warning C6340: Mismatch on sign: '' passed as '' when some signed type is required in call to ''
/wd6340
endef
else
define CFG2
endef
endif

ifdef NDEBUG            # Optimization flags
define CFG3
/DNDEBUG
# Do not use checked iterators
/D_SECURE_SCL=0
# Maximum optimization
/Ox
# Enable intrinsic functions
/Oi
#       warning C4574: yvals.h(vc10): '_SECURE_SCL' is defined to be '0': did you mean to use '#if _SECURE_SCL'?
/wd4574
endef
else                    # Debug flags
define CFG3
/D_DEBUG
# Enable security checks
/GS
# Disable optimizations
/Od
endef
endif

# definition of a single \n character (empty lines are important!)
define newline


endef

all:    $(SYSDIR).cfg
$(SYSDIR).cfg: makeenv_vc.mak allmake.mak defaults.mk makefile
	@echo -e '$(subst $(newline),\n,${CFG0})' | grep -v '^#' >$@
	@echo -e '$(subst $(newline),\n,${CFG1})' | grep -v '^#' >>$@
	@echo -e '$(subst $(newline),\n,${CFG1a})'| grep -v '^#' >>$@
	@echo -e '$(subst $(newline),\n,${CFG2})' | grep -v '^#' >>$@
	@echo -e '$(subst $(newline),\n,${CFG3})' | grep -v '^#' >>$@
ifndef __CEARM__
  ifdef VC_USE_CPUS
	@#       Use up to 'n' processes for compilation (VC9 or higher)
	@echo "/MP$(VC_USE_CPUS)" >>$@
  endif
endif
ifdef __EA64__
	@echo /D__EA64__                        >>$@
endif
ifdef __X64__
	@echo /D__X64__                         >>$@
endif
ifdef USE_STATIC_RUNTIME
	@echo /D__NOEXPORT__                    >>$@
endif
