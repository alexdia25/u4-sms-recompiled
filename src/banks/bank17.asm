; Bank 17 (ROM $44000-$47FFF), mapped at $8000
.BANK 17 SLOT 2
.ORG $0000

	.INCBIN "bank17.bin" SKIP $0000 READ 16384	; data, 16384 bytes
