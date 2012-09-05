/*
 * Sample code to drive the mmc sub system of the
 * beaglebone.
 */
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

#include "sdmmcreg.h"
#include "sdhcreg.h"
#include "sd.h"

/* TODO: Rename to MMCH_0_REG_BASE and add the base address for the other items */
#define MMCHS1_REG_BASE 0x48060000

#ifdef AM_DM37x_Multimedia_Device
#define MMCHS1_REG_BASE 0x4809C000
#define MMCHS1_REG_BASE 0x480B4000
#define MMCHS1_REG_BASE 0x480AD000
#endif

#define MMCHS_SD_SYSCONFIG 0x110 /* SD system configuration */
#define MMCHS_SD_SYSSTATUS 0x114 /* SD system status */
#define MMCHS_SD_CON       0x12c /* Configuration (functional mode,card initialization etc) */
#define MMCHS_SD_BLK       0x204 /* Transfer length configuration */
#define MMCHS_SD_ARG       0x208 /* Command argument bit 38-8 of command format*/
#define MMCHS_SD_CMD       0x20c /* Command and transfer mode */
#define MMCHS_SD_RSP10     0x210 /* Command response 0 and 1 */
#define MMCHS_SD_RSP32     0x214 /* Command response 2 and 3  */
#define MMCHS_SD_RSP54     0x218 /* Command response 4 and 5  */
#define MMCHS_SD_RSP76     0x21c /* Command response 6 and 7  */
#define MMCHS_SD_DATA      0x220 /* Data register */
#define MMCHS_SD_PSTATE    0x224 /* Present state */
#define MMCHS_SD_HCTL      0x228 /* Host control(power ,wake-up and transfer) */
#define MMCHS_SD_SYSCTL    0x22c /* SD System control (reset,clocks and timeout) */
#define MMCHS_SD_STAT      0x230 /* SD Interrupt status */
#define MMCHS_SD_IE        0x234 /* SD Interrupt Enable register */
#define MMCHS_SD_CAPA      0x240 /* Capabilities of the host controller */

#define MMCHS_SD_SYSCONFIG_AUTOIDLE                    (0x1 << 0)  /* Internal clock gating strategy */
#define MMCHS_SD_SYSCONFIG_AUTOIDLE_DIS                (0x0 << 0)  /* Clocks are free running */
#define MMCHS_SD_SYSCONFIG_AUTOIDLE_EN                 (0x1 << 0)  /* Automatic clock gating strategy */
#define MMCHS_SD_SYSCONFIG_SOFTRESET                   (0x1 << 1)  /* Software reset bit writing  */
#define MMCHS_SD_SYSCONFIG_ENAWAKEUP                   (0x1 << 2)  /* Wake-up feature control */
#define MMCHS_SD_SYSCONFIG_ENAWAKEUP_DIS               (0x0 << 2)  /* Disable wake-up capability */
#define MMCHS_SD_SYSCONFIG_ENAWAKEUP_EN                (0x1 << 2)  /* Enable wake-up capability */
#define MMCHS_SD_SYSCONFIG_SIDLEMODE                   (0x3 << 3)  /* Power management */
#define MMCHS_SD_SYSCONFIG_SIDLEMODE_UNCONDITIONAL     (0x0 << 3)  /* Go into idle mode unconditionally upon request */
#define MMCHS_SD_SYSCONFIG_SIDLEMODE_IGNORE            (0x1 << 3)  /* Ignore ILDE requests */
#define MMCHS_SD_SYSCONFIG_SIDLEMODE_IDLE              (0x2 << 3)  /* Acknowledge IDLE request switch to wake-up mode */
#define MMCHS_SD_SYSCONFIG_SIDLEMODE_SMART_IDLE        (0x3 << 3)  /* Smart-idle */
#define MMCHS_SD_SYSCONFIG_CLOCKACTIVITY               (0x3 << 8)  /* Clock activity during wake-up */
#define MMCHS_SD_SYSCONFIG_CLOCKACTIVITY_OFF           (0x0 << 8)  /* Interface and functional clock can be switched off */
#define MMCHS_SD_SYSCONFIG_CLOCKACTIVITY_IF            (0x1 << 8)  /* Only Interface clock (functional can be switched off*/
#define MMCHS_SD_SYSCONFIG_CLOCKACTIVITY_FUNC          (0x2 << 8)  /* Only Functional clock (interface clock can be switched off) */
#define MMCHS_SD_SYSCONFIG_CLOCKACTIVITY_BOOTH         (0x3 << 8)  /* Booth the interface and functional clock are maintained */
#define MMCHS_SD_SYSCONFIG_STANDBYMODE                 (0x3 << 12) /* Configuration for standby */
#define MMCHS_SD_SYSCONFIG_STANDBYMODE_FORCE_STANDBY   (0x0 << 12) /* Force standby mode upon idle request*/
#define MMCHS_SD_SYSCONFIG_STANDBYMODE_NO_STANDBY      (0x1 << 12) /* Never go into standby mode */
#define MMCHS_SD_SYSCONFIG_STANDBYMODE_WAKEUP_INTERNAL (0x2 << 12) /* Go into wake-up mode based on internal knowledge */
#define MMCHS_SD_SYSCONFIG_STANDBYMODE_WAKEUP_SMART    (0x3 << 12) /* Go info wake-up mode when possible */

