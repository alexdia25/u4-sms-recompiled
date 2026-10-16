; Bank 8 (ROM $20000-$23FFF), mapped at $8000
.BANK 8 SLOT 2
.ORG $0000

	.INCBIN "bank08.bin" SKIP $0000 READ 16384	; data, 16384 bytes
