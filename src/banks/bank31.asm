; Bank 31 (ROM $7C000-$7FFFF), mapped at $8000
.BANK 31 SLOT 2
.ORG $0000


sound_vectors:
	jp loc_1F_8018	; $8000

fn_1F_8003:
	jp loc_1F_8ABE	; $8003

fn_1F_8006:
	jp loc_1F_8AF3	; $8006

fn_1F_8009:
	jp loc_1F_8A4A	; $8009

fn_1F_800C:
	jp loc_1F_8F1F	; $800C

fn_1F_800F:
	jp loc_1F_8F28	; $800F

fn_1F_8012:
	jp loc_1F_8B5A	; $8012

fn_1F_8015:
	jp loc_1F_8B60	; $8015

loc_1F_8018:
	ld hl,$DD02	; $8018
	bit 2,(hl)	; $801B
	ret nz	; $801D
	call fn_1F_87AA	; $801E
	set 2,(hl)	; $8021
	bit 0,(hl)	; $8023
	jp z,loc_1F_8047	; $8025
	res 0,(hl)	; $8028
	push hl	; $802A
	sub a	; $802B
	call fn_1F_8B64	; $802C
	ld hl,$DD69	; $802F
	ld de,$0028	; $8032
	ld b,$08	; $8035

loc_1F_8037:
	set 1,(hl)	; $8037
	add hl,de	; $8039
	djnz loc_1F_8037	; $803A
	pop hl	; $803C
	jr loc_1F_8047	; $803D

loc_1F_803F:
	ld b,$01	; $803F
	ld ix,$DE75	; $8041
	jr loc_1F_8057	; $8045

loc_1F_8047:
	bit 1,(hl)	; $8047
	jr nz,loc_1F_803F	; $8049
	ld a,($DE75)	; $804B
	and a	; $804E
	jr nz,loc_1F_803F	; $804F
	ld b,$07	; $8051
	ld ix,$DD5D	; $8053

loc_1F_8057:
	ld a,(ix+$00)	; $8057
	and a	; $805A
	jp z,loc_1F_8107	; $805B
	ld d,a	; $805E
	ld e,(ix+$0B)	; $805F
	ld c,(ix+$0C)	; $8062
	exx	; $8065
	ld a,(ix+$05)	; $8066
	and a	; $8069
	jr z,loc_1F_8074	; $806A
	add a,(ix+$0F)	; $806C
	ld (ix+$0F),a	; $806F
	jr nc,loc_1F_80AA	; $8072

loc_1F_8074:
	ld b,(ix+$0D)	; $8074
	exx	; $8077
	bit 2,e	; $8078
	exx	; $807A
	jr z,loc_1F_809F	; $807B
	ld a,(ix+$15)	; $807D
	and a	; $8080
	jr z,loc_1F_809F	; $8081
	jp m,loc_1F_8091	; $8083
	add a,b	; $8086
	jr c,loc_1F_8098	; $8087
	cp (ix+$0E)	; $8089
	jr c,loc_1F_809F	; $808C
	jp loc_1F_8098	; $808E

loc_1F_8091:
	and $7F	; $8091
	cp b	; $8093
	jr z,loc_1F_8098	; $8094
	jr nc,loc_1F_809F	; $8096

loc_1F_8098:
	exx	; $8098
	res 2,e	; $8099
	ld (ix+$0B),e	; $809B
	exx	; $809E

loc_1F_809F:
	inc b	; $809F
	ld (ix+$0D),b	; $80A0
	ld a,b	; $80A3
	cp (ix+$0E)	; $80A4
	call z,fn_1F_8329	; $80A7

loc_1F_80AA:
	ld a,(ix+$02)	; $80AA
	add a,a	; $80AD
	jr z,loc_1F_80B7	; $80AE
	bit 4,(ix+$01)	; $80B0
	jp z,loc_1F_80C0	; $80B4

loc_1F_80B7:
	ld a,(ix+$01)	; $80B7
	ld ($DD5A),a	; $80BA
	jp loc_1F_80C3	; $80BD

loc_1F_80C0:
	call fn_1F_86E4	; $80C0

loc_1F_80C3:
	ld a,(ix+$03)	; $80C3
	and $3F	; $80C6
	add a,a	; $80C8
	call nz,fn_1F_874E	; $80C9
	exx	; $80CC
	bit 7,e	; $80CD
	exx	; $80CF
	call nz,fn_1F_8295	; $80D0
	call fn_1F_87F3	; $80D3
	exx	; $80D6
	bit 5,e	; $80D7
	jp z,loc_1F_8104	; $80D9
	ld a,(ix+$18)	; $80DC
	add a,(ix+$19)	; $80DF
	ld (ix+$19),a	; $80E2
	jr nc,loc_1F_8104	; $80E5
	ld a,(ix+$01)	; $80E7
	cp (ix+$1A)	; $80EA
	jp nz,loc_1F_80F5	; $80ED
	set 1,e	; $80F0
	jp loc_1F_8104	; $80F2

loc_1F_80F5:
	res 1,e	; $80F5
	ld a,(ix+$01)	; $80F7
	dec a	; $80FA
	bit 4,e	; $80FB
	jr nz,loc_1F_8101	; $80FD
	add a,$02	; $80FF

loc_1F_8101:
	ld (ix+$01),a	; $8101

loc_1F_8104:
	ld (ix+$0B),e	; $8104

loc_1F_8107:
	ld a,($DD02)	; $8107
	and $3F	; $810A
	ld ($DD02),a	; $810C
	ld de,$0028	; $810F
	add ix,de	; $8112
	dec b	; $8114
	jp nz,loc_1F_8057	; $8115
	ld hl,$DD02	; $8118
	bit 5,a	; $811B
	jr z,loc_1F_8146	; $811D
	ld de,($DD0F)	; $811F
	ld a,($DD0E)	; $8123
	add a,e	; $8126
	ld ($DD0F),a	; $8127
	jr nc,loc_1F_8146	; $812A
	ld a,($DD11)	; $812C
	cp d	; $812F
	jr z,loc_1F_8146	; $8130
	bit 4,(hl)	; $8132
	jr nz,loc_1F_8142	; $8134
	inc a	; $8136
	ld ($DD11),a	; $8137
	cp d	; $813A
	jr nz,loc_1F_8146	; $813B
	call fn_1F_84DA	; $813D
	jr loc_1F_8146	; $8140

loc_1F_8142:
	dec a	; $8142
	ld ($DD11),a	; $8143

loc_1F_8146:
	ld hl,$DD02	; $8146
	res 2,(hl)	; $8149

fn_1F_814B:
	ld a,($DD58)	; $814B
	and a	; $814E
	jp p,fn_1F_824C	; $814F
	ld hl,$DD56	; $8152
	bit 7,(hl)	; $8155
	jr z,loc_1F_8178	; $8157
	ld bc,$8EBD	; $8159
	res 7,(hl)	; $815C
	ld a,(hl)	; $815E
	ld l,a	; $815F
	ld h,$00	; $8160
	add hl,hl	; $8162
	add hl,hl	; $8163
	add hl,hl	; $8164
	add hl,bc	; $8165
	ld bc,$0800	; $8166