#define MMCHS_SD_SYSSTATUS_RESETDONE 0x01

#define MMCHS_SD_CON_DW8          (0x1 << 5) /* 8-bit mode MMC select , For SD clear this bit */
#define MMCHS_SD_CON_DW8_1BIT     (0x0 << 5) /* 1 or 4 bits data width configuration(also set SD_HCTL) */
#define MMCHS_SD_CON_DW8_8BITS    (0x1 << 5) /* 8 bits data width configuration */
#define MMCHS_SD_CON_INIT         (0x1 << 1) /* Send initialization stream (all cards) */
#define MMCHS_SD_CON_INIT_NOINIT  (0x0 << 1) /* Do nothing */
#define MMCHS_SD_CON_INIT_INIT    (0x1 << 1) /* Send initialization stream */

#define MMCHS_SD_BLK_NBLK             (0xffffu << 16) /* Block count for the current transfer */
#define MMCHS_SD_BLK_BLEN             (0xfff << 0)     /* Transfer block size */
#define MMCHS_SD_BLK_BLEN_NOTRANSFER  (0x0 << 0)       /* No transfer */

#define MMCHS_SD_CMD_INDX                 (0x3f << 24) /* Command index */
#define MMCHS_SD_CMD_INDX_CMD(x)          (x << 24)    /* MMC command index binary encoded values from 0 to 63 */

#define MMCHS_SD_ARG_MASK                 (0xffffffffu)      /* Mask everything */
#define MMCHS_SD_ARG_CMD8_VHS             (0x1 << (16 - 8))  /* Voltage between 2.7 and 3.6 v*/
#define MMCHS_SD_ARG_CMD8_CHECK_PATTERN   (0xaa <<(8 - 8))   /* 10101010b pattern */

#define MMCHS_SD_CMD_TYPE                 (0x3 << 22) /* Command type. */
#define MMCHS_SD_CMD_TYPE_OTHER           (0x0 << 22) /* Other type of commands (like go idle) */
#define MMCHS_SD_CMD_TYPE_BUS_SUSPEND     (0x1 << 22) /* Upon CMD52 "Bus Suspend" operation */
#define MMCHS_SD_CMD_TYPE_FUNCTION_SELECT (0x2 << 22) /* Upon CMD52 "Function Select" operation */
#define MMCHS_SD_CMD_TYPE_IOABORT         (0x3 << 22) /* Upon CMD12 and CMD21 "I/O Abort */
#define MMCHS_SD_CMD_DP                   (0x1 << 21) /* Data present select */
#define MMCHS_SD_CMD_DP_DATA              (0x1 << 21) /* Additional data is present on the data lines */
#define MMCHS_SD_CMD_DP_NODATA            (0x0 << 21) /* No additional data is present on the data lines */
#define MMCHS_SD_CMD_CICE                 (0x1 << 20) /* Command index response check enable */
#define MMCHS_SD_CMD_CICE_ENABLE          (0x1 << 20) /* Enable index check response  */
#define MMCHS_SD_CMD_CICE_DISABLE         (0x0 << 20) /* Disable index check response */
#define MMCHS_SD_CMD_CCCE                 (0x1 << 19) /* Command CRC7 Check enable on responses*/
#define MMCHS_SD_CMD_CCCE_ENABLE          (0x1 << 19) /* Enable CRC7 Check on response */
#define MMCHS_SD_CMD_CCCE_DISABLE         (0x0 << 19) /* Disable CRC7 Check on response */
#define MMCHS_SD_CMD_RSP_TYPE             (0x3 << 16) /* Response type */
#define MMCHS_SD_CMD_RSP_TYPE_NO_RESP     (0x0 << 16) /* No response */
#define MMCHS_SD_CMD_RSP_TYPE_136B        (0x1 << 16) /* Response length 136 bits */
#define MMCHS_SD_CMD_RSP_TYPE_48B         (0x2 << 16) /* Response length 48 bits */
#define MMCHS_SD_CMD_RSP_TYPE_48B_BUSY    (0x3 << 16) /* Response length 48 bits with busy after response */
#define MMCHS_SD_CMD_MSBS                 (0x1 << 5)  /* Multi/Single block select */
#define MMCHS_SD_CMD_MSBS_SINGLE          (0x0 << 5)  /* Single block mode */
#define MMCHS_SD_CMD_MSBS_MULTI           (0x0 << 5)  /* Multi block mode */
#define MMCHS_SD_CMD_DDIR                 (0x1 << 4)  /* Data transfer direction */
#define MMCHS_SD_CMD_DDIR_READ            (0x1 << 4)  /* Data read (card to host) */
#define MMCHS_SD_CMD_DDIR_WRITE           (0x0 << 4)  /* Data write (host to card)  */
#define MMCHS_SD_CMD_ACEN                 (0x1 << 2)  /* Auto CMD12 Enable */
#define MMCHS_SD_CMD_ACEN_DIS             (0x0 << 2)  /* Auto CMD12 Disable */
#define MMCHS_SD_CMD_ACEN_EN              (0x1 << 2)  /* Auto CMD12 Enable */
#define MMCHS_SD_CMD_BCE                  (0x1 << 1)  /* Block Count Enable(for multi block transfer) */
#define MMCHS_SD_CMD_BCE_DIS              (0x0 << 1)  /* Disabled block count for infinite transfer*/
#define MMCHS_SD_CMD_BCE_EN               (0x1 << 1)  /* Enabled for multi block transfer with know amount of blocks */
#define MMCHS_SD_CMD_DE                   (0x1 << 0)  /* DMA enable */
#define MMCHS_SD_CMD_DE_DIS               (0x0 << 0)  /* Disable DMA */
#define MMCHS_SD_CMD_DE_EN                (0x1 << 0)  /* Enable DMA  */
#define MMCHS_SD_CMD_MASK  				   ~(0x1 << 30  | 0x1 << 31 | 0x1 << 18 | 0x1 <<3) /* bits 30 , 31 and 18 are reserved */

