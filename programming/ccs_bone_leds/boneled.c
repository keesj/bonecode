/*
 * Sample code to drive the leds on the beaglebone.
 *
 * Understanding the initialization flow was done
 * by studying the TI StartWare gpioLEDBlink.c example.
 *
 * Missing from this example is the PIN muxing we need to
 * better understand this.
 */

/**
 * Mapped registers for CM_PER All taken from the Sitara TRM
 *
 */
#define CM_PER_REGS            (0x44E00000)
#define CM_PER_L4LS_CLKSTCTRL  (CM_PER_REGS + 0)    //This register enables the domain power state transition.
#define CM_PER_GPIO1_CLKCTRL   (CM_PER_REGS + 0xac) //This register manages the GPIO1 clocks.

#define CM_PER_GPIO1_CLKCTRL_OPTFCLKEN_GPIO_1_GDBCLK_SHIFT (0x12)
#define CM_PER_GPIO1_CLKCTRL_OPTFCLKEN_GPIO_1_GDBCLK_FLK_EN (0x1 << CM_PER_GPIO1_CLKCTRL_OPTFCLKEN_GPIO_1_GDBCLK_SHIFT)

#define CM_PER_L4LS_CLKSTCTRL_CLKACTIVITY_GPIO_1_GDBCLK (0x80000)


#define GPIO_CTRL 0x130           //GPIO Module control register
#define GPIO_OE 0x134             //GPIO Output enable
#define GPIO_CLEARDATAOUT 0x190	  //CLEAR DATA (Writing 1 clears data)
#define GPIO_SETDATAOUT 0x194     //SET data (writing 1 sets bits)
#define GPIO_DATAIN 0x138         //DATAIN reading from this returns the current pin value
#define GPIO_SYSCONFIG 0x10	      //The GPIO_SYSCONFIG register controls the various parameters of the L4 interconnect.
#define GPIO_SYSSTATUS 0x114	  //The GPIO_SYSSTATUS register provides the reset status information about the GPIO module.


#define GPIO_CTRL_DISABLEMODULE 0x1
#define GPIO_SYSSTATUS_RESETDONE 0x1
#define GPIO_SYSCONFIG_SOFTRESET 0x2

#define GPIO1_BASE 0x4804C000


#define REG(x)(*((volatile unsigned int *)(x)))

/**
 * Enable the functional clocks in the GPIO1 domain.
 * Before this core is called you can not write to the memory
 * mapped registers of the GPIO1 framework
 */
void enableGPIO1Clocks(){
	// SW_WKUP : SW_WKUP: Start a software forced wake-up transition on GPIO1 the domain.
	REG(CM_PER_GPIO1_CLKCTRL) =  (REG(CM_PER_L4LS_CLKSTCTRL) & ~ 0x3) | 0x2;
	// Wait for the transition to be complete
	while( (REG(CM_PER_GPIO1_CLKCTRL) & 0x2) != 0x2);

	// Enable optional function clock
	 REG(CM_PER_GPIO1_CLKCTRL) |= CM_PER_GPIO1_CLKCTRL_OPTFCLKEN_GPIO_1_GDBCLK_FLK_EN;
	 //Wait for functional clock to be enabled
	 while ( (REG(CM_PER_GPIO1_CLKCTRL) & CM_PER_GPIO1_CLKCTRL_OPTFCLKEN_GPIO_1_GDBCLK_FLK_EN) !=  CM_PER_GPIO1_CLKCTRL_OPTFCLKEN_GPIO_1_GDBCLK_FLK_EN);

	 //Next wait for the module status (IDLEST CM_PER_GPIO1_CLKCTRL bit 17 and 16) to be functional.
	 while (REG(CM_PER_GPIO1_CLKCTRL) & 0x30000 );

	 // Also wait for the GPIO1_GDBCLK to be active
	 while( (REG(CM_PER_L4LS_CLKSTCTRL)  &  CM_PER_L4LS_CLKSTCTRL_CLKACTIVITY_GPIO_1_GDBCLK) != CM_PER_L4LS_CLKSTCTRL_CLKACTIVITY_GPIO_1_GDBCLK );
}

/**
 * Enable and reset the GPIO1 block
 */
void enableGPIO1Block(){
	/* Enable GPIO Module by clearing GPIO_CTRL_DISABLEMODULE*/
	REG(GPIO1_BASE + GPIO_CTRL) &= ~(GPIO_CTRL_DISABLEMODULE);

	/* Do a soft reset and wait for the rest to be complete */
	REG(GPIO1_BASE + GPIO_SYSCONFIG) |= GPIO_SYSCONFIG_SOFTRESET;
	while ( (REG(GPIO1_BASE + GPIO_SYSSTATUS) & GPIO_SYSSTATUS_RESETDONE) !=  GPIO_SYSSTATUS_RESETDONE);
}

/**
 * Simple delay function
 */
void delay(volatile int count){
	while(count--);
}

void main(void) {
	int x;

	//Clocks
	enableGPIO1Clocks();

	//Pin muxing
	//@TODO understand pin muxing. In our setup it just "happened" to work

	/* Enable output on pin 21 to 24 the leds on the beaglebone */
	REG(GPIO1_BASE + GPIO_OE) &= ~(0xf << 21);

	/* Display a binary lock */
	for (x = 0 ; x < 16 ; x ++){
		REG(GPIO1_BASE + GPIO_CLEARDATAOUT) = 0xffffff;
		REG(GPIO1_BASE + GPIO_SETDATAOUT) =  (x << 21);
		delay(0x3fffff);
	}
}
