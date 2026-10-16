; Bank 5 (ROM $14000-$17FFF), mapped at $8000
.BANK 5 SLOT 2
.ORG $0000

	.INCBIN "bank05.bin" SKIP $0000 READ 16384	; data, 16384 bytes
