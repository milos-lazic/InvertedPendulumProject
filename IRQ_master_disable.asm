.text .align 2
.thumb

	.global IRQ_master_disable


IRQ_master_disable:
	CPSID   I

EXIT:
	BX      LR
