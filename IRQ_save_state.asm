.text .align 2
.thumb

	.global IRQ_save_state


IRQ_save_state:
	MRS     R0, PRIMASK ; R0 is used for storing result

EXIT:
	BX      LR
