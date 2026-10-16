; Bank 26 (ROM $68000-$6BFFF), mapped at $8000
.BANK 26 SLOT 2
.ORG $0000

	.INCBIN "bank26.bin" SKIP $0000 READ 16384	; data, 16384 bytes
