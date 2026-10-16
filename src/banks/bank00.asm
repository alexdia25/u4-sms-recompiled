; Bank 0 (ROM $00000-$03FFF), mapped at $0000
.BANK 0 SLOT 0
.ORG $0000


fn_00_0000:
	di	; $0000
	im 1	; $0001
	jp reset	; $0003
	.INCBIN "bank00.bin" SKIP $0006 READ 2	; data, 2 bytes

fn_00_0008:
	jp loc_00_006F	; $0008

fn_00_000B:
	or a	; $000B
	ret p	; $000C
	neg	; $000D
	ret	; $000F

fn_00_0010:
	jp loc_00_0078	; $0010
	.INCBIN "bank00.bin" SKIP $0013 READ 5	; data, 5 bytes

fn_00_0018:
	jp loc_00_0083	; $0018
	.INCBIN "bank00.bin" SKIP $001B READ 5	; data, 5 bytes

fn_00_0020:
	jp loc_00_008E	; $0020
	.INCBIN "bank00.bin" SKIP $0023 READ 5	; data, 5 bytes

fn_00_0028:
	add a,l	; $0028
	ld l,a	; $0029
	adc a,h	; $002A
	sub l	; $002B
	ld h,a	; $002C
	ld a,(hl)	; $002D
	ret	; $002E
	.INCBIN "bank00.bin" SKIP $002F READ 1	; data, 1 bytes

fn_00_0030:
	add a,l	; $0030
	ld l,a	; $0031
	ret nc	; $0032
	inc h	; $0033
	ret	; $0034
	.INCBIN "bank00.bin" SKIP $0035 READ 3	; data, 3 bytes

fn_00_0038:
	push af	; $0038
	in a,($BF)	; $0039
	or a	; $003B
	jp p,loc_00_040B	; $003C
	jp irq_handler	; $003F

reset:
	ld hl,$FFFC	; $0042

fn_00_0045:
	ld (hl),$80	; $0045
	inc l	; $0047
	ld (hl),$00	; $0048
	inc l	; $004A

fn_00_004B:
	ld (hl),$01	; $004B
	inc l	; $004D

fn_00_004E:
	ld (hl),$02	; $004E
	ld hl,$C001	; $0050
	ld de,$C002	; $0053
	ld bc,$1FEE	; $0056
	ld (hl),$00	; $0059
	ldir	; $005B
	ld sp,$DFF0	; $005D
	call fn_00_009C	; $0060
	jp loc_00_0CA6	; $0063

fn_00_0066:
	push af	; $0066
	ld a,$FF	; $0067
	ld ($C003),a	; $0069
	pop af	; $006C

fn_00_006D:
	retn	; $006D

loc_00_006F:
	di	; $006F
	ld a,l	; $0070
	out ($BF),a	; $0071
	ld a,h	; $0073
	out ($BF),a	; $0074
	ei	; $0076
	ret	; $0077

loc_00_0078:
	di	; $0078
	ld a,e	; $0079
	out ($BF),a	; $007A
	ld a,d	; $007C
	or $40	; $007D
	out ($BF),a	; $007F
	ei	; $0081
	ret	; $0082

loc_00_0083:
	add a,a	; $0083
	add a,l	; $0084
	ld l,a	; $0085
	adc a,h	; $0086
	sub l	; $0087
	ld h,a	; $0088
	ld a,(hl)	; $0089
	inc hl	; $008A
	ld h,(hl)	; $008B
	ld l,a	; $008C
	jp (hl)	; $008D

loc_00_008E:
	add a,a	; $008E
	jr nc,loc_00_0092	; $008F
	inc h	; $0091

loc_00_0092:
	add a,l	; $0092
	ld l,a	; $0093
	adc a,h	; $0094
	sub l	; $0095
	ld h,a	; $0096

fn_00_0097:
	ld a,(hl)	; $0097
	inc hl	; $0098
	ld h,(hl)	; $0099
	ld l,a	; $009A
	ret	; $009B

fn_00_009C:
	ld hl,$0142	; $009C
	ld bc,$0B7F	; $009F
	otir	; $00A2
	ld hl,$C005	; $00A4
	ld a,$F5	; $00A7
	out ($3F),a	; $00A9
	in a,($DD)	; $00AB
	and $C0	; $00AD
	cp $C0	; $00AF
	jr nz,loc_00_00C4	; $00B1
	ld a,$55	; $00B3
	out ($3F),a	; $00B5
	in a,($DD)	; $00B7
	and $C0	; $00B9
	jr nz,loc_00_00C4	; $00BB
	ld a,$FF	; $00BD
	out ($3F),a	; $00BF
	ld (hl),a	; $00C1
	jr loc_00_00D5	; $00C2

loc_00_00C4:
	ld (hl),$00	; $00C4
	ld d,$02	; $00C6

loc_00_00C8:
	ld bc,$C000	; $00C8

loc_00_00CB:
	ex (sp),hl	; $00CB
	ex (sp),hl	; $00CC
	dec bc	; $00CD
	ld a,b	; $00CE
	or c	; $00CF
	jr nz,loc_00_00CB	; $00D0
	dec d	; $00D2
	jr nz,loc_00_00C8	; $00D3

loc_00_00D5:
	di	; $00D5
	call fn_00_014D	; $00D6
	ld hl,$0000	; $00D9

loc_00_00DC:
	in a,($BF)	; $00DC
	or a	; $00DE
	jp p,loc_00_00DC	; $00DF

loc_00_00E2:
	in a,($BF)	; $00E2
	or a	; $00E4
	jp p,loc_00_00E2	; $00E5

loc_00_00E8:
	inc hl	; $00E8
	in a,($BF)	; $00E9
	or a	; $00EB
	jp p,loc_00_00E8	; $00EC
	xor a	; $00EF
	ld de,$0800	; $00F0
	sbc hl,de	; $00F3
	rla	; $00F5
	ld ($C001),a	; $00F6
	ld a,($C000)	; $00F9
	cpl	; $00FC
	and $E8	; $00FD

fn_00_00FF:
	cp $20	; $00FF

fn_00_0101:
	jp z,loc_00_0113	; $0101
	cp $40	; $0104
	jp z,loc_00_0113	; $0106
	cp $80	; $0109
	jp z,loc_00_0113	; $010B
	ld a,$A8	; $010E
	ld ($C000),a	; $0110

loc_00_0113:
	ld a,($C000)	; $0113
	or $04	; $0116
	out ($3E),a	; $0118
	ld bc,$0700	; $011A

loc_00_011D:
	ld a,b	; $011D
	and $01	; $011E
	out ($F2),a	; $0120
	ld e,a	; $0122
	in a,($F2)	; $0123
	and $07	; $0125
	cp e	; $0127
	jr nz,loc_00_012B	; $0128
	inc c	; $012A

loc_00_012B:
	djnz loc_00_011D	; $012B
	ld a,c	; $012D
	cp $07	; $012E
	jr z,loc_00_0133	; $0130
	xor a	; $0132

loc_00_0133:
	and $01	; $0133
	out ($F2),a	; $0135
	ld ($C002),a	; $0137
	ld a,($C000)	; $013A
	out ($3E),a	; $013D
	jp loc_00_063D	; $013F
	.INCBIN "bank00.bin" SKIP $0142 READ 11	; data, 11 bytes

fn_00_014D:
	di	; $014D
	in a,($BF)	; $014E
	ld hl,$019E	; $0150
	ld de,$C013	; $0153
	ld bc,$000B	; $0156
	ldir	; $0159
	ld hl,$C013	; $015B
	ld bc,$16BF	; $015E
	ld a,$80	; $0161

loc_00_0163:
	outi	; $0163
	out ($BF),a	; $0165
	inc a	; $0167
	djnz loc_00_0163	; $0168
	ld a,$00	; $016A
	out ($BF),a	; $016C
	ld a,$C0	; $016E
	out ($BF),a	; $0170
	ld hl,$C023	; $0172
	xor a	; $0175
	ld b,$20	; $0176

loc_00_0178:
	out ($BE),a	; $0178
	ld (hl),a	; $017A
	inc hl	; $017B
	djnz loc_00_0178	; $017C
	ld a,$00	; $017E
	out ($BF),a	; $0180
	ld a,$7F	; $0182
	out ($BF),a	; $0184
	ld a,$D0	; $0186
	out ($BE),a	; $0188
	xor a	; $018A
	ld ($C007),a	; $018B
	ld ($C00F),a	; $018E
	ld ($C010),a	; $0191
	ld hl,$019D	; $0194
	ld (irq_callback),hl	; $0197
	call fn_00_0453	; $019A
	ret	; $019D
	.INCBIN "bank00.bin" SKIP $019E READ 11	; data, 11 bytes

fn_00_01A9:
	push hl	; $01A9
	ld a,c	; $01AA
	add a,$13	; $01AB
	ld l,a	; $01AD
	ld h,$C0	; $01AE
	ld a,b	; $01B0
	di	; $01B1
	ld (hl),a	; $01B2
	out ($BF),a	; $01B3
	ld a,c	; $01B5
	or $80	; $01B6
	out ($BF),a	; $01B8
	ei	; $01BA
	pop hl	; $01BB
	ret	; $01BC
	.INCBIN "bank00.bin" SKIP $01BD READ 6	; data, 6 bytes

loc_00_01C3:
	ld b,a	; $01C3
	ld c,$01	; $01C4
	call fn_00_01A9	; $01C6
	pop bc	; $01C9
	ret	; $01CA

fn_00_01CB:
	push bc	; $01CB
	ld a,($C014)	; $01CC
	or $20	; $01CF
	jr loc_00_01C3	; $01D1
	.INCBIN "bank00.bin" SKIP $01D3 READ 16	; data, 16 bytes

fn_00_01E3:
	push bc	; $01E3
	ld a,($C013)	; $01E4
	and $EF	; $01E7

loc_00_01E9:
	ld b,a	; $01E9
	ld c,$00	; $01EA
	call fn_00_01A9	; $01EC
	pop bc	; $01EF
	ret	; $01F0

fn_00_01F1:
	push bc	; $01F1
	ld a,($C013)	; $01F2
	or $10	; $01F5
	jr loc_00_01E9	; $01F7
	.INCBIN "bank00.bin" SKIP $01F9 READ 8	; data, 8 bytes

fn_00_0201:
	push bc	; $0201
	ld a,($C013)	; $0202
	or $20	; $0205
	jr loc_00_01E9	; $0207
	.INCBIN "bank00.bin" SKIP $0209 READ 7	; data, 7 bytes

fn_00_0210:
	push af	; $0210
	rst $10	; $0211
	pop af	; $0212

fn_00_0213:
	call fn_00_0262	; $0213
	out ($BE),a	; $0216
	ret	; $0218

fn_00_0219:
	push hl	; $0219
	ld l,a	; $021A
	rst $10	; $021B
	call fn_00_0259	; $021C

loc_00_021F:
	out (c),l	; $021F
	nop	; $0221
	djnz loc_00_021F	; $0222
	dec a	; $0224
	jp nz,loc_00_021F	; $0225
	pop hl	; $0228
	ret	; $0229

loc_00_022A:
	rst $10	; $022A
	call fn_00_0262	; $022B

loc_00_022E:
	ld a,l	; $022E
	out ($BE),a	; $022F
	call fn_00_0262	; $0231
	ld a,h	; $0234
	out ($BE),a	; $0235
	dec bc	; $0237
	ld a,b	; $0238
	or c	; $0239
	jp nz,loc_00_022E	; $023A
	ret	; $023D
	.INCBIN "bank00.bin" SKIP $023E READ 14	; data, 14 bytes

fn_00_024C:
	rst $10	; $024C
	call fn_00_0259	; $024D

loc_00_0250:
	outi	; $0250
	jr nz,loc_00_0250	; $0252
	dec a	; $0254
	jp nz,loc_00_0250	; $0255
	ret	; $0258

fn_00_0259:
	ld a,b	; $0259
	ld b,c	; $025A
	ld c,$BE	; $025B
	inc b	; $025D
	dec b	; $025E
	ret z	; $025F
	inc a	; $0260
	ret	; $0261

fn_00_0262:
	push af	; $0262
	pop af	; $0263
	ret	; $0264

fn_00_0265:
	xor a	; $0265
	ld ($C0A8),a	; $0266
	ld l,a	; $0269
	ld a,($C0A8)	; $026A
	ld h,a	; $026D
	ld de,$3800	; $026E
	ld bc,$0380	; $0271
	jp loc_00_022A	; $0274

fn_00_0277:
	ld de,$0000	; $0277
	ld bc,$2000	; $027A
	xor a	; $027D
	jp fn_00_0219	; $027E

fn_00_0281:
	ld de,$2000	; $0281
	ld bc,$1800	; $0284
	xor a	; $0287
	jp fn_00_0219	; $0288

fn_00_028B:
	ld ($C0A8),a	; $028B
	ld c,(hl)	; $028E
	inc hl	; $028F
	ld b,(hl)	; $0290
	inc hl	; $0291

loc_00_0292:
	push bc	; $0292
	rst $10	; $0293
	ld b,c	; $0294

loc_00_0295:
	ld a,($C0A9)	; $0295
	or a	; $0298
	ld a,(hl)	; $0299
	inc hl	; $029A
	jr z,loc_00_02AA	; $029B
	or a	; $029D
	jr nz,loc_00_02AA	; $029E
	in a,($BE)	; $02A0
	call fn_00_0262	; $02A2
	in a,($BE)	; $02A5
	jp loc_00_02B4	; $02A7

loc_00_02AA:
	out ($BE),a	; $02AA
	call fn_00_0262	; $02AC
	ld a,($C0A8)	; $02AF
	out ($BE),a	; $02B2

loc_00_02B4:
	djnz loc_00_0295	; $02B4
	ex de,hl	; $02B6
	ld bc,$0040	; $02B7
	add hl,bc	; $02BA
	ex de,hl	; $02BB
	pop bc	; $02BC
	djnz loc_00_0292	; $02BD
	ret	; $02BF

irq_handler:
	ld ($C012),a	; $02C0
	push bc	; $02C3
	push de	; $02C4
	push hl	; $02C5
	exx	; $02C6
	ex af,af'	; $02C7
	push af	; $02C8
	push bc	; $02C9
	push de	; $02CA
	push hl	; $02CB
	push ix	; $02CC
	push iy	; $02CE
	ld c,$BE	; $02D0
	ld hl,$FFFC	; $02D2
	ld e,(hl)	; $02D5
	ld a,(slot2_bank)	; $02D6
	ld d,a	; $02D9
	push de	; $02DA
	ld (hl),$80	; $02DB
	ld hl,vblank_count	; $02DD
	inc (hl)	; $02E0
	ld a,(port_dd_prev)	; $02E1
	ld b,a	; $02E4
	in a,($DD)	; $02E5
	ld (port_dd_prev),a	; $02E7
	cpl	; $02EA
	and b	; $02EB
	bit 4,a	; $02EC
	jp nz,loc_00_0CB4	; $02EE
	ld a,($C007)	; $02F1
	or a	; $02F4
	jp p,loc_00_0310	; $02F5
	and $7F	; $02F8
	ld ($C007),a	; $02FA
	ld hl,$C014	; $02FD
	jr z,loc_00_0307	; $0300
	set 6,(hl)	; $0302
	jp loc_00_0309	; $0304

loc_00_0307:
	res 6,(hl)	; $0307

loc_00_0309:
	ld a,(hl)	; $0309
	out ($BF),a	; $030A
	ld a,$81	; $030C
	out ($BF),a	; $030E

loc_00_0310:
	jp z,loc_00_03EA	; $0310
	ld a,($C008)	; $0313
	or a	; $0316
	jp z,loc_00_03EA	; $0317
	xor a	; $031A
	ld ($C008),a	; $031B
	ld hl,($C00F)	; $031E
	ld ($C01B),hl	; $0321
	ld a,l	; $0324
	out ($BF),a	; $0325
	ld a,$88	; $0327
	out ($BF),a	; $0329
	ld a,h	; $032B
	out ($BF),a	; $032C
	ld a,$89	; $032E
	out ($BF),a	; $0330
	ld a,$00	; $0332
	out ($BF),a	; $0334
	ld a,$7F	; $0336
	out ($BF),a	; $0338
	ld hl,$C180	; $033A
	call fn_00_0546	; $033D
	ld a,$80	; $0340
	out ($BF),a	; $0342
	ld a,$7F	; $0344
	out ($BF),a	; $0346
	ld hl,$C100	; $0348
	call fn_00_04C6	; $034B
	ld a,($C009)	; $034E
	or a	; $0351
	jr z,loc_00_0376	; $0352
	xor a	; $0354
	ld ($C009),a	; $0355
	ld b,$70	; $0358

loc_00_035A:
	djnz loc_00_035A	; $035A
	ld a,($C001)	; $035C
	or a	; $035F
	jr nz,loc_00_0368	; $0360
	ld b,$80	; $0362

loc_00_0364:
	djnz loc_00_0364	; $0364

loc_00_0366:
	djnz loc_00_0366	; $0366

loc_00_0368:
	ld a,$00	; $0368
	out ($BF),a	; $036A
	ld a,$C0	; $036C
	out ($BF),a	; $036E
	ld hl,$C023	; $0370
	call fn_00_0586	; $0373

loc_00_0376:
	ld a,($C063)	; $0376
	or a	; $0379
	jp z,loc_00_03E2	; $037A
	ld hl,$C200	; $037D

loc_00_0380:
	ex af,af'	; $0380
	ld a,(hl)	; $0381
	or a	; $0382
	jp m,loc_00_03AF	; $0383
	ld ($FFFF),a	; $0386
	inc l	; $0389
	ld a,(hl)	; $038A
	inc l	; $038B
	di	; $038C
	out ($BF),a	; $038D
	ld a,(hl)	; $038F
	out ($BF),a	; $0390
	ei	; $0392
	inc l	; $0393
	ld e,(hl)	; $0394
	inc l	; $0395
	ld d,(hl)	; $0396
	inc l	; $0397
	ld b,(hl)	; $0398
	inc l	; $0399
	ld a,(hl)	; $039A
	inc l	; $039B
	inc l	; $039C
	ex de,hl	; $039D

loc_00_039E:
	outi	; $039E
	jr nz,loc_00_039E	; $03A0
	dec a	; $03A2
	jp nz,loc_00_039E	; $03A3
	ex de,hl	; $03A6
	ex af,af'	; $03A7
	dec a	; $03A8
	jp nz,loc_00_0380	; $03A9
	jp loc_00_03DF	; $03AC

loc_00_03AF:
	inc l	; $03AF
	ld e,(hl)	; $03B0
	inc l	; $03B1
	ld d,(hl)	; $03B2
	inc l	; $03B3
	push de	; $03B4
	ld e,(hl)	; $03B5
	inc l	; $03B6
	ld d,(hl)	; $03B7
	inc l	; $03B8
	ld a,(hl)	; $03B9
	inc l	; $03BA
	ld b,(hl)	; $03BB
	inc l	; $03BC
	inc l	; $03BD
	ex (sp),hl	; $03BE
	ex de,hl	; $03BF

loc_00_03C0:
	push bc	; $03C0
	push hl	; $03C1
	ld b,a	; $03C2
	inc c	; $03C3
	di	; $03C4
	out (c),e	; $03C5
	out (c),d	; $03C7
	ei	; $03C9
	dec c	; $03CA

loc_00_03CB:
	outi	; $03CB
	jr nz,loc_00_03CB	; $03CD
	pop hl	; $03CF
	ld c,$40	; $03D0
	add hl,bc	; $03D2
	ex de,hl	; $03D3
	add hl,bc	; $03D4
	ex de,hl	; $03D5
	pop bc	; $03D6
	djnz loc_00_03C0	; $03D7
	pop hl	; $03D9
	ex af,af'	; $03DA
	dec a	; $03DB
	jp nz,loc_00_0380	; $03DC

loc_00_03DF:
	ld ($C063),a	; $03DF

loc_00_03E2:
	ld hl,$03EA	; $03E2
	push hl	; $03E5
	ld hl,(irq_callback)	; $03E6
	jp (hl)	; $03E9

loc_00_03EA:
	ld a,$1F	; $03EA
	ld ($FFFF),a	; $03EC
	call sound_vectors	; $03EF
	pop de	; $03F2
	ld a,e	; $03F3
	ld ($FFFC),a	; $03F4
	ld a,d	; $03F7
	ld ($FFFF),a	; $03F8
	pop iy	; $03FB
	pop ix	; $03FD
	pop hl	; $03FF
	pop de	; $0400
	pop bc	; $0401
	pop af	; $0402
	ex af,af'	; $0403
	exx	; $0404

fn_00_0405:
	pop hl	; $0405
	pop de	; $0406
	pop bc	; $0407
	pop af	; $0408
	ei	; $0409

loc_00_040A:
	ret	; $040A

loc_00_040B:
	push hl	; $040B
	in a,($7E)	; $040C
	ld l,a	; $040E
	ld h,$D0	; $040F
	ld a,(hl)	; $0411
	out ($BF),a	; $0412
	ld a,$88	; $0414
	out ($BF),a	; $0416
	pop hl	; $0418
	pop af	; $0419
	ei	; $041A
	ret	; $041B

wait_vblank:
	push af	; $041C
	push hl	; $041D
	call fn_00_01CB	; $041E
	ld a,$FF	; $0421
	ld hl,$C008	; $0423
	ld (hl),a	; $0426

loc_00_0427:
	cp (hl)	; $0427
	jr z,loc_00_0427	; $0428
	pop hl	; $042A
	pop af	; $042B
	ret	; $042C

fn_00_042D:
	push bc	; $042D
	ld b,$06	; $042E
	call fn_00_0437	; $0430
	pop bc	; $0433
	djnz fn_00_042D	; $0434
	ret	; $0436

fn_00_0437:
	call wait_vblank	; $0437
	djnz fn_00_0437	; $043A
	ret	; $043C

fn_00_043D:
	ld a,$81	; $043D
	ld ($C007),a	; $043F
	ret	; $0442

sub_00_0443:
	call fn_00_01CB	; $0443
	ld a,$80	; $0446
	ld ($C007),a	; $0448

loc_00_044B:
	ld a,($C007)	; $044B
	or a	; $044E
	jp m,loc_00_044B	; $044F
	ret	; $0452

fn_00_0453:
	xor a	; $0453
	ld ($C063),a	; $0454
	ret	; $0457

fn_00_0458:
	ld a,$02	; $0458
	ld hl,$CA00	; $045A
	ld de,$3800	; $045D
	ld bc,$0600	; $0460

fn_00_0463:
	ld a,$02	; $0463

queue_vram_copy:
	push hl	; $0465
	push af	; $0466
	ld a,($C063)	; $0467
	cp $20	; $046A
	jp c,loc_00_0477	; $046C
	pop af	; $046F
	pop hl	; $0470
	call wait_vblank	; $0471
	jp queue_vram_copy	; $0474

loc_00_0477:
	inc a	; $0477
	ld ($C063),a	; $0478
	dec a	; $047B
	add a,a	; $047C
	add a,a	; $047D
	add a,a	; $047E
	ld l,a	; $047F
	ld h,$C2	; $0480
	pop af	; $0482
	ld (hl),a	; $0483
	set 6,d	; $0484
	inc l	; $0486
	ld (hl),e	; $0487
	inc l	; $0488
	ld (hl),d	; $0489
	pop de	; $048A
	inc l	; $048B
	ld (hl),e	; $048C
	inc l	; $048D
	ld (hl),d	; $048E
	inc c	; $048F
	dec c	; $0490
	jr z,loc_00_0494	; $0491
	inc b	; $0493

loc_00_0494:
	inc l	; $0494
	ld (hl),c	; $0495
	inc l	; $0496
	ld (hl),b	; $0497
	ret	; $0498

fn_00_0499:
	ld a,($C063)	; $0499
	cp $20	; $049C
	jp c,loc_00_04A7	; $049E
	call wait_vblank	; $04A1
	jp fn_00_0499	; $04A4

loc_00_04A7:
	inc a	; $04A7
	ld ($C063),a	; $04A8
	dec a	; $04AB
	add a,a	; $04AC
	add a,a	; $04AD
	add a,a	; $04AE
	ld l,a	; $04AF
	ld h,$C2	; $04B0
	ld (hl),$80	; $04B2
	inc l	; $04B4
	ld (hl),e	; $04B5
	ld a,d	; $04B6
	add a,$AE	; $04B7
	inc l	; $04B9
	ld (hl),a	; $04BA
	inc l	; $04BB
	ld (hl),e	; $04BC
	inc l	; $04BD
	ld (hl),d	; $04BE
	sla c	; $04BF
	inc l	; $04C1
	ld (hl),c	; $04C2
	inc l	; $04C3
	ld (hl),b	; $04C4
	ret	; $04C5

fn_00_04C6:
	outi	; $04C6
	outi	; $04C8
	outi	; $04CA
	outi	; $04CC
	outi	; $04CE
	outi	; $04D0
	outi	; $04D2
	outi	; $04D4
	outi	; $04D6
	outi	; $04D8
	outi	; $04DA
	outi	; $04DC
	outi	; $04DE
	outi	; $04E0
	outi	; $04E2
	outi	; $04E4
	outi	; $04E6
	outi	; $04E8
	outi	; $04EA
	outi	; $04EC
	outi	; $04EE
	outi	; $04F0
	outi	; $04F2
	outi	; $04F4
	outi	; $04F6
	outi	; $04F8
	outi	; $04FA
	outi	; $04FC
	outi	; $04FE
	outi	; $0500
	outi	; $0502

fn_00_0504:
	outi	; $0504

fn_00_0506:
	outi	; $0506
	outi	; $0508
	outi	; $050A
	outi	; $050C
	outi	; $050E
	outi	; $0510
	outi	; $0512
	outi	; $0514
	outi	; $0516
	outi	; $0518
	outi	; $051A
	outi	; $051C
	outi	; $051E
	outi	; $0520
	outi	; $0522
	outi	; $0524
	outi	; $0526
	outi	; $0528
	outi	; $052A
	outi	; $052C
	outi	; $052E
	outi	; $0530
	outi	; $0532
	outi	; $0534
	outi	; $0536
	outi	; $0538
	outi	; $053A
	outi	; $053C
	outi	; $053E
	outi	; $0540
	outi	; $0542
	outi	; $0544

fn_00_0546:
	outi	; $0546
	outi	; $0548
	outi	; $054A
	outi	; $054C
	outi	; $054E
	outi	; $0550
	outi	; $0552
	outi	; $0554
	outi	; $0556
	outi	; $0558
	outi	; $055A
	outi	; $055C
	outi	; $055E
	outi	; $0560
	outi	; $0562
	outi	; $0564
	outi	; $0566
	outi	; $0568
	outi	; $056A
	outi	; $056C
	outi	; $056E
	outi	; $0570
	outi	; $0572
	outi	; $0574
	outi	; $0576
	outi	; $0578
	outi	; $057A
	outi	; $057C
	outi	; $057E
	outi	; $0580
	outi	; $0582
	outi	; $0584

fn_00_0586:
	outi	; $0586
	outi	; $0588
	outi	; $058A
	outi	; $058C
	outi	; $058E
	outi	; $0590
	outi	; $0592
	outi	; $0594
	outi	; $0596
	outi	; $0598
	outi	; $059A
	outi	; $059C
	outi	; $059E
	outi	; $05A0
	outi	; $05A2
	outi	; $05A4
	outi	; $05A6
	outi	; $05A8
	outi	; $05AA
	outi	; $05AC
	outi	; $05AE
	outi	; $05B0
	outi	; $05B2
	outi	; $05B4
	outi	; $05B6
	outi	; $05B8
	outi	; $05BA
	outi	; $05BC
	outi	; $05BE
	outi	; $05C0
	outi	; $05C2
	outi	; $05C4
	ret	; $05C6

sound_call_8009:
	ld a,(slot2_bank)	; $05C7
	push af	; $05CA
	ld a,$1F	; $05CB
	ld ($FFFF),a	; $05CD
	ld a,($C002)	; $05D0
	rrca	; $05D3
	sra a	; $05D4
	call fn_1F_8009	; $05D6

loc_00_05D9:
	ld a,$FF	; $05D9
	ld (music_playing),a	; $05DB

loc_00_05DE:
	pop af	; $05DE
	ld ($FFFF),a	; $05DF
	ret	; $05E2

fn_00_05E3:
	ld a,(slot2_bank)	; $05E3
	push af	; $05E6
	ld a,$1F	; $05E7
	ld ($FFFF),a	; $05E9
	call fn_1F_8003	; $05EC
	jr loc_00_05D9	; $05EF

fn_00_05F1:
	ld ($C0B9),a	; $05F1
	ld a,(slot2_bank)	; $05F4
	push af	; $05F7
	ld a,$1F	; $05F8
	ld ($FFFF),a	; $05FA
	ld a,($C0B9)	; $05FD
	call fn_1F_8006	; $0600
	jr loc_00_05DE	; $0603

fn_00_0605:
	ld a,(slot2_bank)	; $0605
	push af	; $0608
	ld a,$1F	; $0609
	ld ($FFFF),a	; $060B
	call fn_1F_800C	; $060E
	jr loc_00_05D9	; $0611

sub_00_0613:
	ld a,(slot2_bank)	; $0613
	push af	; $0616
	ld a,$1F	; $0617
	ld ($FFFF),a	; $0619
	call fn_1F_800F	; $061C
	jr loc_00_05DE	; $061F

play_music:
	push hl	; $0621
	ld hl,music_playing	; $0622
	cp (hl)	; $0625
	ld (hl),a	; $0626
	pop hl	; $0627
	ret z	; $0628
	ld a,(slot2_bank)	; $0629
	push af	; $062C
	ld a,$1F	; $062D
	ld ($FFFF),a	; $062F
	call fn_1F_8003	; $0632
	ld a,(music_playing)	; $0635
	call fn_1F_8006	; $0638
	jr loc_00_05DE	; $063B

loc_00_063D:
	ld a,$92	; $063D
	out ($DF),a	; $063F
	ld a,$07	; $0641
	out ($DE),a	; $0643
	ret	; $0645

read_pad1:
	in a,($DC)	; $0646
	cpl	; $0648
	and $3F	; $0649
	ret	; $064B

read_pads_both:
	in a,($DC)	; $064C
	ld b,a	; $064E
	in a,($DD)	; $064F
	rl b	; $0651
	rla	; $0653
	rl b	; $0654
	rla	; $0656
	cpl	; $0657
	and $3F	; $0658
	ret	; $065A

update_joypad:
	ld a,(pad_mode)	; $065B
	or a	; $065E
	jr nz,loc_00_0667	; $065F
	call read_pad1	; $0661
	jp loc_00_066A	; $0664

loc_00_0667:
	call read_pads_both	; $0667

loc_00_066A:
	ld c,a	; $066A
	and $0F	; $066B
	ld e,a	; $066D
	ld d,$00	; $066E
	ld hl,$06BD	; $0670
	add hl,de	; $0673
	ld a,(hl)	; $0674
	ld (joy_dir),a	; $0675
	ld a,(joy_held)	; $0678
	ld b,a	; $067B
	cpl	; $067C
	and c	; $067D
	ld (joy_pressed),a	; $067E
	ld a,c	; $0681
	ld (joy_held),a	; $0682
	ld hl,input_noise	; $0685
	add a,(hl)	; $0688
	ld (hl),a	; $0689
	ld hl,($C0B3)	; $068A
	dec l	; $068D
	ret z	; $068E
	ld a,b	; $068F
	cp c	; $0690
	jr z,loc_00_0698	; $0691
	xor a	; $0693
	ld ($C0B2),a	; $0694
	ret	; $0697

loc_00_0698:
	ld a,($C0B2)	; $0698
	inc a	; $069B
	ld ($C0B2),a	; $069C
	cp h	; $069F
	jr z,loc_00_06B2	; $06A0
	cp l	; $06A2
	jr z,loc_00_06B6	; $06A3
	jr nc,loc_00_06AC	; $06A5
	xor a	; $06A7
	ld (joy_pressed),a	; $06A8
	ret	; $06AB

loc_00_06AC:
	ld a,$80	; $06AC
	ld (joy_pressed),a	; $06AE
	ret	; $06B1

loc_00_06B2:
	ld a,l	; $06B2
	ld ($C0B2),a	; $06B3

loc_00_06B6:
	ld a,c	; $06B6
	or $80	; $06B7
	ld (joy_pressed),a	; $06B9
	ret	; $06BC
	.INCBIN "bank00.bin" SKIP $06BD READ 16	; data, 16 bytes

fn_00_06CD:
	xor a	; $06CD
	ld (pad_mode),a	; $06CE
	call fn_00_06E2	; $06D1

fn_00_06D4:
	ld a,$01	; $06D4
	ld ($C0B3),a	; $06D6
	ret	; $06D9

fn_00_06DA:
	ld ($C0B3),hl	; $06DA
	xor a	; $06DD
	ld ($C0B2),a	; $06DE
	ret	; $06E1

fn_00_06E2:
	xor a	; $06E2
	ld (joy_pressed),a	; $06E3
	ld ($C0B2),a	; $06E6
	ld (input_noise),a	; $06E9
	cpl	; $06EC
	ld (joy_held),a	; $06ED
	ld a,$08	; $06F0
	ld (joy_dir),a	; $06F2
	ret	; $06F5

decode_rle_far:
	call bank_push	; $06F6
	call decode_rle	; $06F9
	jp bank_pop	; $06FC

decode_rle:
	ld a,(hl)	; $06FF
	inc hl	; $0700
	or a	; $0701
	ret z	; $0702
	jp p,loc_00_0710	; $0703
	and $7F	; $0706
	ld c,a	; $0708
	ld b,$00	; $0709
	ldir	; $070B
	jp decode_rle	; $070D

loc_00_0710:
	ld b,a	; $0710
	ld a,(hl)	; $0711

loc_00_0712:
	ld (de),a	; $0712
	inc de	; $0713
	djnz loc_00_0712	; $0714
	inc hl	; $0716
	jp decode_rle	; $0717

fn_00_071A:
	call bank_push	; $071A
	call fn_00_0723	; $071D
	jp bank_pop	; $0720

fn_00_0723:
	push de	; $0723
	call fn_00_0729	; $0724
	pop de	; $0727
	inc de	; $0728

fn_00_0729:
	ld a,(hl)	; $0729
	inc hl	; $072A
	or a	; $072B
	ret z	; $072C
	jp p,loc_00_073E	; $072D
	and $7F	; $0730
	ld c,a	; $0732
	ld b,$00	; $0733

loc_00_0735:
	ldi	; $0735
	inc de	; $0737
	jp pe,loc_00_0735	; $0738
	jp fn_00_0729	; $073B

loc_00_073E:
	ld b,a	; $073E
	ld a,(hl)	; $073F

loc_00_0740:
	ld (de),a	; $0740
	inc de	; $0741
	inc de	; $0742
	djnz loc_00_0740	; $0743
	inc hl	; $0745
	jp fn_00_0729	; $0746

fn_00_0749:
	call bank_push	; $0749
	call fn_00_0752	; $074C
	jp bank_pop	; $074F

fn_00_0752:
	ld bc,$03BE	; $0752

loc_00_0755:
	push bc	; $0755
	push de	; $0756
	call fn_00_075F	; $0757
	pop de	; $075A
	pop bc	; $075B
	inc de	; $075C
	djnz loc_00_0755	; $075D

fn_00_075F:
	rst $10	; $075F

