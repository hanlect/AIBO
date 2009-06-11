 #
 # Copyright 2004 Sony Corporation 
 #
 # Permission to use, copy, modify, and redistribute this software for
 # non-commercial use is hereby granted.
 #
 # This software is provided "as is" without warranty of any kind,
 # either expressed or implied, including but not limited to the
 # implied warranties of fitness for a particular purpose.
 #

OPENRSDK_ROOT?=/usr/local/OPEN_R_SDK
INSTALLDIR=../MS
CXX=$(OPENRSDK_ROOT)/bin/mipsel-linux-g++
STRIP=$(OPENRSDK_ROOT)/bin/mipsel-linux-strip
MKBIN=$(OPENRSDK_ROOT)/OPEN_R/bin/mkbin
STUBGEN=$(OPENRSDK_ROOT)/OPEN_R/bin/stubgen2
MKBINFLAGS=-p $(OPENRSDK_ROOT)
LIBS=-L../lib -L$(OPENRSDK_ROOT)/OPEN_R/lib -lObjectComm -lOPENR
CXXFLAGS= \
	-ftracer \
	-O2 \
	-g \
	-Wall \
	-fomit-frame-pointer \
	-I. \
	-I$(OPENRSDK_ROOT)/OPEN_R/include/R4000 \
	-I$(OPENRSDK_ROOT)/OPEN_R/include 

 #
 # When OPENR_DEBUG is defined, OSYSDEBUG() is available.
 #
CXXFLAGS+= -DOPENR_DEBUG


TARGET_NAME=GeneraMove
TARGET_BIN=$(TARGET_NAME).bin
TARGET_MS=GENMOVE.BIN

OBJ=$(TARGET_NAME)Stub.o \
	$(TARGET_NAME).o \
	$(TARGET_NAME).o \
	./R_list.o \



 .PHONY: all install clean

 all: $(TARGET_BIN)

 %.o: %.cc
	$(CXX) $(CXXFLAGS) -o $@ -c $^

$(TARGET_NAME)Stub.cc: stub.cfg
	$(STUBGEN) stub.cfg	

$(TARGET_BIN): $(OBJ) $(TARGET_NAME).ocf
	$(MKBIN) $(MKBINFLAGS) -o $@ $^ $(LIBS)
	$(STRIP) $@

 install: $(TARGET_BIN)
	gzip -c $(TARGET_BIN) > $(INSTALLDIR)/OPEN-R/MW/OBJS/$(TARGET_MS)

 clean:
	rm -f *.o *.bin *.elf *.snap.cc
	rm -f $(TARGET_NAME)Stub.h $(TARGET_NAME)Stub.cc def.h entry.h
	rm -f $(INSTALLDIR)/OPEN-R/MW/OBJS/$(TARGET_MS)








# # target module name
# TARGET_USER_OBJECT  = ballTrackingHead7
# TARGET_USER_OBJECT_BIN  = BALLTRK7

# STUB_BASE = BallTrackingHead7Stub

# # C++ source files

# SHARED_SRCS = \
# 	../shared_code/AperiosSharedMem.cc \
# 	../shared_code/CircBufPacket.cc \
# 	../shared_code/Config.cc \
# 	../shared_code/Dictionary.cc \
# 	../shared_code/FileSystem.cc \
# 	../shared_code/PacketMux.cc \
# 	../shared_code/Sensors.cc \
# 	../shared_code/SPOutEncoder.cc \
# 	../shared_code/Events.cc \
# 	../Motion/Kinematics.cc \
# 	$(MAIN_SRCS) \

# MOTION_SRCS = \
# 	BallTrackingHead7.cc \
# 	BallTrackingHead7_Image.cc  \
# 	BallTrackingHead7_Joint.cc \
# 	BallTrackingHead7_Sensor.cc \
# 	BallTrackingHead7Stub.cc \
# 	 BMP.cc \


# CC_SRCS   = \
# 	$(MOTION_SRCS) \
# 	$(SHARED_SRCS) \

# CC_OBJS = $(CC_SRCS:.cc=.mips.o)
# CC_DEPS = $(CC_SRCS:.cc=.mips.d)

# # extra unusual generated .h files
# GEN_HS := 

# include ../Makefile.sdk.common
