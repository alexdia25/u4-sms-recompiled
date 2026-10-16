; Bank 22 (ROM $58000-$5BFFF), mapped at $8000
.BANK 22 SLOT 2
.ORG $0000

	.INCBIN "bank22.bin" SKIP $0000 READ 16384	; data, 16384 bytes
