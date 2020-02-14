#############################################################

# Discard this section from all parent makefiles
# Expected variables (with automatic defaults):
#   CSRCS (all "C" files in the dir)
#   SUBDIRS (all subdirs with a Makefile)
#   GEN_LIBS - list of libs to be generated ()
#   GEN_IMAGES - list of object file images to be generated ()
#   GEN_BINS - list of binaries to be generated ()
#   COMPONENTS_xxx - a list of libs/objs in the form
#     subdir/lib to be extracted and rolled up into
#     a generated lib/image xxx.a ()
#
TARGET = eagle
#FLAVOR = release
FLAVOR = debug

SDKDIR = ${SDK_PATH}
#EXTRA_CCFLAGS += -u

ifndef PDIR # {
GEN_IMAGES= eagle.app.v6.out
GEN_BINS= eagle.app.v6.bin
SPECIAL_MKTARGETS=$(APP_MKTARGETS)
SUBDIRS=    \
	httpserver \
	user 

endif # } PDIR

APPDIR = .
LDDIR = ./ld

CCFLAGS += -Os

TARGET_LDFLAGS =		\
	-nostdlib		\
	-Wl,-EL \
	--longcalls \
	--text-section-literals

ifeq ($(FLAVOR),debug)
    TARGET_LDFLAGS += -g -O2
endif

ifeq ($(FLAVOR),release)
    TARGET_LDFLAGS += -g -O0
endif

COMPONENTS_eagle.app.v6 = \
	httpserver/libhttpserver.a \
	user/libuser.a 

LINKFLAGS_eagle.app.v6 = \
	-L$(SDKDIR)/lib        \
	-nostdlib	\
    -T$(LD_FILE)   \
	-Wl,--no-check-sections	\
	-Wl,--gc-sections	\
    -u call_user_start	\
	-Wl,-static						\
	-Wl,--start-group					\
	-lc					\
	-lgcc					\
	-lphy	\
	-lpp	\
	-lnet80211	\
	-llwip	\
	-lwpa	\
	-lcrypto	\
	-lmain	\
	-lupgrade\
	-ldriver \
	-lhal					\
	$(DEP_LIBS_eagle.app.v6)					\
	-Wl,--end-group

#	-ljson	\
#	-lsmartconfig \
#	-lmbedtls	\
#	-lpwm	\
	
DEPENDS_eagle.app.v6 = \
                $(LD_FILE) \
                $(LDDIR)/eagle.rom.addr.v6.ld

#############################################################
# Configuration i.e. compile options etc.
# Target specific stuff (defines etc.) goes in here!  # Generally values applying to a tree are captured in the
#   makefile at its root level - these are then overridden
#   for a subtree within the makefile rooted therein
#

#UNIVERSAL_TARGET_DEFINES =		\

# Other potential configuration flags include:
#	-DTXRX_TXBUF_DEBUG
#	-DTXRX_RXBUF_DEBUG
#	-DWLAN_CONFIG_CCX
CONFIGURATION_DEFINES =	-DICACHE_FLASH \
                        -DGLOBAL_DEBUG_ON

DEFINES +=				\
	$(UNIVERSAL_TARGET_DEFINES)	\
	$(CONFIGURATION_DEFINES)

DDEFINES +=				\
	$(UNIVERSAL_TARGET_DEFINES)	\
	$(CONFIGURATION_DEFINES)


#############################################################
# Recursion Magic - Don't touch this!!
#
# Each subtree potentially has an include directory
#   corresponding to the common APIs applicable to modules
#   rooted at that subtree. Accordingly, the INCLUDE PATH
#   of a module can only contain the include directories up
#   its parent path, and not its siblings
#
# Required for each makefile to inherit from the parent
#

INCLUDES := $(INCLUDES) \
	-I $(PDIR)include \
	-I $(PDIR)/httpserver/include 

PDIR = $(SDKDIR)/
sinclude $(SDKDIR)/Makefile

BAUDRATE = 576000
ESPTOOL = esptool.py --baud $(BAUDRATE) write_flash -u --flash_mode qio --flash_freq 40m
erase_flash:
	 $(ESPTOOL) 0x0 $(SDK_PATH)/bin/blank-1mb.bin

