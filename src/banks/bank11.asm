; Bank 11 (ROM $2C000-$2FFFF), mapped at $8000
.BANK 11 SLOT 2
.ORG $0000

	.INCBIN "bank11.bin" SKIP $0000 READ 16384	; data, 16384 bytes
