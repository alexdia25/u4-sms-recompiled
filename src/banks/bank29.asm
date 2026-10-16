; Bank 29 (ROM $74000-$77FFF), mapped at $8000
.BANK 29 SLOT 2
.ORG $0000

	.INCBIN "bank29.bin" SKIP $0000 READ 16384	; data, 16384 bytes