loc_1F_8169:
	ld a,c	; $8169
	out ($F0),a	; $816A
	ld a,(hl)	; $816C
	out ($F1),a	; $816D
	inc c	; $816F
	push hl	; $8170
	pop hl	; $8171
	push hl	; $8172
	pop hl	; $8173
	nop	; $8174
	inc hl	; $8175
	djnz loc_1F_8169	; $8176

loc_1F_8178:
	ld hl,$DD12	; $8178
	ld de,$DD1B	; $817B
	ld bc,$DD24	; $817E
	exx	; $8181
	ld de,$DD41	; $8182
	ld hl,$DD4A	; $8185
	ld c,$00	; $8188
	ld b,$06	; $818A
	bit 6,a	; $818C
	jr z,loc_1F_8192	; $818E
	ld b,$09	; $8190

loc_1F_8192:
	ld a,c	; $8192
	or $30	; $8193
	exx	; $8195
	out ($F0),a	; $8196
	ld a,(de)	; $8198
	and a	; $8199
	jr nz,loc_1F_81C9	; $819A
	ld a,$BF	; $819C
	out ($F1),a	; $819E
	ld a,(de)	; $81A0
	inc bc	; $81A1
	inc de	; $81A2
	inc hl	; $81A3
	exx	; $81A4
	cp (hl)	; $81A5
	jr z,loc_1F_81C0	; $81A6
	ld (hl),a	; $81A8
	ld a,c	; $81A9
	or $20	; $81AA
	inc de	; $81AC
	inc hl	; $81AD
	out ($F0),a	; $81AE
	ld a,(hl)	; $81B0
	out ($F1),a	; $81B1
	inc c	; $81B3
	push hl	; $81B4
	pop hl	; $81B5
	and $00	; $81B6
	nop	; $81B8
	jp loc_1F_81C3	; $81B9

loc_1F_81BC:
	inc bc	; $81BC
	inc de	; $81BD
	inc hl	; $81BE
	exx	; $81BF

loc_1F_81C0:
	inc de	; $81C0
	inc hl	; $81C1
	inc c	; $81C2

loc_1F_81C3:
	djnz loc_1F_8192	; $81C3
	nop	; $81C5
	jp loc_1F_8212	; $81C6

loc_1F_81C9:
	ld a,(bc)	; $81C9
	out ($F1),a	; $81CA
	ld a,(de)	; $81CC
	exx	; $81CD
	cp (hl)	; $81CE
	jp z,loc_1F_81F2	; $81CF
	bit 6,a	; $81D2
	ex af,af'	; $81D4
	ld a,c	; $81D5
	or $20	; $81D6
	exx	; $81D8
	and $FF	; $81D9
	and $FF	; $81DB
	nop	; $81DD
	out ($F0),a	; $81DE
	ex af,af'	; $81E0
	jr z,loc_1F_81EC	; $81E1
	and $EF	; $81E3
	out ($F1),a	; $81E5
	or $10	; $81E7
	call fn_1F_8290	; $81E9

loc_1F_81EC:
	out ($F1),a	; $81EC
	and $9F	; $81EE
	exx	; $81F0
	ld (hl),a	; $81F1

loc_1F_81F2:
	ld a,(de)	; $81F2
	exx	; $81F3
	cp (hl)	; $81F4
	jp z,loc_1F_81BC	; $81F5
	exx	; $81F8
	ld a,c	; $81F9
	or $10	; $81FA
	exx	; $81FC
	nop	; $81FD
	nop	; $81FE
	out ($F0),a	; $81FF
	ld a,(hl)	; $8201
	out ($F1),a	; $8202
	inc bc	; $8204
	inc de	; $8205
	inc hl	; $8206
	exx	; $8207
	ld (de),a	; $8208
	inc de	; $8209
	inc hl	; $820A
	inc c	; $820B
	dec b	; $820C
	jp nz,loc_1F_8192	; $820D
	ld a,$00	; $8210

loc_1F_8212:
	ld a,$0E	; $8212
	out ($F0),a	; $8214
	ld a,($DD40)	; $8216
	out ($F1),a	; $8219
	ld c,a	; $821B
	ld a,($DD3F)	; $821C
	cp c	; $821F
	jp z,loc_1F_822D	; $8220
	call fn_1F_8294	; $8223
	and $FF	; $8226
	and $FF	; $8228
	nop	; $822A
	out ($F1),a	; $822B

loc_1F_822D:
	and a	; $822D
	jr z,fn_1F_824C	; $822E
	ld bc,$0336	; $8230
	ld de,$DD2A	; $8233
	ld hl,$DD53	; $8236

loc_1F_8239:
	ld a,(de)	; $8239
	cp (hl)	; $823A
	jr z,loc_1F_8247	; $823B
	ex af,af'	; $823D
	ld a,c	; $823E
	out ($F0),a	; $823F
	ex af,af'	; $8241
	out ($F1),a	; $8242
	ld (hl),a	; $8244
	nop	; $8245
	nop	; $8246

loc_1F_8247:
	inc c	; $8247
	inc de	; $8248
	inc hl	; $8249
	djnz loc_1F_8239	; $824A

fn_1F_824C:
	ld hl,$DD03	; $824C
	ld b,$80	; $824F

loc_1F_8251:
	ld a,(hl)	; $8251
	and $0F	; $8252
	or b	; $8254
	out ($7F),a	; $8255
	inc hl	; $8257
	ld a,(hl)	; $8258
	and $3F	; $8259
	out ($7F),a	; $825B
	inc hl	; $825D
	ld a,b	; $825E
	add a,$10	; $825F
	ld b,a	; $8261
	ld a,(hl)	; $8262
	cpl	; $8263
	and $0F	; $8264
	or b	; $8266
	out ($7F),a	; $8267
	inc hl	; $8269
	ld a,b	; $826A
	add a,$10	; $826B
	ld b,a	; $826D
	cp $E0	; $826E
	jp nz,loc_1F_8251	; $8270
	ld a,($DD5C)	; $8273
	cp (hl)	; $8276
	res 3,a	; $8277
	ld (hl),a	; $8279
	ld ($DD5C),a	; $827A
	jp z,loc_1F_8286	; $827D
	and $07	; $8280
	or $E0	; $8282
	out ($7F),a	; $8284

loc_1F_8286:
	inc hl	; $8286
	ld a,(hl)	; $8287
	cpl	; $8288
	and $0F	; $8289
	or $F0	; $828B
	out ($7F),a	; $828D
	ret	; $828F

fn_1F_8290:
	push hl	; $8290
	pop hl	; $8291

fn_1F_8292:
	push hl	; $8292
	pop hl	; $8293

fn_1F_8294:
	ret	; $8294

fn_1F_8295:
	ld a,(ix+$20)	; $8295
	cp $40	; $8298
	jp c,loc_1F_82FB	; $829A
	and $3F	; $829D
	ld c,a	; $829F
	ld a,($DD02)	; $82A0
	bit 7,a	; $82A3
	jr z,loc_1F_82B4	; $82A5
	ld a,(ix+$13)	; $82A7
	and a	; $82AA
	jr z,loc_1F_82B0	; $82AB
	add a,(ix+$04)	; $82AD

loc_1F_82B0:
	ld (ix+$1B),a	; $82B0
	ret	; $82B3

