; Bank 18 (ROM $48000-$4BFFF), mapped at $8000
.BANK 18 SLOT 2
.ORG $0000

	.INCBIN "bank18.bin" SKIP $0000 READ 16384	; data, 16384 bytes
