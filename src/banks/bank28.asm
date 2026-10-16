; Bank 28 (ROM $70000-$73FFF), mapped at $8000
.BANK 28 SLOT 2
.ORG $0000

	.INCBIN "bank28.bin" SKIP $0000 READ 16384	; data, 16384 bytes
