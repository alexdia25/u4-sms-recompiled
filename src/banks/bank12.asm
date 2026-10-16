; Bank 12 (ROM $30000-$33FFF), mapped at $8000
.BANK 12 SLOT 2
.ORG $0000

	.INCBIN "bank12.bin" SKIP $0000 READ 16384	; data, 16384 bytes