loc_00_0760:
	ld a,(hl)	; $0760
	inc hl	; $0761
	or a	; $0762
	ret z	; $0763
	jp p,loc_00_0778	; $0764
	and $7F	; $0767
	ld b,a	; $0769

loc_00_076A:
	outi	; $076A
	in a,($BE)	; $076C
	in a,($BE)	; $076E
	in a,($BE)	; $0770
	jp nz,loc_00_076A	; $0772
	jp loc_00_0760	; $0775

loc_00_0778:
	ld b,a	; $0778

loc_00_0779:
	ld a,(hl)	; $0779
	out ($BE),a	; $077A
	in a,($BE)	; $077C
	in a,($BE)	; $077E
	in a,($BE)	; $0780
	djnz loc_00_0779	; $0782
	inc hl	; $0784
	jp loc_00_0760	; $0785

fn_00_0788:
	call bank_push	; $0788
	call fn_00_0791	; $078B
	jp bank_pop	; $078E

fn_00_0791:
	ld bc,$03BE	; $0791

loc_00_0794:
	push bc	; $0794
	push de	; $0795
	call fn_00_079E	; $0796
	pop de	; $0799
	pop bc	; $079A
	inc de	; $079B
	djnz loc_00_0794	; $079C

fn_00_079E:
	rst $10	; $079E

loc_00_079F:
	ld a,(hl)	; $079F
	inc hl	; $07A0
	or a	; $07A1
	ret z	; $07A2
	jp p,loc_00_07B4	; $07A3
	and $7F	; $07A6
	ld b,a	; $07A8

loc_00_07A9:
	outi	; $07A9
	call fn_00_07C1	; $07AB
	jp nz,loc_00_07A9	; $07AE
	jp loc_00_079F	; $07B1

loc_00_07B4:
	ld b,a	; $07B4

loc_00_07B5:
	ld a,(hl)	; $07B5
	out ($BE),a	; $07B6
	call fn_00_07C1	; $07B8
	djnz loc_00_07B5	; $07BB
	inc hl	; $07BD
	jp loc_00_079F	; $07BE

fn_00_07C1:
	in a,($BE)	; $07C1
	call fn_00_0262	; $07C3
	in a,($BE)	; $07C6
	call fn_00_0262	; $07C8
	in a,($BE)	; $07CB
	ret	; $07CD

bank_push:
	ld ($C0C2),hl	; $07CE
	ld hl,slot2_bank	; $07D1
	ld h,(hl)	; $07D4
	ex (sp),hl	; $07D5
	push hl	; $07D6
	ld hl,($C0C2)	; $07D7
	ld ($FFFF),a	; $07DA
	ret	; $07DD

bank_pop:
	ld ($C0C2),hl	; $07DE
	pop hl	; $07E1
	ld l,a	; $07E2
	ld a,h	; $07E3
	ld ($FFFF),a	; $07E4
	ld a,l	; $07E7
	ld hl,($C0C2)	; $07E8
	ret	; $07EB

fn_00_07EC:
	call bank_push	; $07EC
	ldir	; $07EF
	jp bank_pop	; $07F1

fn_00_07F4:
	call bank_push	; $07F4
	call fn_00_024C	; $07F7
	jp bank_pop	; $07FA

fn_00_07FD:
	call bank_push	; $07FD
	ld b,$00	; $0800
	add hl,bc	; $0802
	ld a,(hl)	; $0803
	jp bank_pop	; $0804

fn_00_0807:
	call bank_push	; $0807
	ld b,$00	; $080A
	add hl,bc	; $080C
	add hl,bc	; $080D
	ld a,(hl)	; $080E
	inc hl	; $080F
	ld h,(hl)	; $0810
	ld l,a	; $0811
	jp bank_pop	; $0812

fn_00_0815:
	call bank_push	; $0815
	ld a,(hl)	; $0818
	jp bank_pop	; $0819
	.INCBIN "bank00.bin" SKIP $081C READ 3	; data, 3 bytes

fn_00_081F:
	ld de,$C023	; $081F
	ld bc,$0020	; $0822

loc_00_0825:
	ldir	; $0825
	ld a,$FF	; $0827
	ld ($C009),a	; $0829
	ret	; $082C
	.INCBIN "bank00.bin" SKIP $082D READ 3	; data, 3 bytes

fn_00_0830:
	ld de,$C023	; $0830
	ld bc,$0010	; $0833
	jr loc_00_0825	; $0836
	.INCBIN "bank00.bin" SKIP $0838 READ 3	; data, 3 bytes

fn_00_083B:
	ld de,$C033	; $083B
	ld bc,$0010	; $083E
	jr loc_00_0825	; $0841

set_palette_entry:
	push de	; $0843
	push hl	; $0844
	ld a,h	; $0845
	ld h,$00	; $0846
	ld de,$C023	; $0848
	add hl,de	; $084B
	ld (hl),a	; $084C
	ld a,$FF	; $084D
	ld ($C009),a	; $084F
	pop hl	; $0852
	pop de	; $0853
	ret	; $0854

loc_00_0855:
	ld de,$C023	; $0855
	ld b,$20	; $0858
	jr loc_00_0868	; $085A

fn_00_085C:
	ld de,$C023	; $085C
	ld b,$10	; $085F
	jr loc_00_0868	; $0861

fn_00_0863:
	ld de,$C033	; $0863
	ld b,$10	; $0866

loc_00_0868:
	ld a,$FF	; $0868
	ld ($C009),a	; $086A

loc_00_086D:
	push bc	; $086D
	ld a,(de)	; $086E
	exx	; $086F
	ld bc,$0000	; $0870
	ld de,$0000	; $0873
	ld hl,$0000	; $0876
	rlca	; $0879
	rlca	; $087A
	rla	; $087B
	rl b	; $087C
	rla	; $087E
	rl b	; $087F
	rla	; $0881
	rl c	; $0882
	rla	; $0884
	rl c	; $0885
	rla	; $0887
	rl d	; $0888
	rla	; $088A
	rl d	; $088B
	exx	; $088D
	ld a,(hl)	; $088E
	exx	; $088F
	rlca	; $0890
	rlca	; $0891
	rla	; $0892
	rl e	; $0893
	rla	; $0895
	rl e	; $0896
	rla	; $0898
	rl h	; $0899
	rla	; $089B
	rl h	; $089C
	rla	; $089E
	rl l	; $089F
	rla	; $08A1
	rl l	; $08A2
	ld a,e	; $08A4
	sub b	; $08A5
	ld e,a	; $08A6
	jr nc,loc_00_08AB	; $08A7
	neg	; $08A9

loc_00_08AB:
	ld b,a	; $08AB
	ld a,h	; $08AC
	sub c	; $08AD
	ld h,a	; $08AE
	jr nc,loc_00_08B3	; $08AF
	neg	; $08B1

loc_00_08B3:
	ld c,a	; $08B3
	ld a,l	; $08B4
	sub d	; $08B5
	ld l,a	; $08B6
	jr nc,loc_00_08BB	; $08B7
	neg	; $08B9

loc_00_08BB:
	ld d,a	; $08BB
	ld a,b	; $08BC
	add a,c	; $08BD
	add a,d	; $08BE
	exx	; $08BF
	cp c	; $08C0
	jp c,loc_00_0908	; $08C1
	exx	; $08C4
	ld a,b	; $08C5
	cp c	; $08C6
	jr c,loc_00_08D4	; $08C7
	cp d	; $08C9
	jr c,loc_00_08D4	; $08CA
	or a	; $08CC
	jp nz,loc_00_08F7	; $08CD
	exx	; $08D0
	jp loc_00_0908	; $08D1

loc_00_08D4:
	ld a,c	; $08D4
	cp d	; $08D5
	jp nc,loc_00_08E8	; $08D6
	xor a	; $08D9
	inc l	; $08DA
	dec l	; $08DB
	jr z,loc_00_0903	; $08DC
	ld a,$01	; $08DE
	jp p,loc_00_0903	; $08E0
	ld a,$FF	; $08E3
	jp loc_00_0903	; $08E5

loc_00_08E8:
	xor a	; $08E8
	inc h	; $08E9
	dec h	; $08EA
	jr z,loc_00_0903	; $08EB
	ld a,$04	; $08ED
	jp p,loc_00_0903	; $08EF
	ld a,$FC	; $08F2
	jp loc_00_0903	; $08F4

loc_00_08F7:
	xor a	; $08F7
	inc e	; $08F8
	dec e	; $08F9
	jr z,loc_00_0903	; $08FA
	ld a,$10	; $08FC
	jp p,loc_00_0903	; $08FE
	ld a,$F0	; $0901

loc_00_0903:
	exx	; $0903
	ld b,a	; $0904
	ld a,(de)	; $0905
	add a,b	; $0906
	ld (de),a	; $0907

loc_00_0908:
	inc hl	; $0908
	inc de	; $0909
	pop bc	; $090A
	dec b	; $090B
	jp nz,loc_00_086D	; $090C
	ret	; $090F
	.INCBIN "bank00.bin" SKIP $0910 READ 64	; data, 64 bytes

fn_00_0950:
	ld hl,$C180	; $0950
	ld de,$C181	; $0953
	ld bc,$003F	; $0956
	ld (hl),$C0	; $0959
	ldir	; $095B
	xor a	; $095D
	ld ($C064),a	; $095E
	ret	; $0961

sub_00_0962:
	call bank_push	; $0962
	call fn_00_096B	; $0965
	jp bank_pop	; $0968

fn_00_096B:
	ld b,(hl)	; $096B
	inc hl	; $096C
	exx	; $096D
	ld a,($C064)	; $096E
	cp $80	; $0971
	ret nc	; $0973
	ld e,a	; $0974
	rra	; $0975
	ld c,a	; $0976
	ld b,$C1	; $0977
	ld d,b	; $0979
	exx	; $097A
	ld d,(ix+$0D)	; $097B
	ld e,(ix+$0B)	; $097E

loc_00_0981:
	ld a,e	; $0981
	add a,(hl)	; $0982
	ld c,a	; $0983
	rra	; $0984
	xor (hl)	; $0985
	ld a,c	; $0986
	jp m,loc_00_09B9	; $0987
	ex af,af'	; $098A
	inc hl	; $098B
	ld a,d	; $098C
	add a,(hl)	; $098D
	ld c,a	; $098E
	rra	; $098F
	xor (hl)	; $0990
	ld a,c	; $0991
	jp p,loc_00_099C	; $0992
	cp $F0	; $0995
	jr nc,loc_00_09A0	; $0997
	jp loc_00_09BA	; $0999

loc_00_099C:
	cp $C0	; $099C
	jr nc,loc_00_09BA	; $099E

loc_00_09A0:
	exx	; $09A0
	ld (bc),a	; $09A1
	inc c	; $09A2
	ex af,af'	; $09A3
	ld (de),a	; $09A4
	inc e	; $09A5
	exx	; $09A6
	inc hl	; $09A7
	ld a,(hl)	; $09A8
	exx	; $09A9
	ld (de),a	; $09AA
	inc e	; $09AB
	jp m,loc_00_09B4	; $09AC
	exx	; $09AF

loc_00_09B0:
	inc hl	; $09B0
	djnz loc_00_0981	; $09B1
	exx	; $09B3

loc_00_09B4:
	ld a,e	; $09B4
	ld ($C064),a	; $09B5
	ret	; $09B8

loc_00_09B9:
	inc hl	; $09B9

loc_00_09BA:
	inc hl	; $09BA
	jp loc_00_09B0	; $09BB

fn_00_09BE:
	call bank_push	; $09BE
	call fn_00_09C7	; $09C1
	jp bank_pop	; $09C4

fn_00_09C7:
	ld b,(hl)	; $09C7
	inc hl	; $09C8
	exx	; $09C9
	ld a,($C064)	; $09CA
	cp $80	; $09CD
	ret nc	; $09CF
	ld e,a	; $09D0
	rra	; $09D1
	ld c,a	; $09D2
	ld b,$C1	; $09D3
	ld d,b	; $09D5
	exx	; $09D6
	jp loc_00_0981	; $09D7

fn_00_09DA:
	ld a,($C064)	; $09DA
	or a	; $09DD
	ret m	; $09DE
	ld l,a	; $09DF
	add a,$02	; $09E0
	ld ($C064),a	; $09E2
	ld h,$C1	; $09E5
	ld (hl),e	; $09E7
	inc l	; $09E8
	ld (hl),c	; $09E9
	scf	; $09EA
	rr l	; $09EB
	dec d	; $09ED
	ld (hl),d	; $09EE
	ret	; $09EF

sub_00_09F0:
	call task_enter_1	; $09F0

loc_00_09F3:
	call yield	; $09F3
	ld a,(joy_pressed)	; $09F6
	and $30	; $09F9
	jr z,loc_00_09F3	; $09FB
	jp task_return_1	; $09FD

sub_00_0A00:
	call task_enter_1	; $0A00
	ld (ix+$16),a	; $0A03

loc_00_0A06:
	ld (ix+$17),$06	; $0A06

loc_00_0A0A:
	call yield	; $0A0A
	ld a,(joy_pressed)	; $0A0D
	and $10	; $0A10
	jp nz,task_return_1	; $0A12
	dec (ix+$17)	; $0A15
	jr nz,loc_00_0A0A	; $0A18
	dec (ix+$16)	; $0A1A
	jr nz,loc_00_0A06	; $0A1D
	jp task_return_1	; $0A1F

fn_00_0A22:
	push bc	; $0A22
	push hl	; $0A23
	ld de,$0A3F	; $0A24
	call spawn_task	; $0A27
	pop hl	; $0A2A
	pop bc	; $0A2B
	ret c	; $0A2C
	ld (iy+$1A),c	; $0A2D
	ld (iy+$1B),b	; $0A30
	bit 7,b	; $0A33
	ret nz	; $0A35
	ld de,$C043	; $0A36
	ld bc,$0020	; $0A39
	ldir	; $0A3C
	ret	; $0A3E
	ld a,$FF	; $0A3F
	ld ($C0C5),a	; $0A41
	ld (ix+$16),$09	; $0A44

loc_00_0A48:
	bit 7,(ix+$1B)	; $0A48
	jr nz,loc_00_0A56	; $0A4C
	ld hl,$C043	; $0A4E
	ld c,(ix+$16)	; $0A51
	jr loc_00_0A5B	; $0A54

loc_00_0A56:
	ld hl,$0910	; $0A56
	ld c,$01	; $0A59

loc_00_0A5B:
	bit 0,(ix+$1B)	; $0A5B
	call nz,fn_00_085C	; $0A5F
	bit 1,(ix+$1B)	; $0A62
	call nz,fn_00_0863	; $0A66
	ld a,(ix+$1A)	; $0A69
	call sleep	; $0A6C
	dec (ix+$16)	; $0A6F
	jr nz,loc_00_0A48	; $0A72
	bit 7,(ix+$1B)	; $0A74
	jp nz,loc_00_0A9D	; $0A78
	xor a	; $0A7B
	ld ($C0C5),a	; $0A7C
	jp loc_00_0A9D	; $0A7F

wait_flag:
	pop hl	; $0A82
	ld (ix+$14),l	; $0A83
	ld (ix+$15),h	; $0A86
	xor a	; $0A89
	ld (wait_flag_value),a	; $0A8A

loc_00_0A8D:
	call yield	; $0A8D
	ld a,(wait_flag_value)	; $0A90
	or a	; $0A93
	jr z,loc_00_0A8D	; $0A94
	ld l,(ix+$14)	; $0A96
	ld h,(ix+$15)	; $0A99
	jp (hl)	; $0A9C

loc_00_0A9D:
	ld a,$FF	; $0A9D
	ld (wait_flag_value),a	; $0A9F
	jp end_task	; $0AA2

task_enter_1:
	pop hl	; $0AA5
	ex (sp),hl	; $0AA6
	ld (ix+$24),l	; $0AA7
	ld (ix+$25),h	; $0AAA
	ret	; $0AAD

task_return_1:
	ld l,(ix+$24)	; $0AAE
	ld h,(ix+$25)	; $0AB1
	jp (hl)	; $0AB4

task_enter_2:
	pop hl	; $0AB5
	ex (sp),hl	; $0AB6
	ld (ix+$26),l	; $0AB7
	ld (ix+$27),h	; $0ABA
	ret	; $0ABD

task_return_2:
	ld l,(ix+$26)	; $0ABE
	ld h,(ix+$27)	; $0AC1
	jp (hl)	; $0AC4

task_enter_3:
	pop hl	; $0AC5
	ex (sp),hl	; $0AC6
	ld (ix+$28),l	; $0AC7
	ld (ix+$29),h	; $0ACA
	ret	; $0ACD

task_return_3:
	ld l,(ix+$28)	; $0ACE
	ld h,(ix+$29)	; $0AD1
	jp (hl)	; $0AD4

task_enter_4:
	pop hl	; $0AD5
	ex (sp),hl	; $0AD6
	ld (ix+$2A),l	; $0AD7
	ld (ix+$2B),h	; $0ADA
	ret	; $0ADD

task_return_4:
	ld l,(ix+$2A)	; $0ADE
	ld h,(ix+$2B)	; $0AE1
	jp (hl)	; $0AE4

run_tasks:
	ld ix,task_slots	; $0AE5
	ld de,$0030	; $0AE9
	ld b,$0A	; $0AEC

loc_00_0AEE:
	ld a,(ix+$01)	; $0AEE
	or a	; $0AF1
	jr z,loc_00_0AFC	; $0AF2
	push bc	; $0AF4
	call run_task	; $0AF5
	pop bc	; $0AF8
	ld de,$0030	; $0AF9

loc_00_0AFC:
	add ix,de	; $0AFC
	djnz loc_00_0AEE	; $0AFE
	ret	; $0B00

run_task:
	dec (ix+$01)	; $0B01
	ret nz	; $0B04
	ld (ix+$01),$01	; $0B05
	ld l,(ix+$02)	; $0B09
	ld h,(ix+$03)	; $0B0C
	ld a,(ix+$04)	; $0B0F
	ld ($FFFF),a	; $0B12
	jp (hl)	; $0B15

sleep:
	ld (ix+$01),a	; $0B16

yield:
	pop hl	; $0B19
	ld (ix+$02),l	; $0B1A
	ld (ix+$03),h	; $0B1D
	ret	; $0B20

clear_tasks:
	ld hl,task_slots	; $0B21
	ld de,$C301	; $0B24
	ld bc,$01DF	; $0B27
	ld (hl),$00	; $0B2A
	ldir	; $0B2C
	xor a	; $0B2E
	ld ($C00C),a	; $0B2F
	ret	; $0B32

spawn_task:
	ld c,$FF	; $0B33

spawn_task_c:
	push de	; $0B35
	ld hl,task_slots	; $0B36
	ld de,$0030	; $0B39
	ld b,$0A	; $0B3C
	xor a	; $0B3E

loc_00_0B3F:
	cp (hl)	; $0B3F
	jr z,loc_00_0B48	; $0B40
	add hl,de	; $0B42
	djnz loc_00_0B3F	; $0B43
	pop de	; $0B45
	scf	; $0B46
	ret	; $0B47

loc_00_0B48:
	pop de	; $0B48
	push hl	; $0B49
	pop iy	; $0B4A
	ld (hl),c	; $0B4C
	inc l	; $0B4D
	ld (hl),$01	; $0B4E
	inc l	; $0B50
	ld (hl),e	; $0B51
	inc l	; $0B52
	ld (hl),d	; $0B53
	inc l	; $0B54
	ld a,(slot2_bank)	; $0B55
	ld (hl),a	; $0B58
	ret	; $0B59

end_task:
	push ix	; $0B5A
	pop hl	; $0B5C
	ld d,h	; $0B5D
	ld e,l	; $0B5E
	inc e	; $0B5F
	ld bc,$002F	; $0B60
	ld (hl),$00	; $0B63
	ldir	; $0B65
	ret	; $0B67
	.INCBIN "bank00.bin" SKIP $0B68 READ 5	; data, 5 bytes

fn_00_0B6D:
	ld hl,task_slots	; $0B6D
	ld de,$0030	; $0B70
	ld bc,$0A00	; $0B73

loc_00_0B76:
	cp (hl)	; $0B76
	jr nz,loc_00_0B89	; $0B77
	push bc	; $0B79
	push hl	; $0B7A
	ld d,h	; $0B7B
	ld e,l	; $0B7C
	inc e	; $0B7D
	ld (hl),c	; $0B7E
	ld bc,$002F	; $0B7F
	ldir	; $0B82
	pop hl	; $0B84
	pop bc	; $0B85
	ld de,$0030	; $0B86

loc_00_0B89:
	add hl,de	; $0B89
	djnz loc_00_0B76	; $0B8A
	ret	; $0B8C
	.INCBIN "bank00.bin" SKIP $0B8D READ 23	; data, 23 bytes

fn_00_0BA4:
	ld a,(ix+$0A)	; $0BA4
	add a,(ix+$0E)	; $0BA7
	ld (ix+$0A),a	; $0BAA
	ld a,(ix+$0B)	; $0BAD
	adc a,(ix+$0F)	; $0BB0
	ld (ix+$0B),a	; $0BB3
	ld a,(ix+$0C)	; $0BB6
	add a,(ix+$10)	; $0BB9
	ld (ix+$0C),a	; $0BBC
	ld a,(ix+$0D)	; $0BBF
	adc a,(ix+$11)	; $0BC2
	ld (ix+$0D),a	; $0BC5
	ret	; $0BC8

fn_00_0BC9:
	ld a,(ix+$0A)	; $0BC9
	sub (ix+$0E)	; $0BCC
	ld (ix+$0A),a	; $0BCF
	ld a,(ix+$0B)	; $0BD2
	sbc a,(ix+$0F)	; $0BD5
	ld (ix+$0B),a	; $0BD8
	ld a,(ix+$0C)	; $0BDB
	sub (ix+$10)	; $0BDE
	ld (ix+$0C),a	; $0BE1
	ld a,(ix+$0D)	; $0BE4
	sbc a,(ix+$11)	; $0BE7
	ld (ix+$0D),a	; $0BEA
	ret	; $0BED

fn_00_0BEE:
	call fn_00_0C19	; $0BEE
	cp $88	; $0BF1
	scf	; $0BF3
	ret z	; $0BF4
	ld b,a	; $0BF5
	rrca	; $0BF6
	rrca	; $0BF7
	rrca	; $0BF8
	rrca	; $0BF9
	sra a	; $0BFA
	sra a	; $0BFC
	sra a	; $0BFE
	sra a	; $0C00
	add a,(ix+$0B)	; $0C02
	ld (ix+$0B),a	; $0C05
	ld a,b	; $0C08
	sra a	; $0C09
	sra a	; $0C0B
	sra a	; $0C0D
	sra a	; $0C0F
	add a,(ix+$0D)	; $0C11
	ld (ix+$0D),a	; $0C14
	or a	; $0C17
	ret	; $0C18

fn_00_0C19:
	ld a,(ix+$08)	; $0C19
	ld l,(ix+$12)	; $0C1C
	ld h,(ix+$13)	; $0C1F
	call fn_00_0815	; $0C22
	inc hl	; $0C25
	ld (ix+$12),l	; $0C26
	ld (ix+$13),h	; $0C29
	ret	; $0C2C

fn_00_0C2D:
	push hl	; $0C2D
	ld hl,$FFFC	; $0C2E
	set 3,(hl)	; $0C31
	pop hl	; $0C33
	ret	; $0C34

loc_00_0C35:
	push hl	; $0C35
	ld hl,$FFFC	; $0C36
	res 3,(hl)	; $0C39
	pop hl	; $0C3B
	ret	; $0C3C

fn_00_0C3D:
	call fn_00_0C2D	; $0C3D
	ld a,(hl)	; $0C40
	jr loc_00_0C35	; $0C41

fn_00_0C43:
	call fn_00_0C2D	; $0C43
	ld (hl),a	; $0C46
	jr loc_00_0C35	; $0C47

fn_00_0C49:
	call fn_00_0C2D	; $0C49
	ldir	; $0C4C
	jr loc_00_0C35	; $0C4E

fn_00_0C50:
	call fn_00_0C2D	; $0C50
	ld de,$8010	; $0C53
	ld hl,$0C7F	; $0C56
	ld b,$27	; $0C59

loc_00_0C5B:
	ld a,(de)	; $0C5B
	cp (hl)	; $0C5C
	jr nz,loc_00_0C65	; $0C5D
	inc de	; $0C5F
	inc hl	; $0C60
	djnz loc_00_0C5B	; $0C61
	jr loc_00_0C35	; $0C63

loc_00_0C65:
	ld hl,$8010	; $0C65
	ld de,$8011	; $0C68
	ld bc,$1FEF	; $0C6B
	ld (hl),$00	; $0C6E
	ldir	; $0C70
	ld hl,$0C7F	; $0C72
	ld de,$8010	; $0C75
	ld bc,$0027	; $0C78
	ldir	; $0C7B
	jr loc_00_0C35	; $0C7D
	.INCBIN "bank00.bin" SKIP $0C7F READ 39	; data, 39 bytes

loc_00_0CA6:
	call read_pads_both	; $0CA6
	cp $30	; $0CA9
	jr nz,loc_00_0CB1	; $0CAB
	xor a	; $0CAD
	ld ($C002),a	; $0CAE

loc_00_0CB1:
	call fn_00_0C50	; $0CB1

loc_00_0CB4:
	di	; $0CB4
	im 1	; $0CB5
	ld sp,$DFF0	; $0CB7
	call fn_00_0DA3	; $0CBA
	ld a,$01	; $0CBD
	ld (game_mode),a	; $0CBF
	jr main_loop	; $0CC2

warm_restart:
	di	; $0CC4
	im 1	; $0CC5
	ld sp,$DFF0	; $0CC7
	call fn_00_0DA3	; $0CCA
	ld a,$02	; $0CCD
	ld (game_mode),a	; $0CCF

main_loop:
	ld sp,$DFF0	; $0CD2
	ld a,(vblank_count)	; $0CD5
	ld (frames_this_loop),a	; $0CD8
	ld hl,(main_loop_count)	; $0CDB
	inc hl	; $0CDE
	ld (main_loop_count),hl	; $0CDF
	call update_joypad	; $0CE2
	call fn_00_0950	; $0CE5
	ld hl,$0D06	; $0CE8
	push hl	; $0CEB
	ld a,(game_mode)	; $0CEC
	ld b,a	; $0CEF
	add a,a	; $0CF0
	jp c,run_tasks	; $0CF1
	add a,b	; $0CF4
	add a,$55	; $0CF5
	ld l,a	; $0CF7
	adc a,$0D	; $0CF8
	sub l	; $0CFA
	ld h,a	; $0CFB
	ld a,(hl)	; $0CFC
	inc hl	; $0CFD
	ld e,(hl)	; $0CFE
	inc hl	; $0CFF
	ld d,(hl)	; $0D00
	ex de,hl	; $0D01
	ld ($FFFF),a	; $0D02
	jp (hl)	; $0D05
	ld hl,vblank_count	; $0D06
	ld bc,(frames_this_loop)	; $0D09

loc_00_0D0D:
	ld a,(hl)	; $0D0D
	sub c	; $0D0E
	cp b	; $0D0F
	jr c,loc_00_0D0D	; $0D10
	call wait_vblank	; $0D12
	ld a,($C0C6)	; $0D15
	ld hl,$C0C5	; $0D18
	or (hl)	; $0D1B
	jr z,loc_00_0D25	; $0D1C

loc_00_0D1E:
	xor a	; $0D1E
	ld ($C003),a	; $0D1F
	jp main_loop	; $0D22

loc_00_0D25:
	ld a,($C003)	; $0D25
	or a	; $0D28
	jp z,main_loop	; $0D29
	ld a,$1F	; $0D2C
	ld ($FFFF),a	; $0D2E
	call fn_1F_8012	; $0D31
	ld b,$03	; $0D34
	call fn_00_042D	; $0D36
	xor a	; $0D39
	ld ($C003),a	; $0D3A

loc_00_0D3D:
	call wait_vblank	; $0D3D
	ld a,($C003)	; $0D40
	or a	; $0D43
	jr z,loc_00_0D3D	; $0D44
	ld a,$1F	; $0D46
	ld ($FFFF),a	; $0D48
	call fn_1F_8015	; $0D4B
	ld b,$03	; $0D4E
	call fn_00_042D	; $0D50
	jr loc_00_0D1E	; $0D53

game_mode_table:
	.db $02
	.dw run_tasks
	.db $03
	.dw bank03_vectors
	.db $03
	.dw fn_03_8003
	.db $03
	.dw fn_03_8006
	.db $03
	.dw fn_03_8009
	.db $03
	.dw fn_03_800C
	.db $03
	.dw fn_03_800F
	.db $03
	.dw fn_03_8012
	.db $03
	.dw fn_03_8015
	.db $03
	.dw fn_03_8018
	.db $02
	.dw $0000
	.db $02
	.dw $0000
	.db $02
	.dw $0000
	.db $02
	.dw $0000
	.db $02
	.dw $0000
	.db $02
	.dw fn_02_BC96
	.db $02
	.dw fn_02_AC07
	.db $02
	.dw fn_02_AC74
	.db $02
	.dw fn_02_AC80
	.db $02
	.dw fn_01_69D8
	.db $03
	.dw fn_03_801B
	.db $03
	.dw fn_03_801E
	.db $03
	.dw fn_03_8021
	.db $03
	.dw fn_03_8024
	.db $03
	.dw fn_03_8027
	.db $03
	.dw fn_03_802A

fn_00_0DA3:
	xor a	; $0DA3
	ld ($C003),a	; $0DA4
	ld (port_dd_prev),a	; $0DA7
	ld ($C008),a	; $0DAA
	ld ($C009),a	; $0DAD
	ld ($C007),a	; $0DB0
	ld ($C0A8),a	; $0DB3
	ld ($C0A9),a	; $0DB6
	ld hl,$0000	; $0DB9
	ld (main_loop_count),hl	; $0DBC
	ld ($C0AC),hl	; $0DBF
	call fn_00_2BF7	; $0DC2

fn_00_0DC5:
	call sound_call_8009	; $0DC5
	call fn_00_014D	; $0DC8
	call fn_00_06CD	; $0DCB
	call fn_00_0950	; $0DCE
	call clear_tasks	; $0DD1
	jp fn_00_043D	; $0DD4

fn_00_0DD7:
	call fn_00_06CD	; $0DD7
	jp clear_tasks	; $0DDA

loc_00_0DDD:
	ld a,$FF	; $0DDD
	jr loc_00_0DE2	; $0DDF

loc_00_0DE1:
	xor a	; $0DE1

loc_00_0DE2:
	ld ($C0C6),a	; $0DE2
	ld hl,game_mode	; $0DE5
	set 7,(hl)	; $0DE8
	ret	; $0DEA

fn_00_0DEB:
	ld a,$03	; $0DEB
	call bank_push	; $0DED
	call $8036	; $0DF0
	jp bank_pop	; $0DF3

fn_00_0DF6:
	ld a,$03	; $0DF6
	call bank_push	; $0DF8
	call $8039	; $0DFB
	jp bank_pop	; $0DFE

fn_00_0E01:
	ld a,$03	; $0E01
	call bank_push	; $0E03
	call $803C	; $0E06
	jp bank_pop	; $0E09

fn_00_0E0C:
	ld a,$03	; $0E0C
	call bank_push	; $0E0E
	call $802D	; $0E11
	jp bank_pop	; $0E14

fn_00_0E17:
	ld a,$03	; $0E17
	call bank_push	; $0E19
	call $803F	; $0E1C
	jp bank_pop	; $0E1F

recenter_map_view:
	ld a,(party_x)	; $0E22
	ld b,a	; $0E25
	call fn_00_1078	; $0E26
	ld (view_chunk_x),a	; $0E29
	ld a,b	; $0E2C
	and $0F	; $0E2D
	ld (view_x),a	; $0E2F
	cp $08	; $0E32
	jr nc,loc_00_0E41	; $0E34
	add a,$10	; $0E36
	ld (view_x),a	; $0E38
	ld hl,view_chunk_x	; $0E3B
	call fn_00_1085	; $0E3E

loc_00_0E41:
	ld a,(party_y)	; $0E41
	ld b,a	; $0E44
	call fn_00_1078	; $0E45
	ld (view_chunk_y),a	; $0E48
	ld a,b	; $0E4B
	and $0F	; $0E4C
	ld (view_y),a	; $0E4E
	cp $08	; $0E51
	jr nc,loc_00_0E60	; $0E53
	add a,$10	; $0E55
	ld (view_y),a	; $0E57
	ld hl,view_chunk_y	; $0E5A
	call fn_00_1085	; $0E5D

loc_00_0E60:
	call load_chunk_top_left	; $0E60
	call load_chunk_top_right	; $0E63
	call load_chunk_bottom_left	; $0E66
	jp load_chunk_bottom_right	; $0E69

load_location:
	ld a,$0F	; $0E6C
	call bank_push	; $0E6E
	ld a,(location)	; $0E71
	ld hl,$8077	; $0E74
	rst $20	; $0E77
	ld de,location_people	; $0E78
	ld bc,$0100	; $0E7B
	ldir	; $0E7E
	ld de,$D000	; $0E80
	call decode_location_map	; $0E83
	call fn_00_11C5	; $0E86
	ld a,(party_size)	; $0E89
	dec a	; $0E8C
	jr z,loc_00_0EB0	; $0E8D
	ld b,a	; $0E8F
	ld a,(location)	; $0E90
	sub $06	; $0E93
	cp $08	; $0E95
	jr nc,loc_00_0EB0	; $0E97
	ld hl,$C789	; $0E99
	ld de,$001D	; $0E9C

loc_00_0E9F:
	cp (hl)	; $0E9F
	jr z,loc_00_0EA7	; $0EA0
	add hl,de	; $0EA2
	djnz loc_00_0E9F	; $0EA3
	jr loc_00_0EB0	; $0EA5

loc_00_0EA7:
	ld hl,$C749	; $0EA7
	ld bc,$0800	; $0EAA
	call fn_00_333B	; $0EAD

loc_00_0EB0:
	jp bank_pop	; $0EB0

load_chunk_top_left:
	ld hl,($DFFE)	; $0EB3
	push hl	; $0EB6
	ld hl,(view_chunk_x)	; $0EB7
	ld de,$D000	; $0EBA
	jp load_world_chunk_at	; $0EBD

load_chunk_top_right:
	ld hl,($DFFE)	; $0EC0
	push hl	; $0EC3
	ld hl,(view_chunk_x)	; $0EC4
	inc l	; $0EC7
	ld de,$D020	; $0EC8
	jp load_world_chunk_at	; $0ECB

load_chunk_bottom_left:
	ld hl,($DFFE)	; $0ECE
	push hl	; $0ED1
	ld hl,(view_chunk_x)	; $0ED2
	inc h	; $0ED5
	ld de,$D400	; $0ED6
	jp load_world_chunk_at	; $0ED9

load_chunk_bottom_right:
	ld hl,($DFFE)	; $0EDC
	push hl	; $0EDF
	ld hl,(view_chunk_x)	; $0EE0
	inc l	; $0EE3
	inc h	; $0EE4
	ld de,$D420	; $0EE5

