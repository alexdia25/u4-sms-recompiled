; Bank 27 (ROM $6C000-$6FFFF), mapped at $8000
.BANK 27 SLOT 2
.ORG $0000

	.INCBIN "bank27.bin" SKIP $0000 READ 16384	; data, 16384 bytes
