RS=$(IDA)bin/

PEUTIL=$(RS)peutil.exe

ULNK_BASE=$(RS)ulink.exe

TLIB=$(RS)tlib.exe

ULNK_CFG_DIR=$(RS)

NASM=$(RS)nasmw.exe

#-------------------------------------------------------
MSSDK?=C:/PROGRA~1/MICROS~3/WINDOWS/v7.0A/
VSPATH?=C:/PROGRA~2/MICROS~2.0/
VSPATH8?=C:/PROGRA~2/MICROS~1.0/
ARMSDK?=$(VSPATH8)SmartDevices/SDK/PocketPC2003/
GCCBINDIR?=c:/cygwin/bin
PYTHON_ROOT?=c:
PYTHON?=$(PYTHON_ROOT)/python27/python.exe
SWIG?=C:/swigwin-2.0.0/swig.exe
  ifdef __X64__
    QTVER?=4.8.4-x64
  else
    QTVER?=4.8.4
  endif
QTDIR?=c:/Qt/$(QTVER)/
QT_QMAKE?=$(QTDIR)/bin/qmake
ANDROID_NDK?=c:/android-ndk-r4b/
SOURCERY?=C:/CODESO~1/SOURCE~1
HHC?=$(IDA)ui/qt/help/chm/hhc.exe
THIRD_PARTY?=$(IDA)../third_party/

# keep all paths in dos format, with double backslashes
MSSDK        :=$(call unixpath,$(MSSDK))
VS11PATH     :=$(call unixpath,$(VS11PATH))
VSPATH8      :=$(call unixpath,$(VSPATH8))
VSPATH       :=$(call unixpath,$(VSPATH))
ARMSDK       :=$(call unixpath,$(ARMSDK))
GCCBINDIR    :=$(call unixpath,$(GCCBINDIR))
PYTHON_ROOT  :=$(call unixpath,$(PYTHON_ROOT))
PYTHON       :=$(call unixpath,$(PYTHON))
SWIG         :=$(call unixpath,$(SWIG))
QTDIR        :=$(call unixpath,$(QTDIR))
ANDROID_NDK  :=$(call unixpath,$(ANDROID_NDK))
SOURCERY     :=$(call unixpath,$(SOURCERY))
STLDIR       :=$(call unixpath,$(STLDIR))
HHC          :=$(call unixpath,$(HHC))
ifdef __NT__
  THIRD_PARTY  :=$(call unixpath,$(THIRD_PARTY))
endif

################################EOF###############################