load_world_chunk_at:
	ld a,l	; $0EE8
	exx	; $0EE9
	and $0F	; $0EEA
	ld h,a	; $0EEC
	exx	; $0EED
	ld a,h	; $0EEE
	ld h,$60	; $0EEF
	exx	; $0EF1
	add a,a	; $0EF2
	add a,a	; $0EF3
	add a,a	; $0EF4
	add a,a	; $0EF5
	ld l,a	; $0EF6
	add hl,hl	; $0EF7
	set 6,h	; $0EF8
	push hl	; $0EFA
	pop iy	; $0EFB
	ld hl,$0504	; $0EFD
	ld ($FFFE),hl	; $0F00
	ld d,$10	; $0F03

loc_00_0F05:
	ld l,(iy+$00)	; $0F05
	ld h,(iy+$01)	; $0F08
	ld e,$10	; $0F0B

loc_00_0F0D:
	ld a,(hl)	; $0F0D
	inc hl	; $0F0E
	or a	; $0F0F
	jp m,loc_00_0F35	; $0F10
	add a,a	; $0F13
	exx	; $0F14
	ld l,a	; $0F15
	ldi	; $0F16
	ldi	; $0F18
	exx	; $0F1A
	dec e	; $0F1B
	jp nz,loc_00_0F0D	; $0F1C

loc_00_0F1F:
	exx	; $0F1F
	ld a,e	; $0F20
	add a,$20	; $0F21
	ld e,a	; $0F23
	jr nc,loc_00_0F27	; $0F24
	inc d	; $0F26

loc_00_0F27:
	exx	; $0F27
	inc iy	; $0F28
	inc iy	; $0F2A
	dec d	; $0F2C
	jp nz,loc_00_0F05	; $0F2D
	pop hl	; $0F30
	ld ($FFFE),hl	; $0F31
	ret	; $0F34

loc_00_0F35:
	and $7F	; $0F35
	ld b,a	; $0F37
	ld a,(hl)	; $0F38
	inc hl	; $0F39
	add a,a	; $0F3A

loc_00_0F3B:
	exx	; $0F3B
	ld l,a	; $0F3C
	ldi	; $0F3D
	ldi	; $0F3F
	exx	; $0F41
	dec e	; $0F42
	jp z,loc_00_0F1F	; $0F43
	djnz loc_00_0F3B	; $0F46
	jp loc_00_0F0D	; $0F48

step_east:
	ld hl,party_x	; $0F4B
	inc (hl)	; $0F4E
	ld hl,view_x	; $0F4F
	inc (hl)	; $0F52
	ld a,(hl)	; $0F53
	cp $1B	; $0F54
	ret c	; $0F56
	add a,$F0	; $0F57
	ld (view_x),a	; $0F59
	ld hl,view_chunk_x	; $0F5C
	call fn_00_107F	; $0F5F
	ld hl,$D020	; $0F62
	ld de,$D000	; $0F65
	call shift_map_buffer_x	; $0F68
	call load_chunk_top_right	; $0F6B
	jp load_chunk_bottom_right	; $0F6E

step_west:
	ld hl,party_x	; $0F71
	dec (hl)	; $0F74
	ld hl,view_x	; $0F75
	dec (hl)	; $0F78
	ld a,(hl)	; $0F79
	cp $05	; $0F7A
	ret nc	; $0F7C
	add a,$10	; $0F7D
	ld (view_x),a	; $0F7F
	ld hl,view_chunk_x	; $0F82
	call fn_00_1085	; $0F85
	ld hl,$D000	; $0F88
	ld de,$D020	; $0F8B
	call shift_map_buffer_x	; $0F8E
	call load_chunk_top_left	; $0F91
	jp load_chunk_bottom_left	; $0F94

step_south:
	ld hl,party_y	; $0F97
	inc (hl)	; $0F9A
	ld hl,view_y	; $0F9B
	inc (hl)	; $0F9E
	ld a,(hl)	; $0F9F
	cp $1B	; $0FA0
	ret c	; $0FA2
	add a,$F0	; $0FA3
	ld (view_y),a	; $0FA5
	ld hl,view_chunk_y	; $0FA8
	call fn_00_107F	; $0FAB
	ld hl,$D400	; $0FAE
	ld de,$D000	; $0FB1
	call shift_map_buffer_y	; $0FB4
	call load_chunk_bottom_left	; $0FB7
	jp load_chunk_bottom_right	; $0FBA

step_north:
	ld hl,party_y	; $0FBD
	dec (hl)	; $0FC0
	ld hl,view_y	; $0FC1
	dec (hl)	; $0FC4
	ld a,(hl)	; $0FC5
	cp $05	; $0FC6
	ret nc	; $0FC8
	add a,$10	; $0FC9
	ld (view_y),a	; $0FCB
	ld hl,view_chunk_y	; $0FCE
	call fn_00_1085	; $0FD1
	ld hl,$D000	; $0FD4
	ld de,$D400	; $0FD7
	call shift_map_buffer_y	; $0FDA
	call load_chunk_top_left	; $0FDD
	jp load_chunk_top_right	; $0FE0

shift_map_buffer_x:
	ld a,$20	; $0FE3

loc_00_0FE5:
	ldi	; $0FE5
	ldi	; $0FE7
	ldi	; $0FE9
	ldi	; $0FEB
	ldi	; $0FED
	ldi	; $0FEF
	ldi	; $0FF1
	ldi	; $0FF3
	ldi	; $0FF5
	ldi	; $0FF7
	ldi	; $0FF9
	ldi	; $0FFB
	ldi	; $0FFD
	ldi	; $0FFF
	ldi	; $1001
	ldi	; $1003
	ldi	; $1005
	ldi	; $1007
	ldi	; $1009
	ldi	; $100B
	ldi	; $100D
	ldi	; $100F
	ldi	; $1011
	ldi	; $1013
	ldi	; $1015
	ldi	; $1017
	ldi	; $1019
	ldi	; $101B
	ldi	; $101D
	ldi	; $101F
	ldi	; $1021
	ldi	; $1023
	ld bc,$0020	; $1025
	add hl,bc	; $1028
	ex de,hl	; $1029
	add hl,bc	; $102A
	ex de,hl	; $102B
	dec a	; $102C
	jp nz,loc_00_0FE5	; $102D
	ret	; $1030

shift_map_buffer_y:
	ld bc,$0400	; $1031

loc_00_1034:
	ldi	; $1034
	ldi	; $1036
	ldi	; $1038
	ldi	; $103A
	ldi	; $103C
	ldi	; $103E
	ldi	; $1040
	ldi	; $1042
	ldi	; $1044
	ldi	; $1046
	ldi	; $1048
	ldi	; $104A
	ldi	; $104C
	ldi	; $104E
	ldi	; $1050
	ldi	; $1052
	ldi	; $1054
	ldi	; $1056
	ldi	; $1058
	ldi	; $105A
	ldi	; $105C
	ldi	; $105E
	ldi	; $1060
	ldi	; $1062
	ldi	; $1064
	ldi	; $1066
	ldi	; $1068
	ldi	; $106A
	ldi	; $106C
	ldi	; $106E
	ldi	; $1070
	ldi	; $1072
	jp pe,loc_00_1034	; $1074
	ret	; $1077

fn_00_1078:
	rrca	; $1078
	rrca	; $1079
	rrca	; $107A
	rrca	; $107B
	and $0F	; $107C
	ret	; $107E

fn_00_107F:
	ld a,(hl)	; $107F
	inc a	; $1080
	and $0F	; $1081
	ld (hl),a	; $1083
	ret	; $1084

fn_00_1085:
	ld a,(hl)	; $1085
	dec a	; $1086
	and $0F	; $1087
	ld (hl),a	; $1089
	ret	; $108A

fn_00_108B:
	ld a,(map_mode)	; $108B
	or a	; $108E
	jp nz,loc_00_10E4	; $108F
	ld a,(view_y)	; $1092
	add a,$FB	; $1095
	add a,a	; $1097
	add a,a	; $1098
	add a,a	; $1099
	ld l,a	; $109A
	ld h,$00	; $109B
	add hl,hl	; $109D
	add hl,hl	; $109E
	add hl,hl	; $109F
	ld a,(view_x)	; $10A0
	add a,$FB	; $10A3
	add a,a	; $10A5
	ld e,a	; $10A6
	ld d,$D0	; $10A7
	add hl,de	; $10A9
	ld de,$D800	; $10AA
	ld a,$0B	; $10AD

loc_00_10AF:
	ldi	; $10AF
	ldi	; $10B1
	ldi	; $10B3
	ldi	; $10B5
	ldi	; $10B7
	ldi	; $10B9
	ldi	; $10BB
	ldi	; $10BD
	ldi	; $10BF
	ldi	; $10C1
	ldi	; $10C3
	ldi	; $10C5
	ldi	; $10C7
	ldi	; $10C9
	ldi	; $10CB

fn_00_10CD:
	ldi	; $10CD
	ldi	; $10CF
	ldi	; $10D1
	ldi	; $10D3
	ldi	; $10D5
	ldi	; $10D7
	ldi	; $10D9
	ld bc,$002A	; $10DB
	add hl,bc	; $10DE
	dec a	; $10DF
	jp nz,loc_00_10AF	; $10E0
	ret	; $10E3

loc_00_10E4:
	ld hl,$0004	; $10E4
	ld ($D800),hl	; $10E7
	ld hl,$D800	; $10EA
	ld de,$D802	; $10ED
	call fn_00_169D	; $10F0
	ld a,(view_y)	; $10F3
	sub $05	; $10F6
	jp p,loc_00_1112	; $10F8
	push af	; $10FB
	neg	; $10FC
	add a,a	; $10FE
	ld b,a	; $10FF
	add a,a	; $1100
	ld c,a	; $1101
	add a,a	; $1102
	add a,a	; $1103
	add a,b	; $1104
	add a,c	; $1105
	ld e,a	; $1106
	ld d,$D8	; $1107
	ld hl,$D000	; $1109
	pop af	; $110C
	add a,$0B	; $110D
	ld b,a	; $110F
	jr loc_00_112E	; $1110

loc_00_1112:
	ld c,a	; $1112
	ld a,$20	; $1113
	sub c	; $1115
	cp $0B	; $1116
	jr c,loc_00_111C	; $1118
	ld a,$0B	; $111A

loc_00_111C:
	ld b,a	; $111C
	ld a,c	; $111D
	add a,a	; $111E
	add a,a	; $111F
	add a,a	; $1120
	ld l,a	; $1121
	ld h,$00	; $1122
	add hl,hl	; $1124
	add hl,hl	; $1125
	add hl,hl	; $1126
	ld a,h	; $1127
	add a,$D0	; $1128
	ld h,a	; $112A
	ld de,$D800	; $112B

loc_00_112E:
	ld a,(view_x)	; $112E
	add a,a	; $1131
	sub $0A	; $1132
	jp p,loc_00_1142	; $1134
	push af	; $1137
	add a,$16	; $1138
	ld c,a	; $113A
	pop af	; $113B
	neg	; $113C
	add a,e	; $113E
	ld e,a	; $113F
	jr loc_00_1155	; $1140

loc_00_1142:
	push af	; $1142
	neg	; $1143
	add a,$40	; $1145
	cp $16	; $1147
	jr c,loc_00_114D	; $1149
	ld a,$16	; $114B

loc_00_114D:
	ld c,a	; $114D
	pop af	; $114E
	add a,l	; $114F
	ld l,a	; $1150
	ld a,h	; $1151
	adc a,$00	; $1152
	ld h,a	; $1154

loc_00_1155:
	push bc	; $1155
	push hl	; $1156
	push de	; $1157
	ld b,$00	; $1158
	ldir	; $115A
	pop hl	; $115C
	ld bc,$0016	; $115D
	add hl,bc	; $1160
	ex de,hl	; $1161
	pop hl	; $1162
	ld bc,$0040	; $1163
	add hl,bc	; $1166
	pop bc	; $1167
	djnz loc_00_1155	; $1168
	ret	; $116A

decode_location_map:
	push bc	; $116B
	push de	; $116C
	push hl	; $116D
	exx	; $116E
	push bc	; $116F
	push de	; $1170
	push hl	; $1171
	ld c,$10	; $1172
	ld d,$81	; $1174
	ld hl,$FFFF	; $1176
	exx	; $1179

loc_00_117A:
	ld a,(hl)	; $117A
	inc hl	; $117B
	or a	; $117C
	jr z,loc_00_11A4	; $117D
	jp p,loc_00_1192	; $117F
	and $7F	; $1182
	ld b,a	; $1184

loc_00_1185:
	ld a,(hl)	; $1185
	ld (de),a	; $1186
	inc de	; $1187
	call fn_00_11AC	; $1188
	ld (de),a	; $118B
	inc de	; $118C
	inc hl	; $118D
	djnz loc_00_1185	; $118E
	jr loc_00_117A	; $1190

loc_00_1192:
	ld b,a	; $1192
	ld a,(hl)	; $1193
	ex af,af'	; $1194
	ld a,(hl)	; $1195
	call fn_00_11AC	; $1196

loc_00_1199:
	ex af,af'	; $1199
	ld (de),a	; $119A
	inc de	; $119B
	ex af,af'	; $119C
	ld (de),a	; $119D
	inc de	; $119E
	djnz loc_00_1199	; $119F
	inc hl	; $11A1
	jr loc_00_117A	; $11A2

loc_00_11A4:
	pop hl	; $11A4
	pop de	; $11A5
	pop bc	; $11A6
	exx	; $11A7
	pop hl	; $11A8
	pop de	; $11A9
	pop bc	; $11AA
	ret	; $11AB

fn_00_11AC:
	exx	; $11AC
	ld b,(hl)	; $11AD
	ld (hl),c	; $11AE
	ld e,a	; $11AF
	ld a,(de)	; $11B0
	ld (hl),b	; $11B1
	exx	; $11B2
	ret	; $11B3

loc_00_11B4:
	ld a,$10	; $11B4
	call bank_push	; $11B6
	push bc	; $11B9
	ld b,$81	; $11BA
	ld a,(bc)	; $11BC
	ld (hl),c	; $11BD
	inc l	; $11BE
	ld (hl),a	; $11BF
	dec l	; $11C0
	pop bc	; $11C1
	jp bank_pop	; $11C2

fn_00_11C5:
	push bc	; $11C5
	push de	; $11C6
	push hl	; $11C7
	push iy	; $11C8
	ld iy,$D000	; $11CA
	ld de,$0002	; $11CE
	ld hl,$2001	; $11D1
	ld c,h	; $11D4

loc_00_11D5:
	ld b,h	; $11D5

loc_00_11D6:
	ld a,(iy+$00)	; $11D6
	cp $03	; $11D9
	jr nc,loc_00_11F7	; $11DB
	ld a,c	; $11DD
	cp h	; $11DE
	ld a,(iy-$40)	; $11DF
	call nz,fn_00_1243	; $11E2
	ld a,c	; $11E5
	cp l	; $11E6
	ld a,(iy+$40)	; $11E7
	call nz,fn_00_1255	; $11EA
	ld a,b	; $11ED
	cp h	; $11EE
	call nz,fn_00_1267	; $11EF
	ld a,b	; $11F2
	cp l	; $11F3
	call nz,fn_00_127C	; $11F4

loc_00_11F7:
	add iy,de	; $11F7
	djnz loc_00_11D6	; $11F9
	dec c	; $11FB
	jr nz,loc_00_11D5	; $11FC
	pop iy	; $11FE
	pop hl	; $1200
	pop de	; $1201
	pop bc	; $1202
	ret	; $1203

terrain_substitute:
	push bc	; $1204
	push de	; $1205
	push hl	; $1206
	push iy	; $1207
	ld iy,$D800	; $1209
	ld de,$0002	; $120D
	ld hl,$0B01	; $1210
	ld c,h	; $1213

loc_00_1214:
	ld b,h	; $1214

loc_00_1215:
	ld a,(iy+$00)	; $1215
	cp $03	; $1218
	jr nc,loc_00_1236	; $121A
	ld a,c	; $121C
	cp h	; $121D
	ld a,(iy-$16)	; $121E
	call nz,fn_00_1243	; $1221
	ld a,c	; $1224
	cp l	; $1225
	ld a,(iy+$16)	; $1226
	call nz,fn_00_1255	; $1229
	ld a,b	; $122C
	cp h	; $122D
	call nz,fn_00_1267	; $122E
	ld a,b	; $1231
	cp l	; $1232
	call nz,fn_00_127C	; $1233

loc_00_1236:
	add iy,de	; $1236
	djnz loc_00_1215	; $1238
	dec c	; $123A
	jr nz,loc_00_1214	; $123B
	pop iy	; $123D
	pop hl	; $123F
	pop de	; $1240
	pop bc	; $1241
	ret	; $1242

fn_00_1243:
	call fn_00_1291	; $1243
	ret z	; $1246
	cp $33	; $1247
	ret z	; $1249
	cp $34	; $124A
	ret z	; $124C
	cp $03	; $124D
	ret c	; $124F
	set 0,(iy+$01)	; $1250
	ret	; $1254

fn_00_1255:
	call fn_00_1291	; $1255
	ret z	; $1258
	cp $31	; $1259
	ret z	; $125B
	cp $32	; $125C
	ret z	; $125E
	cp $03	; $125F
	ret c	; $1261
	set 1,(iy+$01)	; $1262
	ret	; $1266

fn_00_1267:
	ld a,(iy-$02)	; $1267
	call fn_00_1291	; $126A
	ret z	; $126D
	cp $31	; $126E
	ret z	; $1270
	cp $33	; $1271
	ret z	; $1273
	cp $03	; $1274
	ret c	; $1276
	set 2,(iy+$01)	; $1277
	ret	; $127B

fn_00_127C:
	ld a,(iy+$02)	; $127C
	call fn_00_1291	; $127F
	ret z	; $1282
	cp $32	; $1283
	ret z	; $1285
	cp $34	; $1286
	ret z	; $1288
	cp $03	; $1289
	ret c	; $128B
	set 3,(iy+$01)	; $128C
	ret	; $1290

fn_00_1291:
	cp $17	; $1291
	ret z	; $1293
	cp $19	; $1294
	ret z	; $1296
	cp $1A	; $1297
	ret z	; $1299
	cp $48	; $129A
	ret	; $129C

fn_00_129D:
	ld a,(game_mode)	; $129D
	and $7F	; $12A0
	sub $11	; $12A2
	ret c	; $12A4
	push bc	; $12A5
	push de	; $12A6
	push hl	; $12A7
	push ix	; $12A8
	push iy	; $12AA
	ld hl,map_mode	; $12AC
	cp (hl)	; $12AF
	ld (hl),a	; $12B0
	ld hl,$12C5	; $12B1
	jr z,loc_00_12BC	; $12B4
	call fn_00_276D	; $12B6
	ld hl,$12D3	; $12B9

loc_00_12BC:
	rst $18	; $12BC
	pop iy	; $12BD
	pop ix	; $12BF
	pop hl	; $12C1
	pop de	; $12C2
	pop bc	; $12C3
	ret	; $12C4
	.INCBIN "bank00.bin" SKIP $12C5 READ 14	; data, 14 bytes

tbl_00_12D3:
	.dw fn_00_12E1
	.dw fn_00_12F6
	.dw fn_00_1305
	.dw fn_00_1342
	.dw fn_00_13FA
	.dw fn_00_1400
	.dw fn_00_1406

fn_00_12E1:
	call fn_00_141B	; $12E1
	call fn_00_147F	; $12E4
	call fn_00_14A4	; $12E7
	call fn_00_1506	; $12EA
	call fn_00_1581	; $12ED
	call fn_00_108B	; $12F0
	jp loc_00_15E4	; $12F3

fn_00_12F6:
	call fn_00_141B	; $12F6
	call fn_00_14A4	; $12F9
	call fn_00_1581	; $12FC
	call fn_00_108B	; $12FF
	jp loc_00_15E4	; $1302

fn_00_1305:
	call fn_00_1431	; $1305
	call fn_00_1581	; $1308
	call fn_00_2781	; $130B
	call fn_00_27E7	; $130E
	call fn_00_29DF	; $1311
	call fn_00_2A05	; $1314
	call fn_00_1A7C	; $1317
	ld a,($C970)	; $131A
	ld b,a	; $131D
	add a,a	; $131E
	add a,b	; $131F
	ld hl,$132A	; $1320
	rst $30	; $1323
	ld de,$0004	; $1324
	jp loc_00_2DA7	; $1327
	.INCBIN "bank00.bin" SKIP $132A READ 24	; data, 24 bytes

fn_00_1342:
	call fn_00_1431	; $1342

loc_00_1345:
	ld hl,$D800	; $1345
	ld de,$D900	; $1348
	call fn_00_169D	; $134B
	ld iy,room_monsters	; $134E
	ld de,$0008	; $1352
	ld b,$10	; $1355

loc_00_1357:
	ld a,(iy+$05)	; $1357
	or a	; $135A
	jr z,loc_00_13B3	; $135B
	ld c,a	; $135D
	ld a,(iy+$07)	; $135E
	or a	; $1361
	jr nz,loc_00_13A3	; $1362
	ld a,c	; $1364
	cp $90	; $1365
	jr c,loc_00_1387	; $1367
	cp $AC	; $1369
	jr nz,loc_00_1374	; $136B
	ld a,(iy+$06)	; $136D
	cp $3C	; $1370
	jr z,loc_00_13A3	; $1372

loc_00_1374:
	call random	; $1374
	and $0F	; $1377
	jr nz,loc_00_13A3	; $1379
	ld a,(iy+$06)	; $137B
	inc a	; $137E
	and $03	; $137F
	or c	; $1381
	ld (iy+$06),a	; $1382
	jr loc_00_13A3	; $1385

loc_00_1387:
	cp $1F	; $1387
	jr z,loc_00_138F	; $1389
	cp $38	; $138B
	jr nz,loc_00_1394	; $138D

loc_00_138F:
	ld (iy+$06),a	; $138F
	jr loc_00_13A3	; $1392

loc_00_1394:
	call random	; $1394
	and $1F	; $1397
	jr nz,loc_00_13A3	; $1399
	ld a,(iy+$06)	; $139B
	xor $01	; $139E
	ld (iy+$06),a	; $13A0

loc_00_13A3:
	ld l,(iy+$00)	; $13A3
	ld h,(iy+$01)	; $13A6
	call fn_00_13F0	; $13A9
	ld a,(iy+$06)	; $13AC
	ld (hl),a	; $13AF
	inc l	; $13B0
	ld (hl),$00	; $13B1

loc_00_13B3:
	add iy,de	; $13B3
	djnz loc_00_1357	; $13B5
	ld iy,party_objects	; $13B7
	ld de,$0005	; $13BB
	ld b,$08	; $13BE

loc_00_13C0:
	ld a,(iy+$02)	; $13C0
	or a	; $13C3
	jr z,loc_00_13E9	; $13C4
	cp $38	; $13C6
	jr z,loc_00_13D9	; $13C8
	call random	; $13CA
	and $1F	; $13CD
	jr nz,loc_00_13D9	; $13CF
	ld a,(iy+$03)	; $13D1
	xor $01	; $13D4
	ld (iy+$03),a	; $13D6

loc_00_13D9:
	ld l,(iy+$00)	; $13D9
	ld h,(iy+$01)	; $13DC
	call fn_00_13F0	; $13DF
	ld a,(iy+$03)	; $13E2
	ld (hl),a	; $13E5
	inc l	; $13E6
	ld (hl),$00	; $13E7

loc_00_13E9:
	add iy,de	; $13E9
	djnz loc_00_13C0	; $13EB
	jp fn_00_1A7C	; $13ED

fn_00_13F0:
	ld a,h	; $13F0
	call fn_00_36ED	; $13F1
	add a,l	; $13F4
	add a,a	; $13F5
	ld l,a	; $13F6
	ld h,$D9	; $13F7
	ret	; $13F9

fn_00_13FA:
	call fn_00_1431	; $13FA
	jp loc_00_1345	; $13FD

fn_00_1400:
	call fn_00_1431	; $1400
	jp loc_00_1345	; $1403

fn_00_1406:
	call fn_00_1431	; $1406
	ld hl,$D800	; $1409
	ld de,$D900	; $140C
	call fn_00_169D	; $140F
	ld hl,$8A1F	; $1412
	ld ($D98E),hl	; $1415
	jp fn_00_1A7C	; $1418

fn_00_141B:
	ld a,$FF	; $141B
	ld ($C98B),a	; $141D
	ld hl,$1579	; $1420
	call fn_00_2D25	; $1423
	ld a,$FF	; $1426
	ld ($C95D),a	; $1428
	ld ($C95F),a	; $142B
	jp loc_00_1461	; $142E

fn_00_1431:
	xor a	; $1431
	ld ($C602),a	; $1432
	inc a	; $1435
	ld ($C601),a	; $1436
	call fn_00_276D	; $1439
	ld de,$0230	; $143C
	ld b,$10	; $143F

loc_00_1441:
	ld hl,$AFE8	; $1441
	call fn_00_1449	; $1444
	djnz loc_00_1441	; $1447

fn_00_1449:
	push bc	; $1449
	push hl	; $144A
	push de	; $144B
	ld a,$14	; $144C
	ld bc,$0010	; $144E
	call fn_00_07F4	; $1451
	pop hl	; $1454
	ld bc,$0220	; $1455
	add hl,bc	; $1458
	ex de,hl	; $1459
	pop hl	; $145A
	ld bc,$0010	; $145B
	add hl,bc	; $145E
	pop bc	; $145F
	ret	; $1460

loc_00_1461:
	call fn_00_1431	; $1461
	ld hl,$B008	; $1464
	ld de,$0AB0	; $1467
	ld b,$08	; $146A

loc_00_146C:
	call fn_00_1449	; $146C
	djnz loc_00_146C	; $146F
	ld a,$14	; $1471
	ld hl,$B088	; $1473
	ld de,$2620	; $1476
	ld bc,$00C0	; $1479
	jp fn_00_07F4	; $147C

fn_00_147F:
	call $957C	; $147F
	ret nz	; $1482
	ld hl,$C99E	; $1483
	inc (hl)	; $1486
	ld a,(hl)	; $1487
	cp $0F	; $1488
	ret c	; $148A
	ld (hl),$00	; $148B
	call fn_00_372E	; $148D
	ld a,($C98A)	; $1490
	ld hl,$1499	; $1493
	jp fn_00_0018	; $1496

tbl_00_1499:
	.dw step_east
	.dw step_south
	.dw step_west
	.dw step_north
	.dw fn_00_14A3

fn_00_14A3:
	ret	; $14A3

fn_00_14A4:
	ld hl,$C989	; $14A4
	dec (hl)	; $14A7
	jr nz,loc_00_14DB	; $14A8
	call random	; $14AA
	ld b,a	; $14AD
	and $01	; $14AE
	jr z,loc_00_14DB	; $14B0
	ld hl,$C98A	; $14B2
	ld a,(hl)	; $14B5
	cp $04	; $14B6
	jr nc,loc_00_14CF	; $14B8
	ld a,b	; $14BA
	and $E0	; $14BB
	jr nz,loc_00_14C3	; $14BD
	ld (hl),$04	; $14BF
	jr loc_00_14DB	; $14C1

loc_00_14C3:
	call random	; $14C3
	and $02	; $14C6
	dec a	; $14C8
	add a,(hl)	; $14C9
	and $03	; $14CA
	ld (hl),a	; $14CC
	jr loc_00_14DB	; $14CD

loc_00_14CF:
	call random	; $14CF
	ld b,a	; $14D2
	and $80	; $14D3
	jr nz,loc_00_14DB	; $14D5
	ld a,b	; $14D7
	and $03	; $14D8
	ld (hl),a	; $14DA

loc_00_14DB:
	ld a,($C98A)	; $14DB
	ld hl,$C98B	; $14DE
	cp (hl)	; $14E1
	ret z	; $14E2
	ld (hl),a	; $14E3
	add a,a	; $14E4
	add a,a	; $14E5
	add a,a	; $14E6
	add a,a	; $14E7
	add a,a	; $14E8
	ld hl,$AF0F	; $14E9
	rst $30	; $14EC
	ld bc,$0010	; $14ED
	ld de,$1770	; $14F0
	ld a,$13	; $14F3
	push hl	; $14F5
	call queue_vram_copy	; $14F6
	pop hl	; $14F9
	ld bc,$0010	; $14FA
	add hl,bc	; $14FD
	ld de,$26C0	; $14FE
	ld a,$13	; $1501
	jp queue_vram_copy	; $1503

fn_00_1506:
	ld a,(main_loop_count)	; $1506
	and $07	; $1509
	ld bc,$0301	; $150B
	jr z,loc_00_1513	; $150E
	ld bc,$0000	; $1510

loc_00_1513:
	push bc	; $1513
	ld hl,$C95E	; $1514
	ld de,$25A0	; $1517
	ld a,c	; $151A
	call fn_00_155A	; $151B
	pop bc	; $151E
	ld hl,$C960	; $151F
	ld de,$25C0	; $1522
	ld a,b	; $1525
	call fn_00_155A	; $1526
	ld a,($C95D)	; $1529
	ld hl,$7CA3	; $152C
	rst $20	; $152F
	ld ($C98D),hl	; $1530
	ld a,($C95E)	; $1533
	and $1F	; $1536
	ld b,$40	; $1538
	cp $02	; $153A
	jr c,loc_00_1555	; $153C
	cp $1E	; $153E
	jr nc,loc_00_1555	; $1540
	inc b	; $1542
	cp $04	; $1543
	jr c,loc_00_1555	; $1545
	cp $1C	; $1547
	jr nc,loc_00_1555	; $1549
	inc b	; $154B
	cp $06	; $154C
	jr c,loc_00_1555	; $154E
	cp $1A	; $1550
	jr nc,loc_00_1555	; $1552
	inc b	; $1554

loc_00_1555:
	ld a,b	; $1555
	ld ($C98C),a	; $1556
	ret	; $1559

fn_00_155A:
	add a,(hl)	; $155A
	ld (hl),a	; $155B
	rlca	; $155C
	rlca	; $155D
	rlca	; $155E
	and $07	; $155F
	dec hl	; $1561
	cp (hl)	; $1562
	ret z	; $1563
	ld (hl),a	; $1564
	add a,a	; $1565
	add a,a	; $1566
	add a,a	; $1567
	add a,a	; $1568
	add a,a	; $1569
	ld c,a	; $156A
	ld b,$00	; $156B
	ld hl,$AFAF	; $156D
	add hl,bc	; $1570
	ld bc,$0020	; $1571
	ld a,$13	; $1574
	jp queue_vram_copy	; $1576
	.INCBIN "bank00.bin" SKIP $1579 READ 8	; data, 8 bytes

fn_00_1581:
	ld iy,location_people	; $1581
	ld de,$0008	; $1585
	ld b,$20	; $1588

loc_00_158A:
	ld a,(iy+$03)	; $158A
	ld c,a	; $158D
	cp $84	; $158E
	jr c,loc_00_1596	; $1590
	cp $8F	; $1592
	jr c,loc_00_15A6	; $1594

loc_00_1596:
	cp $20	; $1596
	jr c,loc_00_159E	; $1598
	cp $2F	; $159A
	jr c,loc_00_15A6	; $159C

loc_00_159E:
	cp $50	; $159E
	jr c,loc_00_15BD	; $15A0
	cp $5F	; $15A2
	jr nc,loc_00_15BD	; $15A4

loc_00_15A6:
	call random	; $15A6
	and $03	; $15A9
	jr nz,loc_00_15DF	; $15AB
	ld a,c	; $15AD
	and $FE	; $15AE
	ld c,a	; $15B0
	ld a,(iy+$00)	; $15B1
	inc a	; $15B4
	and $01	; $15B5
	or c	; $15B7
	ld (iy+$00),a	; $15B8
	jr loc_00_15DF	; $15BB

loc_00_15BD:
	cp $90	; $15BD
	jr c,loc_00_15D8	; $15BF
	call random	; $15C1
	and $07	; $15C4
	jr nz,loc_00_15DF	; $15C6
	ld a,c	; $15C8
	and $FC	; $15C9
	ld c,a	; $15CB
	ld a,(iy+$00)	; $15CC
	inc a	; $15CF
	and $03	; $15D0
	or c	; $15D2
	ld (iy+$00),a	; $15D3
	jr loc_00_15DF	; $15D6

loc_00_15D8:
	cp $80	; $15D8
	jr z,loc_00_15DF	; $15DA
	ld (iy+$00),a	; $15DC

loc_00_15DF:
	add iy,de	; $15DF
	djnz loc_00_158A	; $15E1
	ret	; $15E3

loc_00_15E4:
	ld a,($C98C)	; $15E4
	or a	; $15E7
	jr z,loc_00_15F7	; $15E8
	ld b,a	; $15EA
	ld hl,($C98D)	; $15EB
	call fn_00_1A4B	; $15EE
	jr nc,loc_00_15F7	; $15F1
	ld (hl),b	; $15F3
	inc l	; $15F4
	ld (hl),$00	; $15F5

loc_00_15F7:
	call fn_00_1691	; $15F7
	ld iy,$C749	; $15FA
	ld de,$FFF8	; $15FE
	ld b,$20	; $1601

loc_00_1603:
	ld a,(iy+$00)	; $1603
	or a	; $1606
	jr z,loc_00_1622	; $1607
	ld c,a	; $1609
	ld l,(iy+$01)	; $160A
	ld h,(iy+$02)	; $160D
	call fn_00_1A51	; $1610
	jr nc,loc_00_1622	; $1613
	ld a,c	; $1615
	cp $14	; $1616
	jr z,loc_00_1653	; $1618
	cp $15	; $161A
	jr z,loc_00_1659	; $161C
	ld (hl),a	; $161E
	inc l	; $161F
	ld (hl),$00	; $1620

loc_00_1622:
	add iy,de	; $1622
	djnz loc_00_1603	; $1624
	ld hl,($D878)	; $1626
	ld ($C9BB),hl	; $1629
	ld hl,($D978)	; $162C
	ld ($C990),hl	; $162F
	ld hl,($D962)	; $1632
	ld ($C992),hl	; $1635
	ld hl,($D98E)	; $1638
	ld ($C994),hl	; $163B
	ld hl,($D97A)	; $163E
	ld ($C996),hl	; $1641
	ld hl,($D976)	; $1644
	ld ($C998),hl	; $1647
	ld hl,($C959)	; $164A
	ld ($D978),hl	; $164D
	jp fn_00_1A7C	; $1650

loc_00_1653:
	ld (hl),a	; $1653
	inc l	; $1654
	ld (hl),$84	; $1655
	jr loc_00_1622	; $1657

loc_00_1659:
	ld (hl),a	; $1659
	inc l	; $165A
	ld (hl),$85	; $165B
	jr loc_00_1622	; $165D

fn_00_165F:
	push ix	; $165F
	push iy	; $1661
	ld c,b	; $1663

loc_00_1664:
	add ix,de	; $1664
	add iy,de	; $1666
	ld a,(ix+$01)	; $1668
	ld (iy+$01),a	; $166B
	ld a,(ix+$00)	; $166E
	ld (iy+$00),a	; $1671
	dec c	; $1674
	jr z,loc_00_168C	; $1675
	cp $06	; $1677
	jr z,loc_00_168C	; $1679
	cp $08	; $167B
	jr z,loc_00_168C	; $167D
	cp $49	; $167F
	jr z,loc_00_168C	; $1681
	cp $7E	; $1683
	jr z,loc_00_168C	; $1685
	cp $7F	; $1687
	jp nz,loc_00_1664	; $1689

loc_00_168C:
	pop iy	; $168C
	pop ix	; $168E
	ret	; $1690

