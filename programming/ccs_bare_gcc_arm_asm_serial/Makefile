default:main.bin

#
# Makefile for creating a stanalone binary  
# that can be used to run directly from the
# u-boot prompt.
#
#CROSS_COMPILE=arm-elf-
CROSS_COMPILE=arm-angstrom-linux-gnueabi-
%.o:%.S Makefile
	${CROSS_COMPILE}as   -o  $@ $< 

%.elf:%.o
	${CROSS_COMPILE}ld  -T am335x.lds    -o $@ $<

%.bin:%.elf
	${CROSS_COMPILE}objcopy -O binary $< $@


