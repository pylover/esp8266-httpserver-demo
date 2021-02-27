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

HOST ?= home.nodemcu
COMPILE ?= gcc

SPI_SIZE_MAP := 6
TARGET = eagle
#FLAVOR = release
FLAVOR = debug

#EXTRA_CCFLAGS += -u

ifndef PDIR # {
GEN_IMAGES= eagle.app.v6.out
GEN_BINS= eagle.app.v6.bin
SPECIAL_MKTARGETS=$(APP_MKTARGETS)
SUBDIRS=    \
	user \
	httpd \
	http \
	uns 

endif # } PDIR

BINDIR = ./bin
APPDIR = .
LDDIR = $(SDK_PATH)/ld

CCFLAGS += -Os -DSPI_SIZE_MAP=$(SPI_SIZE_MAP)

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
	user/libuser.a \
	httpd/libhttpd.a \
	http/libhttp.a \
	uns/libuns.a

LINKFLAGS_eagle.app.v6 = \
	-L$(SDK_PATH)/lib        \
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
	-I $(PDIR)/httpd/include \
	-I $(PDIR)/http/include \
	-I $(PDIR)/uns/include 

PDIR = $(SDK_PATH)/
sinclude $(SDK_PATH)/Makefile

BAUDRATE := 115200
FLASH_BAUDRATE := 460800
PORT := /dev/ttyUSB0
ESPTOOL = esptool.py --baud $(FLASH_BAUDRATE) --port $(PORT)
ESPTOOL_WRITE = $(ESPTOOL)  write_flash -u --flash_mode qio --flash_freq 40m
ESPTOOL_WRITE_DIO = $(ESPTOOL)  write_flash -u --flash_mode dio --flash_freq 40m

erase_flash:
	 $(ESPTOOL) erase_flash

screen:
	screen $(PORT) $(BAUDRATE)

map2user1:
	make clean
	make COMPILE=gcc BOOT=new APP=1 SPI_SPEED=40 SPI_MODE=QIO SPI_SIZE_MAP=2

.PHONY: assets_map2user1
assets_map2user1:
	$(ESPTOOL_WRITE) --flash_size 4MB-c1  \
		0x77000 assets/favicon-16x16.png

flash_map2user1: map2user1
	$(ESPTOOL_WRITE) --flash_size 1MB  \
		0x0 	$(SDK_PATH)/bin/boot_v1.7.bin \
		0x1000  $(BINDIR)/upgrade/user1.1024.new.2.bin \
		0xfc000 $(SDK_PATH)/bin/esp_init_data_default_v08.bin \
		0xfb000 $(SDK_PATH)/bin/blank.bin \
		0xfe000 $(SDK_PATH)/bin/blank.bin

#		0x78000 $(SDK_PATH)/bin/blank.bin \
#		0x79000 $(SDK_PATH)/bin/blank.bin \
#		0x7a000 $(SDK_PATH)/bin/blank.bin \
#		0x7b000 $(SDK_PATH)/bin/blank.bin 

.PHONY: cleanup_map2user1_params
cleanup_map2user1_params:
	$(ESPTOOL_WRITE) --flash_size 1MB  \
		0x78000 $(SDK_PATH)/bin/blank.bin \
		0x79000 $(SDK_PATH)/bin/blank.bin \
		0x7a000 $(SDK_PATH)/bin/blank.bin 


.PHONY: map2user1dio
map2user1dio:
	make clean
	make COMPILE=gcc BOOT=new APP=1 SPI_SPEED=40 SPI_MODE=DIO SPI_SIZE_MAP=2

.PHONY: flash_map2user1dio
flash_map2user1dio: map2user1dio
	$(ESPTOOL_WRITE_DIO) --flash_size 1MB  \
		0x0 	$(SDK_PATH)/bin/boot_v1.7.bin \
		0x1000  $(BINDIR)/upgrade/user1.1024.new.2.bin \
		0xfc000 $(SDK_PATH)/bin/esp_init_data_default_v08.bin \
		0xfb000 $(SDK_PATH)/bin/blank.bin \
		0xfe000 $(SDK_PATH)/bin/blank.bin



.PHONY: bootfotamap2
bootfotamap2:
	$(ESPTOOL) read_flash 0xff000 0x1000 $(BINDIR)/map2-user1-0ff.bin
	printf '\x01' \
		| dd of=$(BINDIR)/map2-user1-0ff.bin bs=1 count=1 conv=notrunc
	$(ESPTOOL_WRITE) --flash_size 1MB \
		0xff000 $(BINDIR)/map2-user1-0ff.bin

.PHONY: bootappmap2
bootappmap2:
	$(ESPTOOL) read_flash 0xff000 0x1000 $(BINDIR)/map2-user2-0ff.bin
	printf '\x00' \
		| dd of=$(BINDIR)/map2-user2-0ff.bin bs=1 count=1 conv=notrunc
	$(ESPTOOL_WRITE) --flash_size 1MB \
		0xff000 $(BINDIR)/map2-user2-0ff.bin