fn_00_1691:
	ld a,($C98F)	; $1691
	or a	; $1694
	jr nz,loc_00_16D0	; $1695
	ld hl,$D800	; $1697
	ld de,$D900	; $169A

fn_00_169D:
	ld bc,$00F2	; $169D

loc_00_16A0:
	ldi	; $16A0
	ldi	; $16A2
	ldi	; $16A4
	ldi	; $16A6
	ldi	; $16A8
	ldi	; $16AA
	ldi	; $16AC
	ldi	; $16AE
	ldi	; $16B0
	ldi	; $16B2
	ldi	; $16B4
	ldi	; $16B6
	ldi	; $16B8
	ldi	; $16BA
	ldi	; $16BC
	ldi	; $16BE
	ldi	; $16C0
	ldi	; $16C2
	ldi	; $16C4
	ldi	; $16C6
	ldi	; $16C8
	ldi	; $16CA
	jp pe,loc_00_16A0	; $16CC
	ret	; $16CF

loc_00_16D0:
	ld a,(map_mode)	; $16D0
	or a	; $16D3
	jp z,loc_00_190D	; $16D4
	dec a	; $16D7
	jp z,loc_00_1931	; $16D8

fn_00_16DB:
	push ix	; $16DB
	ld hl,$007E	; $16DD
	ld ($D900),hl	; $16E0
	ld hl,$D900	; $16E3
	ld de,$D902	; $16E6
	call fn_00_169D	; $16E9
	ld hl,($D878)	; $16EC
	ld ($D978),hl	; $16EF
	ld ix,$D878	; $16F2
	ld iy,$D978	; $16F6
	ld b,$05	; $16FA

loc_00_16FC:
	ld de,$FFEA	; $16FC
	add ix,de	; $16FF
	add iy,de	; $1701
	ld a,(ix+$01)	; $1703
	ld (iy+$01),a	; $1706
	ld a,(ix+$00)	; $1709
	ld (iy+$00),a	; $170C
	dec b	; $170F
	jr z,loc_00_1735	; $1710
	cp $06	; $1712
	jr z,loc_00_1735	; $1714
	cp $08	; $1716
	jr z,loc_00_1735	; $1718
	cp $49	; $171A
	jr z,loc_00_1735	; $171C
	cp $7E	; $171E
	jr z,loc_00_1735	; $1720
	cp $7F	; $1722
	jr z,loc_00_1735	; $1724
	ld de,$FFE8	; $1726
	call fn_00_165F	; $1729
	ld de,$FFEC	; $172C
	call fn_00_165F	; $172F
	jp loc_00_16FC	; $1732

loc_00_1735:
	ld ix,$D878	; $1735
	ld iy,$D978	; $1739
	ld b,$05	; $173D

loc_00_173F:
	ld de,$0016	; $173F
	add ix,de	; $1742
	add iy,de	; $1744
	ld a,(ix+$01)	; $1746
	ld (iy+$01),a	; $1749
	ld a,(ix+$00)	; $174C
	ld (iy+$00),a	; $174F
	dec b	; $1752
	jr z,loc_00_1778	; $1753
	cp $06	; $1755
	jr z,loc_00_1778	; $1757
	cp $08	; $1759
	jr z,loc_00_1778	; $175B
	cp $49	; $175D
	jr z,loc_00_1778	; $175F
	cp $7E	; $1761
	jr z,loc_00_1778	; $1763
	cp $7F	; $1765
	jr z,loc_00_1778	; $1767
	ld de,$0014	; $1769
	call fn_00_165F	; $176C
	ld de,$0018	; $176F
	call fn_00_165F	; $1772
	jp loc_00_173F	; $1775

loc_00_1778:
	ld ix,$D878	; $1778
	ld iy,$D978	; $177C
	ld b,$05	; $1780

loc_00_1782:
	ld de,$FFFE	; $1782
	add ix,de	; $1785
	add iy,de	; $1787
	ld a,(ix+$01)	; $1789
	ld (iy+$01),a	; $178C
	ld a,(ix+$00)	; $178F
	ld (iy+$00),a	; $1792
	dec b	; $1795
	jr z,loc_00_17BB	; $1796
	cp $06	; $1798
	jr z,loc_00_17BB	; $179A
	cp $08	; $179C
	jr z,loc_00_17BB	; $179E
	cp $49	; $17A0
	jr z,loc_00_17BB	; $17A2
	cp $7E	; $17A4
	jr z,loc_00_17BB	; $17A6
	cp $7F	; $17A8
	jr z,loc_00_17BB	; $17AA
	ld de,$FFE8	; $17AC
	call fn_00_165F	; $17AF
	ld de,$0014	; $17B2
	call fn_00_165F	; $17B5
	jp loc_00_1782	; $17B8

loc_00_17BB:
	ld ix,$D878	; $17BB
	ld iy,$D978	; $17BF
	ld b,$05	; $17C3

loc_00_17C5:
	ld de,$0002	; $17C5
	add ix,de	; $17C8
	add iy,de	; $17CA
	ld a,(ix+$01)	; $17CC
	ld (iy+$01),a	; $17CF
	ld a,(ix+$00)	; $17D2
	ld (iy+$00),a	; $17D5
	dec b	; $17D8
	jr z,loc_00_17FE	; $17D9
	cp $06	; $17DB
	jr z,loc_00_17FE	; $17DD
	cp $08	; $17DF
	jr z,loc_00_17FE	; $17E1
	cp $49	; $17E3
	jr z,loc_00_17FE	; $17E5
	cp $7E	; $17E7
	jr z,loc_00_17FE	; $17E9
	cp $7F	; $17EB
	jr z,loc_00_17FE	; $17ED
	ld de,$FFEC	; $17EF
	call fn_00_165F	; $17F2
	ld de,$0018	; $17F5
	call fn_00_165F	; $17F8
	jp loc_00_17C5	; $17FB

loc_00_17FE:
	ld ix,$D878	; $17FE
	ld iy,$D978	; $1802
	ld b,$05	; $1806

loc_00_1808:
	ld de,$FFE8	; $1808
	add ix,de	; $180B
	add iy,de	; $180D
	ld a,(ix+$01)	; $180F
	ld (iy+$01),a	; $1812
	ld a,(ix+$00)	; $1815
	ld (iy+$00),a	; $1818
	dec b	; $181B
	jr z,loc_00_1841	; $181C
	cp $06	; $181E
	jr z,loc_00_1841	; $1820
	cp $08	; $1822
	jr z,loc_00_1841	; $1824
	cp $49	; $1826
	jr z,loc_00_1841	; $1828
	cp $7E	; $182A
	jr z,loc_00_1841	; $182C
	cp $7F	; $182E
	jr z,loc_00_1841	; $1830
	ld de,$FFFE	; $1832
	call fn_00_165F	; $1835
	ld de,$FFEA	; $1838
	call fn_00_165F	; $183B
	jp loc_00_1808	; $183E

loc_00_1841:
	ld ix,$D878	; $1841
	ld iy,$D978	; $1845
	ld b,$05	; $1849

loc_00_184B:
	ld de,$FFEC	; $184B
	add ix,de	; $184E
	add iy,de	; $1850
	ld a,(ix+$01)	; $1852
	ld (iy+$01),a	; $1855
	ld a,(ix+$00)	; $1858
	ld (iy+$00),a	; $185B
	dec b	; $185E
	jr z,loc_00_1884	; $185F
	cp $06	; $1861
	jr z,loc_00_1884	; $1863
	cp $08	; $1865
	jr z,loc_00_1884	; $1867
	cp $49	; $1869
	jr z,loc_00_1884	; $186B
	cp $7E	; $186D
	jr z,loc_00_1884	; $186F
	cp $7F	; $1871
	jr z,loc_00_1884	; $1873
	ld de,$0002	; $1875
	call fn_00_165F	; $1878
	ld de,$FFEA	; $187B
	call fn_00_165F	; $187E
	jp loc_00_184B	; $1881

loc_00_1884:
	ld ix,$D878	; $1884
	ld iy,$D978	; $1888
	ld b,$05	; $188C

loc_00_188E:
	ld de,$0014	; $188E
	add ix,de	; $1891
	add iy,de	; $1893
	ld a,(ix+$01)	; $1895
	ld (iy+$01),a	; $1898
	ld a,(ix+$00)	; $189B
	ld (iy+$00),a	; $189E
	dec b	; $18A1
	jr z,loc_00_18C7	; $18A2
	cp $06	; $18A4
	jr z,loc_00_18C7	; $18A6
	cp $08	; $18A8
	jr z,loc_00_18C7	; $18AA
	cp $49	; $18AC
	jr z,loc_00_18C7	; $18AE
	cp $7E	; $18B0
	jr z,loc_00_18C7	; $18B2
	cp $7F	; $18B4
	jr z,loc_00_18C7	; $18B6
	ld de,$FFFE	; $18B8
	call fn_00_165F	; $18BB
	ld de,$0016	; $18BE
	call fn_00_165F	; $18C1
	jp loc_00_188E	; $18C4

loc_00_18C7:
	ld ix,$D878	; $18C7
	ld iy,$D978	; $18CB
	ld b,$05	; $18CF

loc_00_18D1:
	ld de,$0018	; $18D1
	add ix,de	; $18D4
	add iy,de	; $18D6
	ld a,(ix+$01)	; $18D8
	ld (iy+$01),a	; $18DB
	ld a,(ix+$00)	; $18DE
	ld (iy+$00),a	; $18E1
	dec b	; $18E4
	jr z,loc_00_190A	; $18E5
	cp $06	; $18E7
	jr z,loc_00_190A	; $18E9
	cp $08	; $18EB
	jr z,loc_00_190A	; $18ED
	cp $49	; $18EF
	jr z,loc_00_190A	; $18F1
	cp $7E	; $18F3
	jr z,loc_00_190A	; $18F5
	cp $7F	; $18F7
	jr z,loc_00_190A	; $18F9
	ld de,$0002	; $18FB
	call fn_00_165F	; $18FE
	ld de,$0016	; $1901
	call fn_00_165F	; $1904
	jp loc_00_18D1	; $1907

loc_00_190A:
	pop ix	; $190A
	ret	; $190C

loc_00_190D:
	ld hl,$E0DD	; $190D
	call fn_00_1A4B	; $1910
	jp nc,fn_00_16DB	; $1913
	ld e,(hl)	; $1916
	ld (hl),$08	; $1917
	inc l	; $1919
	ld d,(hl)	; $191A
	ld (hl),$00	; $191B
	push de	; $191D
	push hl	; $191E
	call fn_00_16DB	; $191F
	pop hl	; $1922
	pop de	; $1923
	ld (hl),d	; $1924
	dec l	; $1925
	ld (hl),e	; $1926
	ld h,$D9	; $1927
	ld a,(hl)	; $1929
	cp $7E	; $192A
	ret z	; $192C
	ld (hl),e	; $192D
	inc l	; $192E
	ld (hl),d	; $192F
	ret	; $1930

loc_00_1931:
	ld hl,$D800	; $1931
	ld de,$DC00	; $1934
	ld a,$01	; $1937
	ld b,$0D	; $1939

loc_00_193B:
	ld (de),a	; $193B
	inc e	; $193C
	djnz loc_00_193B	; $193D
	ld c,$0B	; $193F

loc_00_1941:
	ld (de),a	; $1941
	inc e	; $1942
	ld b,$0B	; $1943

loc_00_1945:
	ld a,(hl)	; $1945
	inc l	; $1946
	inc l	; $1947
	cp $06	; $1948
	jr z,loc_00_1969	; $194A
	cp $08	; $194C
	jr z,loc_00_1969	; $194E
	cp $49	; $1950
	jr z,loc_00_1969	; $1952
	cp $7E	; $1954
	jr z,loc_00_1969	; $1956
	cp $7F	; $1958
	jr z,loc_00_1969	; $195A
	cp $3A	; $195C
	jr z,loc_00_1969	; $195E
	cp $3B	; $1960
	jr z,loc_00_1969	; $1962
	cp $70	; $1964
	jr z,loc_00_1969	; $1966
	scf	; $1968

loc_00_1969:
	sbc a,a	; $1969
	inc a	; $196A
	ld (de),a	; $196B
	inc e	; $196C
	djnz loc_00_1945	; $196D
	ld a,$01	; $196F
	ld (de),a	; $1971
	inc e	; $1972
	dec c	; $1973
	jp nz,loc_00_1941	; $1974
	ld b,$0D	; $1977

loc_00_1979:
	ld (de),a	; $1979
	inc e	; $197A
	djnz loc_00_1979	; $197B
	ld b,$70	; $197D
	ld a,($D862)	; $197F
	cp b	; $1982
	jr nz,loc_00_1989	; $1983
	xor a	; $1985
	ld ($DC47),a	; $1986

loc_00_1989:
	ld a,($D876)	; $1989
	cp b	; $198C
	jr nz,loc_00_1993	; $198D
	xor a	; $198F
	ld ($DC53),a	; $1990

loc_00_1993:
	ld a,($D87A)	; $1993
	cp b	; $1996
	jr nz,loc_00_199D	; $1997
	xor a	; $1999
	ld ($DC55),a	; $199A

loc_00_199D:
	ld a,($D88E)	; $199D
	cp b	; $19A0
	jr nz,loc_00_19A7	; $19A1
	xor a	; $19A3
	ld ($DC61),a	; $19A4

loc_00_19A7:
	ld hl,$DC54	; $19A7
	ld de,$DCA9	; $19AA
	ld b,$03	; $19AD
	call fn_00_19EA	; $19AF
	ld hl,$DC55	; $19B2
	ld de,$DCA9	; $19B5
	ld a,l	; $19B8
	call fn_00_1A30	; $19B9
	ld hl,$D800	; $19BC
	ld de,$D900	; $19BF
	exx	; $19C2
	ld hl,$DC0E	; $19C3
	ld c,$0B	; $19C6

loc_00_19C8:
	ld b,$0B	; $19C8

loc_00_19CA:
	bit 1,(hl)	; $19CA
	exx	; $19CC
	jr z,loc_00_19D6	; $19CD
	ldi	; $19CF
	ldi	; $19D1
	jp loc_00_19DF	; $19D3

loc_00_19D6:
	ld a,$7E	; $19D6
	ld (de),a	; $19D8
	inc e	; $19D9
	xor a	; $19DA
	ld (de),a	; $19DB
	inc e	; $19DC
	inc l	; $19DD
	inc l	; $19DE

loc_00_19DF:
	exx	; $19DF
	inc l	; $19E0
	djnz loc_00_19CA	; $19E1
	inc l	; $19E3
	inc l	; $19E4
	dec c	; $19E5
	jp nz,loc_00_19C8	; $19E6
	ret	; $19E9

fn_00_19EA:
	dec l	; $19EA
	bit 0,(hl)	; $19EB
	jr z,fn_00_19EA	; $19ED
	ld a,l	; $19EF
	ld (hl),b	; $19F0
	add a,$0D	; $19F1
	ld l,a	; $19F3
	set 1,(hl)	; $19F4
	add a,$E6	; $19F6
	ld l,a	; $19F8
	set 1,(hl)	; $19F9
	add a,$0E	; $19FB
	ld l,a	; $19FD
	ld c,$00	; $19FE

loc_00_1A00:
	ld (hl),b	; $1A00
	add a,$0D	; $1A01
	ld l,a	; $1A03
	bit 0,(hl)	; $1A04
	jr z,loc_00_1A0F	; $1A06
	res 0,c	; $1A08
	set 1,(hl)	; $1A0A
	jp loc_00_1A17	; $1A0C

loc_00_1A0F:
	bit 0,c	; $1A0F
	jr nz,loc_00_1A17	; $1A11
	set 0,c	; $1A13
	ld (de),a	; $1A15
	inc e	; $1A16

loc_00_1A17:
	add a,$E6	; $1A17
	ld l,a	; $1A19
	bit 0,(hl)	; $1A1A
	jr z,loc_00_1A25	; $1A1C
	res 1,c	; $1A1E
	set 1,(hl)	; $1A20
	jp loc_00_1A2D	; $1A22

loc_00_1A25:
	bit 1,c	; $1A25
	jr nz,loc_00_1A2D	; $1A27
	set 1,c	; $1A29
	ld (de),a	; $1A2B
	inc e	; $1A2C

loc_00_1A2D:
	add a,$0E	; $1A2D
	ld l,a	; $1A2F

fn_00_1A30:
	bit 0,(hl)	; $1A30
	jp z,loc_00_1A00	; $1A32
	set 1,(hl)	; $1A35
	add a,$0D	; $1A37
	ld l,a	; $1A39
	set 1,(hl)	; $1A3A
	add a,$E6	; $1A3C
	ld l,a	; $1A3E
	set 1,(hl)	; $1A3F
	ld a,e	; $1A41
	cp $A9	; $1A42
	ret z	; $1A44
	dec e	; $1A45
	ld a,(de)	; $1A46
	ld l,a	; $1A47
	jp fn_00_19EA	; $1A48

fn_00_1A4B:
	call fn_00_1A5D	; $1A4B
	ld h,$D8	; $1A4E
	ret	; $1A50

fn_00_1A51:
	call fn_00_1A5D	; $1A51
	ret nc	; $1A54
	ld h,$D9	; $1A55
	ld a,(hl)	; $1A57
	cp $7E	; $1A58
	ret z	; $1A5A
	scf	; $1A5B
	ret	; $1A5C

fn_00_1A5D:
	push de	; $1A5D
	ld de,(party_x)	; $1A5E
	ld a,h	; $1A62
	add a,$05	; $1A63
	sub d	; $1A65
	cp $0B	; $1A66
	jr nc,loc_00_1A7A	; $1A68
	call fn_00_36ED	; $1A6A
	ld d,a	; $1A6D
	ld a,l	; $1A6E
	add a,$05	; $1A6F
	sub e	; $1A71
	cp $0B	; $1A72
	jr nc,loc_00_1A7A	; $1A74
	add a,d	; $1A76
	add a,a	; $1A77
	ld l,a	; $1A78
	scf	; $1A79

loc_00_1A7A:
	pop de	; $1A7A
	ret	; $1A7B

fn_00_1A7C:
	push ix	; $1A7C
	push iy	; $1A7E
	ld a,($C601)	; $1A80
	dec a	; $1A83
	cp $0B	; $1A84
	jr nc,loc_00_1AA0	; $1A86
	add a,a	; $1A88
	ld b,a	; $1A89
	add a,a	; $1A8A
	ld c,a	; $1A8B
	add a,a	; $1A8C
	add a,a	; $1A8D
	add a,b	; $1A8E
	add a,c	; $1A8F
	add a,a	; $1A90
	ld l,a	; $1A91
	adc a,$DA	; $1A92
	sub l	; $1A94
	ld h,a	; $1A95
	ld e,l	; $1A96
	ld d,h	; $1A97
	inc e	; $1A98
	ld bc,$002B	; $1A99
	ld (hl),$FF	; $1A9C
	ldir	; $1A9E

loc_00_1AA0:
	call fn_00_1AAB	; $1AA0
	call fn_00_1AE9	; $1AA3
	pop iy	; $1AA6
	pop ix	; $1AA8
	ret	; $1AAA

fn_00_1AAB:
	ld hl,($DFFE)	; $1AAB
	push hl	; $1AAE
	ld ix,$D900	; $1AAF
	ld iy,$DA00	; $1AB3
	ld hl,$22FC	; $1AB7
	ld c,$BF	; $1ABA
	xor a	; $1ABC

loc_00_1ABD:
	push af	; $1ABD
	ld de,($C601)	; $1ABE
	cp e	; $1AC2
	ld a,d	; $1AC3
	adc a,$00	; $1AC4
	and $03	; $1AC6
	ld ($C603),a	; $1AC8
	call fn_00_1BB3	; $1ACB
	call fn_00_1BB3	; $1ACE
	call fn_00_1BB3	; $1AD1
	call fn_00_1BB3	; $1AD4
	call fn_00_1BB3	; $1AD7
	call draw_map_window	; $1ADA
	pop af	; $1ADD
	inc a	; $1ADE
	cp $0B	; $1ADF
	jp c,loc_00_1ABD	; $1AE1
	pop hl	; $1AE4
	ld ($FFFE),hl	; $1AE5
	ret	; $1AE8

fn_00_1AE9:
	ld hl,$C601	; $1AE9
	inc (hl)	; $1AEC
	ld a,(hl)	; $1AED
	cp $0C	; $1AEE
	ret c	; $1AF0
	ld (hl),$01	; $1AF1
	ld hl,$C602	; $1AF3
	inc (hl)	; $1AF6
	ret	; $1AF7

loc_00_1AF8:
	push bc	; $1AF8
	push de	; $1AF9
	push hl	; $1AFA
	push ix	; $1AFB
	push iy	; $1AFD
	call fn_00_1AAB	; $1AFF
	pop iy	; $1B02
	pop ix	; $1B04
	pop hl	; $1B06
	pop de	; $1B07
	pop bc	; $1B08
	ret	; $1B09

loc_00_1B0A:
	xor a	; $1B0A
	ld ($C602),a	; $1B0B
	inc a	; $1B0E
	ld ($C601),a	; $1B0F
	call fn_00_276D	; $1B12
	ld de,$0020	; $1B15
	ld b,$1C	; $1B18

loc_00_1B1A:
	push bc	; $1B1A
	push de	; $1B1B
	ld a,$15	; $1B1C
	ld hl,$81FE	; $1B1E
	ld bc,$0010	; $1B21
	call fn_00_07F4	; $1B24
	pop hl	; $1B27
	ld bc,$0100	; $1B28
	add hl,bc	; $1B2B
	ex de,hl	; $1B2C
	pop bc	; $1B2D
	djnz loc_00_1B1A	; $1B2E
	ld a,$15	; $1B30
	ld hl,$821E	; $1B32
	ld bc,$0010	; $1B35
	call fn_00_07F4	; $1B38

loc_00_1B3B:
	ld hl,($DFFE)	; $1B3B
	push hl	; $1B3E
	push ix	; $1B3F
	push iy	; $1B41
	ld a,($C601)	; $1B43
	dec a	; $1B46
	cp $05	; $1B47
	jr nc,loc_00_1B66	; $1B49
	add a,a	; $1B4B
	ld b,a	; $1B4C
	add a,a	; $1B4D
	ld c,a	; $1B4E
	add a,a	; $1B4F
	add a,a	; $1B50
	add a,a	; $1B51
	add a,b	; $1B52
	add a,c	; $1B53
	add a,$06	; $1B54
	add a,a	; $1B56
	ld l,a	; $1B57
	adc a,$DA	; $1B58
	sub l	; $1B5A
	ld h,a	; $1B5B
	ld e,l	; $1B5C
	ld d,h	; $1B5D
	inc e	; $1B5E
	ld bc,$0037	; $1B5F
	ld (hl),$FF	; $1B62
	ldir	; $1B64

loc_00_1B66:
	ld ix,$D900	; $1B66
	ld iy,$DA00	; $1B6A
	ld hl,$24E0	; $1B6E
	ld c,$BF	; $1B71
	xor a	; $1B73

loc_00_1B74:
	push af	; $1B74
	ld de,($C601)	; $1B75
	cp e	; $1B79
	ld a,d	; $1B7A
	adc a,$00	; $1B7B
	and $03	; $1B7D
	ld ($C603),a	; $1B7F
	call fn_00_1BB3	; $1B82
	call fn_00_1BB3	; $1B85
	call fn_00_1BB3	; $1B88
	call fn_00_1BB3	; $1B8B
	call fn_00_1BB3	; $1B8E
	call fn_00_1BB3	; $1B91
	call fn_00_1BB3	; $1B94
	call fn_00_1BB3	; $1B97
	call fn_00_1BB3	; $1B9A
	call draw_map_window	; $1B9D
	pop af	; $1BA0
	inc a	; $1BA1
	cp $05	; $1BA2
	jp c,loc_00_1B74	; $1BA4
	call fn_00_1AE9	; $1BA7
	pop iy	; $1BAA
	pop ix	; $1BAC
	pop hl	; $1BAE
	ld ($FFFE),hl	; $1BAF
	ret	; $1BB2

fn_00_1BB3:
	ld de,$0004	; $1BB3
	add ix,de	; $1BB6
	ld de,$0008	; $1BB8
	add iy,de	; $1BBB
	ld a,$10	; $1BBD
	ld ($FFFF),a	; $1BBF
	ld d,$80	; $1BC2
	ld e,(ix-$04)	; $1BC4
	ld a,(de)	; $1BC7
	or a	; $1BC8
	jr nz,loc_00_1C0A	; $1BC9
	ld a,e	; $1BCB
	cp (iy-$08)	; $1BCC
	jp nz,loc_00_1CA5	; $1BCF
	ld a,(ix-$03)	; $1BD2
	cp (iy-$07)	; $1BD5
	jp nz,loc_00_1CA5	; $1BD8
	ld a,(iy-$06)	; $1BDB
	or (iy-$05)	; $1BDE
	jp nz,loc_00_1CA5	; $1BE1

loc_00_1BE4:
	ld e,(ix-$02)	; $1BE4
	ld a,(de)	; $1BE7
	or a	; $1BE8
	jp nz,loc_00_1E0D	; $1BE9
	ld a,e	; $1BEC
	cp (iy-$04)	; $1BED
	jp nz,loc_00_1EDB	; $1BF0
	ld a,(ix-$01)	; $1BF3
	cp (iy-$03)	; $1BF6
	jp nz,loc_00_1EDB	; $1BF9
	ld a,(iy-$02)	; $1BFC
	or (iy-$01)	; $1BFF
	jp nz,loc_00_1EDB	; $1C02

loc_00_1C05:
	ld de,$0008	; $1C05
	add hl,de	; $1C08
	ret	; $1C09

loc_00_1C0A:
	exx	; $1C0A
	ld l,(ix-$04)	; $1C0B
	ld h,(ix-$03)	; $1C0E
	dec ixh	; $1C11
	ld e,(ix-$04)	; $1C13
	ld d,(ix-$03)	; $1C16
	inc ixh	; $1C19
	ld a,e	; $1C1B
	cp (iy-$08)	; $1C1C
	jr nz,loc_00_1C37	; $1C1F
	ld a,d	; $1C21
	cp (iy-$07)	; $1C22
	jr nz,loc_00_1C37	; $1C25
	ld a,l	; $1C27
	cp (iy-$06)	; $1C28
	jr nz,loc_00_1C37	; $1C2B
	ld a,h	; $1C2D
	cp (iy-$05)	; $1C2E
	jr nz,loc_00_1C37	; $1C31
	exx	; $1C33
	jp loc_00_1BE4	; $1C34

loc_00_1C37:
	exx	; $1C37
	di	; $1C38
	outi	; $1C39
	outi	; $1C3B
	ei	; $1C3D
	ld de,$C1C0	; $1C3E
	exx	; $1C41
	ld (iy-$08),e	; $1C42
	ld (iy-$07),d	; $1C45
	ld (iy-$06),l	; $1C48
	ld (iy-$05),h	; $1C4B
	call map_graphic_address	; $1C4E
	call fn_00_2125	; $1C51
	ld b,$0C	; $1C54

loc_00_1C56:
	ld c,(hl)	; $1C56
	inc e	; $1C57
	inc l	; $1C58
	ld a,(de)	; $1C59
	and c	; $1C5A
	or (hl)	; $1C5B
	out ($BE),a	; $1C5C
	inc e	; $1C5E
	inc l	; $1C5F
	ld a,(de)	; $1C60
	and c	; $1C61
	or (hl)	; $1C62
	out ($BE),a	; $1C63
	inc e	; $1C65
	inc l	; $1C66
	ld a,(de)	; $1C67
	and c	; $1C68
	or (hl)	; $1C69
	out ($BE),a	; $1C6A
	inc e	; $1C6C
	inc l	; $1C6D
	ld a,(de)	; $1C6E
	and c	; $1C6F
	or (hl)	; $1C70
	out ($BE),a	; $1C71
	inc e	; $1C73
	inc l	; $1C74
	djnz loc_00_1C56	; $1C75
	ld b,$0C	; $1C77

loc_00_1C79:
	ld c,(hl)	; $1C79
	inc e	; $1C7A
	inc l	; $1C7B
	ld a,(de)	; $1C7C
	and c	; $1C7D
	or (hl)	; $1C7E
	exx	; $1C7F
	ld (de),a	; $1C80
	inc e	; $1C81
	exx	; $1C82
	inc e	; $1C83
	inc l	; $1C84
	ld a,(de)	; $1C85
	and c	; $1C86
	or (hl)	; $1C87
	exx	; $1C88
	ld (de),a	; $1C89
	inc e	; $1C8A
	exx	; $1C8B
	inc e	; $1C8C
	inc l	; $1C8D
	ld a,(de)	; $1C8E
	and c	; $1C8F
	or (hl)	; $1C90
	exx	; $1C91
	ld (de),a	; $1C92
	inc e	; $1C93
	exx	; $1C94
	inc e	; $1C95
	inc l	; $1C96
	ld a,(de)	; $1C97
	and c	; $1C98
	or (hl)	; $1C99
	exx	; $1C9A
	ld (de),a	; $1C9B
	inc e	; $1C9C
	exx	; $1C9D
	inc e	; $1C9E
	inc l	; $1C9F
	djnz loc_00_1C79	; $1CA0
	jp loc_00_1CED	; $1CA2

loc_00_1CA5:
	di	; $1CA5
	outi	; $1CA6
	outi	; $1CA8
	ei	; $1CAA
	exx	; $1CAB
	ld l,(ix-$04)	; $1CAC
	ld h,(ix-$03)	; $1CAF
	ld (iy-$08),l	; $1CB2
	ld (iy-$07),h	; $1CB5
	ld (iy-$06),$00	; $1CB8
	ld (iy-$05),$00	; $1CBC
	call map_graphic_address	; $1CC0
	ld bc,$30BE	; $1CC3

loc_00_1CC6:
	inc l	; $1CC6
	outi	; $1CC7
	nop	; $1CC9
	nop	; $1CCA
	nop	; $1CCB
	outi	; $1CCC
	nop	; $1CCE
	nop	; $1CCF
	nop	; $1CD0
	outi	; $1CD1
	nop	; $1CD3
	nop	; $1CD4
	nop	; $1CD5
	outi	; $1CD6
	jp nz,loc_00_1CC6	; $1CD8
	ld de,$C1C0	; $1CDB
	ld bc,$0030	; $1CDE

loc_00_1CE1:
	inc l	; $1CE1
	ldi	; $1CE2
	ldi	; $1CE4
	ldi	; $1CE6
	ldi	; $1CE8
	jp pe,loc_00_1CE1	; $1CEA

loc_00_1CED:
	ld a,$10	; $1CED
	ld ($FFFF),a	; $1CEF
	ld d,$80	; $1CF2
	ld e,(ix-$02)	; $1CF4
	ld a,(de)	; $1CF7
	or a	; $1CF8
	jr nz,loc_00_1D52	; $1CF9
	ld a,e	; $1CFB
	cp (iy-$04)	; $1CFC
	jp nz,loc_00_1DAA	; $1CFF
	ld a,(ix-$01)	; $1D02
	cp (iy-$03)	; $1D05
	jp nz,loc_00_1DAA	; $1D08
	ld a,(iy-$02)	; $1D0B
	or (iy-$01)	; $1D0E
	jp nz,loc_00_1DAA	; $1D11

loc_00_1D14:
	exx	; $1D14
	di	; $1D15
	outi	; $1D16
	outi	; $1D18
	ei	; $1D1A
	exx	; $1D1B
	ld hl,$C1C0	; $1D1C
	ld bc,$0C0F	; $1D1F

loc_00_1D22:
	in a,($BE)	; $1D22
	and c	; $1D24
	or (hl)	; $1D25
	ld (hl),a	; $1D26
	inc l	; $1D27
	in a,($BE)	; $1D28
	and c	; $1D2A
	or (hl)	; $1D2B
	ld (hl),a	; $1D2C
	inc l	; $1D2D
	in a,($BE)	; $1D2E
	and c	; $1D30
	or (hl)	; $1D31
	ld (hl),a	; $1D32
	inc l	; $1D33
	in a,($BE)	; $1D34
	and c	; $1D36
	or (hl)	; $1D37
	ld (hl),a	; $1D38
	inc l	; $1D39
	djnz loc_00_1D22	; $1D3A
	exx	; $1D3C
	di	; $1D3D
	outi	; $1D3E
	outi	; $1D40
	ei	; $1D42
	inc hl	; $1D43
	inc hl	; $1D44
	exx	; $1D45
	ld hl,$C1C0	; $1D46
	ld bc,$30BE	; $1D49

loc_00_1D4C:
	outi	; $1D4C
	jr nz,loc_00_1D4C	; $1D4E
	exx	; $1D50
	ret	; $1D51

loc_00_1D52:
	dec ixh	; $1D52
	ld l,(ix-$02)	; $1D54
	ld h,(ix-$01)	; $1D57
	inc ixh	; $1D5A
	ld a,l	; $1D5C
	cp (iy-$04)	; $1D5D
	jr nz,loc_00_1D77	; $1D60
	ld a,h	; $1D62
	cp (iy-$03)	; $1D63
	jr nz,loc_00_1D77	; $1D66
	ld a,e	; $1D68
	cp (iy-$02)	; $1D69
	jr nz,loc_00_1D77	; $1D6C
	ld a,(ix-$01)	; $1D6E
	cp (iy-$01)	; $1D71
	jp z,loc_00_1D14	; $1D74

loc_00_1D77:
	ld (iy-$04),l	; $1D77
	ld (iy-$03),h	; $1D7A
	call fn_00_21B2	; $1D7D
	ld de,$C1C0	; $1D80
	ld b,$0C	; $1D83

loc_00_1D85:
	inc l	; $1D85
	ld a,(de)	; $1D86
	or (hl)	; $1D87
	ld (de),a	; $1D88
	inc e	; $1D89
	inc l	; $1D8A
	ld a,(de)	; $1D8B
	or (hl)	; $1D8C
	ld (de),a	; $1D8D
	inc e	; $1D8E
	inc l	; $1D8F
	ld a,(de)	; $1D90
	or (hl)	; $1D91
	ld (de),a	; $1D92
	inc e	; $1D93
	inc l	; $1D94
	ld a,(de)	; $1D95
	or (hl)	; $1D96
	ld (de),a	; $1D97
	inc e	; $1D98
	inc l	; $1D99
	djnz loc_00_1D85	; $1D9A
	push hl	; $1D9C
	exx	; $1D9D
	inc hl	; $1D9E
	inc hl	; $1D9F
	di	; $1DA0
	outi	; $1DA1
	outi	; $1DA3
	ei	; $1DA5
	exx	; $1DA6
	jp loc_00_1E79	; $1DA7

loc_00_1DAA:
	ld l,(ix-$02)	; $1DAA
	ld h,(ix-$01)	; $1DAD
	ld (iy-$04),l	; $1DB0
	ld (iy-$03),h	; $1DB3
	ld (iy-$02),$00	; $1DB6
	ld (iy-$01),$00	; $1DBA
	call fn_00_21B2	; $1DBE
	exx	; $1DC1
	inc hl	; $1DC2
	inc hl	; $1DC3
	di	; $1DC4
	outi	; $1DC5
	outi	; $1DC7
	ei	; $1DC9
	exx	; $1DCA
	ld de,$C1C0	; $1DCB
	ld b,$0C	; $1DCE

