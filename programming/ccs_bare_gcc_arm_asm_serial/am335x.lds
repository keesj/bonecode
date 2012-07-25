/* Linker script for the beaglebone */
MEMORY {
	gpmc : ORIGIN =  0x1000000, LENGTH = 0x1FFFFFFF  /* GPMC external memory 512M address space*/
	ram : ORIGIN =  0x8000000, LENGTH = 0xBFFFFFFF  /* EMIF0_SDRAM external memory sdram 1G address space*/
	l3ocmc : ORIGIN = 0x40300000 , LENGTH = 0x0000FFFF /* 64K L3 OCMO memory */
};


SECTIONS {
	.text : { * (.text) } > l3ocmc
	.data : { * (.data) } > l3ocmc	
}