#define MMCHS_SD_PSTATE_CI           (0x1 << 16) /* Card Inserted */
#define MMCHS_SD_PSTATE_CI_INSERTED  (0x1 << 16) /* Card Inserted  is inserted*/
#define MMCHS_SD_PSTATE_BRE          (0x0 << 11) /* Buffer read enable */
#define MMCHS_SD_PSTATE_BRE_DIS      (0x0 << 11) /* Read BLEN bytes disabled*/
#define MMCHS_SD_PSTATE_BRE_EN       (0x1 << 11) /* Read BLEN bytes enabled*/
#define MMCHS_SD_PSTATE_BWE          (0x0 << 10) /* Buffer Write enable */
#define MMCHS_SD_PSTATE_BWE_DIS      (0x0 << 10) /* There is no room left in the buffer to write BLEN bytes of data */
#define MMCHS_SD_PSTATE_BWE_EN       (0x1 << 10) /* There is enough space in the buffer to write BLEN bytes of data*/

#define MMCHS_SD_HCTL_DTW            (0x1 << 1) /*Data transfer width.(must be set after a successful ACMD6) */
#define MMCHS_SD_HCTL_DTW_1BIT       (0x0 << 1) /*1 bit transfer with */
#define MMCHS_SD_HCTL_DTW_4BIT       (0x1 << 1) /*4 bit transfer with */
#define MMCHS_SD_HCTL_SDBP           (0x1 << 8) /*SD bus power */
#define MMCHS_SD_HCTL_SDBP_OFF       (0x0 << 8) /*SD Power off (start card detect?) */
#define MMCHS_SD_HCTL_SDBP_ON        (0x1 << 8) /*SD Power on (start card detect?) */
#define MMCHS_SD_HCTL_SDVS           (0x7 << 9) /*SD bus voltage select */
#define MMCHS_SD_HCTL_SDVS_VS18      (0x5 << 9) /*1.8 V */
#define MMCHS_SD_HCTL_SDVS_VS30      (0x6 << 9) /*3.0 V */
#define MMCHS_SD_HCTL_SDVS_VS33      (0x7 << 9) /*3.3 V */
#define MMCHS_SD_HCTL_IWE            (0x1 << 24)/* wake-up event on SD interrupt */
#define MMCHS_SD_HCTL_IWE_DIS        (0x0 << 24)/* Disable wake-up on SD interrupt */
#define MMCHS_SD_HCTL_IWE_EN         (0x1 << 24)/* Enable wake-up on SD interrupt */

#define MMCHS_SD_SYSCTL_CLKD (0x3ff << 6)  /* 10 bits clock frequency select */
#define MMCHS_SD_SYSCTL_SRD  (0x1   << 26)  /* Soft reset for mmc_dat line */
#define MMCHS_SD_SYSCTL_SRC  (0x1   << 25)  /* Soft reset for mmc_cmd line */
#define MMCHS_SD_SYSCTL_SRA  (0x1   << 24)  /* Soft reset all (host controller) */

#define MMCHS_SD_SYSCTL_ICE     (0x1 << 0) /* Internal clock enable register  */
#define MMCHS_SD_SYSCTL_ICE_DIS (0x0 << 0) /* Disable internal clock */
#define MMCHS_SD_SYSCTL_ICE_EN  (0x1 << 0) /* Enable internal clock */
#define MMCHS_SD_SYSCTL_ICS          (0x1 << 1) /* Internal clock stable register  */
#define MMCHS_SD_SYSCTL_ICS_UNSTABLE (0x0 << 1) /* Internal clock is unstable */
#define MMCHS_SD_SYSCTL_ICS_STABLE   (0x1 << 1) /* Internal clock is stable   */
#define MMCHS_SD_SYSCTL_CEN          (0x1 << 2) /* Card lock enable provide clock to the card */
#define MMCHS_SD_SYSCTL_CEN_DIS      (0x0 << 2) /* Internal clock is unstable */
#define MMCHS_SD_SYSCTL_CEN_EN       (0x1 << 2) /* Internal clock is stable   */

#define MMCHS_SD_SYSCTL_DTO          (0xf << 16) /* Data timeout counter  */
#define MMCHS_SD_SYSCTL_DTO_2POW13   (0x0 << 16) /* TCF x 2^13  */
#define MMCHS_SD_SYSCTL_DTO_2POW14   (0x1 << 16) /* TCF x 2^14  */
#define MMCHS_SD_SYSCTL_DTO_2POW27   (0x3 << 16) /* TCF x 2^27  */

