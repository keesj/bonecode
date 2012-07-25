@
@ Sample raw assembly bare gcc project that can be run on the
@ BeagleBone from Code Composer Studio
@
        .text
        .align
        b main
.equ    UART_BASE, 	0x44e09000    @define a constant that point to the UART register
str:    .asciz "Hello ARM\n\r"    @create a label that points to a 0 terminated string
		.align
main:                             @define the main label where we want the program to start
                                  @at the start of the program we already jump to this main
                                  @function but code composer studio will start the execution
                                  @of a program at that label.
		.global main              @define main to be a global label so CCS can see it
        adr r0 , str              @ Store the address of the string in r0
        bl write_string           @ Call the sub routine

stop:
        b stop                    @loop for ever

        @ Args
        @ r0 Start address of the string
write_string:
		.global write_string
        ldr r2, = UART_BASE @load the address of the uart in r2
loop:
        ldrb r1 , [r0] , #1 @r1 = *r0++
        cmp r1 , #0         @compare r1
        strne r1 , [r2]     @store the value
        bne loop
        mov pc , lr         @jump back
