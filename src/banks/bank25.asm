; Bank 25 (ROM $64000-$67FFF), mapped at $8000
.BANK 25 SLOT 2
.ORG $0000

	.INCBIN "bank25.bin" SKIP $0000 READ 16384	; data, 16384 bytes
