; Bank 6 (ROM $18000-$1BFFF), mapped at $8000
.BANK 6 SLOT 2
.ORG $0000

	.INCBIN "bank06.bin" SKIP $0000 READ 16384	; data, 16384 bytes