loc_1F_82B4:
	ld a,(ix+$1B)	; $82B4
	and a	; $82B7
	ret z	; $82B8
	exx	; $82B9
	bit 6,e	; $82BA
	exx	; $82BC
	jr nz,loc_1F_82C2	; $82BD
	sub c	; $82BF
	jr loc_1F_82C3	; $82C0

loc_1F_82C2:
	add a,c	; $82C2

loc_1F_82C3:
	jr z,loc_1F_82F1	; $82C3
	ld b,(ix+$09)	; $82C5
	rlc b	; $82C8
	jr z,loc_1F_82D3	; $82CA
	ld hl,$8D6D	; $82CC
	cp $61	; $82CF
	jr loc_1F_82DC	; $82D1

loc_1F_82D3:
	ld hl,$8E2D	; $82D3
	cp $19	; $82D6
	jr c,loc_1F_82F0	; $82D8
	cp $79	; $82DA

loc_1F_82DC:
	jr nc,loc_1F_82F0	; $82DC
	ld (ix+$1B),a	; $82DE
	add a,a	; $82E1
	ld e,a	; $82E2
	ld d,$00	; $82E3
	add hl,de	; $82E5
	ld e,(hl)	; $82E6
	inc hl	; $82E7
	ld d,(hl)	; $82E8
	ld (ix+$16),e	; $82E9
	ld (ix+$17),d	; $82EC
	ret	; $82EF

loc_1F_82F0:
	sub a	; $82F0

loc_1F_82F1:
	ld (ix+$1B),a	; $82F1
	ld (ix+$16),a	; $82F4
	ld (ix+$17),a	; $82F7
	ret	; $82FA

loc_1F_82FB:
	ld l,(ix+$16)	; $82FB
	ld h,(ix+$17)	; $82FE
	ld b,h	; $8301
	ld c,l	; $8302

loc_1F_8303:
	srl b	; $8303
	rr c	; $8305
	dec a	; $8307
	jp nz,loc_1F_8303	; $8308
	exx	; $830B
	bit 6,e	; $830C
	exx	; $830E
	jr nz,loc_1F_831A	; $830F
	add hl,bc	; $8311
	ld a,h	; $8312
	cp $04	; $8313
	jr c,loc_1F_8322	; $8315
	jp loc_1F_831F	; $8317

loc_1F_831A:
	and a	; $831A
	sbc hl,bc	; $831B
	jr nc,loc_1F_8322	; $831D

loc_1F_831F:
	ld hl,$0000	; $831F

loc_1F_8322:
	ld (ix+$16),l	; $8322
	ld (ix+$17),h	; $8325
	ret	; $8328

fn_1F_8329:
	ld e,(ix+$07)	; $8329
	ld d,(ix+$08)	; $832C
	jp loc_1F_8333	; $832F
	inc de	; $8332

loc_1F_8333:
	ld a,(de)	; $8333
	and a	; $8334
	jp p,loc_1F_863C	; $8335
	cp $C0	; $8338
	jp nc,loc_1F_85E6	; $833A
	inc de	; $833D
	ld hl,$8332	; $833E
	push hl	; $8341
	ld hl,$8350	; $8342
	ld b,$00	; $8345
	add a,a	; $8347
	ld c,a	; $8348
	add hl,bc	; $8349
	ld a,(hl)	; $834A
	inc hl	; $834B
	ld h,(hl)	; $834C
	ld l,a	; $834D
	ld a,(de)	; $834E
	jp (hl)	; $834F
	.INCBIN "bank31.bin" SKIP $0350 READ 88	; data, 88 bytes

loc_1F_83A8:
	ld l,a	; $83A8
	inc de	; $83A9
	ld a,(de)	; $83AA
	ld h,a	; $83AB
	ex de,hl	; $83AC
	dec de	; $83AD
	ret	; $83AE
	inc de	; $83AF
	cp $11	; $83B0
	jr z,loc_1F_83BD	; $83B2
	dec (ix+$10)	; $83B4
	ld a,(de)	; $83B7
	jp nz,loc_1F_83A8	; $83B8
	inc de	; $83BB
	ret	; $83BC

loc_1F_83BD:
	dec (ix+$11)	; $83BD
	ld a,(de)	; $83C0
	jp nz,loc_1F_83A8	; $83C1
	inc de	; $83C4
	ret	; $83C5
	.INCBIN "bank31.bin" SKIP $03C6 READ 6	; data, 6 bytes
	ld (ix+$00),$00	; $83CC
	pop hl	; $83D0
	pop hl	; $83D1
	exx	; $83D2
	jp loc_1F_8107	; $83D3
	.INCBIN "bank31.bin" SKIP $03D6 READ 4	; data, 4 bytes
	ld (ix+$02),a	; $83DA
	ret	; $83DD
	.INCBIN "bank31.bin" SKIP $03DE READ 16	; data, 16 bytes
	ld c,a	; $83EE
	push ix	; $83EF
	pop hl	; $83F1
	add hl,bc	; $83F2
	inc de	; $83F3
	ld a,(de)	; $83F4
	ld (hl),a	; $83F5
	ret	; $83F6
	.INCBIN "bank31.bin" SKIP $03F7 READ 8	; data, 8 bytes
	ld c,(ix+$01)	; $83FF
	and a	; $8402
	jp p,loc_1F_840E	; $8403
	add a,c	; $8406
	jr c,loc_1F_8415	; $8407
	ld (ix+$01),$00	; $8409
	ret	; $840D

loc_1F_840E:
	add a,c	; $840E
	cp $10	; $840F
	jr c,loc_1F_8415	; $8411
	ld a,$0F	; $8413

loc_1F_8415:
	ld (ix+$01),a	; $8415
	ret	; $8418
	.INCBIN "bank31.bin" SKIP $0419 READ 16	; data, 16 bytes
	ld hl,$DD00	; $8429
	or (hl)	; $842C
	ld (hl),a	; $842D
	ret	; $842E
	.INCBIN "bank31.bin" SKIP $042F READ 19	; data, 19 bytes
	ld c,(ix+$0B)	; $8442
	res 1,c	; $8445
	res 4,c	; $8447
	and a	; $8449
	jp p,loc_1F_844F	; $844A
	set 4,c	; $844D

loc_1F_844F:
	and $0F	; $844F
	ld (ix+$1A),a	; $8451
	inc de	; $8454
	ld a,(de)	; $8455
	ld (ix+$18),a	; $8456
	ld (ix+$19),b	; $8459
	set 5,c	; $845C
	and a	; $845E
	jr nz,loc_1F_8463	; $845F
	res 5,c	; $8461

loc_1F_8463:
	ld (ix+$0B),c	; $8463
	ret	; $8466

fn_1F_8467:
	ld b,(ix+$0B)	; $8467
	res 6,b	; $846A
	and a	; $846C
	jp p,loc_1F_8472	; $846D
	set 6,b	; $8470

loc_1F_8472:
	set 7,b	; $8472
	and $7F	; $8474
	jr z,loc_1F_847F	; $8476
	ld (ix+$20),a	; $8478

loc_1F_847B:
	ld (ix+$0B),b	; $847B
	ret	; $847E

loc_1F_847F:
	res 7,b	; $847F
	jr loc_1F_847B	; $8481
	bit 1,(ix+$0B)	; $8483
	jp z,loc_1F_83A8	; $8487
	inc de	; $848A
	ret	; $848B
	.INCBIN "bank31.bin" SKIP $048C READ 29	; data, 29 bytes
	ld (ix+$05),a	; $84A9
	ret	; $84AC
	.INCBIN "bank31.bin" SKIP $04AD READ 45	; data, 45 bytes

