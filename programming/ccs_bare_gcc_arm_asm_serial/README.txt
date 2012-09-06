The sample it this directory was created to be run from Code Composer studio.
Beware that the dot cproject file does contain some hardcoded values for the
path to the linker script


It is also possible to run the code directly from u-boot. For that you need to
use the Makefile to generate a binary that can be uploaded. You can get the
start address (the symbol called main) from first running make main.elf and
running objdump -t or readelf -s on that file.

U-Boot# mmc rescan
U-Boot# mmc list
OMAP SD/MMC: 0
U-Boot# mmc part

Partition Map for MMC device 0  --   Partition Type: DOS

Partition     Start Sector     Num Sectors     Type
    1                 2048          102400       c
    2               104448         7759872      83
U-Boot# fatls mmc 0:1
    38024   mlo
       56   main.bin
   255256   u-boot.img

3 file(s), 0 dir(s)

U-Boot# fatload mmc 0:1 0x40300000 main.bin
reading main.bin

56 bytes read
U-Boot# jk
Unknown command 'jk' - try 'help'
U-Boot# go 0x40300010
## Starting application at 0x40300010 ...
Hello ARM

