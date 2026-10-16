; Bank 13 (ROM $34000-$37FFF), mapped at $8000
.BANK 13 SLOT 2
.ORG $0000

	.INCBIN "bank13.bin" SKIP $0000 READ 16384	; data, 16384 bytes