fn_1F_84DA:
	res 5,(hl)	; $84DA
	ld a,$0F	; $84DC
	ld ($DD11),a	; $84DE
	ld bc,$0028	; $84E1
	ld hl,$DD68	; $84E4
	ld a,$07	; $84E7

loc_1F_84E9:
	res 3,(hl)	; $84E9
	add hl,bc	; $84EB
	dec a	; $84EC
	jr nz,loc_1F_84E9	; $84ED
	ret	; $84EF
	.INCBIN "bank31.bin" SKIP $04F0 READ 74	; data, 74 bytes
	add a,(ix+$21)	; $853A
	ld (ix+$21),a	; $853D
	ret	; $8540
	.INCBIN "bank31.bin" SKIP $0541 READ 88	; data, 88 bytes
	ld a,($DD58)	; $8599
	and a	; $859C
	jp p,loc_1F_85A2	; $859D

loc_1F_85A0:
	inc de	; $85A0
	ret	; $85A1

loc_1F_85A2:
	dec de	; $85A2
	ret	; $85A3
	ld a,($DD58)	; $85A4
	and a	; $85A7
	jp p,loc_1F_85A0	; $85A8
	jr loc_1F_85A2	; $85AB
	.INCBIN "bank31.bin" SKIP $05AD READ 51	; data, 51 bytes

loc_1F_85E0:
	ld a,(ix+$13)	; $85E0
	jp loc_1F_863D	; $85E3

loc_1F_85E6:
	cp $E0	; $85E6
	jp nc,loc_1F_85E0	; $85E8
	sub $C0	; $85EB
	jr c,loc_1F_85E0	; $85ED
	add a,a	; $85EF
	add a,a	; $85F0
	ld c,a	; $85F1
	ld b,$00	; $85F2
	ld a,(ix+$09)	; $85F4
	and a	; $85F7
	jr z,loc_1F_8600	; $85F8
	ld hl,$8FB1	; $85FA
	jp loc_1F_8603	; $85FD

loc_1F_8600:
	ld hl,$8F9D	; $8600

loc_1F_8603:
	add hl,bc	; $8603
	ld a,(hl)	; $8604
	ld (ix+$02),a	; $8605
	inc hl	; $8608
	ld a,(hl)	; $8609
	ld c,a	; $860A
	inc hl	; $860B
	ld a,(hl)	; $860C
	ld b,a	; $860D
	and $1F	; $860E
	ld (ix+$1D),a	; $8610
	ld (ix+$0A),a	; $8613
	ld a,b	; $8616
	and $60	; $8617
	rlca	; $8619
	rlca	; $861A
	rlca	; $861B
	ld b,a	; $861C
	ld a,(ix+$00)	; $861D
	and $FC	; $8620
	or b	; $8622
	ld (ix+$00),a	; $8623
	bit 7,(hl)	; $8626
	jr nz,loc_1F_8631	; $8628
	res 7,(ix+$0B)	; $862A
	jp loc_1F_8637	; $862E

loc_1F_8631:
	ld a,c	; $8631
	call fn_1F_8467	; $8632
	ld c,$00	; $8635

loc_1F_8637:
	ld (ix+$03),c	; $8637
	inc hl	; $863A
	ld a,(hl)	; $863B

loc_1F_863C:
	inc de	; $863C

loc_1F_863D:
	ex af,af'	; $863D
	ld a,(ix+$13)	; $863E
	and a	; $8641
	ex af,af'	; $8642
	ld (ix+$13),a	; $8643
	call fn_1F_86A9	; $8646
	ld (ix+$17),b	; $8649
	ld (ix+$16),c	; $864C
	ld a,(de)	; $864F
	cp $E0	; $8650
	jp nc,loc_1F_865B	; $8652
	ld a,(ix+$12)	; $8655
	jp loc_1F_866E	; $8658

loc_1F_865B:
	sub $E0	; $865B
	jp nz,loc_1F_8665	; $865D
	inc de	; $8660
	ld a,(de)	; $8661
	jp loc_1F_866D	; $8662

loc_1F_8665:
	ld hl,$8FC4	; $8665
	ld c,a	; $8668
	ld b,$00	; $8669
	add hl,bc	; $866B
	ld a,(hl)	; $866C

loc_1F_866D:
	inc de	; $866D

loc_1F_866E:
	ld (ix+$0E),a	; $866E
	ld (ix+$12),a	; $8671
	ld hl,$DD02	; $8674
	sub a	; $8677
	ld (ix+$0D),a	; $8678
	ld (ix+$1F),a	; $867B
	ex af,af'	; $867E
	jr z,loc_1F_8687	; $867F
	bit 4,(ix+$00)	; $8681
	jr nz,loc_1F_8691	; $8685

loc_1F_8687:
	ld (ix+$1E),$00	; $8687
	set 2,(ix+$0B)	; $868B
	set 6,(hl)	; $868F

loc_1F_8691:
	res 1,(ix+$0C)	; $8691
	set 7,(hl)	; $8695
	ld (ix+$08),d	; $8697
	ld (ix+$07),e	; $869A
	exx	; $869D
	ld d,(ix+$00)	; $869E
	ld e,(ix+$0B)	; $86A1
	ld c,(ix+$0C)	; $86A4
	exx	; $86A7
	ret	; $86A8

fn_1F_86A9:
	ld bc,$0000	; $86A9
	and a	; $86AC
	ret z	; $86AD
	bit 3,(ix+$00)	; $86AE
	jr nz,loc_1F_86DC	; $86B2
	add a,(ix+$04)	; $86B4
	add a,a	; $86B7
	ld c,a	; $86B8
	ld a,(ix+$09)	; $86B9
	and a	; $86BC
	jr z,loc_1F_86C5	; $86BD
	ld hl,$8D6D	; $86BF
	jp loc_1F_86C8	; $86C2

loc_1F_86C5:
	ld hl,$8E2D	; $86C5

loc_1F_86C8:
	add hl,bc	; $86C8
	ld c,(hl)	; $86C9
	inc hl	; $86CA
	ld b,(hl)	; $86CB
	ld a,(ix+$21)	; $86CC
	and a	; $86CF
	ret z	; $86D0
	ld h,$00	; $86D1
	jp p,loc_1F_86D7	; $86D3
	dec h	; $86D6

loc_1F_86D7:
	ld l,a	; $86D7
	add hl,bc	; $86D8
	ld c,l	; $86D9
	ld b,h	; $86DA
	ret	; $86DB

loc_1F_86DC:
	ld c,a	; $86DC
	add a,(ix+$04)	; $86DD
	ld (ix+$1D),a	; $86E0
	ret	; $86E3

fn_1F_86E4:
	ld hl,$9035	; $86E4
	ld c,a	; $86E7
	ld b,$00	; $86E8
	add hl,bc	; $86EA
	ld e,(hl)	; $86EB
	inc hl	; $86EC
	ld d,(hl)	; $86ED
	ex de,hl	; $86EE
	ld c,(ix+$1E)	; $86EF
	add hl,bc	; $86F2
	ld a,(hl)	; $86F3
	and a	; $86F4
	jp p,loc_1F_8704	; $86F5
	cp $80	; $86F8
	jr nz,loc_1F_8701	; $86FA
	dec hl	; $86FC
	ld a,(hl)	; $86FD
	jp loc_1F_8708	; $86FE

