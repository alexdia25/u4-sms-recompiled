; Bank 16 (ROM $40000-$43FFF), mapped at $8000
.BANK 16 SLOT 2
.ORG $0000

	.INCBIN "bank16.bin" SKIP $0000 READ 16384	; data, 16384 bytes
