; Bank 4 (ROM $10000-$13FFF), mapped at $8000
.BANK 4 SLOT 2
.ORG $0000

	.INCBIN "bank04.bin" SKIP $0000 READ 16384	; data, 16384 bytes