loc_1F_8701:
	call fn_1F_8715	; $8701

loc_1F_8704:
	inc c	; $8704
	ld (ix+$1E),c	; $8705

loc_1F_8708:
	add a,(ix+$01)	; $8708
	sub $0F	; $870B
	jp nc,loc_1F_8711	; $870D
	sub a	; $8710

loc_1F_8711:
	ld ($DD5A),a	; $8711
	ret	; $8714

fn_1F_8715:
	cp $81	; $8715
	jp nz,loc_1F_8724	; $8717

loc_1F_871A:
	inc hl	; $871A
	ld a,c	; $871B
	ld c,(hl)	; $871C
	sbc hl,bc	; $871D
	sub c	; $871F
	ld c,a	; $8720
	dec hl	; $8721
	ld a,(hl)	; $8722
	ret	; $8723

loc_1F_8724:
	cp $82	; $8724
	jp nz,loc_1F_872F	; $8726
	sbc hl,bc	; $8729
	ld a,(hl)	; $872B
	ld c,$00	; $872C
	ret	; $872E

loc_1F_872F:
	cp $83	; $872F
	jp nz,loc_1F_8742	; $8731
	exx	; $8734
	bit 2,e	; $8735
	exx	; $8737
	jr z,loc_1F_873E	; $8738
	dec hl	; $873A
	ld a,(hl)	; $873B
	dec c	; $873C
	ret	; $873D

loc_1F_873E:
	inc c	; $873E
	inc hl	; $873F
	ld a,(hl)	; $8740
	ret	; $8741

loc_1F_8742:
	exx	; $8742
	bit 2,e	; $8743
	exx	; $8745
	jr nz,loc_1F_871A	; $8746
	inc c	; $8748
	inc c	; $8749
	inc hl	; $874A
	inc hl	; $874B
	ld a,(hl)	; $874C
	ret	; $874D

fn_1F_874E:
	ld hl,$9016	; $874E
	ld c,a	; $8751
	ld b,$00	; $8752
	add hl,bc	; $8754
	ld e,(hl)	; $8755
	inc hl	; $8756
	ld d,(hl)	; $8757
	ex de,hl	; $8758
	ld c,(ix+$1F)	; $8759
	add hl,bc	; $875C
	ld a,(hl)	; $875D
	and a	; $875E
	jp p,loc_1F_876A	; $875F
	cp $85	; $8762
	jp nc,loc_1F_876A	; $8764
	call fn_1F_8715	; $8767

loc_1F_876A:
	inc c	; $876A
	ld (ix+$1F),c	; $876B
	ld e,(ix+$03)	; $876E
	bit 7,e	; $8771
	jp z,loc_1F_8778	; $8773
	cpl	; $8776
	inc a	; $8777

loc_1F_8778:
	and a	; $8778
	ret z	; $8779
	ld b,$00	; $877A
	ld c,a	; $877C
	jp p,loc_1F_8781	; $877D
	dec b	; $8780

loc_1F_8781:
	ld l,(ix+$16)	; $8781
	ld h,(ix+$17)	; $8784
	ld a,h	; $8787
	or l	; $8788
	ret z	; $8789
	ld a,h	; $878A
	sbc hl,bc	; $878B
	bit 6,e	; $878D
	jp z,loc_1F_87A3	; $878F
	cp h	; $8792
	jr z,loc_1F_87A3	; $8793
	bit 7,b	; $8795
	jr z,loc_1F_879E	; $8797
	ld (ix+$16),$FF	; $8799
	ret	; $879D

loc_1F_879E:
	ld (ix+$16),$00	; $879E
	ret	; $87A2

loc_1F_87A3:
	ld (ix+$16),l	; $87A3
	ld (ix+$17),h	; $87A6
	ret	; $87A9

fn_1F_87AA:
	sub a	; $87AA
	ld ($DD05),a	; $87AB
	ld ($DD08),a	; $87AE
	ld ($DD0B),a	; $87B1
	ld ($DD0D),a	; $87B4
	dec a	; $87B7
	ld ($DD5B),a	; $87B8
	ld a,($DD58)	; $87BB
	and a	; $87BE
	ret p	; $87BF
	and $40	; $87C0
	xor $40	; $87C2
	srl a	; $87C4
	ld ($DD3F),a	; $87C6
	ld ($DD40),a	; $87C9
	sub a	; $87CC
	ld ($DD1B),a	; $87CD
	ld ($DD1C),a	; $87D0
	ld ($DD1D),a	; $87D3
	ld ($DD1E),a	; $87D6
	ld ($DD1F),a	; $87D9
	ld ($DD20),a	; $87DC
	ld ($DD21),a	; $87DF
	ld ($DD22),a	; $87E2
	ld ($DD23),a	; $87E5
	dec a	; $87E8
	ld ($DD2A),a	; $87E9
	ld ($DD2B),a	; $87EC
	ld ($DD2C),a	; $87EF
	ret	; $87F2

fn_1F_87F3:
	exx	; $87F3
	ld a,d	; $87F4
	exx	; $87F5
	ld b,a	; $87F6
	bit 5,a	; $87F7
	call nz,fn_1F_87AA	; $87F9
	exx	; $87FC
	ld a,e	; $87FD
	bit 1,c	; $87FE
	exx	; $8800
	ret nz	; $8801
	bit 3,a	; $8802
	jr z,loc_1F_8825	; $8804
	ld de,($DD11)	; $8806
	ld a,($DD5A)	; $880A
	cp $10	; $880D
	jr c,loc_1F_881C	; $880F
	ld a,($DD11)	; $8811
	cp $0F	; $8814
	jp c,loc_1F_8825	; $8816
	jp loc_1F_8821	; $8819

loc_1F_881C:
	add a,e	; $881C
	sub $0F	; $881D
	jr nc,loc_1F_8822	; $881F

loc_1F_8821:
	sub a	; $8821

loc_1F_8822:
	ld ($DD5A),a	; $8822

loc_1F_8825:
	ld e,(ix+$16)	; $8825
	ld d,(ix+$17)	; $8828
	ld c,(ix+$06)	; $882B
	exx	; $882E
	bit 0,e	; $882F
	exx	; $8831
	jp z,fn_1F_88C5	; $8832
	ld a,(ix+$13)	; $8835
	and a	; $8838
	ret z	; $8839
	push bc	; $883A
	ld h,(ix+$09)	; $883B
	ld a,($DD5A)	; $883E
	ld l,a	; $8841
	push hl	; $8842
	ld l,(ix+$22)	; $8843
	ld h,(ix+$23)	; $8846
	push hl	; $8849
	ld a,(hl)	; $884A
	and a	; $884B
	jp p,loc_1F_8866	; $884C
	exx	; $884F
	bit 3,e	; $8850
	exx	; $8852
	ld a,(ix+$01)	; $8853
	jp z,loc_1F_8863	; $8856
	ld hl,($DD11)	; $8859
	add a,l	; $885C
	sub $0F	; $885D
	jp nc,loc_1F_8863	; $885F
	sub a	; $8862

