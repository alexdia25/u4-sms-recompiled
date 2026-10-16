; Bank 24 (ROM $60000-$63FFF), mapped at $8000
.BANK 24 SLOT 2
.ORG $0000

	.INCBIN "bank24.bin" SKIP $0000 READ 16384	; data, 16384 bytes