loc_00_1DD0:
	inc l	; $1DD0
	ld a,(de)	; $1DD1
	or (hl)	; $1DD2
	out ($BE),a	; $1DD3
	inc e	; $1DD5
	inc l	; $1DD6
	ld a,(de)	; $1DD7
	or (hl)	; $1DD8
	out ($BE),a	; $1DD9
	inc e	; $1DDB
	inc l	; $1DDC
	ld a,(de)	; $1DDD
	or (hl)	; $1DDE
	out ($BE),a	; $1DDF
	inc e	; $1DE1
	inc l	; $1DE2
	ld a,(de)	; $1DE3
	or (hl)	; $1DE4
	out ($BE),a	; $1DE5
	inc e	; $1DE7
	inc l	; $1DE8
	djnz loc_00_1DD0	; $1DE9
	exx	; $1DEB
	di	; $1DEC
	outi	; $1DED
	outi	; $1DEF
	ei	; $1DF1
	exx	; $1DF2
	ld bc,$30BE	; $1DF3

loc_00_1DF6:
	inc l	; $1DF6
	outi	; $1DF7
	nop	; $1DF9
	nop	; $1DFA
	nop	; $1DFB
	outi	; $1DFC
	nop	; $1DFE
	nop	; $1DFF
	nop	; $1E00
	outi	; $1E01
	nop	; $1E03
	nop	; $1E04
	nop	; $1E05
	outi	; $1E06
	jp nz,loc_00_1DF6	; $1E08
	exx	; $1E0B
	ret	; $1E0C

loc_00_1E0D:
	exx	; $1E0D
	dec ixh	; $1E0E
	ld l,(ix-$02)	; $1E10
	ld h,(ix-$01)	; $1E13
	inc ixh	; $1E16
	ld a,l	; $1E18
	cp (iy-$04)	; $1E19
	jr nz,loc_00_1E38	; $1E1C
	ld a,h	; $1E1E
	cp (iy-$03)	; $1E1F
	jr nz,loc_00_1E38	; $1E22
	ld a,(ix-$02)	; $1E24
	cp (iy-$02)	; $1E27
	jr nz,loc_00_1E38	; $1E2A
	ld a,(ix-$01)	; $1E2C
	cp (iy-$01)	; $1E2F
	jr nz,loc_00_1E38	; $1E32
	exx	; $1E34
	jp loc_00_1C05	; $1E35

loc_00_1E38:
	exx	; $1E38
	inc hl	; $1E39
	inc hl	; $1E3A
	di	; $1E3B
	outi	; $1E3C
	outi	; $1E3E
	ei	; $1E40
	exx	; $1E41
	ld (iy-$04),l	; $1E42
	ld (iy-$03),h	; $1E45
	call fn_00_21B2	; $1E48
	ld de,$C1C0	; $1E4B
	ld bc,$0CF0	; $1E4E

loc_00_1E51:
	inc l	; $1E51
	in a,($BE)	; $1E52
	and c	; $1E54
	or (hl)	; $1E55
	ld (de),a	; $1E56
	inc e	; $1E57
	inc l	; $1E58
	in a,($BE)	; $1E59
	and c	; $1E5B
	or (hl)	; $1E5C
	ld (de),a	; $1E5D
	inc e	; $1E5E
	inc l	; $1E5F
	in a,($BE)	; $1E60
	and c	; $1E62
	or (hl)	; $1E63
	ld (de),a	; $1E64
	inc e	; $1E65
	inc l	; $1E66
	in a,($BE)	; $1E67
	and c	; $1E69
	or (hl)	; $1E6A
	ld (de),a	; $1E6B
	inc e	; $1E6C
	inc l	; $1E6D
	djnz loc_00_1E51	; $1E6E
	push hl	; $1E70
	exx	; $1E71
	di	; $1E72
	outi	; $1E73
	outi	; $1E75
	ei	; $1E77
	exx	; $1E78

loc_00_1E79:
	ld l,(ix-$02)	; $1E79
	ld h,(ix-$01)	; $1E7C
	ld (iy-$02),l	; $1E7F
	ld (iy-$01),h	; $1E82
	call fn_00_223F	; $1E85
	ld de,$C1C0	; $1E88
	ld b,$0C	; $1E8B

loc_00_1E8D:
	ld c,(hl)	; $1E8D
	inc l	; $1E8E
	ld a,(de)	; $1E8F
	and c	; $1E90
	or (hl)	; $1E91
	out ($BE),a	; $1E92
	inc e	; $1E94
	inc l	; $1E95
	ld a,(de)	; $1E96
	and c	; $1E97
	or (hl)	; $1E98
	out ($BE),a	; $1E99
	inc e	; $1E9B
	inc l	; $1E9C
	ld a,(de)	; $1E9D
	and c	; $1E9E
	or (hl)	; $1E9F
	out ($BE),a	; $1EA0
	inc e	; $1EA2
	inc l	; $1EA3
	ld a,(de)	; $1EA4
	and c	; $1EA5
	or (hl)	; $1EA6
	out ($BE),a	; $1EA7
	inc e	; $1EA9
	inc l	; $1EAA
	djnz loc_00_1E8D	; $1EAB
	pop de	; $1EAD
	exx	; $1EAE
	di	; $1EAF
	outi	; $1EB0
	outi	; $1EB2
	ei	; $1EB4
	exx	; $1EB5
	ld b,$0C	; $1EB6

loc_00_1EB8:
	ld c,(hl)	; $1EB8
	inc e	; $1EB9
	inc l	; $1EBA
	ld a,(de)	; $1EBB
	and c	; $1EBC
	or (hl)	; $1EBD
	out ($BE),a	; $1EBE
	inc e	; $1EC0
	inc l	; $1EC1
	ld a,(de)	; $1EC2
	and c	; $1EC3
	or (hl)	; $1EC4
	out ($BE),a	; $1EC5
	inc e	; $1EC7
	inc l	; $1EC8
	ld a,(de)	; $1EC9
	and c	; $1ECA
	or (hl)	; $1ECB
	out ($BE),a	; $1ECC
	inc e	; $1ECE
	inc l	; $1ECF
	ld a,(de)	; $1ED0
	and c	; $1ED1
	or (hl)	; $1ED2
	out ($BE),a	; $1ED3
	inc e	; $1ED5
	inc l	; $1ED6
	djnz loc_00_1EB8	; $1ED7
	exx	; $1ED9
	ret	; $1EDA

loc_00_1EDB:
	inc hl	; $1EDB
	inc hl	; $1EDC
	di	; $1EDD
	outi	; $1EDE
	outi	; $1EE0
	ei	; $1EE2
	exx	; $1EE3
	ld l,(ix-$02)	; $1EE4
	ld h,(ix-$01)	; $1EE7
	ld (iy-$04),l	; $1EEA
	ld (iy-$03),h	; $1EED
	ld (iy-$02),$00	; $1EF0
	ld (iy-$01),$00	; $1EF4
	call fn_00_21B2	; $1EF8
	ld de,$C1C0	; $1EFB
	ld bc,$0CF0	; $1EFE

loc_00_1F01:
	inc l	; $1F01
	in a,($BE)	; $1F02
	and c	; $1F04
	or (hl)	; $1F05
	ld (de),a	; $1F06
	inc l	; $1F07
	inc e	; $1F08
	in a,($BE)	; $1F09
	and c	; $1F0B
	or (hl)	; $1F0C
	ld (de),a	; $1F0D
	inc l	; $1F0E
	inc e	; $1F0F
	in a,($BE)	; $1F10
	and c	; $1F12
	or (hl)	; $1F13
	ld (de),a	; $1F14
	inc l	; $1F15
	inc e	; $1F16
	in a,($BE)	; $1F17
	and c	; $1F19
	or (hl)	; $1F1A
	ld (de),a	; $1F1B
	inc l	; $1F1C
	inc e	; $1F1D
	djnz loc_00_1F01	; $1F1E
	exx	; $1F20
	di	; $1F21
	outi	; $1F22
	outi	; $1F24
	ei	; $1F26
	exx	; $1F27
	ex de,hl	; $1F28
	ld hl,$C1C0	; $1F29
	ld bc,$30BE	; $1F2C

loc_00_1F2F:
	outi	; $1F2F
	jr nz,loc_00_1F2F	; $1F31
	ex de,hl	; $1F33
	exx	; $1F34
	di	; $1F35
	outi	; $1F36
	outi	; $1F38
	ei	; $1F3A
	exx	; $1F3B
	ld bc,$30BE	; $1F3C

loc_00_1F3F:
	inc l	; $1F3F
	outi	; $1F40
	nop	; $1F42
	nop	; $1F43
	nop	; $1F44
	outi	; $1F45
	nop	; $1F47
	nop	; $1F48
	nop	; $1F49
	outi	; $1F4A
	nop	; $1F4C
	nop	; $1F4D
	nop	; $1F4E
	outi	; $1F4F
	jp nz,loc_00_1F3F	; $1F51
	exx	; $1F54
	ret	; $1F55

draw_map_window:
	ld de,$0002	; $1F56
	add ix,de	; $1F59
	ld de,$0004	; $1F5B
	add iy,de	; $1F5E
	ld a,$10	; $1F60
	ld ($FFFF),a	; $1F62
	ld d,$80	; $1F65
	ld e,(ix-$02)	; $1F67
	ld a,(de)	; $1F6A
	or a	; $1F6B
	jr nz,loc_00_1F8C	; $1F6C
	ld a,e	; $1F6E
	cp (iy-$04)	; $1F6F
	jp nz,loc_00_2038	; $1F72
	ld a,(ix-$01)	; $1F75
	cp (iy-$03)	; $1F78
	jp nz,loc_00_2038	; $1F7B
	ld a,(iy-$02)	; $1F7E
	or (iy-$01)	; $1F81
	jp nz,loc_00_2038	; $1F84

loc_00_1F87:
	ld de,$0004	; $1F87
	add hl,de	; $1F8A
	ret	; $1F8B

loc_00_1F8C:
	exx	; $1F8C
	ld l,(ix-$02)	; $1F8D
	ld h,(ix-$01)	; $1F90
	dec ixh	; $1F93
	ld e,(ix-$02)	; $1F95
	ld d,(ix-$01)	; $1F98
	inc ixh	; $1F9B
	ld a,e	; $1F9D
	cp (iy-$04)	; $1F9E
	jr nz,loc_00_1FB9	; $1FA1
	ld a,d	; $1FA3
	cp (iy-$03)	; $1FA4
	jr nz,loc_00_1FB9	; $1FA7
	ld a,l	; $1FA9
	cp (iy-$02)	; $1FAA
	jr nz,loc_00_1FB9	; $1FAD
	ld a,h	; $1FAF
	cp (iy-$01)	; $1FB0
	jr nz,loc_00_1FB9	; $1FB3
	exx	; $1FB5
	jp loc_00_1F87	; $1FB6

loc_00_1FB9:
	exx	; $1FB9
	di	; $1FBA
	outi	; $1FBB
	outi	; $1FBD
	ei	; $1FBF
	exx	; $1FC0
	ld (iy-$04),e	; $1FC1
	ld (iy-$03),d	; $1FC4
	ld (iy-$02),l	; $1FC7
	ld (iy-$01),h	; $1FCA

fn_00_1FCD:
	call map_graphic_address	; $1FCD
	call fn_00_2125	; $1FD0
	ld b,$0C	; $1FD3

loc_00_1FD5:
	ld c,(hl)	; $1FD5
	inc e	; $1FD6
	inc l	; $1FD7
	ld a,(de)	; $1FD8
	and c	; $1FD9
	or (hl)	; $1FDA
	out ($BE),a	; $1FDB
	inc e	; $1FDD
	inc l	; $1FDE
	ld a,(de)	; $1FDF
	and c	; $1FE0
	or (hl)	; $1FE1
	out ($BE),a	; $1FE2
	inc e	; $1FE4
	inc l	; $1FE5
	ld a,(de)	; $1FE6
	and c	; $1FE7
	or (hl)	; $1FE8
	out ($BE),a	; $1FE9
	inc e	; $1FEB
	inc l	; $1FEC
	ld a,(de)	; $1FED
	and c	; $1FEE
	or (hl)	; $1FEF
	out ($BE),a	; $1FF0
	inc e	; $1FF2
	inc l	; $1FF3
	djnz loc_00_1FD5	; $1FF4
	exx	; $1FF6
	di	; $1FF7
	outi	; $1FF8
	outi	; $1FFA
	ei	; $1FFC
	ex de,hl	; $1FFD
	ld hl,$22CC	; $1FFE
	exx	; $2001
	ld b,$0C	; $2002

loc_00_2004:
	ld c,(hl)	; $2004
	inc e	; $2005
	inc l	; $2006
	ld a,(de)	; $2007
	and c	; $2008
	or (hl)	; $2009
	exx	; $200A
	or (hl)	; $200B
	inc hl	; $200C
	exx	; $200D
	out ($BE),a	; $200E
	inc e	; $2010
	inc l	; $2011
	ld a,(de)	; $2012
	and c	; $2013
	or (hl)	; $2014
	exx	; $2015
	or (hl)	; $2016
	inc hl	; $2017
	exx	; $2018
	out ($BE),a	; $2019
	inc e	; $201B
	inc l	; $201C
	ld a,(de)	; $201D
	and c	; $201E
	or (hl)	; $201F
	exx	; $2020
	or (hl)	; $2021
	inc hl	; $2022
	exx	; $2023
	out ($BE),a	; $2024
	inc e	; $2026
	inc l	; $2027
	ld a,(de)	; $2028
	and c	; $2029
	or (hl)	; $202A
	exx	; $202B
	or (hl)	; $202C
	inc hl	; $202D
	exx	; $202E
	out ($BE),a	; $202F
	inc e	; $2031
	inc l	; $2032
	djnz loc_00_2004	; $2033
	exx	; $2035
	ex de,hl	; $2036
	ret	; $2037

loc_00_2038:
	di	; $2038
	outi	; $2039
	outi	; $203B
	ei	; $203D
	exx	; $203E
	ld l,(ix-$02)	; $203F
	ld h,(ix-$01)	; $2042
	ld (iy-$04),l	; $2045
	ld (iy-$03),h	; $2048
	ld (iy-$02),$00	; $204B
	ld (iy-$01),$00	; $204F
	call map_graphic_address	; $2053
	ld bc,$30BE	; $2056

loc_00_2059:
	inc l	; $2059
	outi	; $205A
	nop	; $205C
	nop	; $205D
	nop	; $205E
	outi	; $205F
	nop	; $2061
	nop	; $2062
	nop	; $2063
	outi	; $2064
	nop	; $2066
	nop	; $2067
	nop	; $2068
	outi	; $2069
	jp nz,loc_00_2059	; $206B
	exx	; $206E
	di	; $206F
	outi	; $2070
	outi	; $2072
	ei	; $2074
	exx	; $2075
	ld de,$22CC	; $2076
	ld b,$0C	; $2079

loc_00_207B:
	inc l	; $207B
	ld a,(de)	; $207C
	or (hl)	; $207D
	out ($BE),a	; $207E
	inc l	; $2080
	inc de	; $2081
	ld a,(de)	; $2082
	or (hl)	; $2083
	out ($BE),a	; $2084
	inc l	; $2086
	inc de	; $2087
	ld a,(de)	; $2088
	or (hl)	; $2089
	out ($BE),a	; $208A
	inc l	; $208C
	inc de	; $208D
	ld a,(de)	; $208E
	or (hl)	; $208F
	out ($BE),a	; $2090
	inc l	; $2092
	inc de	; $2093
	djnz loc_00_207B	; $2094
	exx	; $2096
	ret	; $2097

map_graphic_address:
	inc h	; $2098
	jr z,loc_00_210F	; $2099
	dec h	; $209B
	jp m,loc_00_20B5	; $209C
	ld a,l	; $209F
	rlca	; $20A0
	ld a,$06	; $20A1
	adc a,$00	; $20A3
	ld ($FFFF),a	; $20A5
	ld a,l	; $20A8
	ld l,$00	; $20A9
	srl a	; $20AB
	rr l	; $20AD
	and $3F	; $20AF
	or $80	; $20B1
	ld h,a	; $20B3
	ret	; $20B4

loc_00_20B5:
	ld a,h	; $20B5
	rlca	; $20B6
	rlca	; $20B7
	rlca	; $20B8
	and $03	; $20B9
	jr z,loc_00_20F3	; $20BB
	ld b,$00	; $20BD
	dec a	; $20BF
	jr z,loc_00_20C9	; $20C0
	ld b,$30	; $20C2
	dec a	; $20C4
	jr z,loc_00_20C9	; $20C5
	ld b,$60	; $20C7

loc_00_20C9:
	ld a,($C603)	; $20C9
	cp $03	; $20CC
	jr nz,loc_00_20D2	; $20CE
	ld a,$01	; $20D0

loc_00_20D2:
	add a,a	; $20D2
	add a,a	; $20D3
	add a,a	; $20D4
	add a,a	; $20D5
	add a,b	; $20D6
	ld b,a	; $20D7
	ld a,h	; $20D8
	and $0F	; $20D9
	add a,b	; $20DB
	ld l,$00	; $20DC
	srl a	; $20DE
	rr l	; $20E0
	ld b,$0A	; $20E2
	cp $40	; $20E4
	jr c,loc_00_20EB	; $20E6
	sub $40	; $20E8
	inc b	; $20EA

loc_00_20EB:
	or $80	; $20EB
	ld h,a	; $20ED
	ld a,b	; $20EE
	ld ($FFFF),a	; $20EF
	ret	; $20F2

loc_00_20F3:
	ld a,$0B	; $20F3
	ld ($FFFF),a	; $20F5
	ld a,h	; $20F8
	and $1F	; $20F9
	cp $10	; $20FB
	jr nc,loc_00_2105	; $20FD
	add a,a	; $20FF
	add a,$88	; $2100
	jp loc_00_2118	; $2102

loc_00_2105:
	ld l,$00	; $2105
	srl a	; $2107
	rr l	; $2109
	add a,$96	; $210B
	ld h,a	; $210D
	ret	; $210E

loc_00_210F:
	ld a,$0E	; $210F
	ld ($FFFF),a	; $2111
	ld a,l	; $2114
	add a,a	; $2115
	add a,$80	; $2116

loc_00_2118:
	ld h,a	; $2118
	ld a,($C603)	; $2119
	ld l,$00	; $211C
	srl a	; $211E
	rr l	; $2120
	or h	; $2122
	ld h,a	; $2123
	ret	; $2124

fn_00_2125:
	inc d	; $2125
	jr z,loc_00_219C	; $2126
	dec d	; $2128
	jp m,loc_00_2142	; $2129
	ld a,e	; $212C
	rlca	; $212D
	ld a,$06	; $212E
	adc a,$00	; $2130
	ld ($FFFE),a	; $2132
	ld a,e	; $2135
	ld e,$00	; $2136
	srl a	; $2138
	rr e	; $213A
	and $3F	; $213C
	or $40	; $213E
	ld d,a	; $2140
	ret	; $2141

loc_00_2142:
	ld a,d	; $2142
	rlca	; $2143
	rlca	; $2144
	rlca	; $2145
	and $03	; $2146
	jr z,loc_00_2180	; $2148
	ld b,$00	; $214A
	dec a	; $214C
	jr z,loc_00_2156	; $214D
	ld b,$30	; $214F
	dec a	; $2151
	jr z,loc_00_2156	; $2152
	ld b,$60	; $2154

loc_00_2156:
	ld a,($C603)	; $2156
	cp $03	; $2159
	jr nz,loc_00_215F	; $215B
	ld a,$01	; $215D

loc_00_215F:
	add a,a	; $215F
	add a,a	; $2160
	add a,a	; $2161
	add a,a	; $2162
	add a,b	; $2163
	ld b,a	; $2164
	ld a,d	; $2165
	and $0F	; $2166
	add a,b	; $2168
	ld e,$00	; $2169
	srl a	; $216B
	rr e	; $216D
	ld b,$0A	; $216F
	cp $40	; $2171
	jr c,loc_00_2178	; $2173
	sub $40	; $2175
	inc b	; $2177

loc_00_2178:
	or $40	; $2178
	ld d,a	; $217A
	ld a,b	; $217B
	ld ($FFFE),a	; $217C
	ret	; $217F

loc_00_2180:
	ld a,$0B	; $2180
	ld ($FFFE),a	; $2182
	ld a,d	; $2185
	and $1F	; $2186
	cp $10	; $2188
	jr nc,loc_00_2192	; $218A
	add a,a	; $218C
	add a,$48	; $218D
	jp loc_00_21A5	; $218F

loc_00_2192:
	ld e,$00	; $2192
	srl a	; $2194
	rr e	; $2196
	add a,$56	; $2198
	ld d,a	; $219A
	ret	; $219B

loc_00_219C:
	ld a,$0E	; $219C
	ld ($FFFE),a	; $219E
	ld a,e	; $21A1
	add a,a	; $21A2
	add a,$40	; $21A3

loc_00_21A5:
	ld d,a	; $21A5
	ld a,($C603)	; $21A6
	ld e,$00	; $21A9
	srl a	; $21AB
	rr e	; $21AD
	or d	; $21AF
	ld d,a	; $21B0
	ret	; $21B1

fn_00_21B2:
	inc h	; $21B2
	jr z,loc_00_2229	; $21B3
	dec h	; $21B5
	jp m,loc_00_21CF	; $21B6
	ld a,l	; $21B9
	rlca	; $21BA
	ld a,$08	; $21BB
	adc a,$00	; $21BD
	ld ($FFFF),a	; $21BF
	ld a,l	; $21C2
	ld l,$00	; $21C3
	srl a	; $21C5
	rr l	; $21C7
	and $3F	; $21C9
	or $80	; $21CB
	ld h,a	; $21CD
	ret	; $21CE

loc_00_21CF:
	ld a,h	; $21CF
	rlca	; $21D0
	rlca	; $21D1
	rlca	; $21D2
	and $03	; $21D3
	jr z,loc_00_220D	; $21D5
	ld b,$00	; $21D7
	dec a	; $21D9
	jr z,loc_00_21E3	; $21DA
	ld b,$30	; $21DC
	dec a	; $21DE
	jr z,loc_00_21E3	; $21DF
	ld b,$60	; $21E1

loc_00_21E3:
	ld a,($C603)	; $21E3
	cp $03	; $21E6
	jr nz,loc_00_21EC	; $21E8
	ld a,$01	; $21EA

loc_00_21EC:
	add a,a	; $21EC
	add a,a	; $21ED
	add a,a	; $21EE
	add a,a	; $21EF
	add a,b	; $21F0
	ld b,a	; $21F1
	ld a,h	; $21F2
	and $0F	; $21F3
	add a,b	; $21F5
	ld l,$00	; $21F6
	srl a	; $21F8
	rr l	; $21FA
	ld b,$0C	; $21FC
	cp $40	; $21FE
	jr c,loc_00_2205	; $2200
	sub $40	; $2202
	inc b	; $2204

loc_00_2205:
	or $80	; $2205
	ld h,a	; $2207
	ld a,b	; $2208
	ld ($FFFF),a	; $2209
	ret	; $220C

loc_00_220D:
	ld a,$0D	; $220D
	ld ($FFFF),a	; $220F
	ld a,h	; $2212
	and $1F	; $2213
	cp $10	; $2215
	jr nc,loc_00_221F	; $2217
	add a,a	; $2219
	add a,$88	; $221A
	jp loc_00_2232	; $221C

loc_00_221F:
	ld l,$00	; $221F
	srl a	; $2221
	rr l	; $2223
	add a,$96	; $2225
	ld h,a	; $2227
	ret	; $2228

loc_00_2229:
	ld a,$0E	; $2229
	ld ($FFFF),a	; $222B
	ld a,l	; $222E
	add a,a	; $222F
	add a,$A0	; $2230

loc_00_2232:
	ld h,a	; $2232
	ld a,($C603)	; $2233
	ld l,$00	; $2236
	srl a	; $2238
	rr l	; $223A
	or h	; $223C
	ld h,a	; $223D
	ret	; $223E

fn_00_223F:
	inc h	; $223F
	jr z,loc_00_22B6	; $2240
	dec h	; $2242
	jp m,loc_00_225C	; $2243
	ld a,l	; $2246
	rlca	; $2247
	ld a,$08	; $2248
	adc a,$00	; $224A
	ld ($FFFE),a	; $224C
	ld a,l	; $224F
	ld l,$00	; $2250
	srl a	; $2252
	rr l	; $2254
	and $3F	; $2256
	or $40	; $2258
	ld h,a	; $225A
	ret	; $225B

loc_00_225C:
	ld a,h	; $225C
	rlca	; $225D
	rlca	; $225E
	rlca	; $225F
	and $03	; $2260
	jr z,loc_00_229A	; $2262
	ld b,$00	; $2264
	dec a	; $2266
	jr z,loc_00_2270	; $2267
	ld b,$30	; $2269
	dec a	; $226B
	jr z,loc_00_2270	; $226C
	ld b,$60	; $226E

loc_00_2270:
	ld a,($C603)	; $2270
	cp $03	; $2273
	jr nz,loc_00_2279	; $2275
	ld a,$01	; $2277

loc_00_2279:
	add a,a	; $2279
	add a,a	; $227A
	add a,a	; $227B
	add a,a	; $227C
	add a,b	; $227D
	ld b,a	; $227E
	ld a,h	; $227F
	and $0F	; $2280
	add a,b	; $2282
	ld l,$00	; $2283
	srl a	; $2285
	rr l	; $2287
	ld b,$0C	; $2289
	cp $40	; $228B
	jr c,loc_00_2292	; $228D
	sub $40	; $228F
	inc b	; $2291

loc_00_2292:
	or $40	; $2292
	ld h,a	; $2294
	ld a,b	; $2295
	ld ($FFFE),a	; $2296
	ret	; $2299

loc_00_229A:
	ld a,$0D	; $229A
	ld ($FFFE),a	; $229C
	ld a,h	; $229F
	and $1F	; $22A0
	cp $10	; $22A2
	jr nc,loc_00_22AC	; $22A4
	add a,a	; $22A6
	add a,$48	; $22A7
	jp loc_00_22BF	; $22A9

loc_00_22AC:
	ld l,$00	; $22AC
	srl a	; $22AE
	rr l	; $22B0
	add a,$56	; $22B2
	ld h,a	; $22B4
	ret	; $22B5

loc_00_22B6:
	ld a,$0E	; $22B6
	ld ($FFFE),a	; $22B8
	ld a,l	; $22BB
	add a,a	; $22BC
	add a,$60	; $22BD

loc_00_22BF:
	ld h,a	; $22BF
	ld a,($C603)	; $22C0
	ld l,$00	; $22C3
	srl a	; $22C5
	rr l	; $22C7
	or h	; $22C9
	ld h,a	; $22CA
	ret	; $22CB
	.INCBIN "bank00.bin" SKIP $22CC READ 1140	; data, 1140 bytes

fn_00_2740:
	ld hl,$0220	; $2740
	ld de,$0020	; $2743
	ld b,$10	; $2746

loc_00_2748:
	push bc	; $2748
	ld bc,$0210	; $2749
	xor a	; $274C
	call fn_00_0219	; $274D
	ex de,hl	; $2750
	add hl,de	; $2751
	ex de,hl	; $2752
	pop bc	; $2753
	djnz loc_00_2748	; $2754
	ld b,$0B	; $2756

loc_00_2758:
	push bc	; $2758
	push de	; $2759
	ld hl,$22CC	; $275A
	ld bc,$0030	; $275D
	call fn_00_024C	; $2760
	pop hl	; $2763
	ld bc,$0030	; $2764
	add hl,bc	; $2767
	ex de,hl	; $2768
	pop bc	; $2769
	djnz loc_00_2758	; $276A
	ret	; $276C

fn_00_276D:
	push bc	; $276D
	push de	; $276E
	push hl	; $276F
	ld hl,$DA00	; $2770
	ld de,$DA01	; $2773
	ld bc,$01E3	; $2776
	ld (hl),$FF	; $2779
	ldir	; $277B
	pop hl	; $277D
	pop de	; $277E
	pop bc	; $277F
	ret	; $2780

fn_00_2781:
	ld a,($C95B)	; $2781
	or a	; $2784
	jr nz,loc_00_2796	; $2785
	ld hl,$007E	; $2787
	ld ($D800),hl	; $278A
	ld hl,$D800	; $278D
	ld de,$D802	; $2790
	jp fn_00_169D	; $2793

loc_00_2796:
	ld a,$10	; $2796
	call bank_push	; $2798
	ld hl,$D800	; $279B
	ld d,$82	; $279E
	ld b,$FF	; $27A0
	exx	; $27A2
	ld a,($C970)	; $27A3
	add a,$D0	; $27A6
	ld h,a	; $27A8
	ld a,(party_y)	; $27A9
	add a,$FB	; $27AC
	add a,a	; $27AE
	add a,a	; $27AF
	add a,a	; $27B0
	add a,a	; $27B1
	ld l,a	; $27B2
	ld a,(party_x)	; $27B3
	add a,$FB	; $27B6
	and $0F	; $27B8
	ld d,a	; $27BA
	or l	; $27BB
	ld l,a	; $27BC
	ld a,$10	; $27BD
	sub d	; $27BF
	ld d,a	; $27C0
	ld c,$0B	; $27C1

loc_00_27C3:
	push hl	; $27C3
	ld e,d	; $27C4
	ld b,$0B	; $27C5

loc_00_27C7:
	ld a,(hl)	; $27C7
	inc l	; $27C8
	exx	; $27C9
	ld e,a	; $27CA
	ld a,(de)	; $27CB
	ld (hl),a	; $27CC
	inc l	; $27CD
	ld (hl),b	; $27CE
	inc l	; $27CF
	exx	; $27D0
	dec e	; $27D1
	jp nz,loc_00_27D9	; $27D2
	ld a,l	; $27D5
	add a,$F0	; $27D6
	ld l,a	; $27D8

loc_00_27D9:
	djnz loc_00_27C7	; $27D9
	pop hl	; $27DB
	ld a,l	; $27DC
	add a,$10	; $27DD
	ld l,a	; $27DF
	dec c	; $27E0
	jp nz,loc_00_27C3	; $27E1
	jp bank_pop	; $27E4

fn_00_27E7:
	ld a,($C98F)	; $27E7
	or a	; $27EA
	jr nz,loc_00_27F6	; $27EB
	ld hl,$D800	; $27ED
	ld de,$D900	; $27F0
	jp fn_00_169D	; $27F3

loc_00_27F6:
	push ix	; $27F6
	ld hl,$007E	; $27F8
	ld ($D900),hl	; $27FB
	ld hl,$D900	; $27FE
	ld de,$D902	; $2801
	call fn_00_169D	; $2804
	ld ix,$D878	; $2807
	ld iy,$D978	; $280B
	ld b,$05	; $280F

loc_00_2811:
	ld de,$FFEA	; $2811
	add ix,de	; $2814
	add iy,de	; $2816
	ld a,(ix+$01)	; $2818
	ld (iy+$01),a	; $281B
	ld a,(ix+$00)	; $281E
	ld (iy+$00),a	; $2821
	dec b	; $2824
	jr z,loc_00_283D	; $2825
	or a	; $2827
	jr z,loc_00_283D	; $2828
	cp $0C	; $282A
	jr nc,loc_00_283D	; $282C
	ld de,$FFE8	; $282E
	call fn_00_29BA	; $2831
	ld de,$FFEC	; $2834
	call fn_00_29BA	; $2837
	jp loc_00_2811	; $283A

loc_00_283D:
	ld ix,$D878	; $283D
	ld iy,$D978	; $2841
	ld b,$05	; $2845

loc_00_2847:
	ld de,$0016	; $2847
	add ix,de	; $284A
	add iy,de	; $284C
	ld a,(ix+$01)	; $284E
	ld (iy+$01),a	; $2851
	ld a,(ix+$00)	; $2854
	ld (iy+$00),a	; $2857
	dec b	; $285A
	jr z,loc_00_2873	; $285B
	or a	; $285D
	jr z,loc_00_2873	; $285E
	cp $0C	; $2860
	jr nc,loc_00_2873	; $2862
	ld de,$0014	; $2864
	call fn_00_29BA	; $2867
	ld de,$0018	; $286A
	call fn_00_29BA	; $286D
	jp loc_00_2847	; $2870

loc_00_2873:
	ld ix,$D878	; $2873
	ld iy,$D978	; $2877
	ld b,$05	; $287B

loc_00_287D:
	ld de,$FFFE	; $287D
	add ix,de	; $2880
	add iy,de	; $2882
	ld a,(ix+$01)	; $2884
	ld (iy+$01),a	; $2887
	ld a,(ix+$00)	; $288A
	ld (iy+$00),a	; $288D
	dec b	; $2890
	jr z,loc_00_28A9	; $2891
	or a	; $2893
	jr z,loc_00_28A9	; $2894
	cp $0C	; $2896
	jr nc,loc_00_28A9	; $2898
	ld de,$FFE8	; $289A
	call fn_00_29BA	; $289D
	ld de,$0014	; $28A0
	call fn_00_29BA	; $28A3
	jp loc_00_287D	; $28A6

loc_00_28A9:
	ld ix,$D878	; $28A9
	ld iy,$D978	; $28AD
	ld b,$05	; $28B1

loc_00_28B3:
	ld de,$0002	; $28B3
	add ix,de	; $28B6
	add iy,de	; $28B8
	ld a,(ix+$01)	; $28BA
	ld (iy+$01),a	; $28BD
	ld a,(ix+$00)	; $28C0
	ld (iy+$00),a	; $28C3
	dec b	; $28C6
	jr z,loc_00_28DF	; $28C7
	or a	; $28C9
	jr z,loc_00_28DF	; $28CA
	cp $0C	; $28CC
	jr nc,loc_00_28DF	; $28CE
	ld de,$FFEC	; $28D0
	call fn_00_29BA	; $28D3
	ld de,$0018	; $28D6
	call fn_00_29BA	; $28D9
	jp loc_00_28B3	; $28DC

loc_00_28DF:
	ld ix,$D878	; $28DF
	ld iy,$D978	; $28E3
	ld b,$05	; $28E7

loc_00_28E9:
	ld de,$FFE8	; $28E9
	add ix,de	; $28EC
	add iy,de	; $28EE
	ld a,(ix+$01)	; $28F0
	ld (iy+$01),a	; $28F3
	ld a,(ix+$00)	; $28F6
	ld (iy+$00),a	; $28F9
	dec b	; $28FC
	jr z,loc_00_2915	; $28FD
	or a	; $28FF
	jr z,loc_00_2915	; $2900
	cp $0C	; $2902
	jr nc,loc_00_2915	; $2904
	ld de,$FFFE	; $2906
	call fn_00_29BA	; $2909
	ld de,$FFEA	; $290C
	call fn_00_29BA	; $290F
	jp loc_00_28E9	; $2912

loc_00_2915:
	ld ix,$D878	; $2915
	ld iy,$D978	; $2919
	ld b,$05	; $291D

loc_00_291F:
	ld de,$FFEC	; $291F
	add ix,de	; $2922
	add iy,de	; $2924
	ld a,(ix+$01)	; $2926
	ld (iy+$01),a	; $2929
	ld a,(ix+$00)	; $292C
	ld (iy+$00),a	; $292F
	dec b	; $2932
	jr z,loc_00_294B	; $2933
	or a	; $2935
	jr z,loc_00_294B	; $2936
	cp $0C	; $2938
	jr nc,loc_00_294B	; $293A
	ld de,$0002	; $293C
	call fn_00_29BA	; $293F
	ld de,$FFEA	; $2942
	call fn_00_29BA	; $2945
	jp loc_00_291F	; $2948