loc_1F_8863:
	ld ($DD5A),a	; $8863

loc_1F_8866:
	push de	; $8866
	call fn_1F_88C5	; $8867
	pop de	; $886A
	pop hl	; $886B
	pop bc	; $886C
	ld a,(hl)	; $886D
	add a,c	; $886E
	and $7F	; $886F
	sub $0F	; $8871
	jr nc,loc_1F_8876	; $8873
	sub a	; $8875

loc_1F_8876:
	ld ($DD5A),a	; $8876
	inc hl	; $8879
	ld a,(hl)	; $887A
	ld (ix+$09),a	; $887B
	inc hl	; $887E
	ld c,(hl)	; $887F
	inc hl	; $8880
	and a	; $8881
	ld a,(hl)	; $8882
	jp nz,loc_1F_88A3	; $8883
	and $F0	; $8886
	jr z,loc_1F_88AC	; $8888
	jp m,loc_1F_8898	; $888A

loc_1F_888D:
	srl d	; $888D
	rr e	; $888F
	sub $10	; $8891
	jr nz,loc_1F_888D	; $8893
	jp loc_1F_88AC	; $8895

loc_1F_8898:
	sla e	; $8898
	rl d	; $889A
	add a,$10	; $889C
	jr nz,loc_1F_8898	; $889E
	jp loc_1F_88AC	; $88A0

loc_1F_88A3:
	and $F0	; $88A3
	rrca	; $88A5
	rrca	; $88A6
	rrca	; $88A7
	add a,d	; $88A8
	and $0F	; $88A9
	ld d,a	; $88AB

loc_1F_88AC:
	ld a,b	; $88AC
	ld b,$00	; $88AD
	bit 7,c	; $88AF
	jr z,loc_1F_88B4	; $88B1
	dec b	; $88B3

loc_1F_88B4:
	ex de,hl	; $88B4
	add hl,bc	; $88B5
	ex de,hl	; $88B6
	pop bc	; $88B7
	push af	; $88B8
	ld a,(hl)	; $88B9
	and $0F	; $88BA
	ld c,a	; $88BC
	call fn_1F_88C5	; $88BD
	pop af	; $88C0
	ld (ix+$09),a	; $88C1
	ret	; $88C4

fn_1F_88C5:
	ld a,(ix+$09)	; $88C5
	and a	; $88C8
	jp z,loc_1F_89D2	; $88C9
	bit 0,b	; $88CC
	jp z,loc_1F_892A	; $88CE
	ld hl,$DD12	; $88D1
	ld b,$00	; $88D4
	add hl,bc	; $88D6
	ld c,$09	; $88D7
	ld a,d	; $88D9
	or e	; $88DA
	jr nz,loc_1F_88E6	; $88DB
	add hl,bc	; $88DD
	ld (hl),$00	; $88DE
	add hl,bc	; $88E0
	ld (hl),$1F	; $88E1
	jp loc_1F_892A	; $88E3

loc_1F_88E6:
	ld (hl),e	; $88E6
	add hl,bc	; $88E7
	ld a,(ix+$0A)	; $88E8
	and $20	; $88EB
	or d	; $88ED
	or $80	; $88EE
	exx	; $88F0
	bit 2,e	; $88F1
	exx	; $88F3
	jr z,loc_1F_88F8	; $88F4
	set 4,a	; $88F6

loc_1F_88F8:
	ld (hl),a	; $88F8
	ld a,($DD02)	; $88F9
	bit 7,a	; $88FC
	jr z,loc_1F_8906	; $88FE
	bit 6,a	; $8900
	jr z,loc_1F_8906	; $8902
	set 6,(hl)	; $8904

loc_1F_8906:
	add hl,bc	; $8906
	ld a,(ix+$09)	; $8907
	ld b,a	; $890A
	cp $10	; $890B
	jr c,loc_1F_8910	; $890D
	sub a	; $890F

loc_1F_8910:
	rlca	; $8910
	rlca	; $8911
	rlca	; $8912
	rlca	; $8913
	ld c,a	; $8914
	jr nz,loc_1F_8923	; $8915
	ld a,($DD56)	; $8917
	cp b	; $891A
	jr z,loc_1F_8923	; $891B
	ld a,b	; $891D
	set 7,a	; $891E
	ld ($DD56),a	; $8920

loc_1F_8923:
	ld a,($DD5A)	; $8923
	xor $0F	; $8926
	or c	; $8928
	ld (hl),a	; $8929

loc_1F_892A:
	exx	; $892A
	bit 1,d	; $892B
	exx	; $892D
	ret z	; $892E
	ld a,(ix+$0A)	; $892F
	and $07	; $8932
	cp $05	; $8934
	ld b,$00	; $8936
	jr c,loc_1F_895F	; $8938
	ld a,e	; $893A
	or d	; $893B
	jr nz,loc_1F_8944	; $893C
	ld ($DD0D),a	; $893E
	jp loc_1F_8959	; $8941

loc_1F_8944:
	ld a,($DD02)	; $8944
	bit 7,a	; $8947
	ld a,(ix+$1D)	; $8949
	jr nz,loc_1F_8950	; $894C
	res 3,a	; $894E

loc_1F_8950:
	ld ($DD5C),a	; $8950
	ld a,($DD5A)	; $8953
	ld ($DD0D),a	; $8956

loc_1F_8959:
	ld a,$FF	; $8959
	ld ($DD5B),a	; $895B
	ret	; $895E

loc_1F_895F:
	ld c,a	; $895F
	ld hl,$89CD	; $8960
	add hl,bc	; $8963
	ld a,e	; $8964
	or d	; $8965
	jr nz,loc_1F_897F	; $8966
	call fn_1F_8970	; $8968
	ld e,$0F	; $896B
	jp loc_1F_89A6	; $896D

fn_1F_8970:
	ld a,(hl)	; $8970
	xor $FF	; $8971
	ld b,a	; $8973
	ld a,($DD3F)	; $8974
	and b	; $8977
	ld ($DD3F),a	; $8978
	ld ($DD40),a	; $897B
	ret	; $897E

loc_1F_897F:
	exx	; $897F
	bit 2,e	; $8980
	exx	; $8982
	jr nz,loc_1F_898B	; $8983
	call fn_1F_8970	; $8985
	jp loc_1F_89A0	; $8988

loc_1F_898B:
	ld a,($DD3F)	; $898B
	or (hl)	; $898E
	ld ($DD3F),a	; $898F
	ld a,($DD02)	; $8992
	bit 7,a	; $8995
	jr nz,loc_1F_89A0	; $8997
	ld a,($DD40)	; $8999
	or (hl)	; $899C
	ld ($DD40),a	; $899D

loc_1F_89A0:
	ld a,($DD5A)	; $89A0
	xor $0F	; $89A3
	ld e,a	; $89A5

loc_1F_89A6:
	ld hl,$DD2A	; $89A6
	ld a,c	; $89A9
	cp $03	; $89AA
	jr c,loc_1F_89AF	; $89AC
	inc hl	; $89AE

loc_1F_89AF:
	cp $01	; $89AF
	jr c,loc_1F_89B5	; $89B1
	inc hl	; $89B3
	inc a	; $89B4

