# -*- mode: Makefile; -*-
# -----------------------------------------
# project webEngine


export PATH := /opt/wx/2.8/bin:$(PATH)
export LD_LIBRARY_PATH := /opt/wx/2.8/lib:$(LD_LIBRARY_PATH)

_WX = /home/gr/projects/gui/codeblocks/wx
_WX.LIB = $(_WX)/lib
_WX.INCLUDE = $(_WX)/include

_CB = /home/gr/projects/gui/codeblocks/cb/src
_CB.INCLUDE = $(_CB)/include
_CB.LIB = $(_CB)/devel



CFLAGS_C = $(filter-out -include "sdk.h",$(CFLAGS))

# -----------------------------------------

# MAKE_DEP = -MMD -MT $@ -MF $(@:.o=.d)

CFLAGS = -Wno-write-strings 
INCLUDES = -Iinclude 
LDFLAGS =  -s
RCFLAGS = 
LDLIBS = $(T_LDLIBS)  -lstdc++

LINK_exe = gcc -o $@ $^ $(LDFLAGS) $(LDLIBS)
LINK_con = gcc -o $@ $^ $(LDFLAGS) $(LDLIBS)
LINK_dll = gcc -o $@ $^ $(LDFLAGS) $(LDLIBS) -shared
LINK_lib = rm -f $@ && ar rcs $@ $^
COMPILE_c = gcc $(CFLAGS_C) -o $@ -c $< $(MAKEDEP) $(INCLUDES)
COMPILE_cpp = g++ $(CFLAGS) -o $@ -c $< $(MAKEDEP) $(INCLUDES)
COMPILE_rc = windres $(RCFLAGS) -J rc -O coff -i $< -o $@ -I$(dir $<)

%.o : %.c ; $(COMPILE_c)
%.o : %.cpp ; $(COMPILE_cpp)
%.o : %.cxx ; $(COMPILE_cpp)
%.o : %.rc ; $(COMPILE_rc)
.SUFFIXES: .o .d .c .cpp .cxx .rc

all: all.before all.targets all.after

all.before :
	-
all.after : $(FIRST_TARGET)
	
all.targets : Debug_target Release_target 

clean :
	rm -fv $(clean.OBJ)
	rm -fv $(DEP_FILES)

.PHONY: all clean distclean

# -----------------------------------------
# Debug_target

Debug_target.BIN = libs/debug/libwebEngine.a
Debug_target.OBJ = sources/externals/shared_object_posix.o sources/weBlob.o sources/weDiffLib.o sources/weDispatch.o sources/weHTTP.o sources/weHelper.o sources/weHtmlEntity.o sources/weHttpInvent.o sources/weHttpResp.o sources/weLogger.o sources/weMemStorage.o sources/weOptions.o sources/weScan.o sources/weStrings.o sources/weTagScanner.o sources/weTask.o sources/weUrl.o sources/weiInventory.o sources/weiParser.o sources/weiPlugin.o sources/weiStorage.o sources/weiTransport.o 
DEP_FILES += sources/externals/shared_object_posix.d sources/weBlob.d sources/weDiffLib.d sources/weDispatch.d sources/weHTTP.d sources/weHelper.d sources/weHtmlEntity.d sources/weHttpInvent.d sources/weHttpResp.d sources/weLogger.d sources/weMemStorage.d sources/weOptions.d sources/weScan.d sources/weStrings.d sources/weTagScanner.d sources/weTask.d sources/weUrl.d sources/weiInventory.d sources/weiParser.d sources/weiPlugin.d sources/weiStorage.d sources/weiTransport.d 
clean.OBJ += $(Debug_target.BIN) $(Debug_target.OBJ)

Debug_target : Debug_target.before $(Debug_target.BIN) Debug_target.after_always
Debug_target : CFLAGS += -Wall -g  -Os
Debug_target : INCLUDES += -Isources -Iinclude 
Debug_target : RCFLAGS += 
Debug_target : LDFLAGS +=   $(CREATE_DEF)
Debug_target : T_LDLIBS = 
ifdef LMAKE
Debug_target : CFLAGS -= -O1 -O2 -g -pipe
endif

Debug_target.before :
	
	
Debug_target.after_always : $(Debug_target.BIN)
	
$(Debug_target.BIN) : $(Debug_target.OBJ)
	$(LINK_lib)
	

# -----------------------------------------
# Release_target

Release_target.BIN = libs/release/libwebEngine.a
Release_target.OBJ = sources/externals/shared_object_posix.o sources/weBlob.o sources/weDiffLib.o sources/weDispatch.o sources/weHTTP.o sources/weHelper.o sources/weHtmlEntity.o sources/weHttpInvent.o sources/weHttpResp.o sources/weLogger.o sources/weMemStorage.o sources/weOptions.o sources/weScan.o sources/weStrings.o sources/weTagScanner.o sources/weTask.o sources/weUrl.o sources/weiInventory.o sources/weiParser.o sources/weiPlugin.o sources/weiStorage.o sources/weiTransport.o 
DEP_FILES += sources/externals/shared_object_posix.d sources/weBlob.d sources/weDiffLib.d sources/weDispatch.d sources/weHTTP.d sources/weHelper.d sources/weHtmlEntity.d sources/weHttpInvent.d sources/weHttpResp.d sources/weLogger.d sources/weMemStorage.d sources/weOptions.d sources/weScan.d sources/weStrings.d sources/weTagScanner.d sources/weTask.d sources/weUrl.d sources/weiInventory.d sources/weiParser.d sources/weiPlugin.d sources/weiStorage.d sources/weiTransport.d 
clean.OBJ += $(Release_target.BIN) $(Release_target.OBJ)

Release_target : Release_target.before $(Release_target.BIN) Release_target.after_always
Release_target : CFLAGS += -O2 -Wall  -Os
Release_target : INCLUDES += -Isources -Iinclude 
Release_target : RCFLAGS += 
Release_target : LDFLAGS += -s   $(CREATE_DEF)
Release_target : T_LDLIBS = 
ifdef LMAKE
Release_target : CFLAGS -= -O1 -O2 -g -pipe
endif

Release_target.before :
	
	
Release_target.after_always : $(Release_target.BIN)
	
$(Release_target.BIN) : $(Release_target.OBJ)
	$(LINK_lib)
	

# -----------------------------------------
ifdef MAKE_DEP
-include $(DEP_FILES)
endif