loc_00_294B:
	ld ix,$D878	; $294B
	ld iy,$D978	; $294F
	ld b,$05	; $2953

loc_00_2955:
	ld de,$0014	; $2955
	add ix,de	; $2958
	add iy,de	; $295A
	ld a,(ix+$01)	; $295C
	ld (iy+$01),a	; $295F
	ld a,(ix+$00)	; $2962
	ld (iy+$00),a	; $2965
	dec b	; $2968
	jr z,loc_00_2981	; $2969
	or a	; $296B
	jr z,loc_00_2981	; $296C
	cp $0C	; $296E
	jr nc,loc_00_2981	; $2970
	ld de,$FFFE	; $2972
	call fn_00_29BA	; $2975
	ld de,$0016	; $2978
	call fn_00_29BA	; $297B
	jp loc_00_2955	; $297E

loc_00_2981:
	ld ix,$D878	; $2981
	ld iy,$D978	; $2985
	ld b,$05	; $2989

loc_00_298B:
	ld de,$0018	; $298B
	add ix,de	; $298E
	add iy,de	; $2990
	ld a,(ix+$01)	; $2992
	ld (iy+$01),a	; $2995
	ld a,(ix+$00)	; $2998
	ld (iy+$00),a	; $299B
	dec b	; $299E
	jr z,loc_00_29B7	; $299F
	or a	; $29A1
	jr z,loc_00_29B7	; $29A2
	cp $0C	; $29A4
	jr nc,loc_00_29B7	; $29A6
	ld de,$0002	; $29A8
	call fn_00_29BA	; $29AB
	ld de,$0016	; $29AE
	call fn_00_29BA	; $29B1
	jp loc_00_298B	; $29B4

loc_00_29B7:
	pop ix	; $29B7
	ret	; $29B9

fn_00_29BA:
	push ix	; $29BA
	push iy	; $29BC
	ld c,b	; $29BE

loc_00_29BF:
	add ix,de	; $29BF
	add iy,de	; $29C1
	ld a,(ix+$01)	; $29C3
	ld (iy+$01),a	; $29C6
	ld a,(ix+$00)	; $29C9
	ld (iy+$00),a	; $29CC
	dec c	; $29CF
	jr z,loc_00_29DA	; $29D0
	or a	; $29D2
	jr z,loc_00_29DA	; $29D3
	cp $0C	; $29D5
	jp c,loc_00_29BF	; $29D7

loc_00_29DA:
	pop iy	; $29DA
	pop ix	; $29DC
	ret	; $29DE

fn_00_29DF:
	ld a,$0F	; $29DF
	call bank_push	; $29E1
	ld hl,$D900	; $29E4
	ld de,$8000	; $29E7
	ld bc,$7980	; $29EA

loc_00_29ED:
	ld a,(hl)	; $29ED
	cp $7E	; $29EE
	jr z,loc_00_29F7	; $29F0
	ld a,(de)	; $29F2
	or a	; $29F3
	jp nz,loc_00_29FB	; $29F4

loc_00_29F7:
	inc l	; $29F7
	jp loc_00_29FE	; $29F8

loc_00_29FB:
	ld (hl),c	; $29FB
	inc l	; $29FC
	ld (hl),a	; $29FD

loc_00_29FE:
	inc l	; $29FE
	inc e	; $29FF
	djnz loc_00_29ED	; $2A00
	jp bank_pop	; $2A02

fn_00_2A05:
	call fn_01_6C97	; $2A05

loc_00_2A08:
	ld a,(iy+$03)	; $2A08
	or a	; $2A0B
	jr z,loc_00_2A3E	; $2A0C
	ld hl,(party_x)	; $2A0E
	ld a,(iy+$02)	; $2A11
	add a,$05	; $2A14
	sub h	; $2A16
	and $0F	; $2A17
	cp $0B	; $2A19
	jr nc,loc_00_2A3E	; $2A1B
	call fn_00_36ED	; $2A1D
	ld h,a	; $2A20
	ld a,(iy+$01)	; $2A21
	add a,$05	; $2A24
	sub l	; $2A26
	and $0F	; $2A27
	cp $0B	; $2A29
	jr nc,loc_00_2A3E	; $2A2B
	add a,h	; $2A2D
	add a,a	; $2A2E
	ld l,a	; $2A2F
	ld h,$D9	; $2A30
	ld a,(hl)	; $2A32
	cp $7E	; $2A33
	jr z,loc_00_2A3E	; $2A35
	ld a,(iy+$00)	; $2A37
	ld (hl),a	; $2A3A
	inc l	; $2A3B
	ld (hl),$00	; $2A3C

loc_00_2A3E:
	add iy,de	; $2A3E
	djnz loc_00_2A08	; $2A40
	ld hl,(party_x)	; $2A42
	call fn_01_6DC0	; $2A45
	ld a,(hl)	; $2A48
	ld ($C990),a	; $2A49
	ld hl,($C959)	; $2A4C
	ld ($D978),hl	; $2A4F
	ld hl,$D878	; $2A52
	ld a,(hl)	; $2A55
	cp $0C	; $2A56
	ret nz	; $2A58
	ld (hl),$01	; $2A59
	ret	; $2A5B

load_dungeon:
	call dungeon_header_address	; $2A5C
	call bank_push	; $2A5F
	call fn_00_0097	; $2A62
	ld de,$D000	; $2A65
	call decode_rle	; $2A68
	jp bank_pop	; $2A6B

dungeon_header_address:
	ld a,(location)	; $2A6E
	sub $12	; $2A71
	ld b,a	; $2A73
	add a,a	; $2A74
	add a,b	; $2A75
	ld hl,$2A81	; $2A76
	rst $30	; $2A79
	ld a,(hl)	; $2A7A
	inc hl	; $2A7B
	ld e,(hl)	; $2A7C
	inc hl	; $2A7D
	ld d,(hl)	; $2A7E
	ex de,hl	; $2A7F
	ret	; $2A80
	.INCBIN "bank00.bin" SKIP $2A81 READ 32	; data, 32 bytes

multiply_8x8:
	push de	; $2AA1
	ld e,l	; $2AA2
	ld d,$00	; $2AA3
	ld l,d	; $2AA5
	add hl,hl	; $2AA6
	jr nc,loc_00_2AAA	; $2AA7
	add hl,de	; $2AA9

loc_00_2AAA:
	add hl,hl	; $2AAA
	jr nc,loc_00_2AAE	; $2AAB
	add hl,de	; $2AAD

loc_00_2AAE:
	add hl,hl	; $2AAE
	jr nc,loc_00_2AB2	; $2AAF
	add hl,de	; $2AB1

loc_00_2AB2:
	add hl,hl	; $2AB2
	jr nc,loc_00_2AB6	; $2AB3
	add hl,de	; $2AB5

loc_00_2AB6:
	add hl,hl	; $2AB6
	jr nc,loc_00_2ABA	; $2AB7
	add hl,de	; $2AB9

loc_00_2ABA:
	add hl,hl	; $2ABA
	jr nc,loc_00_2ABE	; $2ABB
	add hl,de	; $2ABD

loc_00_2ABE:
	add hl,hl	; $2ABE
	jr nc,loc_00_2AC2	; $2ABF
	add hl,de	; $2AC1

loc_00_2AC2:
	add hl,hl	; $2AC2
	jr nc,loc_00_2AC6	; $2AC3
	add hl,de	; $2AC5

loc_00_2AC6:
	pop de	; $2AC6
	ret	; $2AC7

fn_00_2AC8:
	ld a,l	; $2AC8
	ex af,af'	; $2AC9
	ld a,h	; $2ACA
	ld hl,$0000	; $2ACB
	or a	; $2ACE
	call nz,fn_00_2AD3	; $2ACF
	ex af,af'	; $2AD2

fn_00_2AD3:
	add hl,hl	; $2AD3
	rlca	; $2AD4
	jr nc,loc_00_2AD8	; $2AD5
	add hl,de	; $2AD7

loc_00_2AD8:
	add hl,hl	; $2AD8
	rlca	; $2AD9
	jr nc,loc_00_2ADD	; $2ADA
	add hl,de	; $2ADC

loc_00_2ADD:
	add hl,hl	; $2ADD
	rlca	; $2ADE
	jr nc,loc_00_2AE2	; $2ADF
	add hl,de	; $2AE1

loc_00_2AE2:
	add hl,hl	; $2AE2
	rlca	; $2AE3
	jr nc,loc_00_2AE7	; $2AE4
	add hl,de	; $2AE6

loc_00_2AE7:
	add hl,hl	; $2AE7
	rlca	; $2AE8
	jr nc,loc_00_2AEC	; $2AE9
	add hl,de	; $2AEB

loc_00_2AEC:
	add hl,hl	; $2AEC
	rlca	; $2AED
	jr nc,loc_00_2AF1	; $2AEE
	add hl,de	; $2AF0

loc_00_2AF1:
	add hl,hl	; $2AF1
	rlca	; $2AF2
	jr nc,loc_00_2AF6	; $2AF3
	add hl,de	; $2AF5

loc_00_2AF6:
	add hl,hl	; $2AF6
	rlca	; $2AF7
	jr nc,loc_00_2AFB	; $2AF8
	add hl,de	; $2AFA

loc_00_2AFB:
	ret	; $2AFB

divide_8:
	xor a	; $2AFC
	sla l	; $2AFD
	rla	; $2AFF
	cp h	; $2B00
	jr c,loc_00_2B05	; $2B01
	sub h	; $2B03
	inc l	; $2B04

loc_00_2B05:
	sla l	; $2B05
	rla	; $2B07
	cp h	; $2B08
	jr c,loc_00_2B0D	; $2B09
	sub h	; $2B0B
	inc l	; $2B0C

loc_00_2B0D:
	sla l	; $2B0D
	rla	; $2B0F
	cp h	; $2B10
	jr c,loc_00_2B15	; $2B11
	sub h	; $2B13
	inc l	; $2B14

loc_00_2B15:
	sla l	; $2B15
	rla	; $2B17
	cp h	; $2B18
	jr c,loc_00_2B1D	; $2B19
	sub h	; $2B1B
	inc l	; $2B1C

loc_00_2B1D:
	sla l	; $2B1D
	rla	; $2B1F
	cp h	; $2B20
	jr c,loc_00_2B25	; $2B21
	sub h	; $2B23
	inc l	; $2B24

loc_00_2B25:
	sla l	; $2B25
	rla	; $2B27
	cp h	; $2B28
	jr c,loc_00_2B2D	; $2B29
	sub h	; $2B2B
	inc l	; $2B2C

loc_00_2B2D:
	sla l	; $2B2D
	rla	; $2B2F
	cp h	; $2B30
	jr c,loc_00_2B35	; $2B31
	sub h	; $2B33
	inc l	; $2B34

loc_00_2B35:
	sla l	; $2B35
	rla	; $2B37
	cp h	; $2B38
	jr c,loc_00_2B3D	; $2B39
	sub h	; $2B3B
	inc l	; $2B3C

loc_00_2B3D:
	ld h,a	; $2B3D
	ret	; $2B3E

fn_00_2B3F:
	ld a,d	; $2B3F
	or e	; $2B40
	jr z,loc_00_2B56	; $2B41
	push bc	; $2B43
	ld a,h	; $2B44
	ld b,l	; $2B45
	ld hl,$0000	; $2B46
	call fn_00_2B5A	; $2B49
	ld c,a	; $2B4C
	ld a,b	; $2B4D
	call fn_00_2B5A	; $2B4E
	ex de,hl	; $2B51
	ld h,c	; $2B52
	ld l,a	; $2B53
	pop bc	; $2B54
	ret	; $2B55

loc_00_2B56:
	ld hl,$0000	; $2B56
	ret	; $2B59

fn_00_2B5A:
	add a,a	; $2B5A
	adc hl,hl	; $2B5B
	inc a	; $2B5D
	sbc hl,de	; $2B5E
	jr nc,loc_00_2B64	; $2B60
	add hl,de	; $2B62
	dec a	; $2B63

loc_00_2B64:
	add a,a	; $2B64
	adc hl,hl	; $2B65
	inc a	; $2B67
	sbc hl,de	; $2B68
	jr nc,loc_00_2B6E	; $2B6A
	add hl,de	; $2B6C
	dec a	; $2B6D

loc_00_2B6E:
	add a,a	; $2B6E
	adc hl,hl	; $2B6F
	inc a	; $2B71
	sbc hl,de	; $2B72
	jr nc,loc_00_2B78	; $2B74
	add hl,de	; $2B76
	dec a	; $2B77

loc_00_2B78:
	add a,a	; $2B78
	adc hl,hl	; $2B79
	inc a	; $2B7B
	sbc hl,de	; $2B7C
	jr nc,loc_00_2B82	; $2B7E
	add hl,de	; $2B80
	dec a	; $2B81

loc_00_2B82:
	add a,a	; $2B82
	adc hl,hl	; $2B83
	inc a	; $2B85
	sbc hl,de	; $2B86
	jr nc,loc_00_2B8C	; $2B88
	add hl,de	; $2B8A
	dec a	; $2B8B

loc_00_2B8C:
	add a,a	; $2B8C
	adc hl,hl	; $2B8D
	inc a	; $2B8F
	sbc hl,de	; $2B90
	jr nc,loc_00_2B96	; $2B92
	add hl,de	; $2B94
	dec a	; $2B95

loc_00_2B96:
	add a,a	; $2B96
	adc hl,hl	; $2B97
	inc a	; $2B99
	sbc hl,de	; $2B9A
	jr nc,loc_00_2BA0	; $2B9C
	add hl,de	; $2B9E
	dec a	; $2B9F

loc_00_2BA0:
	add a,a	; $2BA0
	adc hl,hl	; $2BA1
	inc a	; $2BA3
	sbc hl,de	; $2BA4
	jr nc,loc_00_2BAA	; $2BA6
	add hl,de	; $2BA8
	dec a	; $2BA9

loc_00_2BAA:
	ret	; $2BAA
	.INCBIN "bank00.bin" SKIP $2BAB READ 20	; data, 20 bytes

random:
	push de	; $2BBF
	push hl	; $2BC0
	ld hl,(rng_seed)	; $2BC1
	ld d,h	; $2BC4
	ld e,l	; $2BC5
	add hl,de	; $2BC6
	add hl,de	; $2BC7
	ld a,e	; $2BC8
	add a,h	; $2BC9
	ld h,a	; $2BCA
	ld (rng_seed),hl	; $2BCB
	ld hl,input_noise	; $2BCE
	add a,(hl)	; $2BD1
	pop hl	; $2BD2
	pop de	; $2BD3
	ret	; $2BD4
	.INCBIN "bank00.bin" SKIP $2BD5 READ 22	; data, 22 bytes

random_below:
	push hl	; $2BEB
	ld l,a	; $2BEC
	call random	; $2BED
	ld h,a	; $2BF0
	call multiply_8x8	; $2BF1
	ld a,h	; $2BF4
	pop hl	; $2BF5
	ret	; $2BF6

fn_00_2BF7:
	ld hl,$55AA	; $2BF7
	ld (rng_seed),hl	; $2BFA
	ret	; $2BFD

fn_00_2BFE:
	ld bc,$0000	; $2BFE
	ld a,e	; $2C01
	sub l	; $2C02
	jr nc,loc_00_2C08	; $2C03
	inc c	; $2C05
	neg	; $2C06

loc_00_2C08:
	ld l,a	; $2C08
	ld a,d	; $2C09
	sub h	; $2C0A
	jr nc,loc_00_2C11	; $2C0B
	set 1,c	; $2C0D
	neg	; $2C0F

loc_00_2C11:
	ld h,a	; $2C11
	cp l	; $2C12
	jr c,loc_00_2C19	; $2C13
	set 2,c	; $2C15
	ld h,l	; $2C17
	ld l,a	; $2C18

loc_00_2C19:
	ld d,b	; $2C19
	ld e,b	; $2C1A
	ld a,h	; $2C1B
	add a,a	; $2C1C
	jr c,loc_00_2C22	; $2C1D
	cp l	; $2C1F
	jr c,loc_00_2C24	; $2C20

loc_00_2C22:
	sub l	; $2C22
	inc e	; $2C23

loc_00_2C24:
	sla e	; $2C24
	add a,a	; $2C26
	jr c,loc_00_2C2C	; $2C27
	cp l	; $2C29
	jr c,loc_00_2C2E	; $2C2A

loc_00_2C2C:
	sub l	; $2C2C
	inc e	; $2C2D

loc_00_2C2E:
	sla e	; $2C2E
	add a,a	; $2C30
	jr c,loc_00_2C36	; $2C31
	cp l	; $2C33
	jr c,loc_00_2C38	; $2C34

loc_00_2C36:
	sub l	; $2C36
	inc e	; $2C37

loc_00_2C38:
	sla e	; $2C38
	add a,a	; $2C3A
	jr c,loc_00_2C40	; $2C3B
	cp l	; $2C3D
	jr c,loc_00_2C42	; $2C3E

loc_00_2C40:
	sub l	; $2C40
	inc e	; $2C41

loc_00_2C42:
	sla e	; $2C42
	add a,a	; $2C44
	jr c,loc_00_2C4A	; $2C45
	cp l	; $2C47
	jr c,loc_00_2C4C	; $2C48

loc_00_2C4A:
	sub l	; $2C4A
	inc e	; $2C4B

loc_00_2C4C:
	sla e	; $2C4C
	add a,a	; $2C4E
	jr c,loc_00_2C54	; $2C4F
	cp l	; $2C51
	jr c,loc_00_2C56	; $2C52

loc_00_2C54:
	sub l	; $2C54
	inc e	; $2C55

loc_00_2C56:
	ld hl,$2C7C	; $2C56
	add hl,de	; $2C59
	ld a,(hl)	; $2C5A
	ld hl,$2CBD	; $2C5B
	add hl,de	; $2C5E
	ld l,(hl)	; $2C5F
	ld e,a	; $2C60
	ld d,b	; $2C61
	ld h,b	; $2C62
	bit 2,c	; $2C63
	jr z,loc_00_2C68	; $2C65
	ex de,hl	; $2C67

loc_00_2C68:
	bit 1,c	; $2C68
	jr z,loc_00_2C72	; $2C6A
	xor a	; $2C6C
	sub e	; $2C6D
	ld e,a	; $2C6E
	sbc a,a	; $2C6F
	sub d	; $2C70
	ld d,a	; $2C71

loc_00_2C72:
	bit 0,c	; $2C72
	ret z	; $2C74
	xor a	; $2C75
	sub l	; $2C76
	ld l,a	; $2C77
	sbc a,a	; $2C78
	sub h	; $2C79
	ld h,a	; $2C7A
	ret	; $2C7B
	.INCBIN "bank00.bin" SKIP $2C7C READ 130	; data, 130 bytes

loc_00_2CFE:
	ld hl,$CA00	; $2CFE
	ld de,$CA01	; $2D01
	ld bc,$05FF	; $2D04
	ld (hl),$00	; $2D07
	ldir	; $2D09
	ret	; $2D0B

fn_00_2D0C:
	ld a,e	; $2D0C
	add a,a	; $2D0D
	add a,a	; $2D0E
	add a,a	; $2D0F
	ld e,a	; $2D10
	ld a,d	; $2D11
	rra	; $2D12
	rr e	; $2D13
	rra	; $2D15
	rr e	; $2D16
	add a,$CA	; $2D18
	ld d,a	; $2D1A
	ret	; $2D1B

loc_00_2D1C:
	call bank_push	; $2D1C
	call fn_00_2D25	; $2D1F
	jp bank_pop	; $2D22

fn_00_2D25:
	ld e,(hl)	; $2D25
	inc hl	; $2D26
	ld d,(hl)	; $2D27
	inc hl	; $2D28
	ld c,(hl)	; $2D29
	inc hl	; $2D2A
	ld b,(hl)	; $2D2B
	inc hl	; $2D2C
	push bc	; $2D2D
	push de	; $2D2E
	sla c	; $2D2F

loc_00_2D31:
	push bc	; $2D31
	push de	; $2D32
	ld b,$00	; $2D33
	ldir	; $2D35
	pop de	; $2D37
	ld a,e	; $2D38
	add a,$40	; $2D39
	ld e,a	; $2D3B
	adc a,d	; $2D3C
	sub e	; $2D3D
	ld d,a	; $2D3E
	pop bc	; $2D3F
	djnz loc_00_2D31	; $2D40
	pop de	; $2D42
	pop bc	; $2D43
	jp fn_00_0499	; $2D44

loc_00_2D47:
	ld hl,$0000	; $2D47

loc_00_2D4A:
	ex de,hl	; $2D4A

loc_00_2D4B:
	push bc	; $2D4B
	push hl	; $2D4C
	ld b,c	; $2D4D

loc_00_2D4E:
	ld (hl),e	; $2D4E
	inc l	; $2D4F
	ld (hl),d	; $2D50
	inc l	; $2D51
	djnz loc_00_2D4E	; $2D52
	pop hl	; $2D54
	ld bc,$0040	; $2D55
	add hl,bc	; $2D58
	pop bc	; $2D59
	djnz loc_00_2D4B	; $2D5A
	ret	; $2D5C

fn_00_2D5D:
	ld a,(hl)	; $2D5D
	inc a	; $2D5E
	ret z	; $2D5F
	call fn_00_2D65	; $2D60
	jr fn_00_2D5D	; $2D63

fn_00_2D65:
	ld e,(hl)	; $2D65
	inc hl	; $2D66
	ld d,(hl)	; $2D67
	inc hl	; $2D68

fn_00_2D69:
	call fn_00_2D0C	; $2D69

fn_00_2D6C:
	ld a,(hl)	; $2D6C
	inc hl	; $2D6D
	or a	; $2D6E
	ret z	; $2D6F
	add a,$40	; $2D70
	ld (de),a	; $2D72
	inc e	; $2D73
	ld a,$01	; $2D74
	ld (de),a	; $2D76
	inc e	; $2D77
	jp fn_00_2D6C	; $2D78

fn_00_2D7B:
	ld bc,$03E8	; $2D7B
	call fn_00_2D90	; $2D7E

fn_00_2D81:
	ld bc,$0064	; $2D81
	call fn_00_2D90	; $2D84

fn_00_2D87:
	ld bc,$000A	; $2D87
	call fn_00_2D90	; $2D8A
	ld a,l	; $2D8D
	jr loc_00_2D98	; $2D8E

fn_00_2D90:
	xor a	; $2D90

loc_00_2D91:
	sbc hl,bc	; $2D91
	inc a	; $2D93
	jr nc,loc_00_2D91	; $2D94
	add hl,bc	; $2D96
	dec a	; $2D97

loc_00_2D98:
	add a,$30	; $2D98

fn_00_2D9A:
	add a,$40	; $2D9A
	ld (de),a	; $2D9C
	inc e	; $2D9D
	ld a,$01	; $2D9E
	ld (de),a	; $2DA0
	inc e	; $2DA1
	ret	; $2DA2
	.INCBIN "bank00.bin" SKIP $2DA3 READ 4	; data, 4 bytes

loc_00_2DA7:
	call fn_00_2D0C	; $2DA7
	push de	; $2DAA
	ld c,$00	; $2DAB

loc_00_2DAD:
	ld a,(hl)	; $2DAD
	inc hl	; $2DAE
	or a	; $2DAF
	jr z,loc_00_2DBE	; $2DB0
	inc c	; $2DB2
	add a,$40	; $2DB3
	ld (de),a	; $2DB5
	inc e	; $2DB6
	ld a,$01	; $2DB7
	ld (de),a	; $2DB9
	inc e	; $2DBA
	jp loc_00_2DAD	; $2DBB

loc_00_2DBE:
	pop de	; $2DBE
	ld b,$01	; $2DBF
	jp fn_00_0499	; $2DC1

fn_00_2DC4:
	call fn_00_2D0C	; $2DC4
	push hl	; $2DC7
	push bc	; $2DC8
	push de	; $2DC9
	ex de,hl	; $2DCA
	ld de,$0040	; $2DCB

loc_00_2DCE:
	push bc	; $2DCE
	push hl	; $2DCF
	ld b,c	; $2DD0

loc_00_2DD1:
	inc l	; $2DD1
	res 3,(hl)	; $2DD2
	inc l	; $2DD4
	djnz loc_00_2DD1	; $2DD5
	pop hl	; $2DD7
	pop bc	; $2DD8
	add hl,de	; $2DD9
	djnz loc_00_2DCE	; $2DDA
	pop de	; $2DDC
	pop bc	; $2DDD
	call fn_00_0499	; $2DDE
	pop hl	; $2DE1
	ret	; $2DE2

fn_00_2DE3:
	call fn_00_2D0C	; $2DE3
	push hl	; $2DE6
	push bc	; $2DE7
	push de	; $2DE8
	ex de,hl	; $2DE9
	ld de,$0040	; $2DEA

loc_00_2DED:
	push bc	; $2DED
	push hl	; $2DEE
	ld b,c	; $2DEF

loc_00_2DF0:
	inc l	; $2DF0
	set 3,(hl)	; $2DF1
	inc l	; $2DF3
	djnz loc_00_2DF0	; $2DF4
	pop hl	; $2DF6
	pop bc	; $2DF7
	add hl,de	; $2DF8
	djnz loc_00_2DED	; $2DF9
	pop de	; $2DFB
	pop bc	; $2DFC
	call fn_00_0499	; $2DFD
	pop hl	; $2E00
	ret	; $2E01

loc_00_2E02:
	push bc	; $2E02
	ld l,e	; $2E03
	ld h,d	; $2E04
	inc l	; $2E05

loc_00_2E06:
	push bc	; $2E06
	push hl	; $2E07
	ld b,c	; $2E08

loc_00_2E09:
	set 4,(hl)	; $2E09
	inc l	; $2E0B
	inc l	; $2E0C
	djnz loc_00_2E09	; $2E0D
	pop hl	; $2E0F
	ld bc,$0040	; $2E10
	add hl,bc	; $2E13
	pop bc	; $2E14
	djnz loc_00_2E06	; $2E15
	pop bc	; $2E17
	jp fn_00_0499	; $2E18

fn_00_2E1B:
	push bc	; $2E1B
	push de	; $2E1C
	push hl	; $2E1D
	ld de,$CA64	; $2E1E
	ld a,$10	; $2E21

loc_00_2E23:
	ld hl,$2E38	; $2E23
	ld bc,$001C	; $2E26
	ldir	; $2E29
	ex de,hl	; $2E2B
	ld bc,$0024	; $2E2C
	add hl,bc	; $2E2F
	ex de,hl	; $2E30
	dec a	; $2E31
	jr nz,loc_00_2E23	; $2E32
	pop hl	; $2E34
	pop de	; $2E35
	pop bc	; $2E36
	ret	; $2E37
	.INCBIN "bank00.bin" SKIP $2E38 READ 28	; data, 28 bytes

fn_00_2E54:
	push bc	; $2E54
	push de	; $2E55
	push hl	; $2E56
	ld hl,$CA64	; $2E57
	ld de,$3864	; $2E5A
	ld bc,$10BE	; $2E5D

loc_00_2E60:
	rst $10	; $2E60
	push bc	; $2E61
	ld b,$1C	; $2E62

loc_00_2E64:
	outi	; $2E64
	jr nz,loc_00_2E64	; $2E66
	ld bc,$0024	; $2E68
	add hl,bc	; $2E6B
	ex de,hl	; $2E6C
	ld bc,$0040	; $2E6D
	add hl,bc	; $2E70
	ex de,hl	; $2E71
	pop bc	; $2E72
	djnz loc_00_2E60	; $2E73
	pop hl	; $2E75
	pop de	; $2E76
	pop bc	; $2E77
	ret	; $2E78

fn_00_2E79:
	call fn_00_2E1B	; $2E79
	jp fn_00_2D5D	; $2E7C

fn_00_2E7F:
	call fn_00_2E1B	; $2E7F
	call fn_00_2D5D	; $2E82
	jp fn_00_2E54	; $2E85

loc_00_2E88:
	call fn_00_2EB8	; $2E88
	ld ($C608),de	; $2E8B
	call fn_00_2EB8	; $2E8F
	ld ($C60A),de	; $2E92
	call fn_00_2EB8	; $2E96
	push de	; $2E99
	call fn_00_2EB8	; $2E9A
	ld b,(hl)	; $2E9D
	inc hl	; $2E9E
	ld c,(hl)	; $2E9F
	pop hl	; $2EA0

loc_00_2EA1:
	push bc	; $2EA1
	push hl	; $2EA2
	ld b,c	; $2EA3

loc_00_2EA4:
	ld (hl),e	; $2EA4
	inc l	; $2EA5
	ld (hl),d	; $2EA6
	ld a,l	; $2EA7
	add a,$3F	; $2EA8
	ld l,a	; $2EAA
	adc a,h	; $2EAB
	sub l	; $2EAC
	ld h,a	; $2EAD
	inc de	; $2EAE
	djnz loc_00_2EA4	; $2EAF
	pop hl	; $2EB1
	pop bc	; $2EB2
	inc l	; $2EB3
	inc l	; $2EB4
	djnz loc_00_2EA1	; $2EB5
	ret	; $2EB7

fn_00_2EB8:
	ld e,(hl)	; $2EB8
	inc hl	; $2EB9
	ld d,(hl)	; $2EBA
	inc hl	; $2EBB
	ret	; $2EBC

fn_00_2EBD:
	sub $20	; $2EBD
	ld l,a	; $2EBF
	ld h,$00	; $2EC0
	add hl,hl	; $2EC2
	add hl,hl	; $2EC3
	add hl,hl	; $2EC4
	ld de,$BB58	; $2EC5
	add hl,de	; $2EC8
	ld a,$0F	; $2EC9
	call bank_push	; $2ECB
	ld a,($C60C)	; $2ECE
	ld b,a	; $2ED1
	and $07	; $2ED2
	inc a	; $2ED4
	ld c,a	; $2ED5
	ld a,b	; $2ED6
	exx	; $2ED7
	rrca	; $2ED8
	rrca	; $2ED9
	rrca	; $2EDA
	and $1F	; $2EDB
	ld hl,($C608)	; $2EDD
	jr z,loc_00_2EEA	; $2EE0
	ld de,($C60A)	; $2EE2
	ld b,a	; $2EE6

loc_00_2EE7:
	add hl,de	; $2EE7
	djnz loc_00_2EE7	; $2EE8

loc_00_2EEA:
	ex de,hl	; $2EEA
	ld a,($C60D)	; $2EEB
	ld l,a	; $2EEE
	ld h,$00	; $2EEF
	add hl,hl	; $2EF1
	add hl,hl	; $2EF2
	add hl,de	; $2EF3
	push hl	; $2EF4
	rst $08	; $2EF5
	ex de,hl	; $2EF6
	ld hl,$C1C0	; $2EF7
	ld bc,$20BE	; $2EFA

loc_00_2EFD:
	ini	; $2EFD
	jr nz,loc_00_2EFD	; $2EFF
	ld hl,($C60A)	; $2F01
	add hl,de	; $2F04
	rst $08	; $2F05
	ld hl,$C1E0	; $2F06
	ld b,$20	; $2F09

loc_00_2F0B:
	ini	; $2F0B
	jr nz,loc_00_2F0B	; $2F0D
	pop de	; $2F0F
	rst $10	; $2F10
	push de	; $2F11
	exx	; $2F12
	push iy	; $2F13
	ld iy,$C1C0	; $2F15
	ld d,$00	; $2F19
	ld a,$08	; $2F1B

loc_00_2F1D:
	ex af,af'	; $2F1D
	ld a,(iy+$00)	; $2F1E
	out ($BE),a	; $2F21
	ld a,(hl)	; $2F23
	inc hl	; $2F24
	ld e,d	; $2F25
	ld b,c	; $2F26
	dec b	; $2F27
	jr z,loc_00_2F30	; $2F28
	or a	; $2F2A

loc_00_2F2B:
	rra	; $2F2B
	rr e	; $2F2C
	djnz loc_00_2F2B	; $2F2E

loc_00_2F30:
	or (iy+$01)	; $2F30
	out ($BE),a	; $2F33
	ld a,e	; $2F35
	or (iy+$21)	; $2F36
	ld (iy+$21),a	; $2F39
	ld a,(iy+$02)	; $2F3C
	out ($BE),a	; $2F3F
	ld a,(iy+$03)	; $2F41
	out ($BE),a	; $2F44
	ld e,$04	; $2F46
	add iy,de	; $2F48
	ex af,af'	; $2F4A
	dec a	; $2F4B
	jp nz,loc_00_2F1D	; $2F4C
	pop iy	; $2F4F
	pop de	; $2F51
	ld hl,($C60A)	; $2F52
	add hl,de	; $2F55
	ex de,hl	; $2F56
	rst $10	; $2F57
	ld hl,$C1E0	; $2F58
	ld bc,$20BE	; $2F5B

loc_00_2F5E:
	outi	; $2F5E
	jr nz,loc_00_2F5E	; $2F60
	ld a,($C60C)	; $2F62
	add a,$08	; $2F65
	ld ($C60C),a	; $2F67
	jp bank_pop	; $2F6A

fn_00_2F6D:
	ld e,(hl)	; $2F6D
	inc hl	; $2F6E
	ld d,(hl)	; $2F6F
	inc hl	; $2F70
	ld ($C60C),de	; $2F71

loc_00_2F75:
	ld a,(hl)	; $2F75
	inc hl	; $2F76
	or a	; $2F77
	ret z	; $2F78
	push hl	; $2F79
	call fn_00_2EBD	; $2F7A
	pop hl	; $2F7D
	jr loc_00_2F75	; $2F7E

loc_00_2F80:
	push bc	; $2F80
	call fn_00_2F6D	; $2F81
	pop bc	; $2F84
	djnz loc_00_2F80	; $2F85
	ret	; $2F87

loc_00_2F88:
	ld e,(hl)	; $2F88
	inc hl	; $2F89
	ld d,(hl)	; $2F8A
	inc hl	; $2F8B
	ld ($C60C),de	; $2F8C

loc_00_2F90:
	ld a,(hl)	; $2F90
	inc hl	; $2F91
	push hl	; $2F92
	call fn_00_2EBD	; $2F93
	pop hl	; $2F96
	ld b,(hl)	; $2F97
	inc hl	; $2F98
	inc b	; $2F99
	dec b	; $2F9A
	ret z	; $2F9B
	ld a,($C60C)	; $2F9C
	add a,$F8	; $2F9F
	add a,b	; $2FA1
	ld ($C60C),a	; $2FA2
	jr loc_00_2F90	; $2FA5

fn_00_2FA7:
	ld a,$0F	; $2FA7
	call bank_push	; $2FA9
	ld de,$2BA0	; $2FAC
	rst $10	; $2FAF
	ld hl,$BB40	; $2FB0
	ld c,$63	; $2FB3

loc_00_2FB5:
	ld b,$08	; $2FB5

loc_00_2FB7:
	ld a,(hl)	; $2FB7
	inc hl	; $2FB8
	cpl	; $2FB9
	out ($BE),a	; $2FBA
	call fn_00_0262	; $2FBC
	cpl	; $2FBF
	out ($BE),a	; $2FC0
	call fn_00_0262	; $2FC2
	xor a	; $2FC5
	out ($BE),a	; $2FC6
	call fn_00_0262	; $2FC8
	out ($BE),a	; $2FCB
	call fn_00_0262	; $2FCD
	djnz loc_00_2FB7	; $2FD0
	dec c	; $2FD2
	jr nz,loc_00_2FB5	; $2FD3
	jp bank_pop	; $2FD5