loc_1F_89B5:
	and $01	; $89B5
	jr nz,loc_1F_89BF	; $89B7
	ld a,(hl)	; $89B9
	and $F0	; $89BA
	or e	; $89BC
	ld (hl),a	; $89BD
	ret	; $89BE

loc_1F_89BF:
	ld a,(hl)	; $89BF
	and $0F	; $89C0
	rlc e	; $89C2
	rlc e	; $89C4
	rlc e	; $89C6
	rlc e	; $89C8
	or e	; $89CA
	ld (hl),a	; $89CB
	ret	; $89CC
	.INCBIN "bank31.bin" SKIP $09CD READ 5	; data, 5 bytes

loc_1F_89D2:
	ld a,e	; $89D2
	or d	; $89D3
	jr nz,loc_1F_89D9	; $89D4
	ld ($DD5A),a	; $89D6

loc_1F_89D9:
	bit 1,b	; $89D9
	jp z,loc_1F_8A01	; $89DB
	ld a,($DD02)	; $89DE
	bit 7,a	; $89E1
	ld a,(ix+$1D)	; $89E3
	jr nz,loc_1F_89EA	; $89E6
	res 3,a	; $89E8

loc_1F_89EA:
	ld ($DD5C),a	; $89EA
	ld a,($DD5A)	; $89ED
	ld ($DD0D),a	; $89F0
	ld a,c	; $89F3
	ld ($DD5B),a	; $89F4
	cp $02	; $89F7
	jr z,loc_1F_8A1C	; $89F9
	bit 0,b	; $89FB
	ret z	; $89FD
	jp loc_1F_8A24	; $89FE

loc_1F_8A01:
	ld a,($DD5B)	; $8A01
	cp c	; $8A04
	jr nz,loc_1F_8A1C	; $8A05
	cp $02	; $8A07
	jr nz,loc_1F_8A1C	; $8A09
	ld a,($DD5C)	; $8A0B
	and $03	; $8A0E
	cp $03	; $8A10
	jr nz,loc_1F_8A1C	; $8A12
	sub a	; $8A14
	ld ($DD0D),a	; $8A15
	dec a	; $8A18
	ld ($DD5B),a	; $8A19

loc_1F_8A1C:
	bit 0,b	; $8A1C
	jr nz,loc_1F_8A24	; $8A1E
	sub a	; $8A20
	ld ($DD5A),a	; $8A21

loc_1F_8A24:
	ld a,c	; $8A24
	cp $03	; $8A25
	ret z	; $8A27
	add a,a	; $8A28
	add a,c	; $8A29
	ld c,a	; $8A2A
	ld b,$00	; $8A2B
	ld hl,$DD03	; $8A2D
	add hl,bc	; $8A30
	ld (hl),e	; $8A31
	inc hl	; $8A32
	sla e	; $8A33
	rl d	; $8A35
	sla e	; $8A37
	rl d	; $8A39
	sla e	; $8A3B
	rl d	; $8A3D
	sla e	; $8A3F
	rl d	; $8A41
	ld (hl),d	; $8A43
	inc hl	; $8A44
	ld a,($DD5A)	; $8A45
	ld (hl),a	; $8A48
	ret	; $8A49

loc_1F_8A4A:
	ld ($DD58),a	; $8A4A
	and a	; $8A4D
	jp z,loc_1F_8AA6	; $8A4E
	ld b,$09	; $8A51
	bit 6,a	; $8A53
	call z,fn_1F_8A5A	; $8A55
	jr loc_1F_8A87	; $8A58

fn_1F_8A5A:
	ld hl,$8F31	; $8A5A
	ld bc,$06F0	; $8A5D

loc_1F_8A60:
	ld a,(hl)	; $8A60
	inc hl	; $8A61
	out (c),a	; $8A62
	inc c	; $8A64
	ld a,(hl)	; $8A65
	out (c),a	; $8A66
	inc hl	; $8A68
	dec c	; $8A69
	call fn_1F_8292	; $8A6A
	djnz loc_1F_8A60	; $8A6D
	ld bc,$0336	; $8A6F
	ld hl,$DD53	; $8A72

loc_1F_8A75:
	ld a,c	; $8A75
	out ($F0),a	; $8A76
	ld a,$FF	; $8A78
	out ($F1),a	; $8A7A
	ld (hl),a	; $8A7C
	inc c	; $8A7D
	inc hl	; $8A7E
	call fn_1F_8292	; $8A7F
	djnz loc_1F_8A75	; $8A82
	ld b,$06	; $8A84
	ret	; $8A86

loc_1F_8A87:
	call fn_1F_8A8C	; $8A87
	jr loc_1F_8AA6	; $8A8A

fn_1F_8A8C:
	ld c,$10	; $8A8C
	ld de,$DD41	; $8A8E
	ld hl,$DD4A	; $8A91

loc_1F_8A94:
	ld a,c	; $8A94
	out ($F0),a	; $8A95
	sub a	; $8A97
	out ($F1),a	; $8A98
	ld (de),a	; $8A9A
	ld (hl),$FF	; $8A9B
	inc de	; $8A9D
	inc hl	; $8A9E
	inc c	; $8A9F
	call fn_1F_8294	; $8AA0
	djnz loc_1F_8A94	; $8AA3
	ret	; $8AA5

loc_1F_8AA6:
	sub a	; $8AA6
	ld ($DD56),a	; $8AA7
	ld a,($DD58)	; $8AAA
	and a	; $8AAD
	ld a,$00	; $8AAE
	jp p,loc_1F_8AB4	; $8AB0
	inc a	; $8AB3

loc_1F_8AB4:
	out ($F2),a	; $8AB4
	sub a	; $8AB6
	ld ($DD02),a	; $8AB7
	dec a	; $8ABA
	ld ($DD0C),a	; $8ABB

loc_1F_8ABE:
	push ix	; $8ABE
	ld ix,$DD5D	; $8AC0
	ld de,$0028	; $8AC4
	ld b,$08	; $8AC7
	sub a	; $8AC9

loc_1F_8ACA:
	ld (ix+$00),a	; $8ACA
	add ix,de	; $8ACD
	djnz loc_1F_8ACA	; $8ACF
	pop ix	; $8AD1
	ld a,($DD02)	; $8AD3
	res 5,a	; $8AD6
	ld ($DD02),a	; $8AD8
	ld a,$0F	; $8ADB
	ld ($DD11),a	; $8ADD
	di	; $8AE0
	call fn_1F_87AA	; $8AE1
	call fn_1F_814B	; $8AE4
	ld a,($DD58)	; $8AE7
	and a	; $8AEA
	call m,fn_1F_824C	; $8AEB
	ei	; $8AEE
	ld a,($DD58)	; $8AEF
	ret	; $8AF2

loc_1F_8AF3:
	push bc	; $8AF3
	push de	; $8AF4
	push hl	; $8AF5
	push ix	; $8AF6
	di	; $8AF8
	call fn_1F_8B64	; $8AF9
	ei	; $8AFC
	pop ix	; $8AFD
	pop hl	; $8AFF
	pop de	; $8B00
	pop bc	; $8B01
	ret	; $8B02

loc_1F_8B03:
	di	; $8B03
	push de	; $8B04
	push hl	; $8B05
	push ix	; $8B06
	ld ($DD0E),a	; $8B08
	sub a	; $8B0B
	ld ($DD0F),a	; $8B0C
	ld ix,$DD5D	; $8B0F

