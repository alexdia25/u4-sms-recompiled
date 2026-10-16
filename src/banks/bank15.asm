; Bank 15 (ROM $3C000-$3FFFF), mapped at $8000
.BANK 15 SLOT 2
.ORG $0000

	.INCBIN "bank15.bin" SKIP $0000 READ 16384	; data, 16384 bytes