loc_00_2FD8:
	ld a,$0F	; $2FD8
	call bank_push	; $2FDA
	ld de,$2C00	; $2FDD
	rst $10	; $2FE0
	ld hl,$BB58	; $2FE1
	ld c,$BE	; $2FE4
	ld d,$60	; $2FE6
	xor a	; $2FE8

loc_00_2FE9:
	ld b,$08	; $2FE9

loc_00_2FEB:
	outi	; $2FEB
	out ($BE),a	; $2FED
	out ($BE),a	; $2FEF
	out ($BE),a	; $2FF1
	jr nz,loc_00_2FEB	; $2FF3
	dec d	; $2FF5
	jr nz,loc_00_2FE9	; $2FF6
	jp bank_pop	; $2FF8

frame_tick:
	ld a,(map_mode)	; $2FFB
	cp $03	; $2FFE
	jr z,loc_00_3006	; $3000
	cp $05	; $3002
	jr nz,loc_00_302A	; $3004

loc_00_3006:
	call fn_01_48E1	; $3006
	ld l,(iy+$00)	; $3009
	ld h,(iy+$01)	; $300C
	ld a,$01	; $300F
	call fn_00_30B6	; $3011
	ld a,($C63C)	; $3014
	inc a	; $3017
	cp $09	; $3018
	jr c,loc_00_301D	; $301A
	xor a	; $301C

loc_00_301D:
	ld ($C63C),a	; $301D
	ld hl,$30D1	; $3020
	rst $28	; $3023
	ld h,a	; $3024
	ld l,$1E	; $3025
	call set_palette_entry	; $3027

loc_00_302A:
	call fn_00_335C	; $302A
	call fn_00_129D	; $302D
	jp yield	; $3030

wait_button:
	call task_enter_4	; $3033

loc_00_3036:
	call frame_tick	; $3036
	ld a,(joy_pressed)	; $3039
	and $30	; $303C
	jr z,loc_00_3036	; $303E
	call fn_01_40D9	; $3040
	jp task_return_4	; $3043

fn_00_3046:
	call task_enter_4	; $3046
	ld (ix+$19),a	; $3049

loc_00_304C:
	call fn_00_129D	; $304C
	call yield	; $304F
	dec (ix+$19)	; $3052
	jr nz,loc_00_304C	; $3055
	jp task_return_4	; $3057

fn_00_305A:
	pop hl	; $305A
	ld (ix+$14),l	; $305B
	ld (ix+$15),h	; $305E
	xor a	; $3061
	ld ($C00C),a	; $3062
	call yield	; $3065
	ld a,$02	; $3068
	ld ($C00C),a	; $306A
	ld l,(ix+$14)	; $306D
	ld h,(ix+$15)	; $3070
	jp (hl)	; $3073

fn_00_3074:
	ld a,(ix+$09)	; $3074
	xor $01	; $3077
	ld (ix+$09),a	; $3079

fn_00_307C:
	ld e,(ix+$0B)	; $307C
	ld d,(ix+$0D)	; $307F

fn_00_3082:
	ex af,af'	; $3082
	ld a,$13	; $3083
	call bank_push	; $3085
	ex af,af'	; $3088
	push hl	; $3089
	ld hl,$A1B6	; $308A
	rst $20	; $308D
	call fn_00_09C7	; $308E
	pop hl	; $3091
	jp bank_pop	; $3092

fn_00_3095:
	push bc	; $3095
	push de	; $3096
	push hl	; $3097
	xor a	; $3098
	call fn_00_30B6	; $3099
	ld a,($C63B)	; $309C
	inc a	; $309F
	cp $06	; $30A0
	jr c,loc_00_30A5	; $30A2
	xor a	; $30A4

loc_00_30A5:
	ld ($C63B),a	; $30A5
	ld hl,$30CB	; $30A8
	rst $28	; $30AB
	ld h,a	; $30AC
	ld l,$1F	; $30AD
	call set_palette_entry	; $30AF
	pop hl	; $30B2
	pop de	; $30B3
	pop bc	; $30B4
	ret	; $30B5

fn_00_30B6:
	ld b,a	; $30B6
	ld a,l	; $30B7
	call fn_00_30C5	; $30B8
	ld e,a	; $30BB
	ld a,h	; $30BC
	call fn_00_30C5	; $30BD
	ld d,a	; $30C0
	ld a,b	; $30C1
	jp fn_00_3082	; $30C2

fn_00_30C5:
	add a,a	; $30C5
	add a,a	; $30C6
	ld c,a	; $30C7
	add a,a	; $30C8
	add a,c	; $30C9
	ret	; $30CA
	.INCBIN "bank00.bin" SKIP $30CB READ 15	; data, 15 bytes

fn_00_30DA:
	push bc	; $30DA
	push de	; $30DB
	push hl	; $30DC
	add a,$A4	; $30DD
	ld h,a	; $30DF
	ld l,$0F	; $30E0
	ld a,$13	; $30E2
	ld de,$2920	; $30E4
	ld bc,$0100	; $30E7
	call queue_vram_copy	; $30EA
	pop hl	; $30ED
	pop de	; $30EE
	pop bc	; $30EF
	ret	; $30F0

loc_00_30F1:
	ld hl,$B9AC	; $30F1
	jr loc_00_30F9	; $30F4

fn_00_30F6:
	ld hl,$BC10	; $30F6

loc_00_30F9:
	ld (ix+$1A),$82	; $30F9
	ld (ix+$1B),$CC	; $30FD
	ld (ix+$1C),$42	; $3101
	ld (ix+$1D),$CC	; $3105
	jr loc_00_311E	; $3109

fn_00_310B:
	ld hl,$B748	; $310B
	ld (ix+$1A),$42	; $310E
	ld (ix+$1B),$CA	; $3112
	ld (ix+$1C),$82	; $3116
	ld (ix+$1D),$CE	; $311A

loc_00_311E:
	call fn_00_3255	; $311E
	call task_enter_1	; $3121
	ld (ix+$16),$09	; $3124

loc_00_3128:
	ld e,(ix+$1A)	; $3128
	ld d,(ix+$1B)	; $312B
	ld hl,$0040	; $312E
	add hl,de	; $3131
	ld (ix+$1A),l	; $3132
	ld (ix+$1B),h	; $3135
	ld bc,$0111	; $3138
	call fn_00_0499	; $313B
	ld e,(ix+$1C)	; $313E
	ld d,(ix+$1D)	; $3141
	ld hl,$FFC0	; $3144
	add hl,de	; $3147
	ld (ix+$1C),l	; $3148
	ld (ix+$1D),h	; $314B
	ld bc,$0111	; $314E
	call fn_00_0499	; $3151
	call fn_00_305A	; $3154
	dec (ix+$16)	; $3157
	jr nz,loc_00_3128	; $315A
	jp task_return_1	; $315C

fn_00_315F:
	ld hl,$B9AC	; $315F
	jr loc_00_3167	; $3162

fn_00_3164:
	ld hl,$BC10	; $3164

loc_00_3167:
	call fn_00_3255	; $3167
	call task_enter_1	; $316A
	ld (ix+$1A),$52	; $316D
	ld (ix+$1B),$CC	; $3171
	ld (ix+$1C),$01	; $3175
	ld (ix+$1D),$02	; $3179
	ld (ix+$16),$09	; $317D

loc_00_3181:
	ld e,(ix+$1A)	; $3181
	ld d,(ix+$1B)	; $3184
	ld c,(ix+$1C)	; $3187
	ld b,(ix+$1D)	; $318A
	call fn_00_0499	; $318D
	ld hl,$FFBE	; $3190
	add hl,de	; $3193
	ld (ix+$1A),l	; $3194
	ld (ix+$1B),h	; $3197
	inc (ix+$1C)	; $319A
	inc (ix+$1C)	; $319D
	inc (ix+$1D)	; $31A0
	inc (ix+$1D)	; $31A3
	call fn_00_305A	; $31A6
	dec (ix+$16)	; $31A9
	jr nz,loc_00_3181	; $31AC
	jp task_return_1	; $31AE

fn_00_31B1:
	call task_enter_1	; $31B1
	ld hl,$B748	; $31B4
	call fn_00_3255	; $31B7
	ld (ix+$1A),$42	; $31BA
	ld (ix+$1B),$CA	; $31BE
	ld (ix+$1C),$82	; $31C2
	ld (ix+$1D),$CE	; $31C6
	ld (ix+$1E),$42	; $31CA
	ld (ix+$1F),$CA	; $31CE
	ld (ix+$20),$62	; $31D2
	ld (ix+$21),$CA	; $31D6
	ld (ix+$16),$09	; $31DA

loc_00_31DE:
	ld e,(ix+$1A)	; $31DE
	ld d,(ix+$1B)	; $31E1
	ld bc,$0111	; $31E4
	call fn_00_0499	; $31E7
	ld hl,$0040	; $31EA
	add hl,de	; $31ED
	ld (ix+$1A),l	; $31EE
	ld (ix+$1B),h	; $31F1
	ld e,(ix+$1C)	; $31F4
	ld d,(ix+$1D)	; $31F7
	ld bc,$0111	; $31FA
	call fn_00_0499	; $31FD
	ld hl,$FFC0	; $3200
	add hl,de	; $3203
	ld (ix+$1C),l	; $3204
	ld (ix+$1D),h	; $3207
	ld e,(ix+$1E)	; $320A
	ld d,(ix+$1F)	; $320D
	ld bc,$1201	; $3210
	call fn_00_0499	; $3213
	inc e	; $3216
	inc e	; $3217
	ld (ix+$1E),e	; $3218
	ld (ix+$1F),d	; $321B
	ld e,(ix+$20)	; $321E
	ld d,(ix+$21)	; $3221
	ld bc,$1201	; $3224
	call fn_00_0499	; $3227
	dec e	; $322A
	dec e	; $322B
	ld (ix+$20),e	; $322C
	ld (ix+$21),d	; $322F
	call fn_00_305A	; $3232
	dec (ix+$16)	; $3235
	jr nz,loc_00_31DE	; $3238
	jp task_return_1	; $323A

fn_00_323D:
	push bc	; $323D
	push de	; $323E
	push hl	; $323F
	ld hl,$B748	; $3240
	call fn_00_3255	; $3243
	ld de,$CA42	; $3246
	ld bc,$1211	; $3249
	call fn_00_0499	; $324C
	pop hl	; $324F
	pop de	; $3250
	pop bc	; $3251
	jp wait_vblank	; $3252

fn_00_3255:
	ld de,$CA42	; $3255
	ld b,$12	; $3258

loc_00_325A:
	push bc	; $325A
	ld a,$14	; $325B
	ld bc,$0022	; $325D
	call fn_00_07EC	; $3260
	ex de,hl	; $3263
	ld bc,$001E	; $3264
	add hl,bc	; $3267
	ex de,hl	; $3268
	pop bc	; $3269
	djnz loc_00_325A	; $326A
	ret	; $326C

loc_00_326D:
	push bc	; $326D
	push hl	; $326E
	ld b,a	; $326F
	add a,a	; $3270
	add a,b	; $3271
	ld hl,$32A6	; $3272
	rst $30	; $3275
	push de	; $3276
	ld a,(hl)	; $3277
	inc hl	; $3278
	ld e,(hl)	; $3279
	inc hl	; $327A
	ld d,(hl)	; $327B
	ex de,hl	; $327C
	pop de	; $327D
	push af	; $327E
	call fn_00_0788	; $327F
	pop af	; $3282
	pop de	; $3283
	pop bc	; $3284
	call bank_push	; $3285
	ld a,$0E	; $3288

loc_00_328A:
	ex af,af'	; $328A
	ld b,$06	; $328B

loc_00_328D:
	ld a,(hl)	; $328D
	inc hl	; $328E
	add a,c	; $328F
	ld (de),a	; $3290
	inc e	; $3291
	ld a,(hl)	; $3292
	inc hl	; $3293
	ld (de),a	; $3294
	inc e	; $3295
	djnz loc_00_328D	; $3296
	ld a,e	; $3298
	add a,$34	; $3299
	ld e,a	; $329B
	jr nc,loc_00_329F	; $329C
	inc d	; $329E

loc_00_329F:
	ex af,af'	; $329F
	dec a	; $32A0
	jr nz,loc_00_328A	; $32A1
	jp bank_pop	; $32A3
	.INCBIN "bank00.bin" SKIP $32A6 READ 24	; data, 24 bytes

loc_00_32BE:
	ld l,a	; $32BE
	ld h,$00	; $32BF
	add hl,hl	; $32C1
	add hl,hl	; $32C2
	add hl,hl	; $32C3
	ld de,$9410	; $32C4
	add hl,de	; $32C7
	ld a,$14	; $32C8
	call bank_push	; $32CA
	ld de,$2840	; $32CD
	rst $10	; $32D0
	ld de,$8840	; $32D1
	ld a,$7E	; $32D4

loc_00_32D6:
	ex af,af'	; $32D6
	push hl	; $32D7
	ld b,$08	; $32D8

loc_00_32DA:
	ld c,(hl)	; $32DA
	ld a,(de)	; $32DB
	and c	; $32DC
	out ($BE),a	; $32DD
	inc de	; $32DF
	ld a,(de)	; $32E0
	and c	; $32E1
	out ($BE),a	; $32E2
	inc de	; $32E4
	ld a,(de)	; $32E5
	and c	; $32E6
	out ($BE),a	; $32E7
	inc de	; $32E9
	inc hl	; $32EA
	nop	; $32EB
	in a,($BE)	; $32EC
	nop	; $32EE
	djnz loc_00_32DA	; $32EF
	pop hl	; $32F1
	ex af,af'	; $32F2
	dec a	; $32F3
	jp nz,loc_00_32D6	; $32F4
	jp bank_pop	; $32F7

loc_00_32FA:
	ld l,a	; $32FA
	ld h,$00	; $32FB
	ld c,l	; $32FD
	ld b,h	; $32FE
	add hl,hl	; $32FF
	add hl,bc	; $3300
	add hl,hl	; $3301
	add hl,hl	; $3302
	ld bc,$ABB0	; $3303
	add hl,bc	; $3306
	ld a,$14	; $3307
	call bank_push	; $3309
	rst $10	; $330C
	ld a,$0C	; $330D
	ld c,$BE	; $330F
	ld de,$9610	; $3311

loc_00_3314:
	push hl	; $3314
	ld l,(hl)	; $3315
	ld h,$00	; $3316
	add hl,hl	; $3318
	add hl,hl	; $3319
	add hl,hl	; $331A
	add hl,hl	; $331B
	add hl,hl	; $331C
	add hl,de	; $331D
	ld b,$20	; $331E

loc_00_3320:
	outi	; $3320
	jr nz,loc_00_3320	; $3322
	pop hl	; $3324
	inc hl	; $3325
	dec a	; $3326
	jp nz,loc_00_3314	; $3327
	jp bank_pop	; $332A

loc_00_332D:
	ld hl,location_people	; $332D
	ld de,$C652	; $3330
	ld bc,$00FF	; $3333
	ld (hl),$00	; $3336
	ldir	; $3338
	ret	; $333A

fn_00_333B:
	ld (hl),c	; $333B
	inc hl	; $333C
	djnz fn_00_333B	; $333D
	ret	; $333F

fn_00_3340:
	ld a,(de)	; $3340
	ld c,(hl)	; $3341
	ld (hl),a	; $3342
	ld a,c	; $3343
	ld (de),a	; $3344
	inc hl	; $3345
	inc de	; $3346
	djnz fn_00_3340	; $3347
	ret	; $3349

switch_on_a:
	ld b,(hl)	; $334A
	inc b	; $334B
	ret z	; $334C
	cp (hl)	; $334D
	inc hl	; $334E
	jr z,loc_00_3356	; $334F
	inc hl	; $3351
	inc hl	; $3352
	jp switch_on_a	; $3353

loc_00_3356:
	pop af	; $3356
	ld a,(hl)	; $3357
	inc hl	; $3358
	ld h,(hl)	; $3359
	ld l,a	; $335A
	jp (hl)	; $335B

fn_00_335C:
	ld a,(text_col)	; $335C
	add a,a	; $335F
	add a,a	; $3360
	add a,a	; $3361
	ld ($C604),a	; $3362
	ld a,(text_row)	; $3365
	add a,a	; $3368
	add a,a	; $3369
	add a,a	; $336A
	ld ($C605),a	; $336B

loc_00_336E:
	ld a,($C00C)	; $336E
	ld b,a	; $3371
	ld a,$02	; $3372
	dec b	; $3374
	jr z,loc_00_337D	; $3375
	dec a	; $3377
	dec b	; $3378
	jr z,loc_00_337D	; $3379
	ld a,$05	; $337B

loc_00_337D:
	ld hl,$C607	; $337D
	inc (hl)	; $3380
	cp (hl)	; $3381
	jr nc,loc_00_338F	; $3382
	ld (hl),$00	; $3384
	ld a,($C606)	; $3386
	dec a	; $3389
	and $03	; $338A
	ld ($C606),a	; $338C

loc_00_338F:
	ld c,$F2	; $338F
	ld de,($C604)	; $3391
	call fn_00_09DA	; $3395
	ld a,($C606)	; $3398
	add a,a	; $339B
	add a,a	; $339C
	add a,a	; $339D
	add a,a	; $339E
	add a,a	; $339F
	ld hl,$B0AF	; $33A0
	rst $30	; $33A3
	ld de,$3E40	; $33A4
	ld bc,$0020	; $33A7
	ld a,$13	; $33AA
	jp queue_vram_copy	; $33AC

play_music_later:
	push bc	; $33AF
	push de	; $33B0
	push hl	; $33B1
	push iy	; $33B2
	push af	; $33B4
	ld a,$F0	; $33B5
	call fn_00_0B6D	; $33B7
	ld c,$F0	; $33BA
	ld de,$33D4	; $33BC
	call spawn_task_c	; $33BF
	jr nc,loc_00_33CA	; $33C2
	pop af	; $33C4
	call play_music	; $33C5
	jr loc_00_33CE	; $33C8

loc_00_33CA:
	pop af	; $33CA
	ld (iy+$1A),a	; $33CB

loc_00_33CE:
	pop iy	; $33CE
	pop hl	; $33D0
	pop de	; $33D1
	pop bc	; $33D2
	ret	; $33D3

music_later:
	call fn_00_0605	; $33D4
	ld a,$14	; $33D7
	call sleep	; $33D9
	ld a,(ix+$1A)	; $33DC
	call play_music	; $33DF
	jp end_task	; $33E2

fn_00_33E5:
	call place_index	; $33E5
	ld a,$06	; $33E8
	ret z	; $33EA
	ld a,$01	; $33EB
	ret	; $33ED

place_index:
	ld a,(location)	; $33EE
	sub $02	; $33F1
	adc a,$00	; $33F3
	ret	; $33F5

fn_00_33F6:
	push bc	; $33F6
	push de	; $33F7
	ld de,($C8FD)	; $33F8
	ld a,($C8FF)	; $33FC
	ld b,$04	; $33FF

loc_00_3401:
	rra	; $3401
	rr d	; $3402
	rr e	; $3404
	djnz loc_00_3401	; $3406
	or a	; $3408
	sbc hl,de	; $3409
	ex de,hl	; $340B
	pop de	; $340C
	pop bc	; $340D
	ret	; $340E

fn_00_340F:
	push bc	; $340F
	push de	; $3410
	ld c,a	; $3411
	ld a,$10	; $3412
	ld hl,$8300	; $3414
	call fn_00_0807	; $3417
	ld de,$C065	; $341A
	ld bc,$0019	; $341D
	ld a,$10	; $3420
	call fn_00_07EC	; $3422
	ld hl,$C065	; $3425
	pop de	; $3428
	pop bc	; $3429
	ret	; $342A

fn_00_342B:
	ld a,($C976)	; $342B
	or a	; $342E
	jr z,loc_00_3454	; $342F
	dec a	; $3431
	jr z,loc_00_347E	; $3432
	ld a,$0F	; $3434
	call bank_push	; $3436
	push bc	; $3439
	push de	; $343A
	push hl	; $343B
	ld a,($C976)	; $343C
	sub $20	; $343F
	ld l,a	; $3441
	ld h,$00	; $3442
	add hl,hl	; $3444
	add hl,hl	; $3445
	add hl,hl	; $3446
	ld de,$BB58	; $3447
	add hl,de	; $344A
	call fn_00_3493	; $344B
	pop hl	; $344E
	pop de	; $344F
	pop bc	; $3450
	jp bank_pop	; $3451

loc_00_3454:
	push bc	; $3454
	push de	; $3455
	push hl	; $3456
	ld hl,$34B3	; $3457
	ld de,$C1C0	; $345A
	ld bc,$0008	; $345D
	ldir	; $3460
	ld hl,$C1C0	; $3462
	ld de,karma	; $3465
	ld bc,$0800	; $3468

loc_00_346B:
	ld a,(de)	; $346B
	or a	; $346C
	jr z,loc_00_3470	; $346D
	ld (hl),c	; $346F

loc_00_3470:
	inc l	; $3470
	inc de	; $3471
	djnz loc_00_346B	; $3472
	ld hl,$C1C0	; $3474
	call fn_00_3493	; $3477
	pop hl	; $347A
	pop de	; $347B
	pop bc	; $347C
	ret	; $347D

loc_00_347E:
	push bc	; $347E
	push de	; $347F
	push hl	; $3480
	ld a,$13	; $3481
	ld hl,$B12F	; $3483
	ld de,$25F0	; $3486
	ld bc,$0020	; $3489
	call fn_00_07F4	; $348C
	pop hl	; $348F
	pop de	; $3490
	pop bc	; $3491
	ret	; $3492

fn_00_3493:
	ld de,$25F0	; $3493
	rst $10	; $3496
	xor a	; $3497
	ld bc,$08BE	; $3498

loc_00_349B:
	out ($BE),a	; $349B
	call fn_00_0262	; $349D
	outi	; $34A0
	call fn_00_0262	; $34A2
	out ($BE),a	; $34A5
	call fn_00_0262	; $34A7
	out ($BE),a	; $34AA
	call fn_00_0262	; $34AC
	jp nz,loc_00_349B	; $34AF
	ret	; $34B2
	.INCBIN "bank00.bin" SKIP $34B3 READ 8	; data, 8 bytes

fn_00_34BB:
	push bc	; $34BB
	push de	; $34BC
	push hl	; $34BD

loc_00_34BE:
	ld a,(hl)	; $34BE
	inc hl	; $34BF
	call fn_00_34D4	; $34C0
	ld b,a	; $34C3
	ld a,(de)	; $34C4
	inc de	; $34C5
	call fn_00_34D4	; $34C6
	cp b	; $34C9
	scf	; $34CA
	jr nz,loc_00_34D0	; $34CB
	or a	; $34CD
	jr nz,loc_00_34BE	; $34CE

loc_00_34D0:
	pop hl	; $34D0
	pop de	; $34D1
	pop bc	; $34D2
	ret	; $34D3

fn_00_34D4:
	cp $61	; $34D4
	ret c	; $34D6
	cp $7B	; $34D7
	ret nc	; $34D9
	add a,$E0	; $34DA
	ret	; $34DC

talk_setup:
	ld ($C9A5),iy	; $34DD
	call place_index	; $34E1
	ld b,a	; $34E4
	add a,a	; $34E5
	add a,a	; $34E6
	add a,a	; $34E7
	add a,a	; $34E8
	ld c,(iy+$07)	; $34E9
	dec c	; $34EC
	add a,c	; $34ED
	ld (ix+$09),a	; $34EE
	ld a,b	; $34F1
	add a,a	; $34F2
	add a,b	; $34F3
	ld hl,$351D	; $34F4
	rst $30	; $34F7
	ld a,(hl)	; $34F8
	ld ($C9A8),a	; $34F9
	call bank_push	; $34FC
	inc hl	; $34FF
	call fn_00_0097	; $3500
	ld a,c	; $3503
	rst $20	; $3504
	ld de,talk_question_topic	; $3505
	ld bc,$0004	; $3508
	ldir	; $350B
	ld ($C9AD),hl	; $350D
	ld a,$FF	; $3510
	ld bc,$0000	; $3512
	cpir	; $3515
	ld ($C9AF),hl	; $3517
	jp bank_pop	; $351A

dialogue_tables:
	.INCBIN "bank00.bin" SKIP $351D READ 48	; data, 48 bytes

fn_00_354D:
	push hl	; $354D
	call fn_00_3553	; $354E
	pop hl	; $3551
	ret	; $3552

fn_00_3553:
	call dialogue_string	; $3553
	call bank_push	; $3556
	call print_text_hl	; $3559
	jp bank_pop	; $355C

dialogue_string:
	ld a,($C9A8)	; $355F
	call bank_push	; $3562
	ld hl,($C9AF)	; $3565
	xor a	; $3568

loc_00_3569:
	dec b	; $3569
	jr z,loc_00_3572	; $356A

loc_00_356C:
	cp (hl)	; $356C
	inc hl	; $356D
	jr nz,loc_00_356C	; $356E
	jr loc_00_3569	; $3570

loc_00_3572:
	ld a,($C9A8)	; $3572
	jp bank_pop	; $3575

talk_topics:
	ld a,($C9A8)	; $3578
	call bank_push	; $357B
	ld hl,($C9AD)	; $357E
	ld d,$C5	; $3581

loc_00_3583:
	ld a,(hl)	; $3583
	cp $FF	; $3584
	jr z,loc_00_359C	; $3586
	inc hl	; $3588
	ld e,a	; $3589
	ld a,(de)	; $358A
	or a	; $358B
	jr z,loc_00_3599	; $358C
	ld a,(hl)	; $358E
	or a	; $358F
	jp p,loc_00_3595	; $3590
	add a,$10	; $3593

loc_00_3595:
	ld e,a	; $3595
	ld a,$01	; $3596
	ld (de),a	; $3598

loc_00_3599:
	inc hl	; $3599
	jr loc_00_3583	; $359A

loc_00_359C:
	ld hl,$3606	; $359C
	ld de,$C63D	; $359F
	ld bc,$0007	; $35A2
	ldir	; $35A5
	ex de,hl	; $35A7
	ld (ix+$1B),$07	; $35A8
	ld a,($C595)	; $35AC
	or a	; $35AF
	jr z,loc_00_35B8	; $35B0
	ld (hl),$0A	; $35B2
	inc hl	; $35B4
	inc (ix+$1B)	; $35B5

loc_00_35B8:
	ld a,($C596)	; $35B8
	or a	; $35BB
	jr z,loc_00_35C3	; $35BC
	ld (hl),$0B	; $35BE
	inc (ix+$1B)	; $35C0

loc_00_35C3:
	call fn_00_2E1B	; $35C3
	ld de,$CAA6	; $35C6
	ld c,(ix+$1C)	; $35C9
	ld a,(ix+$1B)	; $35CC
	ld b,a	; $35CF
	cp $08	; $35D0
	jr c,loc_00_35D6	; $35D2
	ld b,$08	; $35D4

loc_00_35D6:
	ld a,c	; $35D6
	ld hl,$C63D	; $35D7
	rst $28	; $35DA
	cp $0A	; $35DB
	jr c,loc_00_35E7	; $35DD
	push bc	; $35DF
	ld b,a	; $35E0
	call dialogue_string	; $35E1
	pop bc	; $35E4
	jr loc_00_35EB	; $35E5

loc_00_35E7:
	ld hl,$360D	; $35E7
	rst $20	; $35EA

loc_00_35EB:
	push de	; $35EB
	call fn_00_2D6C	; $35EC
	pop de	; $35EF
	ld hl,$0080	; $35F0
	add hl,de	; $35F3
	ex de,hl	; $35F4
	inc c	; $35F5
	djnz loc_00_35D6	; $35F6
	ld a,(ix+$1D)	; $35F8
	add a,a	; $35FB
	inc a	; $35FC
	call fn_01_49D5	; $35FD
	call fn_00_2E54	; $3600
	jp bank_pop	; $3603
	.INCBIN "bank00.bin" SKIP $3606 READ 7	; data, 7 bytes

talk_topic_names:
	.dw $361F
	.dw $3623
	.dw $3628
	.dw $362D
	.dw $3631
	.dw $3638
	.dw $3638
	.dw $3638
	.dw $363D
	.dw fn_01_5942
	.dw fn_00_0045
	.dw $414E
	.dw $454D
	.dw fn_01_4C00
	.dw fn_01_4F4F
	.dw fn_00_004B
	.dw fn_01_4F4A
	.dw reset
	.dw fn_01_4548
	.dw $4C41
	.dw fn_01_4854
	.dw $4A00
	.dw $494F
	.dw fn_00_004E
	.dw fn_01_4947
	.dw fn_01_4556
	.INCBIN "bank00.bin" SKIP $3641 READ 1	; data, 1 bytes

fn_00_3642:
	ld b,$0A	; $3642
	cp $05	; $3644
	jr z,loc_00_364D	; $3646
	inc b	; $3648
	cp $06	; $3649
	jr nz,loc_00_365C	; $364B

loc_00_364D:
	call dialogue_string	; $364D
	call bank_push	; $3650
	call print_text_hl	; $3653
	call fn_01_54CB	; $3656
	jp bank_pop	; $3659

loc_00_365C:
	ld hl,$360D	; $365C
	rst $20	; $365F
	call print_text_hl	; $3660
	jp fn_01_54CB	; $3663

fn_00_3666:
	ld a,(message_bank)	; $3666
	call bank_push	; $3669
	ld b,$01	; $366C
	call dialogue_string	; $366E
	ld de,$3686	; $3671
	ld b,$02	; $3674
	call fn_00_368C	; $3676
	jr z,loc_00_3683	; $3679
	ld de,$3688	; $367B
	ld b,$04	; $367E
	call fn_00_368C	; $3680

loc_00_3683:
	jp bank_pop	; $3683
	.INCBIN "bank00.bin" SKIP $3686 READ 6	; data, 6 bytes

fn_00_368C:
	push bc	; $368C
	push de	; $368D
	push hl	; $368E

loc_00_368F:
	ld a,(hl)	; $368F
	inc hl	; $3690
	call fn_00_34D4	; $3691
	ld c,a	; $3694
	ld a,(de)	; $3695
	inc de	; $3696
	call fn_00_34D4	; $3697
	cp c	; $369A
	jr nz,loc_00_369F	; $369B
	djnz loc_00_368F	; $369D

loc_00_369F:
	pop hl	; $369F
	pop de	; $36A0
	pop bc	; $36A1
	ret	; $36A2

fn_00_36A3:
	ld a,$1A	; $36A3
	call bank_push	; $36A5
	call fn_00_2E1B	; $36A8
	ld de,$CAA6	; $36AB
	ld c,(ix+$1B)	; $36AE
	ld b,$08	; $36B1

loc_00_36B3:
	ld a,c	; $36B3
	ld hl,$C5E0	; $36B4
	rst $28	; $36B7
	or a	; $36B8
	jr z,loc_00_36CC	; $36B9
	ld hl,$8162	; $36BB
	rst $20	; $36BE
	push de	; $36BF
	call fn_00_2D6C	; $36C0
	pop de	; $36C3
	ld hl,$0080	; $36C4
	add hl,de	; $36C7
	ex de,hl	; $36C8
	inc c	; $36C9
	djnz loc_00_36B3	; $36CA

loc_00_36CC:
	ld a,(ix+$1C)	; $36CC
	add a,a	; $36CF
	inc a	; $36D0
	call fn_01_49D5	; $36D1
	call fn_00_2E54	; $36D4
	jp bank_pop	; $36D7

cap_9999:
	push de	; $36DA
	ld de,$270F	; $36DB
	or a	; $36DE
	sbc hl,de	; $36DF
	jr c,loc_00_36E9	; $36E1
	jr z,loc_00_36E9	; $36E3
	ex de,hl	; $36E5
	scf	; $36E6
	jr loc_00_36EB	; $36E7

loc_00_36E9:
	add hl,de	; $36E9
	or a	; $36EA

loc_00_36EB:
	pop de	; $36EB
	ret	; $36EC

fn_00_36ED:
	push bc	; $36ED
	ld b,a	; $36EE
	add a,a	; $36EF
	ld c,a	; $36F0
	add a,a	; $36F1
	add a,a	; $36F2
	add a,b	; $36F3
	add a,c	; $36F4
	pop bc	; $36F5
	ret	; $36F6

fn_00_36F7:
	push bc	; $36F7
	add a,a	; $36F8
	add a,a	; $36F9
	ld b,a	; $36FA
	add a,a	; $36FB
	add a,b	; $36FC
	pop bc	; $36FD
	ret	; $36FE

fn_00_36FF:
	push bc	; $36FF
	push de	; $3700
	ex de,hl	; $3701
	ld hl,$C06F	; $3702
	ld (hl),$00	; $3705
	ld b,$05	; $3707

loc_00_3709:
	ld a,(de)	; $3709
	call fn_00_3727	; $370A
	ld a,(de)	; $370D
	rrca	; $370E
	rrca	; $370F
	rrca	; $3710
	rrca	; $3711
	call fn_00_3727	; $3712
	inc de	; $3715
	djnz loc_00_3709	; $3716
	ld a,$30	; $3718
	ld b,$09	; $371A

loc_00_371C:
	cp (hl)	; $371C
	jr nz,loc_00_3724	; $371D
	ld (hl),$20	; $371F
	inc hl	; $3721
	djnz loc_00_371C	; $3722

loc_00_3724:
	pop de	; $3724
	pop bc	; $3725
	ret	; $3726

fn_00_3727:
	and $0F	; $3727
	add a,$30	; $3729
	dec hl	; $372B
	ld (hl),a	; $372C
	ret	; $372D

fn_00_372E:
	push bc	; $372E
	push hl	; $372F
	ld hl,($C8FD)	; $3730
	ld bc,$0001	; $3733
	add hl,bc	; $3736
	ld ($C8FD),hl	; $3737
	jr nc,loc_00_3743	; $373A
	ld hl,($C8FF)	; $373C
	inc hl	; $373F
	ld ($C8FF),hl	; $3740

loc_00_3743:
	ld hl,move_count	; $3743
	ld bc,$0500	; $3746
	scf	; $3749

loc_00_374A:
	ld a,(hl)	; $374A
	adc a,c	; $374B
	daa	; $374C
	ld (hl),a	; $374D
	inc hl	; $374E
	djnz loc_00_374A	; $374F
	pop hl	; $3751
	pop bc	; $3752
	ret	; $3753

fn_00_3754:
	ld a,($C977)	; $3754
	or a	; $3757
	ret z	; $3758
	dec a	; $3759
	ld ($C977),a	; $375A
	ret nz	; $375D
	ld ($C976),a	; $375E
	ret	; $3761

fn_00_3762:
	ex af,af'	; $3762
	ld a,$0F	; $3763
	call bank_push	; $3765
	ex af,af'	; $3768
	or a	; $3769
	jr z,loc_00_376D	; $376A
	inc a	; $376C

loc_00_376D:
	ld hl,$8079	; $376D
	rst $20	; $3770
	ld de,$0100	; $3771
	add hl,de	; $3774
	ld de,$D000	; $3775
	call decode_location_map	; $3778
	jp bank_pop	; $377B