loc_1F_8B13:
	ld e,a	; $8B13
	ld a,(ix+$00)	; $8B14
	and a	; $8B17
	ld a,e	; $8B18
	jr z,loc_1F_8B22	; $8B19
	or (ix+$0B)	; $8B1B
	set 3,(ix+$0B)	; $8B1E

loc_1F_8B22:
	ld de,$0028	; $8B22
	add ix,de	; $8B25
	djnz loc_1F_8B13	; $8B27
	pop ix	; $8B29
	push af	; $8B2B
	ld hl,$DD02	; $8B2C
	bit 7,c	; $8B2F
	res 7,c	; $8B31
	jr nz,loc_1F_8B3F	; $8B33
	res 4,(hl)	; $8B35
	ld a,c	; $8B37
	ld ($DD10),a	; $8B38
	ld c,$00	; $8B3B
	jr loc_1F_8B47	; $8B3D

loc_1F_8B3F:
	set 4,(hl)	; $8B3F
	ld a,c	; $8B41
	ld ($DD10),a	; $8B42
	ld c,$0F	; $8B45

loc_1F_8B47:
	pop af	; $8B47
	bit 5,(hl)	; $8B48
	set 5,(hl)	; $8B4A
	jr z,loc_1F_8B52	; $8B4C
	bit 3,a	; $8B4E
	jr nz,loc_1F_8B56	; $8B50

loc_1F_8B52:
	ld a,c	; $8B52
	ld ($DD11),a	; $8B53

loc_1F_8B56:
	pop hl	; $8B56
	pop de	; $8B57
	ei	; $8B58
	ret	; $8B59

loc_1F_8B5A:
	ld a,$03	; $8B5A

loc_1F_8B5C:
	ld ($DD02),a	; $8B5C
	ret	; $8B5F

loc_1F_8B60:
	ld a,$01	; $8B60
	jr loc_1F_8B5C	; $8B62

fn_1F_8B64:
	ld c,a	; $8B64
	add a,a	; $8B65
	ld e,a	; $8B66
	ld d,$00	; $8B67
	ld hl,$8FD6	; $8B69
	add hl,de	; $8B6C
	ld e,(hl)	; $8B6D
	inc hl	; $8B6E
	ld d,(hl)	; $8B6F
	ex de,hl	; $8B70
	ld a,(hl)	; $8B71
	inc hl	; $8B72
	and a	; $8B73
	jr nz,loc_1F_8BA4	; $8B74
	jp (hl)	; $8B76
	.INCBIN "bank31.bin" SKIP $0B77 READ 45	; data, 45 bytes

loc_1F_8BA4:
	ld b,a	; $8BA4
	jp p,loc_1F_8BC0	; $8BA5
	ld a,($DD58)	; $8BA8
	and a	; $8BAB
	jp p,loc_1F_8BB3	; $8BAC
	inc hl	; $8BAF
	jp loc_1F_8BB8	; $8BB0

loc_1F_8BB3:
	ld a,(hl)	; $8BB3
	inc hl	; $8BB4
	ld h,(hl)	; $8BB5
	ld l,a	; $8BB6
	ld b,(hl)	; $8BB7

loc_1F_8BB8:
	inc hl	; $8BB8
	ld a,b	; $8BB9
	and $7F	; $8BBA
	jp nz,loc_1F_8BC0	; $8BBC
	jp (hl)	; $8BBF

loc_1F_8BC0:
	ld a,(hl)	; $8BC0
	inc hl	; $8BC1
	push hl	; $8BC2
	add a,a	; $8BC3
	ld e,a	; $8BC4
	ld d,$00	; $8BC5
	ld hl,$8C53	; $8BC7
	add hl,de	; $8BCA
	ld e,(hl)	; $8BCB
	inc hl	; $8BCC
	ld d,(hl)	; $8BCD
	pop hl	; $8BCE
	ld a,(hl)	; $8BCF
	and a	; $8BD0
	jr z,loc_1F_8C49	; $8BD1
	ld a,(de)	; $8BD3
	bit 6,a	; $8BD4
	ret nz	; $8BD6
	push de	; $8BD7
	pop ix	; $8BD8
	ld (ix+$0C),$00	; $8BDA
	and a	; $8BDE
	jr z,loc_1F_8BEE	; $8BDF
	ld a,c	; $8BE1
	cp (ix+$14)	; $8BE2
	jr nz,loc_1F_8BEE	; $8BE5
	set 2,(ix+$0C)	; $8BE7
	jp loc_1F_8BF1	; $8BEB

loc_1F_8BEE:
	ld (ix+$14),c	; $8BEE

loc_1F_8BF1:
	ld a,($DD58)	; $8BF1
	and a	; $8BF4
	jp m,loc_1F_8C05	; $8BF5

loc_1F_8BF8:
	ld (ix+$09),$00	; $8BF8
	ld (ix+$0A),$00	; $8BFC
	ld c,$09	; $8C00
	jp loc_1F_8C0B	; $8C02

loc_1F_8C05:
	bit 7,b	; $8C05
	jr z,loc_1F_8BF8	; $8C07
	ld c,$0B	; $8C09

loc_1F_8C0B:
	ld a,b	; $8C0B
	ld b,$00	; $8C0C
	ldir	; $8C0E
	ld b,a	; $8C10
	ld c,(ix+$14)	; $8C11
	ld a,($C001)	; $8C14
	or a	; $8C17
	ld a,(ix+$05)	; $8C18
	jr nz,loc_1F_8C2A	; $8C1B
	push hl	; $8C1D
	add a,$6F	; $8C1E
	ld l,a	; $8C20
	adc a,$8C	; $8C21
	sub l	; $8C23
	ld h,a	; $8C24
	ld a,(hl)	; $8C25
	pop hl	; $8C26
	ld (ix+$05),a	; $8C27

loc_1F_8C2A:
	neg	; $8C2A
	ld (ix+$0F),a	; $8C2C
	sub a	; $8C2F
	ld (ix+$0B),a	; $8C30
	ld (ix+$0D),a	; $8C33
	ld (ix+$15),a	; $8C36
	ld (ix+$21),a	; $8C39
	ld (ix+$13),a	; $8C3C
	ld (ix+$1D),$04	; $8C3F
	inc a	; $8C43
	ld (ix+$0E),a	; $8C44
	jr loc_1F_8C4B	; $8C47

loc_1F_8C49:
	ld (de),a	; $8C49
	inc hl	; $8C4A

loc_1F_8C4B:
	dec b	; $8C4B
	ld a,b	; $8C4C
	and $7F	; $8C4D
	jp nz,loc_1F_8BC0	; $8C4F
	ret	; $8C52
	.INCBIN "bank31.bin" SKIP $0C53 READ 716	; data, 716 bytes

loc_1F_8F1F:
	ld b,$07	; $8F1F
	ld a,$20	; $8F21
	ld c,$80	; $8F23
	jp loc_1F_8B03	; $8F25

loc_1F_8F28:
	ld b,$07	; $8F28
	ld a,$20	; $8F2A
	ld c,$0F	; $8F2C
	jp loc_1F_8B03	; $8F2E
	.INCBIN "bank31.bin" SKIP $0F31 READ 12495	; data, 12495 bytes