#define MMCHS_SD_STAT_ERRI            (0x01 << 15) /* Error interrupt */
#define MMCHS_SD_STAT_ERROR_MASK     (0xff << 15 | 0x3 << 24 | 0x03 << 28)
#define MMCHS_SD_STAT_CC              (0x1 << 0) /* Command complete status */
#define MMCHS_SD_STAT_CC_UNRAISED     (0x0 << 0) /* Command not completed */
#define MMCHS_SD_STAT_CC_RAISED       (0x1 << 0) /* Command completed */

#define MMCHS_SD_IE_ERROR_MASK     (0xff << 15 | 0x3 << 24 | 0x03 << 28)

#define MMCHS_SD_IE_CC_ENABLE        (0x1 << 0) /* Command complete interrupt enable */
#define MMCHS_SD_IE_CC_ENABLE_ENABLE (0x1 << 0) /* Command complete Interrupts are enabled */
#define MMCHS_SD_IE_CC_ENABLE_CLEAR  (0x1 << 0) /* Clearing is done by writing a 0x1 */

#define MMCHS_SD_IE_TC_ENABLE        (0x1 << 1) /* Transfer complete interrupt enable */
#define MMCHS_SD_IE_TC_ENABLE_ENABLE (0x1 << 1) /* Transfer complete Interrupts are enabled */
#define MMCHS_SD_IE_TC_ENABLE_CLEAR  (0x1 << 1) /* Clearing TC is done by writing a 0x1 */

#define MMCHS_SD_IE_BRR_ENABLE         (0x1 << 5) /* Buffer read ready interrupt  */
#define MMCHS_SD_IE_BRR_ENABLE_DISABLE (0x0 << 5) /* Buffer read ready interrupt disable */
#define MMCHS_SD_IE_BRR_ENABLE_ENABLE  (0x1 << 5) /* Buffer read ready interrupt enable */
#define MMCHS_SD_IE_BRR_ENABLE_CLEAR   (0x1 << 5) /* Buffer read ready interrupt clear */

#define MMCHS_SD_IE_BWR_ENABLE         (0x1 << 4) /* Buffer write ready interrupt  */
#define MMCHS_SD_IE_BWR_ENABLE_DISABLE (0x0 << 4) /* Buffer write ready interrupt disable */
#define MMCHS_SD_IE_BWR_ENABLE_ENABLE  (0x1 << 4) /* Buffer write ready interrupt enable */
#define MMCHS_SD_IE_BWR_ENABLE_CLEAR   (0x1 << 4) /* Buffer write ready interrupt clear */

#define MMCHS_SD_CAPA_VS_MASK (0x7 << 24 )  /* voltage mask */
#define MMCHS_SD_CAPA_VS18 (0x01 << 26 )  /* 1.8 volt */
#define MMCHS_SD_CAPA_VS30 (0x01 << 25 )  /* 3.0 volt */
#define MMCHS_SD_CAPA_VS33 (0x01 << 24 )  /* 3.3 volt */

#define REG(x)(*((volatile uint32_t *)(x)))
#define BIT(x)(0x1 << x)

/**
 * Write a uint32_t value to a memory address
 */
inline void write32(uint32_t address, uint32_t value)
{
	REG(address) = value;
}

/**
 * Read an uint32_t from a memory address
 */
inline uint32_t read32(uint32_t address)
{
	return REG(address);
}

/**
 * Set a 32 bits value depending on a mask
 */
inline void set32(uint32_t address, uint32_t mask, uint32_t value)
{
	uint32_t val;
	val = read32(address);
	val &= ~(mask); /* clear the bits */
	val |= (value & mask); /* apply the value using the mask */
	write32(address, val);
}

static uint32_t base_address;

