ifeq ($(PLATFORM),psx)
BIN_PREFIX=mipsel-unknown-elf-
endif

CXX = $(BIN_PREFIX)g++
LD = $(BIN_PREFIX)g++

CXXFLAGS = -Wall -g -O2 -std=c++0x
LIBS =
LDFLAGS =

CXXFILES = game.cc grid.cc

ifeq ($(PLATFORM),psx)
CXXFILES += \
	main-psx.cc \
	gfx-psx.cc

CXXFLAGS += \
	-fsigned-char \
	-msoft-float \
	-mno-gpopt \
	-fno-builtin \
	-fno-rtti \
	-fno-exceptions \
	-G0 \
	-I$(PSXSDK_ROOT)/include

LDFLAGS += \
	-nostdlib \
	-lgcc \
	-T $(PSXSDK_ROOT)/mipsel-unknown-elf/lib/ldscripts/playstation.x

CD_IMAGE_ROOT = cdroot
LICENSE_FILE  = $(PSXSDK_ROOT)/share/licenses/infoeur.dat
MKISOFS = genisoimage
else
CXXFILES += \
	main-sdl.cc \
	gfx-sdl.cc

CXXFLAGS += `pkg-config --cflags sdl gl glu`
LIBS += `pkg-config --libs sdl gl glu`
endif

OBJS = $(CXXFILES:.cc=.o)

TARGET = game

$(TARGET): $(OBJS)
	$(LD) $(OBJS) -o $@ $(LIBS) $(LDFLAGS)

.cc.o:
	$(CXX) $(CXXFLAGS) -c $<

ifeq ($(PLATFORM),psx)
$(TARGET).exe: $(TARGET)
	elf2exe $(TARGET) $(TARGET).exe

cdimage: $(TARGET).exe
	mkdir -p $(CD_IMAGE_ROOT)
	cp $(TARGET).exe $(CD_IMAGE_ROOT)
	systemcnf $(TARGET).exe > $(CD_IMAGE_ROOT)/system.cnf
	$(MKISOFS) -o $(TARGET).hsf -V $(TARGET) -sysid PLAYSTATION $(CD_IMAGE_ROOT)
	mkpsxiso $(TARGET).hsf $(TARGET).bin $(LICENSE_FILE)
	rm -f $(TARGET).hsf
endif

depend: .depend

.depend: $(CXXFILES)
	rm -f .depend
	$(CXX) $(CXXFLAGS) -MM $^ > .depend;

clean:
	rm -f *o $(TARGET)
ifeq ($(PLATFORM),psx)
	rm -rf $(CD_IMAGE_ROOT) $(TARGET).hsf $(TARGET).bin $(TARGET).cue $(TARGET).exe
endif

include .depend

.PHONY: all clean depend
