; Bank 14 (ROM $38000-$3BFFF), mapped at $8000
.BANK 14 SLOT 2
.ORG $0000

	.INCBIN "bank14.bin" SKIP $0000 READ 16384	; data, 16384 bytes
