.text .align 2
.thumb

	.global IRQ_master_enable


IRQ_master_enable:
	CPSIE   I

EXIT:
	BX      LR
