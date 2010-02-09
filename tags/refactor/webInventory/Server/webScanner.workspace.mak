# -*- mode: Makefile; -*-
# -----------------------------------------
# workspace webScanner

all :

% :
	$(MAKE) -C ../../webEngine -f webEngine.mak $@
	$(MAKE) -C ../Plugins/FsStorage -f storageFS.mak $@
	$(MAKE) -C ../Plugins/Demo -f Demo.mak $@
	$(MAKE) -C ../Scanner -f tskScanner.mak $@
	$(MAKE) -C . -f scanServer.mak $@