int mmchs_init(uint32_t instance)
{

	int counter;
	counter = 0;

	/* Set the base address to use */
	base_address = MMCHS1_REG_BASE;
	/*
	 * Soft reset of the controller
	 */
	/* Write 1 to sysconfig[0] to trigger a reset*/
	set32(base_address + MMCHS_SD_SYSCONFIG, MMCHS_SD_SYSCONFIG_SOFTRESET,
			MMCHS_SD_SYSCONFIG_SOFTRESET);

	/* read sysstatus to know it's done */
	while (!(read32(base_address + MMCHS_SD_SYSSTATUS)
			& MMCHS_SD_SYSSTATUS_RESETDONE)) {
		counter++;
	}

	/*
	 * Set SD default capabilities
	 */
	set32(base_address + MMCHS_SD_CAPA, MMCHS_SD_CAPA_VS_MASK,
			MMCHS_SD_CAPA_VS18 | MMCHS_SD_CAPA_VS30);

	/*
	 * wake-up configuration
	 */
	set32(
			base_address + MMCHS_SD_SYSCONFIG,
			MMCHS_SD_SYSCONFIG_AUTOIDLE | MMCHS_SD_SYSCONFIG_ENAWAKEUP
					| MMCHS_SD_SYSCONFIG_STANDBYMODE
					| MMCHS_SD_SYSCONFIG_CLOCKACTIVITY
					| MMCHS_SD_SYSCONFIG_SIDLEMODE,
			MMCHS_SD_SYSCONFIG_AUTOIDLE_EN /* Automatic clock gating strategy */
			| MMCHS_SD_SYSCONFIG_ENAWAKEUP_EN /*  Enable wake-up capability */
			| MMCHS_SD_SYSCONFIG_SIDLEMODE_IDLE /*  Smart-idle */
			| MMCHS_SD_SYSCONFIG_CLOCKACTIVITY_OFF /* Booth the interface and functional can be switched off */
			| MMCHS_SD_SYSCONFIG_STANDBYMODE_WAKEUP_INTERNAL /* Go info wake-up mode when possible */
			);

	/* Wake-up on sd interrupt SDIO */
	set32(base_address + MMCHS_SD_HCTL, MMCHS_SD_HCTL_IWE,
			MMCHS_SD_HCTL_IWE_EN);

	/*
	 * MMC host and bus configuration
	 */

	/* Configure data and command transfer (1 bit mode)*/
	set32(base_address + MMCHS_SD_CON, MMCHS_SD_CON_DW8,
			MMCHS_SD_CON_DW8_1BIT);
	set32(base_address + MMCHS_SD_HCTL, MMCHS_SD_HCTL_DTW,
			MMCHS_SD_HCTL_DTW_1BIT);

	/* Configure card voltage  */
	set32(base_address + MMCHS_SD_HCTL, MMCHS_SD_HCTL_SDVS,
			MMCHS_SD_HCTL_SDVS_VS30 /* Configure 3.0 volt */
			);

	/* Power on the host controller and wait for the  MMCHS_SD_HCTL_SDBP_POWER_ON to be set */
	set32(base_address + MMCHS_SD_HCTL, MMCHS_SD_HCTL_SDBP,
			MMCHS_SD_HCTL_SDBP_ON);

	while ((read32(base_address + MMCHS_SD_HCTL) & MMCHS_SD_HCTL_SDBP)
			!= MMCHS_SD_HCTL_SDBP_ON) {
		counter++;
	}

	/* Enable internal clock and clock to the card*/
	set32(base_address + MMCHS_SD_SYSCTL, MMCHS_SD_SYSCTL_ICE,
			MMCHS_SD_SYSCTL_ICE_EN);

	//@TODO Fix external clock enable , this one is very slow
	set32(base_address + MMCHS_SD_SYSCTL, MMCHS_SD_SYSCTL_CLKD,
			(0x3ff << 6));
	set32(base_address + MMCHS_SD_SYSCTL, MMCHS_SD_SYSCTL_CEN,
			MMCHS_SD_SYSCTL_CEN_EN);
	while ((read32(base_address + MMCHS_SD_SYSCTL) & MMCHS_SD_SYSCTL_ICS)
			!= MMCHS_SD_SYSCTL_ICS_STABLE)
		;

	/*
	 * See spruh73e page 3576  Card Detection, Identification, and Selection
	 */

	/* enable command interrupt */
	set32(base_address + MMCHS_SD_IE, MMCHS_SD_IE_CC_ENABLE,
			MMCHS_SD_IE_CC_ENABLE_ENABLE);
	/* enable transfer complete interrupt */
	set32(base_address + MMCHS_SD_IE, MMCHS_SD_IE_TC_ENABLE,
			MMCHS_SD_IE_TC_ENABLE_ENABLE);

	/* enable error interrupts */
	/* NOTE: We are currently skipping the BADA interrupt it does get raised for unknown reasons */
	set32(base_address + MMCHS_SD_IE, MMCHS_SD_IE_ERROR_MASK, 0x0fffffffu);
	//set32(base_address + MMCHS_SD_IE,MMCHS_SD_IE_ERROR_MASK, 0xffffffffu);

	/* clean the error interrupts */
	set32(base_address + MMCHS_SD_STAT, MMCHS_SD_STAT_ERROR_MASK,
			0xffffffffu); // clear errors
	//set32(base_address + MMCHS_SD_STAT,MMCHS_SD_STAT_ERROR_MASK, 0xffffffffu);// clear errors

	/* send a init signal to the host controller. This does not actually
	 * send a command to a card manner
	 */
	set32(base_address + MMCHS_SD_CON, MMCHS_SD_CON_INIT,
			MMCHS_SD_CON_INIT_INIT);
	write32(base_address + MMCHS_SD_CMD, 0x00); /* command 0 , type other commands , not response etc) */

	while ((read32(base_address + MMCHS_SD_STAT) & MMCHS_SD_STAT_CC)
			!= MMCHS_SD_STAT_CC_RAISED) {
		if (read32(base_address + MMCHS_SD_STAT) & 0x8000) {
			printf("%s, error stat  %x\n", __FUNCTION__,
					read32(base_address + MMCHS_SD_STAT));
			return 1;
		}
		counter++;
	}

	/* clear the cc interrupt status */
	set32(base_address + MMCHS_SD_STAT, MMCHS_SD_IE_CC_ENABLE,
			MMCHS_SD_IE_CC_ENABLE_ENABLE);

	/*
	 * Set Set SD_CON[1] INIT bit to 0x0 to end the initialization sequence
	 */
	set32(base_address + MMCHS_SD_CON, MMCHS_SD_CON_INIT,
			MMCHS_SD_CON_INIT_NOINIT);
	return 0;
}