#map3user1:
#	make clean
#	make COMPILE=gcc BOOT=new APP=1 SPI_SPEED=40 SPI_MODE=QIO SPI_SIZE_MAP=3
#
#flash_map3user1:
#	$(ESPTOOL_WRITE) --flash_size 2MB  \
#		0x0 	$(SDK_PATH)/bin/boot_v1.7.bin \
#		0x1000  $(BINDIR)/upgrade/user1.2048.new.3.bin \
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
#	$(ESPTOOL_WRITE) --flash_size 2MB  \
#		0x0 	$(SDK_PATH)/bin/boot_v1.7.bin \
#		0x1000  $(BINDIR)/upgrade/user1.2048.new.5.bin \
#		0x1fc000 $(SDK_PATH)/bin/esp_init_data_default_v08.bin \
#		0x1fb000 $(SDK_PATH)/bin/blank.bin \
#		0x1fe000 $(SDK_PATH)/bin/blank.bin \

.PHONY: map6user1dio
map6user1dio:
	make clean
	make COMPILE=gcc BOOT=new APP=1 SPI_SPEED=40 SPI_MODE=DIO SPI_SIZE_MAP=6

.PHONY: flash_map6user1dio
flash_map6user1dio: map6user1dio
	$(ESPTOOL_WRITE_DIO) --flash_size 4MB-c1  \
		0x0 	$(SDK_PATH)/bin/boot_v1.7.bin \
		0x1000  $(BINDIR)/upgrade/user1.4096.new.6.bin \
		0x3fc000 $(SDK_PATH)/bin/esp_init_data_default_v08.bin \
		0x3fb000 $(SDK_PATH)/bin/blank.bin \
		0x3fe000 $(SDK_PATH)/bin/blank.bin 


.PHONY: map6user1
map6user1:
	make clean
	make COMPILE=gcc BOOT=new APP=1 SPI_SPEED=40 SPI_MODE=QIO SPI_SIZE_MAP=6


.PHONY: map6user2
map6user2:
	make clean
	make COMPILE=gcc BOOT=new APP=2 SPI_SPEED=40 SPI_MODE=QIO SPI_SIZE_MAP=6


.PHONY: bootfotamap6
bootfotamap6:
	$(ESPTOOL) read_flash 0x3ff000 0x1000 $(BINDIR)/map6-user1-3ff.bin
	printf '\x01' \
		| dd of=$(BINDIR)/map6-user1-3ff.bin bs=1 count=1 conv=notrunc
	$(ESPTOOL_WRITE) --flash_size 4MB-c1 \
		0x3ff000 $(BINDIR)/map6-user1-3ff.bin


.PHONY: bootappmap6
bootappmap6:
	$(ESPTOOL) read_flash 0x3ff000 0x1000 $(BINDIR)/map6-user2-3ff.bin
	printf '\x00' \
		| dd of=$(BINDIR)/map6-user2-3ff.bin bs=1 count=1 conv=notrunc
	$(ESPTOOL_WRITE) --flash_size 4MB-c1 \
		0x3ff000 $(BINDIR)/map6-user2-3ff.bin

	
.PHONY: flash_map6user1
flash_map6user1: map6user1
	$(ESPTOOL_WRITE) --flash_size 4MB-c1  \
		0x0 	$(SDK_PATH)/bin/boot_v1.7.bin \
		0x1000  $(BINDIR)/upgrade/user1.4096.new.6.bin \
		0x3fc000 $(SDK_PATH)/bin/esp_init_data_default_v08.bin \
		0x3fb000 $(SDK_PATH)/bin/blank.bin \
		0x3fe000 $(SDK_PATH)/bin/blank.bin 

.PHONY: cleanup_map6user1_params
cleanup_map6user1_params:
	$(ESPTOOL_WRITE) --flash_size 4MB-c1  \
		0xf8000 $(SDK_PATH)/bin/blank.bin \
		0xf9000 $(SDK_PATH)/bin/blank.bin \
		0xfa000 $(SDK_PATH)/bin/blank.bin 

.PHONY: assets_map6user1
assets_map6user1:
	$(ESPTOOL_WRITE) --flash_size 4MB-c1  \
		0x200000 assets/favicon-16x16.png 

.PHONY: erase_flash
earase_flash:
	$(ESPTOOL) earase_flash

.PHONY: rebootfota
rebootfota:
	-curl `uns resolve --short $(HOST)`/ -XFOTA

.PHONY: fotamap6
fotamap6: map6user2 rebootfota
	@sleep 3
	-curl -F firmware=@"$(BINDIR)/upgrade/user2.4096.new.6.bin" \
		`uns resolve --short $(HOST)`/firmware
	-echo

.PHONY: fotamap2
fotamap2: map2user2 rebootfota
	@sleep 5
	-curl -F firmware=@"$(BINDIR)/upgrade/user2.1024.new.2.bin" \
		`uns resolve --short $(HOST)`/firmware
	-echo


.PHONY: fota
fota: fotamap6
