; Bank 30 (ROM $78000-$7BFFF), mapped at $8000
.BANK 30 SLOT 2
.ORG $0000

	.INCBIN "bank30.bin" SKIP $0000 READ 16384	; data, 16384 bytes
