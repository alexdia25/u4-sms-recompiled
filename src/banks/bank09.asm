; Bank 9 (ROM $24000-$27FFF), mapped at $8000
.BANK 9 SLOT 2
.ORG $0000

	.INCBIN "bank09.bin" SKIP $0000 READ 16384	; data, 16384 bytes