#map2user1:
#	make clean
#	make COMPILE=gcc BOOT=new APP=1 SPI_SPEED=40 SPI_MODE=QIO SPI_SIZE_MAP=2
#
#flash_map2user1: map2user1
#	$(ESPTOOL) --flash_size 1MB  \
#		0x0 	$(SDK_PATH)/bin/boot_v1.7.bin \
#		0x1000  $(BIN_DIR)/upgrade/user1.1024.new.2.bin \
#		0xfc000 $(SDK_PATH)/bin/esp_init_data_default_v08.bin \
#		0xfb000 $(SDK_PATH)/bin/blank.bin \
#		0xfe000 $(SDK_PATH)/bin/blank.bin
#
##		0x78000 $(SDK_PATH)/bin/blank.bin \
##		0x79000 $(SDK_PATH)/bin/blank.bin \
##		0x7a000 $(SDK_PATH)/bin/blank.bin \
##		0x7b000 $(SDK_PATH)/bin/blank.bin 
#
#map3user1:
#	make clean
#	make COMPILE=gcc BOOT=new APP=1 SPI_SPEED=40 SPI_MODE=QIO SPI_SIZE_MAP=3
#
#flash_map3user1:
#	$(ESPTOOL) --flash_size 2MB  \
#		0x0 	$(SDK_PATH)/bin/boot_v1.7.bin \
#		0x1000  $(BIN_PATH)/upgrade/user1.2048.new.3.bin \
#		0x1fc000 $(SDK_PATH)/bin/esp_init_data_default_v08.bin \
#		0x1fb000 $(SDK_PATH)/bin/blank.bin \
#		0x1fe000 $(SDK_PATH)/bin/blank.bin \
#		0x78000 $(SDK_PATH)/bin/blank.bin \
#		0x79000 $(SDK_PATH)/bin/blank.bin \
#		0x7a000 $(SDK_PATH)/bin/blank.bin \
#		0x7b000 $(SDK_PATH)/bin/blank.bin 
#
#map5user1:
#	make clean
#	make COMPILE=gcc BOOT=new APP=1 SPI_SPEED=40 SPI_MODE=QIO SPI_SIZE_MAP=5
#
#flash_map5user1: map5user1
#	$(ESPTOOL) --flash_size 2MB  \
#		0x0 	$(SDK_PATH)/bin/boot_v1.7.bin \
#		0x1000  $(BIN_PATH)/upgrade/user1.2048.new.5.bin \
#		0x1fc000 $(SDK_PATH)/bin/esp_init_data_default_v08.bin \
#		0x1fb000 $(SDK_PATH)/bin/blank.bin \
#		0x1fe000 $(SDK_PATH)/bin/blank.bin \

map6user1:
	make clean
	make COMPILE=gcc BOOT=new APP=1 SPI_SPEED=40 SPI_MODE=QIO SPI_SIZE_MAP=6

flash_map6user1: map6user1
	$(ESPTOOL) --flash_size 4MB-c1  \
		0x0 	$(SDK_PATH)/bin/boot_v1.7.bin \
		0x1000  $(BIN_PATH)/upgrade/user1.4096.new.6.bin \
		0x3fc000 $(SDK_PATH)/bin/esp_init_data_default_v08.bin \
		0x3fb000 $(SDK_PATH)/bin/blank.bin \
		0x3fe000 $(SDK_PATH)/bin/blank.bin 

cleanup_map6user1:
	$(ESPTOOL) --flash_size 4MB-c1  \
		0xf8000 $(SDK_PATH)/bin/blank.bin \
		0xf9000 $(SDK_PATH)/bin/blank.bin \
		0xfa000 $(SDK_PATH)/bin/blank.bin 

assets_map6user1:
	$(ESPTOOL) --flash_size 4MB-c1  \
		0x200000 assets/favicon-16x16.png 

fota: map6user2
	-curl 192.168.0.162/firmware -XPOST -F"firmware=@../bin/upgrade/user2.4096.new.6.bin"
	-echo

.PHONY: 