int mmchs_send_cmd(uint32_t command, uint32_t arg)
{
	int count = 0;

	/* Read current interrupt status and fail it an interrupt is already asserted */
	if ((read32(base_address + MMCHS_SD_STAT) & 0xffffu)) {
		printf("%s, interrupt already raised stat  %08x\n", __FUNCTION__,
				read32(base_address + MMCHS_SD_STAT));
		return 1;
	}

	/* Set arguments */
	write32(base_address + MMCHS_SD_ARG, arg);
	/* Set command */
	set32(base_address + MMCHS_SD_CMD, MMCHS_SD_CMD_MASK, command);

	/* Wait for completion */
	while ((read32(base_address + MMCHS_SD_STAT) & 0xffffu) == 0x0) {
		count++;
	}

	if (read32(base_address + MMCHS_SD_STAT) & 0x8000) {
		printf("%s, error stat  %08x\n", __FUNCTION__,
				read32(base_address + MMCHS_SD_STAT));
		set32(base_address + MMCHS_SD_STAT, MMCHS_SD_STAT_ERROR_MASK,
				0xffffffffu);	// clear errors
		// We currently only support 2.0, not responding to
		return 1;
	}

	if ((command & MMCHS_SD_CMD_RSP_TYPE) == MMCHS_SD_CMD_RSP_TYPE_48B_BUSY) {
		/*
		 * Command with busy response *CAN* also set the TC bit if they exit busy
		 */
		while ((read32(base_address + MMCHS_SD_STAT)
				& MMCHS_SD_IE_TC_ENABLE_ENABLE) == 0) {
			count++;
		}
		write32(base_address + MMCHS_SD_STAT, MMCHS_SD_IE_TC_ENABLE_CLEAR);
	}

	/* clear the cc status */
	write32(base_address + MMCHS_SD_STAT, MMCHS_SD_IE_CC_ENABLE_CLEAR);
	return 0;
}

int mmc_send_cmd(struct mmc_command *c)
{

	/* convert the command to a hsmmc command */
	int ret;
	uint32_t cmd, arg;
	cmd = MMCHS_SD_CMD_INDX_CMD(c->cmd);
	arg = c->args;

	switch (c->resp_type) {
		case RESP_LEN_48_CHK_BUSY:
			cmd |= MMCHS_SD_CMD_RSP_TYPE_48B_BUSY;
			break;
		case RESP_LEN_48:
			cmd |= MMCHS_SD_CMD_RSP_TYPE_48B;
			break;
		case RESP_LEN_136:
			cmd |= MMCHS_SD_CMD_RSP_TYPE_136B;
			break;
		case NO_RESPONSE:
			cmd |= MMCHS_SD_CMD_RSP_TYPE_NO_RESP;
			break;
		default:
			return 1;
	}

	ret = mmchs_send_cmd(cmd, arg);

	/* copy response into cmd->resp we don't really
	 * care at this stage about the response length
	 * and copy everything. We also put the value in the array
	 * in such a way that it mimics a memory mapped register
	 */
	c->resp[3] = read32(base_address + MMCHS_SD_RSP10);
	c->resp[2] = read32(base_address + MMCHS_SD_RSP32);
	c->resp[1] = read32(base_address + MMCHS_SD_RSP54);
	c->resp[0] = read32(base_address + MMCHS_SD_RSP76);
	return ret;
}

static struct mmc_command command;

int card_goto_idle_state()
{
	command.cmd = MMC_GO_IDLE_STATE;
	command.resp_type = NO_RESPONSE;
	command.args = 0x00;
	if (mmc_send_cmd(&command)) {
		// Failure
		return 1;
	}
	return 0;
}

int card_identification()
{
	command.cmd = MMC_SEND_EXT_CSD;
	command.resp_type = RESP_LEN_48;
	command.args = MMCHS_SD_ARG_CMD8_VHS | MMCHS_SD_ARG_CMD8_CHECK_PATTERN;

	if (mmc_send_cmd(&command)) {
		// We currently only support 2.0,
		return 1;
	}

	if (!(command.resp[3]
			== (MMCHS_SD_ARG_CMD8_VHS | MMCHS_SD_ARG_CMD8_CHECK_PATTERN))) {
		printf("%s, check pattern check failed  %x\n", __FUNCTION__);
		return 1;
	}
	return 0;
}

int card_query_voltage_and_type(struct sd_card * card)
{

	command.cmd = MMC_APP_CMD;
	command.resp_type = RESP_LEN_48;
	command.args = 0x0; /* RCA=0000 */
	if (mmc_send_cmd(&command)) {
		return 1;
	}

	command.cmd = SD_APP_OP_COND;
	command.resp_type = RESP_LEN_48;

	/* 0x1 << 30 == send HCS (Host capacity support) and get OCR register */
	command.args = (0x3F << 15) | (0x1 << 30); /* RCA=0000 */
	if (mmc_send_cmd(&command)) {
		return 1;
	}

	while (1) {		//@TODO wait for max 1
		command.cmd = MMC_APP_CMD;
		command.resp_type = RESP_LEN_48;
		command.args = 0x0; /* RCA=0000 */
		if (mmc_send_cmd(&command)) {
			return 1;
		}

		/* Send ADMD41 */
		/* 0x1 << 30 == send HCS (Host capacity support) and get OCR register */
		command.cmd = SD_APP_OP_COND;
		command.resp_type = RESP_LEN_48;
		/* 0x1 << 30 == send HCS (Host capacity support) */
		command.args = (0x1FF << 15) | (0x1 << 30);
		if (mmc_send_cmd(&command)) {
			return 1;
		}

		/* if bit 31 is set the response is valid */
		if ((command.resp[3] & (0x1u << 31))) {
			break;
		}

	}
	card->ocr = command.resp[3];
	return 0;
}

