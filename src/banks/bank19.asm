; Bank 19 (ROM $4C000-$4FFFF), mapped at $8000
.BANK 19 SLOT 2
.ORG $0000

	.INCBIN "bank19.bin" SKIP $0000 READ 16384	; data, 16384 bytes
