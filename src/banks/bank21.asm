; Bank 21 (ROM $54000-$57FFF), mapped at $8000
.BANK 21 SLOT 2
.ORG $0000

	.INCBIN "bank21.bin" SKIP $0000 READ 16384	; data, 16384 bytes
