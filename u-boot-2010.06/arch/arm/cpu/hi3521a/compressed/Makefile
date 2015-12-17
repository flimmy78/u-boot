################################################################################
#    Create By Czyong
################################################################################

PWD           = $(shell pwd)
CROSS_COMPILE = arm-hisiv300-linux-
TOPDIR        = ../../../../..
BINIMAGE      = $(TOPDIR)/u-boot.bin
TARGETDIR     = $(TOPDIR)/../u-boot-tools
################################################################################
CC       := $(CROSS_COMPILE)gcc
AR       := $(CROSS_COMPILE)ar
LD       := $(CROSS_COMPILE)ld
OBJCOPY  := $(CROSS_COMPILE)objcopy

################################################################################
BOOT     := u-boot
TEXTBASE := 0x80700000

CFLAGS   := -g -Os -fno-strict-aliasing -fno-common -ffixed-r8 -fno-aggressive-loop-optimizations \
	-D__KERNEL__ -DTEXT_BASE=$(TEXTBASE) \
	-I$(TOPDIR)/include  \
	-fno-builtin -ffreestanding -nostdinc -isystem /opt/hisi-linux/x86-arm/arm-hisiv300-linux/bin/../lib/gcc/arm-hisiv300-linux-uclibcgnueabi/4.8.3/include \
	-pipe  -DCONFIG_ARM -D__ARM__ -marm  -mabi=aapcs-linux  \
	-mno-thumb-interwork -march=armv7-a -mno-unaligned-access $(MKFLAGS)

################################################################################

START := start.o
COBJS := startup.o \
	cache-cp15.o \
	cache_ops.o \
	mmu.o image_data.o

#SSRC  := arch/arm/cpu/hi3520d/start.S \
#	arch/arm/cpu/hi3520d/lowlevel_init.S \
#	arch/arm/cpu/hi3520d/ddrphy_train_route.S \

SSRC  := 

REG1   := $(wildcard $(TOPDIR)/*.reg1 $(TOPDIR)/.reg1)
#REG2   := $(wildcard $(TOPDIR)/*.reg2 $(TOPDIR)/.reg2)
SRC   := $(notdir $(SSRC))

################################################################################
#.PHONY: $(BOOT).bin
#$(BOOT).bin: $(BOOT).tmp regfile
#	dd if=./$(BOOT).tmp of=./tmp1 bs=1 count=64 2>/dev/null
#	dd if=$(REG1) of=./tmp2 bs=4096 conv=sync 2>/dev/null
#	@dd if=$(REG2) of=./tmp3 bs=2400 conv=sync 2>/dev/null
#	dd if=./$(BOOT).tmp of=./tmp4 bs=1 skip=4160 2>/dev/null
#	@cat tmp1 tmp2 tmp3 tmp4 > $(BOOT).bin
#	cat tmp1 tmp2 tmp4 > $(BOOT).bin
#	@rm -f tmp1 tmp2 tmp3 tmp4
#	@rm -f tmp1 tmp2 tmp3 tmp4
#	@chmod 754 $(BOOT).bin
#	@cp -fv $@ $(TOPDIR)
#	@echo $(BOOT).bin is Ready.

$(BOOT).bin: $(BOOT).elf
	$(OBJCOPY) -O srec $< $(BOOT).srec
	$(OBJCOPY) -j .text -O binary $< $(BOOT).text
	$(OBJCOPY) --gap-fill=0xff -O binary $< $@
	cp $(BOOT).bin $(TARGETDIR)
	
$(BOOT).elf: image_data.lzma $(SRC) $(START) $(COBJS)
	$(LD) -Bstatic -T mini-boot.lds -Ttext $(TEXTBASE) $(START) \
		$(COBJS) -Map $(BOOT).map -o $@

.PHONY: regfile
regfile:
	@if [ "$(words $(REG1))" == "0" ]; then ( \
		echo '***' Need '.reg1' or '*.reg1' file in directory $(TOPDIR); \
		exit 1; \
	) fi
#	@if [ "$(words $(REG2))" == "0" ]; then ( \
		echo '***' Need '.reg2' or '*.reg2' file in directory $(TOPDIR); \
		exit 1; \
	) fi
	@if [ "$(words $(REG1))" != "1" ]; then ( \
		echo '***' Found multi '.reg1' or '*.reg1' file in directory $(TOPDIR); \
		echo '***' Files: $(notdir $(REG1)); \
		exit 1; \
	) fi
#	@if [ "$(words $(REG2))" != "1" ]; then ( \
		echo '***' Found multi '.reg2' or '*.reg2' file in directory $(TOPDIR); \
		echo '***' Files: $(notdir $(REG2)); \
		exit 1; \
	) fi

################################################################################
start.o: start.S
	$(CC) -D__ASSEMBLY__ $(CFLAGS) -o $@ $< -c

image_data.lzma: $(BINIMAGE)
	lzma -fkzc -7 $< > $@

%.o: %.c
	$(CC) $(CFLAGS) -Wall -Wstrict-prototypes \
		-fno-stack-protector -o $@ $< -c

%.o: %.S
	$(CC) -D__ASSEMBLY__ $(CFLAGS) -o $@ $< -c

image_data.o: image_data.S image_data.lzma
	$(CC) -D__ASSEMBLY__ $(CFLAGS) -o $@ $< -c

$(SRC):
	ln -sf ../../../../../$(filter %/$@,$(SSRC)) $@

################################################################################
TMPS := $(COBJS) start.o $(SRC) \
	$(BOOT).map $(BOOT).elf $(BOOT).srec $(BOOT).bin \
	$(BOOT).tmp $(BOOT).text image_data.lzma

distclean: clean

clean:
	@rm -f $(TMPS)

################################################################################
.PHONY: clean
################################################################################
