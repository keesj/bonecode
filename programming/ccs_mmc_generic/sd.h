struct sd_card
{
	uint32_t cid[4]; /* Card Identification */
	uint32_t rca; /* Relative card address */
	uint32_t dsr; /* Driver stage register */
	uint32_t csd[4]; /* Card specific data */
	uint32_t scr[2]; /* SD configuration */
	uint32_t ocr; /* Operation conditions */
	uint32_t ssr[5]; /* SD Status */
	uint32_t csr; /* Card status */
};

/* struct representing an mmc command */
struct mmc_command
{
	uint32_t cmd;
	uint32_t args;
	uint32_t resp_type;

#define  RESP_LEN_48_CHK_BUSY (3<<0)
#define  RESP_LEN_48		  (2<<0)
#define  RESP_LEN_136		  (1<<0)
#define  NO_RESPONSE		  (0<<0)

	uint32_t resp[4];
	unsigned char* data;
	uint32_t data_len;
};

//int send_cmd(struct sd_card* card, struct mmc_command *);
