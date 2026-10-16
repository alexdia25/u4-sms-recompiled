; Bank 7 (ROM $1C000-$1FFFF), mapped at $8000
.BANK 7 SLOT 2
.ORG $0000

	.INCBIN "bank07.bin" SKIP $0000 READ 16384	; data, 16384 bytes