int card_identify(struct sd_card * card)
{

	/* Send cmd 2 (all_send_cid) and expect 136 bits response*/
	command.cmd = MMC_ALL_SEND_CID;
	command.resp_type = RESP_LEN_136;
	command.args = 0x0; /* RCA=0000 */

	if (mmc_send_cmd(&command)) {
		return 1;
	}

	card->cid[0] = command.resp[3];
	card->cid[1] = command.resp[2];
	card->cid[2] = command.resp[1];
	card->cid[3] = command.resp[0];

	command.cmd = MMC_SET_RELATIVE_ADDR;
	command.resp_type = RESP_LEN_48;
	command.args = 0x0; /* RCA=0000 */

	/* R6 response */
	if (mmc_send_cmd(&command)) {
		return 1;
	}

	card->rca = (command.resp[3] & 0xffff0000u) >> 16;

	/* MMHCS only supports a single card so sending MMCHS_SD_CMD_CMD2 is useless
	 * Still we should make it possible in the API to support multiple cards
	 */

	return 0;
}

int card_csd(struct sd_card *card)
{
	/* send_csd -> r2 response */
	command.cmd = MMC_SEND_CSD;
	command.resp_type = RESP_LEN_136;
	command.args = MMC_ARG_RCA(card->rca); /* card rca */

	if (mmc_send_cmd(&command)) {
		return 1;
	}

	card->csd[0] = command.resp[0];
	card->csd[1] = command.resp[1];
	card->csd[2] = command.resp[2];
	card->csd[3] = command.resp[3];

	/* sanity check */
	if (((card->csd[0] >> 30) & 0x3) != 0x1) {
		printf("Version 2.0 of CSD register expected\n");
		return 1;
	}
	long c_size = (card->csd[2] >> 16) | ((card->csd[1] & 0x3F) << 16);
	printf("size = %lu bytes\n", (c_size + 1) * 512 * 1024);
	return 0;
}

int select_card(struct sd_card *card)
{

	command.cmd = MMC_SELECT_CARD;
	command.resp_type = RESP_LEN_48_CHK_BUSY;
	command.args = MMC_ARG_RCA(card->rca); /* card rca */

	if (mmc_send_cmd(&command)) {
		return 1;
	}

	return 0;
}

int read_single_block(struct sd_card *card, uint32_t blknr, unsigned char * buf)
{
	uint32_t count;
	uint32_t value;

	count = 0;

	set32(base_address + MMCHS_SD_IE, MMCHS_SD_IE_BRR_ENABLE,
			MMCHS_SD_IE_BRR_ENABLE_ENABLE);

	set32(base_address + MMCHS_SD_BLK, MMCHS_SD_BLK_BLEN, 512);


	if (mmchs_send_cmd(MMCHS_SD_CMD_INDX_CMD(MMC_READ_BLOCK_SINGLE) /* read single block */
	| MMCHS_SD_CMD_DP_DATA /* Command with data transfer */
	| MMCHS_SD_CMD_RSP_TYPE_48B /* type (R1) */
	| MMCHS_SD_CMD_MSBS_SINGLE /* single block */
	| MMCHS_SD_CMD_DDIR_READ /* read data from card */
	, blknr)) {
		return 1;
	}

	while ((read32(base_address + MMCHS_SD_STAT)
			& MMCHS_SD_IE_BRR_ENABLE_ENABLE) == 0) {
		count++;
	}

	if (!(read32(base_address + MMCHS_SD_PSTATE) & MMCHS_SD_PSTATE_BRE_EN)) {
		return 1; /* We are not allowed to read data from the data buffer */
	}

	for (count = 0; count < 512; count += 4) {
		value = read32(base_address + MMCHS_SD_DATA);
		buf[count] = *((char*) &value);
		buf[count + 1] = *((char*) &value + 1);
		buf[count + 2] = *((char*) &value + 2);
		buf[count + 3] = *((char*) &value + 3);
	}

	/* Wait for TC */

	while ((read32(base_address + MMCHS_SD_STAT)
			& MMCHS_SD_IE_TC_ENABLE_ENABLE) == 0) {
		count++;
	}
	write32(base_address + MMCHS_SD_STAT, MMCHS_SD_IE_TC_ENABLE_CLEAR);

	/* clear and disable the bbr interrupt */
	write32(base_address + MMCHS_SD_STAT, MMCHS_SD_IE_BRR_ENABLE_CLEAR);
	set32(base_address + MMCHS_SD_IE, MMCHS_SD_IE_BRR_ENABLE,
			MMCHS_SD_IE_BRR_ENABLE_DISABLE);
	return 0;
}

