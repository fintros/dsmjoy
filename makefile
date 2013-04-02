MCU          = at90usb162
ARCH         = AVR8
BOARD        = DSMJOY
F_CPU        = 16000000
F_USB        = $(F_CPU)
OPTIMIZATION = s
TARGET       = DSMJoy
SRC          = src/$(TARGET).c src/USBFuncs.c src/DSMSat.c $(LUFA_SRC_USB)
CC_FLAGS     = -DUSE_LUFA_CONFIG_HEADER -Isrc/ -Isrc/Config/ -D_SN=L\"SN0000-AlSp\"
LD_FLAGS     =
OBJDIR       = out


# Default target
all:

# Include LUFA build script makefiles
include $(LUFA_PATH)/Build/lufa_core.mk
include $(LUFA_PATH)/Build/lufa_sources.mk
include $(LUFA_PATH)/Build/lufa_build.mk
include $(LUFA_PATH)/Build/lufa_cppcheck.mk
include $(LUFA_PATH)/Build/lufa_doxygen.mk
include $(LUFA_PATH)/Build/lufa_dfu.mk
include $(LUFA_PATH)/Build/lufa_hid.mk
include $(LUFA_PATH)/Build/lufa_avrdude.mk
include $(LUFA_PATH)/Build/lufa_atprogram.mk

avrdude-erase: 
	@echo $(MSG_AVRDUDE_CMD) Erasing device \"$(MCU)\" using \"$(AVRDUDE_PROGRAMMER)\" on port \"$(AVRDUDE_PORT)\"
	avrdude $(BASE_AVRDUDE_FLAGS) -e $(AVRDUDE_FLAGS) -U efuse:w:0xf4:m -U hfuse:w:0xd9:m -U lfuse:w:0xde:m
