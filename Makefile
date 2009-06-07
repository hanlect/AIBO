#
# Copyright 2002 Sony Corporation 
#
# Permission to use, copy, modify, and redistribute this software for
# non-commercial use is hereby granted.
#
# This software is provided "as is" without warranty of any kind,
# either expressed or implied, including but not limited to the
# implied warranties of fitness for a particular purpose.
#

OPENRSDK_ROOT=/usr/local/OPEN_R_SDK

COMPONENTS=./Motion  \
	./GeneraMove  \
	./SharedMemMgr  \


INSTALLDIR=$(shell pwd)/MS
MEMSTICKMP=/mnt/memstick/
TARGETS=all install clean help

.PHONY: $(TARGETS)

all:
	for dir in $(COMPONENTS); do \
	if ! (cd $$dir && $(MAKE) OPENRSDK_ROOT=$(OPENRSDK_ROOT) $@);  \
	then \
	exit -1; \
	fi \
	done ; \
	echo "done."

install:
	for dir in $(COMPONENTS); do \
	if ! (cd $$dir && $(MAKE) INSTALLDIR=$(INSTALLDIR) OPENRSDK_ROOT=$(OPENRSDK_ROOT) $@);  \
	then \
	exit -1; \
	fi \
	done ; \

	echo "done."

clean:
	for dir in $(COMPONENTS); do \
	if ! (cd $$dir && $(MAKE) INSTALLDIR=$(INSTALLDIR) OPENRSDK_ROOT=$(OPENRSDK_ROOT) $@);  \
	then \
	exit -1; \
	fi \
	done ;

memstick: install
	umount $(MEMSTICKMP) ;  \
	mount $(MEMSTICKMP) ;  \
	echo "Copio i file nella memorystick..." ;  \
	cp -R $(INSTALLDIR)/* $(MEMSTICKMP) ;  \
	echo "File copiati." ; \
	echo "AIBO IP: " ; \
	grep "ETHER_IP" /mnt/memstick/open-r/system/conf/wlanconf.txt ;  \
	umount $(MEMSTICKMP) ;  \
	echo "done."


help:
	@echo 
	@echo "Help per la creazione degli eseguibili."
	@echo
	@echo " make (all):         compila tutto"
	@echo " make install:       compila e installa tutti gli eseguibili"
	@echo "                     nelle rispettive directory MS"
	@echo " make memstick:      copia nella memory stick (mount point"
	@echo "                     $(MEMSTICKMP) ) gli oggetti creati"	
	@echo " make clean:         cancella tutti gli oggetti creati"
	@echo	


#$(TARGETS):
#	for dir in $(COMPONENTS); do  \
#		(cd $$dir && $(MAKE) INSTALLDIR=$(INSTALLDIR) OPENRSDK_ROOT=$(OPENRSDK_ROOT) $@)  \
#	done
