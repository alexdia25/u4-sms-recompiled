; Bank 23 (ROM $5C000-$5FFFF), mapped at $8000
.BANK 23 SLOT 2
.ORG $0000

	.INCBIN "bank23.bin" SKIP $0000 READ 16384	; data, 16384 bytes