int write_single_block(struct sd_card *card,
		uint32_t blknr,
		unsigned char * buf)
{
	uint32_t count;
	uint32_t value;

	count = 0;

	set32(base_address + MMCHS_SD_IE, MMCHS_SD_IE_BWR_ENABLE,
			MMCHS_SD_IE_BWR_ENABLE_ENABLE);
	//set32(base_address + MMCHS_SD_IE, 0xfff , 0xfff);
	set32(base_address + MMCHS_SD_BLK, MMCHS_SD_BLK_BLEN, 512);

	/* Set timeout */
	set32(base_address + MMCHS_SD_SYSCTL, MMCHS_SD_SYSCTL_DTO,
			MMCHS_SD_SYSCTL_DTO_2POW27);

	if (mmchs_send_cmd(MMCHS_SD_CMD_INDX_CMD(MMC_WRITE_BLOCK_SINGLE) /* write single block */
	| MMCHS_SD_CMD_DP_DATA /* Command with data transfer */
	| MMCHS_SD_CMD_RSP_TYPE_48B /* type (R1b) */
	| MMCHS_SD_CMD_MSBS_SINGLE /* single block */
	| MMCHS_SD_CMD_DDIR_WRITE /* write to the card */
	, blknr)) {
		return 1;
	}

	/* Wait for the MMCHS_SD_IE_BWR_ENABLE interrupt */
	while ((read32(base_address + MMCHS_SD_STAT) & MMCHS_SD_IE_BWR_ENABLE)
			== 0) {
		count++;
	}

	if (!(read32(base_address + MMCHS_SD_PSTATE) & MMCHS_SD_PSTATE_BWE_EN)) {
		return 1; /* not ready to write data */
	}
	for (count = 0; count < 512; count += 4) {
		*((char*) &value) = buf[count];
		*((char*) &value + 1) = buf[count + 1];
		*((char*) &value + 2) = buf[count + 2];
		*((char*) &value + 3) = buf[count + 3];
		write32(base_address + MMCHS_SD_DATA, value);
	}

	/* Wait for TC */
	while ((read32(base_address + MMCHS_SD_STAT)
			& MMCHS_SD_IE_TC_ENABLE_ENABLE) == 0) {
		count++;
	}
	write32(base_address + MMCHS_SD_STAT, MMCHS_SD_IE_TC_ENABLE_CLEAR);
	write32(base_address + MMCHS_SD_STAT, MMCHS_SD_IE_CC_ENABLE_CLEAR);/* finished.  */
	/* clear the bwr interrupt FIXME is this right when writing?*/
	write32(base_address + MMCHS_SD_STAT, MMCHS_SD_IE_BWR_ENABLE_CLEAR);
	set32(base_address + MMCHS_SD_IE, MMCHS_SD_IE_BWR_ENABLE,
			MMCHS_SD_IE_BWR_ENABLE_DISABLE);
	return 0;
}

int main(void)
{
	struct sd_card card;
	int i;

	unsigned char buf[1024];
	memset(buf, 0, 1024);

	if (mmchs_init(1)) {
		printf("Failed to initialize the host controller\n");
		return 1;
	}

	if (card_goto_idle_state()) {
		printf("Failed to go into idle state\n");
		return 1;
	}
	if (card_identification()) {
		printf("Failed to go card_identification\n");
		return 1;
	}
	if (card_query_voltage_and_type(&card)) {
		printf("Failed to go card_query_voltage_and_type\n");
		return 1;
	}
	if (card_identify(&card)) {
		printf("Failed to identify card\n");
		return 1;
	}
	/* We have now initialized the hardware identified the card */
	if (card_csd(&card)) {
		printf("failed to read csd (card specific data)\n");
		return 1;
	}
	if (select_card(&card)) {
		printf("Failed to select card\n");
		return 1;
	}

	if (read_single_block(&card, 0, buf)) {
		printf("Failed to read a single block\n");
		return 1;
	}

	/* check signature */
	if (!(buf[0x01fe] == 0x55 && buf[0x01ff] == 0xaa)) {
		printf("Failed to find MBR signature\n");
		return 1;
	}

	for (i = 0; i < 512; i++) {
		buf[i] = i % 256;
	}

	if (read_single_block(&card, 0, buf)) {
		printf("Failed to read a single block\n");
		return 1;
	}

	/* check signature */
	if (!(buf[0x01fe] == 0x55 && buf[0x01ff] == 0xaa)) {
		printf("Failed to find MBR signature\n");
		return 1;
	}

	/* DESCUCTIVE... */
	for (i = 0; i < 512; i++) {
		buf[i] = i % 256;
	}

	if (write_single_block(&card, 0xfffff, buf)) {
		printf("Failed to write a single block\n");
		return 1;
	}

	for (i = 0; i < 512; i++) {
		buf[i] = 0;
	}

	if (read_single_block(&card, 0xfffff, buf)) {
		printf("Failed to write a single block (check)\n");
		return 1;
	}

	for (i = 0; i < 512; i++) {
		if (!buf[i] == i % 256) {
			printf("Failed to write a single block and read it again \n");
			return 1;
		}
	}
	printf("Finished\n");
	return 0;
}
