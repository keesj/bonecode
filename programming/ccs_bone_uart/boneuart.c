/*
 * Sample code to write to the UART0 of the beaglebone. This code is
 * run from Code Composer Studio just after booting the device.
 * u-boot probably already did setup part of the serial and the output
 * looks like this:
 *
 *  U-Boot SPL 2011.09-00274-g90d48a1 (Apr 22 2012 - 10:26:29)
 *  Texas Instruments Revision detection unimplemented
 *  1
 */
/*
 * Taken from the sitara TRM
 */
#define UART0_BASE 0x44e09000


#define REG(x)(*((volatile unsigned int *)(x)))

void main(void) {
	REG(UART0_BASE) = '1';
}
