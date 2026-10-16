; Bank 20 (ROM $50000-$53FFF), mapped at $8000
.BANK 20 SLOT 2
.ORG $0000

	.INCBIN "bank20.bin" SKIP $0000 READ 16384	; data, 16384 bytes
