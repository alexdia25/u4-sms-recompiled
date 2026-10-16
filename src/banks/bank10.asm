; Bank 10 (ROM $28000-$2BFFF), mapped at $8000
.BANK 10 SLOT 2
.ORG $0000

	.INCBIN "bank10.bin" SKIP $0000 READ 16384	; data, 16384 bytes