load_arena:
	ex af,af'	; $377E
	ld a,$05	; $377F
	call bank_push	; $3781
	ex af,af'	; $3784
	push bc	; $3785
	push de	; $3786
	push hl	; $3787
	push iy	; $3788
	ld hl,$A4DF	; $378A
	rst $20	; $378D
	ld iy,room_monsters	; $378E
	ld de,$0008	; $3792
	ld b,$10	; $3795

loc_00_3797:
	ld a,(hl)	; $3797
	inc hl	; $3798
	ld (iy+$00),a	; $3799
	ld a,(hl)	; $379C
	inc hl	; $379D
	ld (iy+$01),a	; $379E
	ld (iy+$05),$00	; $37A1
	ld (iy+$06),$00	; $37A5
	add iy,de	; $37A9
	djnz loc_00_3797	; $37AB
	ld iy,party_objects	; $37AD
	ld de,$0005	; $37B1
	ld b,$08	; $37B4

loc_00_37B6:
	ld a,(hl)	; $37B6
	inc hl	; $37B7
	ld (iy+$00),a	; $37B8
	ld a,(hl)	; $37BB
	inc hl	; $37BC
	ld (iy+$01),a	; $37BD
	ld (iy+$02),$00	; $37C0
	ld (iy+$03),$00	; $37C4
	add iy,de	; $37C8
	djnz loc_00_37B6	; $37CA
	ld de,$D800	; $37CC
	call decode_location_map	; $37CF
	call terrain_substitute	; $37D2
	pop iy	; $37D5
	pop hl	; $37D7
	pop de	; $37D8
	pop bc	; $37D9
	jp bank_pop	; $37DA

fn_00_37DD:
	call task_enter_3	; $37DD
	call print_inline	; $37E0
	.INCBIN "bank00.bin" SKIP $37E3 READ 6	; text, 6 bytes
	call fn_01_48E1	; $37E9
	ld a,(iy+$04)	; $37EC

loc_00_37EF:
	ld (ix+$08),a	; $37EF
	ld c,a	; $37F2
	call fn_01_48EB	; $37F3
	ld a,(iy+$05)	; $37F6
	or a	; $37F9
	jr nz,loc_00_3804	; $37FA
	ld a,(ix+$08)	; $37FC
	inc a	; $37FF
	and $0F	; $3800
	jr loc_00_37EF	; $3802

loc_00_3804:
	ld c,(ix+$08)	; $3804
	call fn_01_48EB	; $3807
	ld l,(iy+$00)	; $380A
	ld h,(iy+$01)	; $380D
	ld (ix+$20),l	; $3810
	ld (ix+$21),h	; $3813
	call fn_00_3095	; $3816
	call frame_tick	; $3819
	ld a,(joy_pressed)	; $381C
	ld hl,$3827	; $381F
	call switch_on_a	; $3822
	jr loc_00_3804	; $3825

tbl_00_3827:
	.db $01
	.dw loc_00_383A
	.db $02
	.dw loc_00_383E
	.db $04
	.dw loc_00_3842
	.db $08
	.dw loc_00_3846
	.db $10
	.dw loc_00_386E
	.db $20
	.dw loc_00_3874
	.db $FF

loc_00_383A:
	ld c,$00	; $383A
	jr loc_00_3848	; $383C

loc_00_383E:
	ld c,$01	; $383E
	jr loc_00_3848	; $3840

loc_00_3842:
	ld c,$02	; $3842
	jr loc_00_3848	; $3844

loc_00_3846:
	ld c,$03	; $3846

loc_00_3848:
	call fn_01_40D9	; $3848
	ld e,(ix+$20)	; $384B
	ld d,(ix+$21)	; $384E
	ld b,$79	; $3851

loc_00_3853:
	ld a,c	; $3853
	ld hl,$387B	; $3854
	rst $18	; $3857
	ex de,hl	; $3858
	call fn_00_38BD	; $3859
	ex de,hl	; $385C
	jr nc,loc_00_3863	; $385D
	djnz loc_00_3853	; $385F
	jr loc_00_3804	; $3861

loc_00_3863:
	ld (ix+$08),a	; $3863
	call fn_01_48E1	; $3866
	ld (iy+$04),a	; $3869
	jr loc_00_3804	; $386C

loc_00_386E:
	call fn_01_40D1	; $386E
	scf	; $3871
	jr loc_00_3878	; $3872

loc_00_3874:
	call fn_01_40C9	; $3874
	or a	; $3877

loc_00_3878:
	jp task_return_3	; $3878

tbl_00_387B:
	.dw fn_00_3883
	.dw fn_00_388A
	.dw fn_00_3891
	.dw fn_00_3898

fn_00_3883:
	call fn_00_38B6	; $3883
	call nc,fn_00_38A7	; $3886
	ret	; $3889

fn_00_388A:
	call fn_00_38AE	; $388A
	call nc,fn_00_389F	; $388D
	ret	; $3890

fn_00_3891:
	call fn_00_38A7	; $3891
	call nc,fn_00_38B6	; $3894
	ret	; $3897

fn_00_3898:
	call fn_00_389F	; $3898
	call nc,fn_00_38AE	; $389B
	ret	; $389E

fn_00_389F:
	inc e	; $389F
	ld a,e	; $38A0
	cp $0B	; $38A1
	ret c	; $38A3
	ld e,$00	; $38A4
	ret	; $38A6

fn_00_38A7:
	dec e	; $38A7
	scf	; $38A8
	ret p	; $38A9
	ld e,$0A	; $38AA
	or a	; $38AC
	ret	; $38AD

fn_00_38AE:
	inc d	; $38AE
	ld a,d	; $38AF
	cp $0B	; $38B0
	ret c	; $38B2
	ld d,$00	; $38B3
	ret	; $38B5

fn_00_38B6:
	dec d	; $38B6
	scf	; $38B7
	ret p	; $38B8
	ld d,$0A	; $38B9
	or a	; $38BB
	ret	; $38BC

fn_00_38BD:
	push bc	; $38BD
	push de	; $38BE
	push iy	; $38BF
	ld iy,room_monsters	; $38C1
	ld de,$0008	; $38C5
	ld b,$10	; $38C8

loc_00_38CA:
	ld a,(iy+$05)	; $38CA
	or a	; $38CD
	jr z,loc_00_38E1	; $38CE
	ld a,(iy+$00)	; $38D0
	cp l	; $38D3
	jr nz,loc_00_38E1	; $38D4
	ld a,(iy+$01)	; $38D6
	cp h	; $38D9
	jr nz,loc_00_38E1	; $38DA
	ld a,$10	; $38DC
	sub b	; $38DE
	jr loc_00_38E6	; $38DF

loc_00_38E1:
	add iy,de	; $38E1
	djnz loc_00_38CA	; $38E3
	scf	; $38E5

loc_00_38E6:
	pop iy	; $38E6
	pop de	; $38E8
	pop bc	; $38E9
	ret	; $38EA

fn_00_38EB:
	push hl	; $38EB
	call fn_00_38F2	; $38EC
	ld a,(hl)	; $38EF
	pop hl	; $38F0
	ret	; $38F1

fn_00_38F2:
	ld a,h	; $38F2
	call fn_00_36ED	; $38F3
	add a,l	; $38F6
	add a,a	; $38F7
	ld l,a	; $38F8
	ld h,$D8	; $38F9
	ret	; $38FB

fn_00_38FC:
	call fn_00_30DA	; $38FC
	push hl	; $38FF
	call fn_00_3991	; $3900
	ld (ix+$0B),l	; $3903
	ld (ix+$0D),h	; $3906
	ld (ix+$0A),$00	; $3909
	ld (ix+$0C),$00	; $390D
	pop hl	; $3911
	call fn_00_2BFE	; $3912
	add hl,hl	; $3915
	add hl,hl	; $3916
	ld (ix+$0E),l	; $3917
	ld (ix+$0F),h	; $391A
	ex de,hl	; $391D
	add hl,hl	; $391E
	add hl,hl	; $391F
	ld (ix+$10),l	; $3920
	ld (ix+$11),h	; $3923
	ld (ix+$09),$08	; $3926
	ld a,$18	; $392A
	jp fn_00_05F1	; $392C

fn_00_392F:
	call task_enter_4	; $392F
	call fn_00_0BA4	; $3932
	call fn_00_3074	; $3935
	call fn_00_305A	; $3938
	ld l,(ix+$2A)	; $393B
	ld h,(ix+$2B)	; $393E
	push hl	; $3941
	jp fn_00_399C	; $3942

fn_00_3945:
	call task_enter_4	; $3945
	call fn_00_0BA4	; $3948
	call fn_00_399C	; $394B
	ld (ix+$2C),l	; $394E
	ld (ix+$2D),h	; $3951
	ld a,h	; $3954
	call fn_00_36ED	; $3955
	add a,l	; $3958
	add a,a	; $3959
	ld l,a	; $395A
	ld h,$D9	; $395B
	ld a,(hl)	; $395D
	cp $7E	; $395E
	call nz,fn_00_3074	; $3960
	call fn_00_305A	; $3963
	ld l,(ix+$2A)	; $3966
	ld h,(ix+$2B)	; $3969
	push hl	; $396C
	ld l,(ix+$2C)	; $396D
	ld h,(ix+$2D)	; $3970
	ret	; $3973

fn_00_3974:
	call fn_00_3EB7	; $3974
	ret nc	; $3977
	push bc	; $3978
	push hl	; $3979
	ld hl,$3989	; $397A
	ld bc,$0008	; $397D
	cpir	; $3980
	scf	; $3982
	jr nz,loc_00_3986	; $3983
	or a	; $3985

loc_00_3986:
	pop hl	; $3986
	pop bc	; $3987
	ret	; $3988
	.INCBIN "bank00.bin" SKIP $3989 READ 8	; data, 8 bytes

fn_00_3991:
	ld a,l	; $3991
	call fn_00_36F7	; $3992
	ld l,a	; $3995
	ld a,h	; $3996
	call fn_00_36F7	; $3997
	ld h,a	; $399A
	ret	; $399B

fn_00_399C:
	ld a,(ix+$0D)	; $399C
	add a,$06	; $399F
	ld l,a	; $39A1
	ld h,$0C	; $39A2
	call divide_8	; $39A4
	ld b,l	; $39A7
	ld a,(ix+$0B)	; $39A8
	add a,$06	; $39AB
	ld l,a	; $39AD
	ld h,$0C	; $39AE
	call divide_8	; $39B0
	ld h,b	; $39B3
	ret	; $39B4

damage_monster:
	call print_newline	; $39B5
	call fn_01_48EB	; $39B8
	ld c,a	; $39BB
	ld a,(iy+$05)	; $39BC
	call fn_01_47D3	; $39BF
	call print_text_hl	; $39C2
	ld a,$20	; $39C5
	call print_char	; $39C7
	ld a,(iy+$05)	; $39CA
	cp $5E	; $39CD
	jr z,loc_00_3A13	; $39CF
	ld a,(iy+$04)	; $39D1
	sub c	; $39D4
	ld (iy+$04),a	; $39D5
	jr nc,loc_00_3A13	; $39D8
	call print_inline	; $39DA
	.INCBIN "bank00.bin" SKIP $39DD READ 7	; text, 7 bytes
	ld a,b	; $39E4
	inc a	; $39E5
	jr z,loc_00_3A0E	; $39E6
	call print_inline	; $39E8
	.INCBIN "bank00.bin" SKIP $39EB READ 8	; text, 8 bytes
	ld a,(iy+$05)	; $39F3
	call combat_strength_index_b0	; $39F6
	ld hl,$3CFB	; $39F9
	rst $28	; $39FC
	rrca	; $39FD
	rrca	; $39FE
	rrca	; $39FF
	rrca	; $3A00
	and $0F	; $3A01
	inc a	; $3A03
	ld l,a	; $3A04
	ld h,$00	; $3A05
	ld c,b	; $3A07
	call member_gain_xp	; $3A08
	call fn_01_5627	; $3A0B

loc_00_3A0E:
	ld (iy+$05),$00	; $3A0E
	ret	; $3A12

loc_00_3A13:
	ld b,$00	; $3A13
	ld c,a	; $3A15
	cp $18	; $3A16
	jr c,loc_00_3A3A	; $3A18
	ld a,(iy+$05)	; $3A1A
	call combat_strength_index_b0	; $3A1D
	ld hl,$3CFB	; $3A20
	rst $28	; $3A23
	srl a	; $3A24
	ld e,a	; $3A26
	srl a	; $3A27
	ld d,a	; $3A29
	add a,e	; $3A2A
	ld h,a	; $3A2B
	ld a,c	; $3A2C
	inc b	; $3A2D
	cp d	; $3A2E
	jr c,loc_00_3A3A	; $3A2F
	inc b	; $3A31
	cp e	; $3A32
	jr c,loc_00_3A3A	; $3A33
	inc b	; $3A35
	cp h	; $3A36
	jr c,loc_00_3A3A	; $3A37
	inc b	; $3A39

loc_00_3A3A:
	ld a,b	; $3A3A
	ld hl,$3A42	; $3A3B
	rst $20	; $3A3E
	jp print_text_hl	; $3A3F

tbl_00_3A42:
	.dw $3A4C
	.dw $3A55
	.dw $3A5F
	.dw $3A70
	.dw $3A81
	.dw fn_01_6C46
	.dw fn_01_6565
	.dw $6E69
	.INCBIN "bank00.bin" SKIP $3A52 READ 3	; data, 3 bytes

fn_00_3A55:
	.INCBIN "bank00.bin" SKIP $3A55 READ 10	; data, 10 bytes

fn_00_3A5F:
	.INCBIN "bank00.bin" SKIP $3A5F READ 17	; data, 17 bytes

fn_00_3A70:
	.INCBIN "bank00.bin" SKIP $3A70 READ 17	; data, 17 bytes

fn_00_3A81:
	.INCBIN "bank00.bin" SKIP $3A81 READ 16	; data, 16 bytes

fn_00_3A91:
	call task_enter_3	; $3A91
	ld a,$14	; $3A94
	call fn_00_05F1	; $3A96
	call fn_00_399C	; $3A99
	call fn_00_3991	; $3A9C
	ld (ix+$0B),l	; $3A9F
	ld (ix+$0D),h	; $3AA2
	ld (ix+$17),$00	; $3AA5

loc_00_3AA9:
	ld a,(ix+$17)	; $3AA9
	ld hl,$3AC1	; $3AAC
	rst $28	; $3AAF
	or a	; $3AB0
	jr z,loc_00_3ABE	; $3AB1
	call fn_00_307C	; $3AB3
	call fn_00_305A	; $3AB6
	inc (ix+$17)	; $3AB9
	jr loc_00_3AA9	; $3ABC

loc_00_3ABE:
	jp task_return_3	; $3ABE
	.INCBIN "bank00.bin" SKIP $3AC1 READ 13	; data, 13 bytes

combat_strength_index_b0:
	cp $80	; $3ACE
	jr c,loc_00_3ADD	; $3AD0
	rrca	; $3AD2
	and $3F	; $3AD3
	cp $08	; $3AD5
	ret c	; $3AD7
	srl a	; $3AD8
	add a,$04	; $3ADA
	ret	; $3ADC

loc_00_3ADD:
	and $1F	; $3ADD
	add a,$24	; $3ADF
	ret	; $3AE1

fn_00_3AE2:
	ld a,($C959)	; $3AE2
	cp $14	; $3AE5
	jr c,loc_00_3AF2	; $3AE7
	ld a,($C990)	; $3AE9
	and $FC	; $3AEC
	cp $10	; $3AEE
	jr nz,loc_00_3B0F	; $3AF0

loc_00_3AF2:
	ld a,($C97D)	; $3AF2
	cp $80	; $3AF5
	jr nz,loc_00_3B02	; $3AF7
	ld a,$C8	; $3AF9
	ld ($C97E),a	; $3AFB
	ld a,$0B	; $3AFE
	jr loc_00_3B3B	; $3B00

loc_00_3B02:
	ld a,($C97C)	; $3B02
	cp $03	; $3B05
	ld a,$08	; $3B07
	jr c,loc_00_3B3B	; $3B09
	ld a,$09	; $3B0B
	jr loc_00_3B3B	; $3B0D

loc_00_3B0F:
	ld a,($C97D)	; $3B0F
	cp $80	; $3B12
	jr nz,loc_00_3B1F	; $3B14
	ld a,$C8	; $3B16
	ld ($C97E),a	; $3B18
	ld a,$0C	; $3B1B
	jr loc_00_3B3B	; $3B1D

loc_00_3B1F:
	ld a,($C97C)	; $3B1F
	cp $03	; $3B22
	ld a,$0A	; $3B24
	jr c,loc_00_3B3B	; $3B26
	ld a,($C990)	; $3B28
	ld hl,$3B3E	; $3B2B
	ld b,$0A	; $3B2E

loc_00_3B30:
	cp (hl)	; $3B30
	inc hl	; $3B31
	jr z,loc_00_3B3A	; $3B32
	inc hl	; $3B34
	djnz loc_00_3B30	; $3B35
	xor a	; $3B37
	jr loc_00_3B3B	; $3B38

loc_00_3B3A:
	ld a,(hl)	; $3B3A

loc_00_3B3B:
	jp load_arena	; $3B3B
	.INCBIN "bank00.bin" SKIP $3B3E READ 20	; data, 20 bytes

loc_00_3B52:
	call fn_00_3BC0	; $3B52
	ld a,(iy+$03)	; $3B55
	ld ($C97D),a	; $3B58
	ld ($C97E),a	; $3B5B
	ld l,(iy+$01)	; $3B5E
	ld h,(iy+$02)	; $3B61
	ld ($C97A),hl	; $3B64
	ld (iy+$03),$00	; $3B67
	ld (iy+$00),$00	; $3B6B
	ld a,($C990)	; $3B6F
	rrca	; $3B72
	rrca	; $3B73
	rrca	; $3B74
	rrca	; $3B75
	and $0F	; $3B76
	ld hl,$3B84	; $3B78
	rst $28	; $3B7B
	add a,$0F	; $3B7C
	call load_arena	; $3B7E
	jp loc_00_3CEA	; $3B81
	.INCBIN "bank00.bin" SKIP $3B84 READ 16	; data, 16 bytes

loc_00_3B94:
	call fn_00_3BC0	; $3B94
	ld a,$16	; $3B97
	call load_arena	; $3B99
	jp loc_00_3CEA	; $3B9C

loc_00_3B9F:
	call fn_00_3BC0	; $3B9F
	ld a,($C97D)	; $3BA2
	ld ($C97E),a	; $3BA5
	ld hl,room_monsters	; $3BA8
	ld bc,$8000	; $3BAB
	call fn_00_333B	; $3BAE
	ld hl,party_objects	; $3BB1
	ld bc,$2800	; $3BB4
	call fn_00_333B	; $3BB7
	call fn_00_3AE2	; $3BBA
	jp loc_00_3CEA	; $3BBD

fn_00_3BC0:
	call print_inline	; $3BC0
	.INCBIN "bank00.bin" SKIP $3BC3 READ 21	; text, 21 bytes
	ret	; $3BD8

loc_00_3BD9:
	ld a,(iy+$03)	; $3BD9
	ld ($C97D),a	; $3BDC
	ld e,(iy+$01)	; $3BDF
	ld d,(iy+$02)	; $3BE2
	ld ($C97A),de	; $3BE5
	call fn_02_BACF	; $3BE9
	ld ($C97C),a	; $3BEC
	xor a	; $3BEF
	ld (iy+$06),a	; $3BF0
	ld (iy+$00),a	; $3BF3
	ld (iy+$03),a	; $3BF6
	jp loc_00_3B9F	; $3BF9

fn_00_3BFC:
	push iy	; $3BFC
	ld hl,$C83E	; $3BFE
	ld de,$0008	; $3C01
	ld bc,$1000	; $3C04

loc_00_3C07:
	ld (hl),c	; $3C07
	add hl,de	; $3C08
	djnz loc_00_3C07	; $3C09
	ld a,($C975)	; $3C0B
	cp $01	; $3C0E
	jr nz,loc_00_3C2D	; $3C10
	ld a,(map_mode)	; $3C12
	cp $04	; $3C15
	jr z,loc_00_3C2D	; $3C17
	ld a,($C97D)	; $3C19
	cp $50	; $3C1C
	jr nz,loc_00_3C27	; $3C1E
	ld a,(party_size)	; $3C20
	add a,a	; $3C23
	dec a	; $3C24
	jr loc_00_3C28	; $3C25

loc_00_3C27:
	xor a	; $3C27

loc_00_3C28:
	ld (ix+$17),a	; $3C28
	jr loc_00_3C4C	; $3C2B

loc_00_3C2D:
	ld a,($C97E)	; $3C2D
	cp $80	; $3C30
	jr c,loc_00_3C3B	; $3C32
	call random	; $3C34
	and $07	; $3C37
	jr nz,loc_00_3C49	; $3C39

loc_00_3C3B:
	call combat_strength_index_b0	; $3C3B
	ld hl,$3D53	; $3C3E
	rst $28	; $3C41
	ld h,a	; $3C42
	call random_below	; $3C43
	srl h	; $3C46
	add a,h	; $3C48

loc_00_3C49:
	ld (ix+$17),a	; $3C49

loc_00_3C4C:
	ld a,(ix+$17)	; $3C4C
	srl a	; $3C4F
	ld hl,party_size	; $3C51
	cp (hl)	; $3C54
	jr c,loc_00_3C5F	; $3C55
	ld a,(hl)	; $3C57
	add a,a	; $3C58
	call random_below	; $3C59
	ld (ix+$17),a	; $3C5C

loc_00_3C5F:
	call random	; $3C5F
	and $0F	; $3C62
	ld c,a	; $3C64
	call fn_01_48EB	; $3C65
	ld a,($C97E)	; $3C68
	ld d,a	; $3C6B
	jr loc_00_3CA6	; $3C6C

loc_00_3C6E:
	call random	; $3C6E

loc_00_3C71:
	and $0F	; $3C71
	ld c,a	; $3C73
	call fn_01_48EB	; $3C74
	ld a,(iy+$05)	; $3C77
	or a	; $3C7A
	jr z,loc_00_3C81	; $3C7B
	ld a,c	; $3C7D
	inc a	; $3C7E
	jr loc_00_3C71	; $3C7F

loc_00_3C81:
	ld a,($C97E)	; $3C81
	ld d,a	; $3C84
	cp $81	; $3C85
	jr c,loc_00_3CA6	; $3C87
	call random	; $3C89
	and $1F	; $3C8C
	ld b,$02	; $3C8E
	jr z,loc_00_3C9B	; $3C90
	call random	; $3C92
	and $07	; $3C95
	ld b,$01	; $3C97
	jr nz,loc_00_3CA6	; $3C99

loc_00_3C9B:
	ld a,d	; $3C9B
	call combat_strength_index_b0	; $3C9C
	ld hl,$3D2F	; $3C9F
	rst $28	; $3CA2
	ld d,a	; $3CA3
	djnz loc_00_3C6E	; $3CA4

loc_00_3CA6:
	ld (iy+$05),d	; $3CA6
	ld (iy+$06),d	; $3CA9
	ld a,d	; $3CAC
	call combat_strength_index_b0	; $3CAD
	ld hl,$3CFB	; $3CB0
	rst $28	; $3CB3
	ld e,a	; $3CB4
	call random_below	; $3CB5
	srl e	; $3CB8
	or e	; $3CBA
	ld (iy+$04),a	; $3CBB
	dec (ix+$17)	; $3CBE
	jp p,loc_00_3C6E	; $3CC1
	ld iy,party_objects	; $3CC4
	ld de,$0005	; $3CC8
	ld bc,$0800	; $3CCB

loc_00_3CCE:
	call fn_01_4445	; $3CCE
	jr nc,loc_00_3CD9	; $3CD1
	ld (iy+$02),$00	; $3CD3
	jr loc_00_3CE2	; $3CD7

loc_00_3CD9:
	call fn_01_42F3	; $3CD9
	ld (iy+$02),a	; $3CDC
	ld (iy+$03),a	; $3CDF

loc_00_3CE2:
	add iy,de	; $3CE2
	inc c	; $3CE4
	djnz loc_00_3CCE	; $3CE5
	pop iy	; $3CE7
	ret	; $3CE9

loc_00_3CEA:
	ld a,(map_mode)	; $3CEA
	ld ($C975),a	; $3CED
	call fn_00_3BFC	; $3CF0
	ld a,$14	; $3CF3
	ld (game_mode),a	; $3CF5
	jp main_loop	; $3CF8

combat_strength_b0:
	.INCBIN "bank00.bin" SKIP $3CFB READ 125	; data, 125 bytes

fn_00_3D78:
	xor a	; $3D78
	ld ($C970),a	; $3D79
	call load_dungeon	; $3D7C
	jp loc_00_332D	; $3D7F

load_dungeon_room:
	call dungeon_header_address	; $3D82
	call bank_push	; $3D85
	ld a,(dungeon_room)	; $3D88
	inc a	; $3D8B
	rst $20	; $3D8C
	ld de,$C8E1	; $3D8D
	ld b,$04	; $3D90

loc_00_3D92:
	push bc	; $3D92
	ldi	; $3D93
	ld b,$03	; $3D95

loc_00_3D97:
	call unpack_position	; $3D97
	ex af,af'	; $3D9A
	ld (de),a	; $3D9B
	inc de	; $3D9C
	ex af,af'	; $3D9D
	ld (de),a	; $3D9E
	inc de	; $3D9F
	djnz loc_00_3D97	; $3DA0
	pop bc	; $3DA2
	djnz loc_00_3D92	; $3DA3
	ld iy,room_monsters	; $3DA5
	ld de,$0008	; $3DA9
	ld b,$10	; $3DAC

loc_00_3DAE:
	ld a,(hl)	; $3DAE
	inc hl	; $3DAF
	ld (iy+$05),a	; $3DB0
	cp $AC	; $3DB3
	jr nz,loc_00_3DB9	; $3DB5
	ld a,$3C	; $3DB7

loc_00_3DB9:
	ld (iy+$06),a	; $3DB9
	call unpack_position	; $3DBC
	ld (iy+$00),a	; $3DBF
	ld (iy+$02),a	; $3DC2
	ex af,af'	; $3DC5
	ld (iy+$01),a	; $3DC6
	ld (iy+$03),a	; $3DC9
	ld (iy+$07),$00	; $3DCC
	push hl	; $3DD0
	ld a,(iy+$05)	; $3DD1
	call combat_strength_index_b0	; $3DD4
	ld hl,$3CFB	; $3DD7
	rst $28	; $3DDA
	ld c,a	; $3DDB
	call random_below	; $3DDC
	srl c	; $3DDF
	or c	; $3DE1
	ld (iy+$04),a	; $3DE2
	pop hl	; $3DE5
	add iy,de	; $3DE6
	djnz loc_00_3DAE	; $3DE8
	push hl	; $3DEA
	ld a,(entry_direction)	; $3DEB
	inc a	; $3DEE
	and $03	; $3DEF
	add a,a	; $3DF1
	add a,a	; $3DF2
	add a,a	; $3DF3
	rst $30	; $3DF4
	ld iy,party_objects	; $3DF5
	ld de,$0005	; $3DF9
	ld bc,$0800	; $3DFC

loc_00_3DFF:
	call unpack_position	; $3DFF
	ld (iy+$00),a	; $3E02
	ex af,af'	; $3E05
	ld (iy+$01),a	; $3E06
	call fn_01_4445	; $3E09
	ld a,$00	; $3E0C
	jr c,loc_00_3E13	; $3E0E
	call fn_01_42F3	; $3E10

loc_00_3E13:
	ld (iy+$02),a	; $3E13
	ld (iy+$03),a	; $3E16
	add iy,de	; $3E19
	inc c	; $3E1B
	djnz loc_00_3DFF	; $3E1C
	pop hl	; $3E1E
	ld de,$0020	; $3E1F
	add hl,de	; $3E22
	ld de,$D800	; $3E23
	call decode_location_map	; $3E26
	call terrain_substitute	; $3E29
	ld hl,$D800	; $3E2C
	ld bc,$9038	; $3E2F
	ld de,$923C	; $3E32
	ld a,$79	; $3E35

loc_00_3E37:
	ex af,af'	; $3E37
	ld a,(hl)	; $3E38
	inc hl	; $3E39
	cp c	; $3E3A
	jr nz,loc_00_3E3E	; $3E3B
	ld (hl),b	; $3E3D

loc_00_3E3E:
	cp e	; $3E3E
	jr nz,loc_00_3E42	; $3E3F
	ld (hl),d	; $3E41

loc_00_3E42:
	inc hl	; $3E42
	ex af,af'	; $3E43
	dec a	; $3E44
	jp nz,loc_00_3E37	; $3E45
	ld a,$FF	; $3E48
	ld ($C984),a	; $3E4A
	ld a,(location)	; $3E4D
	cp $19	; $3E50
	jr nc,loc_00_3EA7	; $3E52
	ld a,(dungeon_room)	; $3E54
	cp $0F	; $3E57
	jr nz,loc_00_3EA7	; $3E59
	call print_inline	; $3E5B
	.INCBIN "bank00.bin" SKIP $3E5E READ 19	; text, 19 bytes
	ld a,(party_x)	; $3E71
	cp $03	; $3E74
	jr nc,loc_00_3E84	; $3E76
	call print_inline	; $3E78
	.INCBIN "bank00.bin" SKIP $3E7B READ 6	; text, 6 bytes
	xor a	; $3E81
	jr loc_00_3EA1	; $3E82

loc_00_3E84:
	cp $09	; $3E84
	jr nc,loc_00_3E94	; $3E86
	call print_inline	; $3E88
	.INCBIN "bank00.bin" SKIP $3E8B READ 5	; text, 5 bytes
	ld a,$01	; $3E90
	jr loc_00_3EA1	; $3E92

loc_00_3E94:
	call print_inline	; $3E94
	.INCBIN "bank00.bin" SKIP $3E97 READ 8	; text, 8 bytes
	ld a,$02	; $3E9F

loc_00_3EA1:
	ld ($C984),a	; $3EA1
	call print_newline	; $3EA4

loc_00_3EA7:
	jp bank_pop	; $3EA7

unpack_position:
	ld a,(hl)	; $3EAA
	rlca	; $3EAB
	rlca	; $3EAC
	rlca	; $3EAD
	rlca	; $3EAE
	and $0F	; $3EAF
	ex af,af'	; $3EB1
	ld a,(hl)	; $3EB2
	and $0F	; $3EB3
	inc hl	; $3EB5
	ret	; $3EB6

fn_00_3EB7:
	push bc	; $3EB7
	push hl	; $3EB8
	ld hl,$3EC8	; $3EB9
	ld bc,$0027	; $3EBC
	cpir	; $3EBF
	scf	; $3EC1
	jr nz,loc_00_3EC5	; $3EC2
	or a	; $3EC4

loc_00_3EC5:
	pop hl	; $3EC5
	pop bc	; $3EC6
	ret	; $3EC7
	.INCBIN "bank00.bin" SKIP $3EC8 READ 39	; data, 39 bytes

fn_00_3EEF:
	push bc	; $3EEF
	ld b,$07	; $3EF0
	cp $03	; $3EF2
	jr z,loc_00_3F0A	; $3EF4
	ld b,$03	; $3EF6
	cp $06	; $3EF8
	jr z,loc_00_3F0A	; $3EFA
	cp $05	; $3EFC
	jr z,loc_00_3F0A	; $3EFE
	ld b,$01	; $3F00
	cp $07	; $3F02
	jr z,loc_00_3F0A	; $3F04
	cp $46	; $3F06
	jr nz,loc_00_3F11	; $3F08

loc_00_3F0A:
	ld a,(main_loop_count)	; $3F0A
	and b	; $3F0D
	scf	; $3F0E
	jr z,loc_00_3F12	; $3F0F

loc_00_3F11:
	or a	; $3F11

loc_00_3F12:
	pop bc	; $3F12
	ret	; $3F13

fn_00_3F14:
	ld a,(map_mode)	; $3F14
	ld hl,$3F1D	; $3F17
	jp fn_00_0018	; $3F1A

tbl_00_3F1D:
	.dw fn_00_3F2B
	.dw fn_00_3F67
	.dw fn_00_3F84
	.dw fn_00_3FDC
	.dw fn_00_3FDC
	.dw fn_00_3FDC
	.dw fn_00_3FDC

fn_00_3F2B:
	call $957C	; $3F2B
	ret z	; $3F2E
	ld a,($C990)	; $3F2F
	cp $17	; $3F32
	jr nz,fn_00_3F67	; $3F34
	call random	; $3F36
	and $07	; $3F39
	jr nz,fn_00_3F67	; $3F3B
	call print_inline	; $3F3D
	.INCBIN "bank00.bin" SKIP $3F40 READ 17	; text, 17 bytes
	ld a,$A4	; $3F51
	ld ($C97D),a	; $3F53
	ld hl,(world_x_saved)	; $3F56
	ld (party_x),hl	; $3F59
	ld ($C97A),hl	; $3F5C
	ld a,$17	; $3F5F
	ld ($C97C),a	; $3F61
	jp loc_00_3B9F	; $3F64

fn_00_3F67:
	ld a,($C990)	; $3F67
	cp $03	; $3F6A
	jp z,loc_01_4071	; $3F6C
	cp $44	; $3F6F
	jp z,loc_01_4071	; $3F71
	cp $4C	; $3F74
	jp z,fn_01_4812	; $3F76
	cp $46	; $3F79
	jp z,fn_01_4812	; $3F7B
	cp $47	; $3F7E
	jp z,loc_01_404A	; $3F80
	ret	; $3F83

fn_00_3F84:
	ld a,($C990)	; $3F84
	ld b,a	; $3F87
	and $F0	; $3F88
	cp $A0	; $3F8A
	jr nz,loc_00_3F9F	; $3F8C
	ld a,b	; $3F8E
	and $0F	; $3F8F
	jp z,loc_01_4071	; $3F91
	cp $02	; $3F94
	jp z,fn_01_4812	; $3F96
	cp $03	; $3F99
	jp z,loc_01_404A	; $3F9B
	ret	; $3F9E

loc_00_3F9F:
	cp $80	; $3F9F
	ret nz	; $3FA1
	ld a,b	; $3FA2
	cp $80	; $3FA3
	jr nz,loc_00_3FB7	; $3FA5
	call print_inline	; $3FA7
	.INCBIN "bank00.bin" SKIP $3FAA READ 8	; text, 8 bytes
	xor a	; $3FB2
	ld ($C95B),a	; $3FB3
	ret	; $3FB6

loc_00_3FB7:
	cp $88	; $3FB7
	jr nc,loc_00_3FD0	; $3FB9
	call print_inline	; $3FBB
	.INCBIN "bank00.bin" SKIP $3FBE READ 16	; text, 16 bytes
	jr loc_00_3FD9	; $3FCE

loc_00_3FD0:
	call print_inline	; $3FD0
	.INCBIN "bank00.bin" SKIP $3FD3 READ 6	; text, 6 bytes

loc_00_3FD9:
	jp fn_01_4812	; $3FD9

fn_00_3FDC:
	call fn_01_48E1	; $3FDC
	ld a,(iy+$02)	; $3FDF
	or a	; $3FE2
	ret z	; $3FE3
	ld l,(iy+$00)	; $3FE4
	ld h,(iy+$01)	; $3FE7
	call fn_00_38EB	; $3FEA
	ld hl,$3FF4	; $3FED
	call switch_on_a	; $3FF0
	ret	; $3FF3
	.INCBIN "bank00.bin" SKIP $3FF4 READ 12	; data, 12 bytes
