; Bank 3 (ROM $0C000-$0FFFF), mapped at $8000
.BANK 3 SLOT 2
.ORG $0000


bank03_vectors:
	jp loc_03_8042	; $8000

fn_03_8003:
	jp loc_03_818B	; $8003

fn_03_8006:
	jp loc_03_81A0	; $8006

fn_03_8009:
	jp loc_03_81DD	; $8009

fn_03_800C:
	jp loc_03_8B70	; $800C

fn_03_800F:
	jp loc_03_8BAE	; $800F

fn_03_8012:
	jp loc_03_8BCB	; $8012

fn_03_8015:
	jp loc_03_8BE0	; $8015

fn_03_8018:
	jp loc_03_9A72	; $8018

fn_03_801B:
	jp loc_03_9CFE	; $801B

fn_03_801E:
	jp loc_03_AD03	; $801E

fn_03_8021:
	jp loc_03_9D0A	; $8021

fn_03_8024:
	jp loc_03_AE57	; $8024

fn_03_8027:
	jp loc_03_B109	; $8027

fn_03_802A:
	jp loc_03_B115	; $802A

fn_03_802D:
	jp loc_03_B9E0	; $802D

fn_03_8030:
	jp loc_03_BA4C	; $8030

fn_03_8033:
	jp loc_03_BA0B	; $8033

fn_03_8036:
	jp loc_03_B9FA	; $8036

fn_03_8039:
	jp loc_03_BA3B	; $8039

fn_03_803C:
	jp loc_03_BA2A	; $803C

fn_03_803F:
	jp loc_03_B9CF	; $803F

loc_03_8042:
	call fn_01_7E87	; $8042
	ld a,($C005)	; $8045
	or a	; $8048
	jr z,loc_03_8051	; $8049
	ld a,$02	; $804B
	ld (game_mode),a	; $804D
	ret	; $8050

loc_03_8051:
	call fn_00_0265	; $8051
	ld a,$13	; $8054

fn_03_8056:
	ld hl,$9EB3	; $8056
	ld de,$0000	; $8059
	call fn_00_0749	; $805C
	ld hl,$8161	; $805F
	ld de,$3A96	; $8062
	xor a	; $8065
	call fn_00_028B	; $8066
	call fn_00_0201	; $8069
	ld de,$812F	; $806C
	call spawn_task	; $806F
	ld de,$807B	; $8072
	call spawn_task	; $8075
	jp fn_01_7E5A	; $8078

sub_03_807B:
	ld hl,$8141	; $807B
	ld bc,$0102	; $807E
	call fn_00_0A22	; $8081
	ld bc,$000A	; $8084
	call fn_00_01A9	; $8087
	call fn_00_0201	; $808A
	ld (ix+$1A),$09	; $808D
	ld (ix+$1B),$F7	; $8091
	ld (ix+$16),$0B	; $8095

loc_03_8099:
	ld l,(ix+$1A)	; $8099
	ld h,(ix+$1B)	; $809C
	ld ($D000),hl	; $809F
	ld hl,$D000	; $80A2
	ld de,$D002	; $80A5
	ld bc,$00BE	; $80A8
	ldir	; $80AB
	call fn_00_01F1	; $80AD
	ld a,$02	; $80B0
	call sleep	; $80B2
	dec (ix+$1A)	; $80B5
	inc (ix+$1B)	; $80B8
	dec (ix+$16)	; $80BB
	jr nz,loc_03_8099	; $80BE
	call fn_00_01E3	; $80C0
	ld hl,$8151	; $80C3
	ld bc,$0101	; $80C6
	call fn_00_0A22	; $80C9
	call wait_flag	; $80CC
	ld hl,$813B	; $80CF
	ld bc,$0601	; $80D2

loc_03_80D5:
	push bc	; $80D5
	push hl	; $80D6
	ld de,$80FE	; $80D7
	call spawn_task	; $80DA
	pop hl	; $80DD
	pop bc	; $80DE
	ld a,(hl)	; $80DF
	ld (iy+$07),a	; $80E0
	ld (iy+$01),c	; $80E3
	inc hl	; $80E6
	inc c	; $80E7
	djnz loc_03_80D5	; $80E8
	ld a,$5A	; $80EA
	call sleep	; $80EC
	ld bc,$8102	; $80EF
	call fn_00_0A22	; $80F2
	call wait_flag	; $80F5
	ld a,$02	; $80F8
	ld (game_mode),a	; $80FA
	ret	; $80FD

sub_03_80FE:
	ld (ix+$16),$0F	; $80FE
	ld (ix+$1A),$01	; $8102

loc_03_8106:
	ld a,(ix+$1A)	; $8106
	cp $01	; $8109
	jr z,loc_03_8113	; $810B
	ld l,a	; $810D
	ld h,$34	; $810E
	call set_palette_entry	; $8110

loc_03_8113:
	inc (ix+$1A)	; $8113
	ld a,(ix+$1A)	; $8116
	cp $10	; $8119
	jr z,loc_03_8124	; $811B
	ld l,a	; $811D
	ld h,(ix+$07)	; $811E
	call set_palette_entry	; $8121

loc_03_8124:
	call yield	; $8124
	dec (ix+$16)	; $8127
	jr nz,loc_03_8106	; $812A
	jp end_task	; $812C
	.INCBIN "bank03.bin" SKIP $012F READ 92	; data, 92 bytes

loc_03_818B:
	call fn_01_7E87	; $818B
	call fn_00_0277	; $818E
	call fn_00_0281	; $8191
	call fn_00_0265	; $8194
	ld de,$8220	; $8197
	call spawn_task	; $819A
	jp fn_01_7E5A	; $819D

loc_03_81A0:
	call fn_01_7E87	; $81A0
	call fn_00_0277	; $81A3
	call fn_00_0281	; $81A6
	call fn_00_0265	; $81A9
	ld a,$15	; $81AC
	ld hl,$80BE	; $81AE
	ld de,$1D20	; $81B1
	ld bc,$0140	; $81B4
	call fn_00_07F4	; $81B7
	call fn_01_7EED	; $81BA
	ld a,$14	; $81BD
	ld hl,$8000	; $81BF
	ld de,$CA00	; $81C2
	ld bc,$0380	; $81C5
	call fn_00_07EC	; $81C8
	call fn_00_0458	; $81CB
	ld hl,$9C18	; $81CE
	call fn_00_081F	; $81D1
	ld de,$8305	; $81D4
	call spawn_task	; $81D7
	jp fn_01_7E5A	; $81DA

loc_03_81DD:
	call fn_00_014D	; $81DD
	call fn_00_06CD	; $81E0
	call clear_tasks	; $81E3
	call fn_00_01CB	; $81E6
	call fn_01_7EED	; $81E9
	call fn_01_7EF3	; $81EC
	ld hl,$8000	; $81EF
	ld a,$11	; $81F2
	ld de,$0000	; $81F4
	call fn_00_0749	; $81F7
	ld a,$11	; $81FA
	ld de,$CB42	; $81FC
	call fn_00_071A	; $81FF
	ld de,$CA00	; $8202
	ld bc,$1801	; $8205
	call fn_01_7E0C	; $8208
	ld hl,$8A6D	; $820B
	call fn_01_7F0B	; $820E
	call fn_00_0458	; $8211
	call fn_00_043D	; $8214
	ld de,$894D	; $8217
	call spawn_task	; $821A
	jp fn_01_7E5A	; $821D
	call fn_01_7EED	; $8220
	ld hl,$8262	; $8223
	call fn_01_7EFF	; $8226
	ld hl,$826C	; $8229
	ld b,$09	; $822C
	call fn_01_7E51	; $822E
	call fn_00_0458	; $8231
	ld hl,$9C18	; $8234
	ld bc,$0301	; $8237
	call fn_00_0A22	; $823A
	call wait_flag	; $823D
	ld (ix+$16),$78	; $8240

loc_03_8244:
	call yield	; $8244
	ld a,(joy_pressed)	; $8247
	and $30	; $824A
	jr nz,loc_03_8253	; $824C
	dec (ix+$16)	; $824E
	jr nz,loc_03_8244	; $8251

loc_03_8253:
	ld bc,$8301	; $8253
	call fn_00_0A22	; $8256
	call wait_flag	; $8259
	ld a,$03	; $825C
	ld (game_mode),a	; $825E
	ret	; $8261
	.INCBIN "bank03.bin" SKIP $0262 READ 163	; data, 163 bytes
	ld a,$15	; $8305
	ld hl,$833E	; $8307
	ld de,$CD80	; $830A
	ld bc,$0280	; $830D
	call fn_00_07EC	; $8310
	ld a,$15	; $8313
	ld hl,$8000	; $8315
	ld de,$D800	; $8318
	ld bc,$00BE	; $831B
	call fn_00_07EC	; $831E
	call fn_03_8691	; $8321
	call fn_01_7E42	; $8324
	ld (ix+$1A),$A0	; $8327
	ld (ix+$1B),$A0	; $832B
	ld (ix+$16),$10	; $832F

loc_03_8333:
	ld d,$CD	; $8333
	ld e,(ix+$1A)	; $8335
	ld bc,$0A01	; $8338
	call queue_text_copy	; $833B
	inc e	; $833E
	inc e	; $833F
	ld (ix+$1A),e	; $8340
	ld e,(ix+$1B)	; $8343
	ld bc,$0A01	; $8346
	call queue_text_copy	; $8349
	dec e	; $834C
	dec e	; $834D
	ld (ix+$1B),e	; $834E
	call yield	; $8351
	dec (ix+$16)	; $8354
	jr nz,loc_03_8333	; $8357
	ld (ix+$08),$14	; $8359
	ld (ix+$12),$80	; $835D
	ld (ix+$13),$83	; $8361
	ld (ix+$16),$02	; $8365

loc_03_8369:
	call fn_00_0C19	; $8369
	or a	; $836C
	jr z,loc_03_8396	; $836D
	ld (ix+$0B),a	; $836F
	call fn_00_0C19	; $8372
	ld (ix+$0D),a	; $8375

loc_03_8378:
	ld l,(ix+$0B)	; $8378
	ld h,(ix+$0D)	; $837B
	ld a,$01	; $837E
	call fn_03_8B16	; $8380
	dec (ix+$16)	; $8383
	jr nz,loc_03_838F	; $8386
	ld (ix+$16),$02	; $8388
	call fn_03_859F	; $838C

loc_03_838F:
	call fn_00_0BEE	; $838F
	jr nc,loc_03_8378	; $8392
	jr loc_03_8369	; $8394

loc_03_8396:
	ld a,$14	; $8396
	ld hl,$8500	; $8398
	ld de,$21C0	; $839B
	ld bc,$0060	; $839E
	call fn_00_07F4	; $83A1
	call fn_03_859F	; $83A4
	ld a,$14	; $83A7
	ld hl,$8560	; $83A9
	ld de,$3E40	; $83AC
	ld bc,$0080	; $83AF
	call fn_00_07F4	; $83B2
	ld (ix+$1A),$0E	; $83B5
	ld (ix+$1B),$39	; $83B9
	ld (ix+$16),$12	; $83BD

loc_03_83C1:
	ld (ix+$1C),$F2	; $83C1
	ld (ix+$1D),$09	; $83C5
	ld (ix+$17),$04	; $83C9

loc_03_83CD:
	call fn_03_85B8	; $83CD
	call fn_03_859F	; $83D0
	dec (ix+$17)	; $83D3
	jr nz,loc_03_83CD	; $83D6
	inc (ix+$1A)	; $83D8
	inc (ix+$1A)	; $83DB
	dec (ix+$16)	; $83DE
	jr nz,loc_03_83C1	; $83E1
	ld (ix+$1C),$F2	; $83E3
	ld (ix+$1D),$09	; $83E7
	call fn_03_85B8	; $83EB
	call fn_03_859F	; $83EE
	call fn_03_85B8	; $83F1
	call fn_03_859F	; $83F4
	ld de,$23C0	; $83F7
	ld b,$06	; $83FA

loc_03_83FC:
	push bc	; $83FC
	push de	; $83FD
	ld a,$14	; $83FE
	ld hl,$85C0	; $8400
	ld bc,$0020	; $8403
	call fn_00_07F4	; $8406
	pop hl	; $8409
	ld bc,$0020	; $840A
	add hl,bc	; $840D
	ex de,hl	; $840E
	pop bc	; $840F
	djnz loc_03_83FC	; $8410
	ld hl,$85CE	; $8412
	ld de,$391A	; $8415
	ld bc,$000E	; $8418
	call fn_00_024C	; $841B
	call fn_03_859F	; $841E
	ld (ix+$1A),$3C	; $8421
	ld (ix+$1B),$22	; $8425
	ld (ix+$1C),$04	; $8429
	ld (ix+$16),$08	; $842D

loc_03_8431:
	ld hl,$85E0	; $8431
	ld e,(ix+$1A)	; $8434
	ld d,(ix+$1B)	; $8437
	ld b,$0D	; $843A
	call fn_03_85DC	; $843C
	call fn_03_859F	; $843F
	call fn_03_859F	; $8442
	call fn_03_859F	; $8445
	dec (ix+$16)	; $8448
	jp nz,loc_03_8431	; $844B
	ld (ix+$1A),$90	; $844E
	ld (ix+$1B),$87	; $8452
	ld (ix+$1C),$04	; $8456
	ld (ix+$16),$05	; $845A

loc_03_845E:
	ld l,(ix+$1A)	; $845E
	ld h,(ix+$1B)	; $8461
	ld de,$23CC	; $8464
	ld b,$06	; $8467
	call fn_03_85DC	; $8469
	call fn_03_859F	; $846C
	call fn_03_859F	; $846F
	call fn_03_859F	; $8472
	dec (ix+$16)	; $8475
	jp nz,loc_03_845E	; $8478
	ld (ix+$1A),$00	; $847B
	ld (ix+$16),$40	; $847F

loc_03_8483:
	bit 0,(ix+$06)	; $8483
	jr z,loc_03_8491	; $8487
	ld (ix+$1A),$3F	; $8489
	ld (ix+$16),$01	; $848D

loc_03_8491:
	ld a,(ix+$1A)	; $8491
	call fn_01_7F3E	; $8494
	call fn_03_859F	; $8497
	inc (ix+$1A)	; $849A
	dec (ix+$16)	; $849D
	jr nz,loc_03_8483	; $84A0
	ld a,$01	; $84A2
	ld ($C00C),a	; $84A4
	ld a,$01	; $84A7
	call play_music	; $84A9
	ld c,$40	; $84AC
	ld de,$87AD	; $84AE
	call spawn_task_c	; $84B1
	xor a	; $84B4
	ld ($C650),a	; $84B5
	ld hl,$FFFF	; $84B8
	ld ($C64E),hl	; $84BB
	ld hl,location_people	; $84BE
	ld de,$C652	; $84C1
	ld bc,$00FF	; $84C4
	ld (hl),$00	; $84C7
	ldir	; $84C9

loc_03_84CB:
	ld (ix+$16),$02	; $84CB

loc_03_84CF:
	ld a,(joy_pressed)	; $84CF
	and $30	; $84D2
	jr nz,loc_03_84FE	; $84D4
	call fn_01_7E3F	; $84D6
	call yield	; $84D9
	dec (ix+$16)	; $84DC
	jr nz,loc_03_84CF	; $84DF
	call fn_03_869D	; $84E1
	call fn_03_86E3	; $84E4
	ld hl,($C64E)	; $84E7
	inc hl	; $84EA
	ld a,h	; $84EB
	or l	; $84EC
	jr nz,loc_03_84CB	; $84ED
	ld bc,$8301	; $84EF
	call fn_00_0A22	; $84F2
	call wait_flag	; $84F5
	ld a,$04	; $84F8
	ld (game_mode),a	; $84FA
	ret	; $84FD

loc_03_84FE:
	call fn_03_8A97	; $84FE
	ld hl,$0030	; $8501
	ld ($C608),hl	; $8504
	ld hl,$0100	; $8507
	ld ($C60A),hl	; $850A
	ld hl,$8610	; $850D
	call fn_01_7E4E	; $8510
	ld hl,$864C	; $8513
	ld b,$04	; $8516
	call fn_01_7E51	; $8518
	ld (ix+$1A),$00	; $851B

loc_03_851F:
	ld a,(ix+$1A)	; $851F
	ld hl,$860D	; $8522
	rst $28	; $8525
	ld h,a	; $8526
	ld l,$34	; $8527
	ld ($C604),hl	; $8529
	call fn_01_7F5F	; $852C
	call yield	; $852F
	ld a,(joy_pressed)	; $8532
	bit 0,a	; $8535
	jr z,loc_03_853E	; $8537
	dec (ix+$1A)	; $8539
	jr loc_03_8545	; $853C

loc_03_853E:
	bit 1,a	; $853E
	jr z,loc_03_8557	; $8540
	inc (ix+$1A)	; $8542

loc_03_8545:
	call fn_01_7F14	; $8545
	ld a,(ix+$1A)	; $8548
	ld h,$03	; $854B
	add a,h	; $854D
	ld l,a	; $854E
	call fn_01_7E3C	; $854F
	ld (ix+$1A),h	; $8552
	jr loc_03_851F	; $8555

loc_03_8557:
	and $30	; $8557
	jr z,loc_03_851F	; $8559
	call fn_01_7F17	; $855B
	ld a,(ix+$1A)	; $855E
	cp $02	; $8561
	jr c,loc_03_856E	; $8563
	call yield	; $8565
	call fn_01_7E42	; $8568
	jp loc_03_84CB	; $856B

loc_03_856E:
	call fn_00_0605	; $856E
	ld bc,$8201	; $8571
	call fn_00_0A22	; $8574
	call wait_flag	; $8577
	ld hl,$CA00	; $857A
	ld de,$CA01	; $857D
	ld bc,$037F	; $8580
	ld (hl),$00	; $8583
	ldir	; $8585
	ld hl,$CA00	; $8587
	ld de,$3800	; $858A
	ld bc,$0380	; $858D
	call fn_00_0463	; $8590
	call wait_vblank	; $8593
	ld a,(ix+$1A)	; $8596
	add a,$05	; $8599
	ld (game_mode),a	; $859B
	ret	; $859E

fn_03_859F:
	bit 0,(ix+$06)	; $859F
	ret nz	; $85A3
	ld a,(joy_pressed)	; $85A4
	and $30	; $85A7
	jr z,loc_03_85B0	; $85A9
	set 0,(ix+$06)	; $85AB
	ret	; $85AF

loc_03_85B0:
	pop hl	; $85B0
	ld (ix+$02),l	; $85B1
	ld (ix+$03),h	; $85B4
	ret	; $85B7

fn_03_85B8:
	ld e,(ix+$1A)	; $85B8
	ld d,(ix+$1B)	; $85BB
	ld a,(ix+$1C)	; $85BE
	inc (ix+$1C)	; $85C1
	call fn_00_0210	; $85C4
	inc e	; $85C7
	ld a,(ix+$1D)	; $85C8
	jp fn_00_0210	; $85CB
	.INCBIN "bank03.bin" SKIP $05CE READ 14	; data, 14 bytes

fn_03_85DC:
	ld c,(ix+$1C)	; $85DC

loc_03_85DF:
	push bc	; $85DF

fn_03_85E0:
	push hl	; $85E0
	push de	; $85E1
	ld b,$00	; $85E2
	ld a,$14	; $85E4
	call fn_00_07F4	; $85E6
	ld bc,$0020	; $85E9
	pop hl	; $85EC
	add hl,bc	; $85ED
	ex de,hl	; $85EE
	pop hl	; $85EF
	add hl,bc	; $85F0
	pop bc	; $85F1
	djnz loc_03_85DF	; $85F2
	ld l,(ix+$1A)	; $85F4
	ld h,(ix+$1B)	; $85F7
	ld de,$FFFC	; $85FA
	add hl,de	; $85FD
	ld (ix+$1A),l	; $85FE
	ld (ix+$1B),h	; $8601
	ld a,(ix+$1C)	; $8604
	add a,$04	; $8607
	ld (ix+$1C),a	; $8609
	ret	; $860C
	.INCBIN "bank03.bin" SKIP $060D READ 132	; data, 132 bytes

fn_03_8691:
	ld hl,$D800	; $8691
	ld de,$D900	; $8694
	ld bc,$00BE	; $8697
	ldir	; $869A
	ret	; $869C

fn_03_869D:
	ld iy,location_people	; $869D
	ld de,$0008	; $86A1
	ld b,$20	; $86A4

loc_03_86A6:
	ld a,(iy+$03)	; $86A6
	cp $84	; $86A9
	jr c,loc_03_86B1	; $86AB
	cp $8F	; $86AD
	jr c,loc_03_86C1	; $86AF

loc_03_86B1:
	cp $20	; $86B1
	jr c,loc_03_86B9	; $86B3
	cp $2F	; $86B5
	jr c,loc_03_86C1	; $86B7

loc_03_86B9:
	cp $50	; $86B9
	jr c,loc_03_86C6	; $86BB
	cp $5F	; $86BD
	jr nc,loc_03_86C6	; $86BF

loc_03_86C1:
	ld hl,$01FE	; $86C1
	jr loc_03_86CD	; $86C4

loc_03_86C6:
	cp $90	; $86C6
	jr c,loc_03_86DE	; $86C8
	ld hl,$03FC	; $86CA

loc_03_86CD:
	call fn_01_7ED8	; $86CD
	cp $C0	; $86D0
	jr nc,loc_03_86DE	; $86D2
	and h	; $86D4
	ld h,a	; $86D5
	ld a,(iy+$00)	; $86D6
	and l	; $86D9
	or h	; $86DA
	ld (iy+$00),a	; $86DB

loc_03_86DE:
	add iy,de	; $86DE
	djnz loc_03_86A6	; $86E0
	ret	; $86E2

fn_03_86E3:
	ld a,($C650)	; $86E3
	or a	; $86E6
	jr z,loc_03_86EE	; $86E7
	dec a	; $86E9
	ld ($C650),a	; $86EA
	ret	; $86ED

loc_03_86EE:
	call fn_03_8788	; $86EE
	cp $70	; $86F1
	jp c,loc_03_8707	; $86F3
	cp $80	; $86F6
	jp c,loc_03_8742	; $86F8
	cp $90	; $86FB
	jp c,loc_03_8752	; $86FD
	ld hl,$FFFF	; $8700
	ld ($C64E),hl	; $8703
	ret	; $8706

loc_03_8707:
	ld b,a	; $8707
	and $0F	; $8708
	call fn_01_7E93	; $870A
	ld a,b	; $870D
	call fn_03_879D	; $870E
	ld c,a	; $8711
	ld a,b	; $8712
	rrca	; $8713
	rrca	; $8714
	rrca	; $8715
	rrca	; $8716
	and $0F	; $8717
	ld (iy+$02),a	; $8719
	call fn_03_8788	; $871C
	ld d,a	; $871F
	and $1F	; $8720
	ld (iy+$01),a	; $8722
	ld a,d	; $8725
	rlca	; $8726
	rlca	; $8727
	rlca	; $8728
	and $07	; $8729
	add a,c	; $872B
	ld (iy+$00),a	; $872C
	ld e,a	; $872F
	ld a,b	; $8730
	and $0F	; $8731
	dec a	; $8733
	jr nz,loc_03_873C	; $8734
	ld (iy+$03),c	; $8736
	jp loc_03_86EE	; $8739

loc_03_873C:
	ld (iy+$03),e	; $873C
	jp loc_03_86EE	; $873F

loc_03_8742:
	and $0F	; $8742
	call fn_01_7E93	; $8744
	ld (iy+$00),$00	; $8747
	ld (iy+$03),$00	; $874B
	jp loc_03_86EE	; $874F

loc_03_8752:
	and $0F	; $8752
	ld ($C650),a	; $8754
	call fn_03_8691	; $8757
	ld iy,location_people	; $875A
	ld de,$0008	; $875E
	ld b,$20	; $8761

loc_03_8763:
	ld a,(iy+$00)	; $8763
	or a	; $8766
	jr z,loc_03_8783	; $8767
	ld c,a	; $8769
	ld a,(iy+$02)	; $876A
	add a,a	; $876D
	ld l,a	; $876E
	add a,a	; $876F
	add a,a	; $8770
	ld h,a	; $8771
	add a,a	; $8772
	add a,a	; $8773
	add a,h	; $8774
	sub l	; $8775
	ld l,a	; $8776
	ld a,(iy+$01)	; $8777
	add a,a	; $877A
	add a,l	; $877B
	ld l,a	; $877C
	ld h,$D9	; $877D
	ld (hl),c	; $877F
	inc l	; $8780
	ld (hl),$00	; $8781

loc_03_8783:
	add iy,de	; $8783
	djnz loc_03_8763	; $8785
	ret	; $8787

fn_03_8788:
	push de	; $8788
	push hl	; $8789
	ld hl,($C64E)	; $878A
	inc hl	; $878D
	ld ($C64E),hl	; $878E
	ld de,$929D	; $8791
	add hl,de	; $8794
	ld a,$15	; $8795
	call fn_00_0815	; $8797
	pop hl	; $879A
	pop de	; $879B
	ret	; $879C

fn_03_879D:
	push bc	; $879D
	push hl	; $879E
	and $0F	; $879F
	ld c,a	; $87A1
	ld a,$15	; $87A2
	ld hl,$928E	; $87A4
	call fn_00_07FD	; $87A7
	pop hl	; $87AA
	pop bc	; $87AB
	ret	; $87AC

loc_03_87AD:
	ld a,(ix+$1A)	; $87AD
	ld hl,$889D	; $87B0
	rst $28	; $87B3
	ld de,$2480	; $87B4
	call fn_01_7EE7	; $87B7
	call fn_03_883D	; $87BA
	call yield	; $87BD
	ld a,(ix+$1A)	; $87C0
	ld hl,$88D5	; $87C3
	rst $28	; $87C6
	ld de,$2600	; $87C7
	call fn_01_7EE7	; $87CA
	call fn_03_883D	; $87CD
	call yield	; $87D0
	ld a,(ix+$1A)	; $87D3
	ld hl,$890D	; $87D6
	rst $28	; $87D9
	inc a	; $87DA
	jr z,loc_03_87F4	; $87DB
	dec a	; $87DD
	ld h,a	; $87DE
	ld l,$00	; $87DF
	srl h	; $87E1
	rr l	; $87E3
	ld de,$AC88	; $87E5
	add hl,de	; $87E8
	ld de,$2780	; $87E9
	ld bc,$0080	; $87EC
	ld a,$14	; $87EF
	call fn_00_07F4	; $87F1

loc_03_87F4:
	call fn_03_883D	; $87F4
	call yield	; $87F7
	call fn_03_883D	; $87FA
	call yield	; $87FD
	inc (ix+$1A)	; $8800
	ld a,(ix+$1A)	; $8803
	cp $38	; $8806
	jr c,loc_03_880E	; $8808
	ld (ix+$1A),$00	; $880A

loc_03_880E:
	bit 0,(ix+$06)	; $880E
	jr nz,loc_03_87AD	; $8812
	inc (ix+$0D)	; $8814
	ld a,(ix+$0D)	; $8817
	cp $18	; $881A
	jr c,loc_03_87AD	; $881C
	ld hl,$8945	; $881E
	ld de,$3872	; $8821
	ld bc,$0004	; $8824
	call fn_00_024C	; $8827
	ld hl,$8949	; $882A
	ld de,$38B2	; $882D
	ld bc,$0004	; $8830
	call fn_00_024C	; $8833
	set 0,(ix+$06)	; $8836
	jp loc_03_87AD	; $883A

fn_03_883D:
	ld d,(ix+$0D)	; $883D
	ld e,$08	; $8840
	ld hl,$8853	; $8842
	call fn_00_09C7	; $8845
	ld d,(ix+$0D)	; $8848
	ld e,$D8	; $884B
	ld hl,$8878	; $884D
	jp fn_00_09C7	; $8850
	.INCBIN "bank03.bin" SKIP $0853 READ 148	; data, 148 bytes

fn_03_88E7:
	.INCBIN "bank03.bin" SKIP $08E7 READ 102	; data, 102 bytes

sub_03_894D:
	ld de,$8A61	; $894D
	call spawn_task	; $8950
	ld hl,$9C38	; $8953
	ld bc,$0303	; $8956
	call fn_00_0A22	; $8959
	ld de,$89A4	; $895C
	call spawn_task	; $895F
	ld (iy+$1A),$04	; $8962
	ld (iy+$1B),$FF	; $8966
	ld (ix+$16),$0A	; $896A

loc_03_896E:
	ld (ix+$17),$3C	; $896E

loc_03_8972:
	call yield	; $8972
	dec (ix+$17)	; $8975
	jr nz,loc_03_8972	; $8978
	dec (ix+$16)	; $897A
	jr nz,loc_03_896E	; $897D
	ld de,$89A4	; $897F
	call spawn_task	; $8982
	ld (iy+$1A),$00	; $8985
	ld (iy+$1B),$01	; $8989
	ld a,$32	; $898D
	call sleep	; $898F
	call fn_00_0605	; $8992
	ld bc,$8303	; $8995
	call fn_00_0A22	; $8998
	call wait_flag	; $899B
	ld a,$01	; $899E
	ld (game_mode),a	; $89A0
	ret	; $89A3

sub_03_89A4:
	call fn_03_8A03	; $89A4
	ld bc,$000A	; $89A7
	call fn_00_01A9	; $89AA
	call fn_00_0201	; $89AD
	ld (ix+$16),$05	; $89B0

loc_03_89B4:
	ld (ix+$1C),$18	; $89B4
	ld (ix+$17),$10	; $89B8

loc_03_89BC:
	ld a,(ix+$1A)	; $89BC
	add a,a	; $89BF
	add a,a	; $89C0
	add a,a	; $89C1
	add a,a	; $89C2
	ld e,a	; $89C3
	ld d,$00	; $89C4
	ld hl,$8A11	; $89C6
	add hl,de	; $89C9
	ld e,(ix+$1C)	; $89CA
	ld d,$D0	; $89CD
	ld a,$08	; $89CF

loc_03_89D1:
	push hl	; $89D1
	ld bc,$0010	; $89D2
	ldir	; $89D5
	pop hl	; $89D7
	dec a	; $89D8
	jr nz,loc_03_89D1	; $89D9
	call fn_00_01F1	; $89DB
	call yield	; $89DE
	inc (ix+$1C)	; $89E1
	dec (ix+$17)	; $89E4
	jr nz,loc_03_89BC	; $89E7
	ld a,(ix+$1A)	; $89E9
	add a,(ix+$1B)	; $89EC
	ld (ix+$1A),a	; $89EF
	dec (ix+$16)	; $89F2
	jr nz,loc_03_89B4	; $89F5
	call fn_03_8A03	; $89F7
	call yield	; $89FA
	call fn_00_01E3	; $89FD
	jp end_task	; $8A00

fn_03_8A03:
	ld hl,$D000	; $8A03
	ld de,$D001	; $8A06
	ld bc,$00BF	; $8A09
	ld (hl),$00	; $8A0C
	ldir	; $8A0E
	ret	; $8A10
	.INCBIN "bank03.bin" SKIP $0A11 READ 30	; data, 30 bytes

loc_03_8A2F:
	rst $38	; $8A2F
	rst $38	; $8A30
	nop	; $8A31
	ld bc,$0302	; $8A32
	inc bc	; $8A35
	inc bc	; $8A36
	ld (bc),a	; $8A37
	ld bc,$FF00	; $8A38
	cp $FD	; $8A3B
	.INCBIN "bank03.bin" SKIP $0A3D READ 11	; data, 11 bytes

fn_03_8A48:
	ld (bc),a	; $8A48
	nop	; $8A49
	cp $FD	; $8A4A
	call m,$FCFC	; $8A4C
	.INCBIN "bank03.bin" SKIP $0A4F READ 22	; data, 22 bytes

fn_03_8A65:
	jr nc,loc_03_8A2F	; $8A65
	ld a,$03	; $8A67
	ld (game_mode),a	; $8A69
	ret	; $8A6C
	.INCBIN "bank03.bin" SKIP $0A6D READ 21	; data, 21 bytes

fn_03_8A82:
	dec d	; $8A82
	ld l,c	; $8A83
	ld l,(hl)	; $8A84
	jr nz,$61	; $8A85
	jr nz,$74	; $8A87
	ld l,c	; $8A89
	ld l,l	; $8A8A
	ld h,l	; $8A8B
	jr nz,$74	; $8A8C
	ld l,a	; $8A8E
	jr nz,$63	; $8A8F
	ld l,a	; $8A91
	ld l,l	; $8A92
	ld h,l	; $8A93
	ld l,$00	; $8A94
	rst $38	; $8A96

fn_03_8A97:
	call task_enter_1	; $8A97
	ld a,$15	; $8A9A
	ld hl,$833E	; $8A9C

fn_03_8A9F:
	ld de,$CD80	; $8A9F
	ld bc,$0280	; $8AA2
	call fn_00_07EC	; $8AA5
	ld hl,$CD80	; $8AA8
	ld de,$3B80	; $8AAB
	ld bc,$0280	; $8AAE
	call fn_00_0463	; $8AB1
	call yield	; $8AB4
	ld hl,$0100	; $8AB7
	ld de,$0030	; $8ABA
	ld b,$1C	; $8ABD

loc_03_8ABF:
	push bc	; $8ABF
	ld bc,$00F0	; $8AC0
	xor a	; $8AC3
	call fn_00_0219	; $8AC4
	ex de,hl	; $8AC7
	add hl,de	; $8AC8
	ex de,hl	; $8AC9
	pop bc	; $8ACA
	djnz loc_03_8ABF	; $8ACB
	ld b,$05	; $8ACD

loc_03_8ACF:
	push bc	; $8ACF
	push de	; $8AD0
	ld hl,$8AE6	; $8AD1
	ld bc,$0030	; $8AD4
	call fn_00_024C	; $8AD7
	pop hl	; $8ADA
	ld bc,$0030	; $8ADB
	add hl,bc	; $8ADE
	ex de,hl	; $8ADF
	pop bc	; $8AE0
	djnz loc_03_8ACF	; $8AE1
	jp task_return_1	; $8AE3
	.INCBIN "bank03.bin" SKIP $0AE6 READ 2	; data, 2 bytes

loc_03_8AE8:
	.INCBIN "bank03.bin" SKIP $0AE8 READ 12	; data, 12 bytes

loc_03_8AF4:
	.INCBIN "bank03.bin" SKIP $0AF4 READ 9	; data, 9 bytes

loc_03_8AFD:
	.INCBIN "bank03.bin" SKIP $0AFD READ 5	; data, 5 bytes

loc_03_8B02:
	.INCBIN "bank03.bin" SKIP $0B02 READ 20	; data, 20 bytes

fn_03_8B16:
	push bc	; $8B16
	push de	; $8B17
	push hl	; $8B18
	ld e,a	; $8B19
	ld a,l	; $8B1A
	and $07	; $8B1B
	inc a	; $8B1D
	ld b,a	; $8B1E
	ld a,$01	; $8B1F

loc_03_8B21:
	rrca	; $8B21
	djnz loc_03_8B21	; $8B22
	ld d,a	; $8B24
	ld a,l	; $8B25
	and $F8	; $8B26
	rrca	; $8B28
	rrca	; $8B29
	inc a	; $8B2A
	ld c,a	; $8B2B
	ld b,$CA	; $8B2C
	ld a,h	; $8B2E
	ex af,af'	; $8B2F
	ld a,h	; $8B30
	and $F8	; $8B31
	ld l,a	; $8B33
	ld h,$00	; $8B34
	add hl,hl	; $8B36
	add hl,hl	; $8B37
	add hl,hl	; $8B38
	add hl,bc	; $8B39
	ld a,(hl)	; $8B3A
	dec l	; $8B3B
	ld l,(hl)	; $8B3C
	and $01	; $8B3D
	ld h,a	; $8B3F
	add hl,hl	; $8B40
	add hl,hl	; $8B41
	add hl,hl	; $8B42
	ex af,af'	; $8B43
	and $07	; $8B44
	ld c,a	; $8B46
	ld b,$00	; $8B47
	add hl,bc	; $8B49
	add hl,hl	; $8B4A
	add hl,hl	; $8B4B
	push hl	; $8B4C
	rst $08	; $8B4D
	ld hl,$C1C0	; $8B4E
	ld bc,$04BE	; $8B51

loc_03_8B54:
	ini	; $8B54
	jr nz,loc_03_8B54	; $8B56
	pop hl	; $8B58
	ex de,hl	; $8B59
	rst $10	; $8B5A
	ld de,$C1C0	; $8B5B
	ld b,$04	; $8B5E

loc_03_8B60:
	ld a,(de)	; $8B60
	inc e	; $8B61
	or h	; $8B62
	rr l	; $8B63
	jr c,loc_03_8B68	; $8B65
	xor h	; $8B67

loc_03_8B68:
	out ($BE),a	; $8B68
	djnz loc_03_8B60	; $8B6A
	pop hl	; $8B6C
	pop de	; $8B6D
	pop bc	; $8B6E
	ret	; $8B6F

loc_03_8B70:
	call fn_01_7E84	; $8B70
	ld hl,$8040	; $8B73
	call fn_00_0C3D	; $8B76
	ld b,a	; $8B79
	ld hl,$8080	; $8B7A
	call fn_00_0C3D	; $8B7D
	ld c,a	; $8B80
	ld hl,$80C0	; $8B81
	call fn_00_0C3D	; $8B84
	or b	; $8B87
	or c	; $8B88
	jr nz,loc_03_8B91	; $8B89
	ld a,$06	; $8B8B
	ld (game_mode),a	; $8B8D
	ret	; $8B90

loc_03_8B91:
	call fn_01_7EF0	; $8B91
	ld a,$13	; $8B94
	ld hl,$B4C9	; $8B96
	ld de,$2100	; $8B99
	call fn_00_0788	; $8B9C
	ld hl,$9C68	; $8B9F
	call fn_00_081F	; $8BA2
	ld de,$8BF8	; $8BA5
	call spawn_task	; $8BA8
	jp fn_01_7E5A	; $8BAB

loc_03_8BAE:
	call fn_01_7E84	; $8BAE
	call fn_01_7EF0	; $8BB1
	ld a,$15	; $8BB4
	ld hl,$823E	; $8BB6
	ld de,$2000	; $8BB9
	ld bc,$0100	; $8BBC
	call fn_00_07F4	; $8BBF
	ld de,$8EBD	; $8BC2
	call spawn_task	; $8BC5
	jp fn_01_7E5A	; $8BC8

loc_03_8BCB:
	call fn_01_7E87	; $8BCB
	call fn_01_7EF3	; $8BCE
	call fn_01_7EED	; $8BD1
	call fn_00_0458	; $8BD4
	ld de,$92D4	; $8BD7
	call spawn_task	; $8BDA
	jp fn_01_7E5A	; $8BDD

loc_03_8BE0:
	call fn_01_7E84	; $8BE0
	ld hl,$9A12	; $8BE3
	ld de,$2BA0	; $8BE6
	ld bc,$0060	; $8BE9
	call fn_00_024C	; $8BEC
	ld de,$945C	; $8BEF
	call spawn_task	; $8BF2
	jp fn_01_7E5A	; $8BF5

sub_03_8BF8:
	ld a,$07	; $8BF8
	call play_music	; $8BFA
	call fn_03_8A97	; $8BFD
	ld hl,$8E69	; $8C00
	call fn_01_7E4E	; $8C03
	ld hl,$8E85	; $8C06
	call fn_01_7E4E	; $8C09
	ld a,$13	; $8C0C
	ld hl,$B189	; $8C0E
	ld de,$CA00	; $8C11
	ld bc,$0340	; $8C14
	call fn_00_07EC	; $8C17
	ld hl,$8040	; $8C1A
	ld de,$CA4E	; $8C1D
	call fn_03_8DD3	; $8C20
	ld hl,$8080	; $8C23
	ld de,$CB4E	; $8C26
	call fn_03_8DD3	; $8C29
	ld hl,$80C0	; $8C2C
	ld de,$CC4E	; $8C2F
	call fn_03_8DD3	; $8C32
	ld (ix+$1C),$40	; $8C35
	ld (ix+$1D),$CD	; $8C39
	ld (ix+$1E),$00	; $8C3D
	ld (ix+$1F),$00	; $8C41
	ld (ix+$16),$0D	; $8C45

loc_03_8C49:
	ld l,(ix+$1E)	; $8C49
	ld h,(ix+$1F)	; $8C4C
	ld de,$0040	; $8C4F
	add hl,de	; $8C52
	ld (ix+$1E),l	; $8C53
	ld (ix+$1F),h	; $8C56
	ld c,l	; $8C59
	ld b,h	; $8C5A
	ld l,(ix+$1C)	; $8C5B
	ld h,(ix+$1D)	; $8C5E
	ld de,$FFC0	; $8C61
	add hl,de	; $8C64
	ld (ix+$1C),l	; $8C65
	ld (ix+$1D),h	; $8C68
	ld de,$3800	; $8C6B
	call fn_00_0463	; $8C6E
	call yield	; $8C71
	dec (ix+$16)	; $8C74
	jr nz,loc_03_8C49	; $8C77
	ld a,$DF	; $8C79
	ld ($C010),a	; $8C7B
	ld a,$03	; $8C7E
	call sleep	; $8C80
	xor a	; $8C83
	ld ($C010),a	; $8C84
	call yield	; $8C87
	ld hl,$8100	; $8C8A
	call fn_00_0C3D	; $8C8D
	ld (ix+$1A),a	; $8C90
	ld a,($C60C)	; $8C93
	add a,$10	; $8C96
	ld ($C604),a	; $8C98
	ld a,($C60D)	; $8C9B
	add a,$7C	; $8C9E
	ld ($C605),a	; $8CA0

loc_03_8CA3:
	call fn_01_7F5F	; $8CA3
	ld hl,$9C68	; $8CA6
	call fn_00_0830	; $8CA9
	ld a,(ix+$1B)	; $8CAC
	inc a	; $8CAF
	cp $18	; $8CB0
	jr c,loc_03_8CB5	; $8CB2
	xor a	; $8CB4

loc_03_8CB5:
	ld (ix+$1B),a	; $8CB5
	srl a	; $8CB8
	ld hl,$8EB1	; $8CBA
	rst $28	; $8CBD
	ld b,(ix+$1A)	; $8CBE
	dec b	; $8CC1
	jr z,loc_03_8CCF	; $8CC2
	dec b	; $8CC4
	jr z,loc_03_8CDA	; $8CC5
	ld ($C028),a	; $8CC7
	ld ($C029),a	; $8CCA
	jr loc_03_8CE0	; $8CCD

loc_03_8CCF:
	ld ($C029),a	; $8CCF
	ld ($C02A),a	; $8CD2
	ld ($C02B),a	; $8CD5
	jr loc_03_8CE0	; $8CD8

loc_03_8CDA:
	ld ($C02B),a	; $8CDA
	ld ($C02C),a	; $8CDD

loc_03_8CE0:
	call yield	; $8CE0
	ld a,(joy_pressed)	; $8CE3
	ld hl,$8CEE	; $8CE6
	call fn_01_7E18	; $8CE9
	jr loc_03_8CA3	; $8CEC
	.INCBIN "bank03.bin" SKIP $0CEE READ 16	; data, 16 bytes

sub_03_8CFE:
	ld (bc),a	; $8CFE
	ld b,$04	; $8CFF
	ld a,(ix+$1A)	; $8D01
	add a,b	; $8D04
	ld l,a	; $8D05
	ld h,$03	; $8D06
	call fn_01_7E3C	; $8D08
	ld (ix+$1A),h	; $8D0B
	call fn_01_7F14	; $8D0E
	jp loc_03_8CA3	; $8D11

sub_03_8D14:
	call fn_01_7F17	; $8D14
	ld a,(ix+$1A)	; $8D17
	ld ($C0CD),a	; $8D1A
	ld hl,$8100	; $8D1D
	call fn_00_0C43	; $8D20
	add a,$31	; $8D23
	call fn_01_7E4B	; $8D25
	ld a,(ix+$1A)	; $8D28
	ld hl,$9C88	; $8D2B
	rst $20	; $8D2E
	call fn_00_0830	; $8D2F
	call yield	; $8D32
	ld a,(ix+$1A)	; $8D35
	ld hl,$8E49	; $8D38
	rst $20	; $8D3B
	call fn_00_0C3D	; $8D3C
	or a	; $8D3F
	jr nz,loc_03_8D9C	; $8D40
	call fn_00_0605	; $8D42
	ld (ix+$1C),$00	; $8D45
	ld (ix+$1D),$CA	; $8D49
	ld (ix+$1E),$80	; $8D4D
	ld (ix+$1F),$03	; $8D51
	ld (ix+$16),$0D	; $8D55

loc_03_8D59:
	ld l,(ix+$1E)	; $8D59
	ld h,(ix+$1F)	; $8D5C
	ld de,$FFC0	; $8D5F
	add hl,de	; $8D62
	ld (ix+$1E),l	; $8D63
	ld (ix+$1F),h	; $8D66
	ld c,l	; $8D69
	ld b,h	; $8D6A
	ld l,(ix+$1C)	; $8D6B
	ld h,(ix+$1D)	; $8D6E
	ld de,$0040	; $8D71
	add hl,de	; $8D74
	ld (ix+$1C),l	; $8D75
	ld (ix+$1D),h	; $8D78
	ld de,$3800	; $8D7B
	call fn_00_0463	; $8D7E
	call yield	; $8D81
	dec (ix+$16)	; $8D84
	jr nz,loc_03_8D59	; $8D87
	ld hl,$CA00	; $8D89
	ld de,$CA01	; $8D8C
	ld bc,$037F	; $8D8F
	ld (hl),$00	; $8D92
	ldir	; $8D94
	ld a,$06	; $8D96
	ld (game_mode),a	; $8D98
	ret	; $8D9B

loc_03_8D9C:
	ld a,(ix+$1A)	; $8D9C
	push af	; $8D9F
	ld hl,$8E55	; $8DA0
	rst $20	; $8DA3
	call fn_00_0C3D	; $8DA4
	ld ($C0CE),a	; $8DA7
	pop af	; $8DAA
	ld hl,$8E4F	; $8DAB
	rst $20	; $8DAE
	ld de,$C500	; $8DAF
	ld bc,$0100	; $8DB2
	call fn_00_0C49	; $8DB5
	ld de,location_people	; $8DB8
	ld bc,$036C	; $8DBB
	call fn_00_0C49	; $8DBE
	call fn_00_0605	; $8DC1
	ld bc,$8301	; $8DC4
	call fn_00_0A22	; $8DC7
	call wait_flag	; $8DCA
	ld a,$10	; $8DCD
	ld (game_mode),a	; $8DCF
	ret	; $8DD2

fn_03_8DD3:
	push de	; $8DD3
	ld de,$C1C0	; $8DD4
	ld bc,$0040	; $8DD7
	call fn_00_0C49	; $8DDA
	pop de	; $8DDD
	ld a,($C1C0)	; $8DDE
	or a	; $8DE1
	ret z	; $8DE2
	push de	; $8DE3
	ld hl,$C1C1	; $8DE4
	call fn_01_7F08	; $8DE7
	pop de	; $8DEA
	ld hl,$001C	; $8DEB
	add hl,de	; $8DEE
	ex de,hl	; $8DEF
	push de	; $8DF0
	ld a,($C1CE)	; $8DF1
	cp $1E	; $8DF4
	ld hl,$91E3	; $8DF6
	jr z,loc_03_8DFE	; $8DF9
	ld hl,$91E8	; $8DFB

loc_03_8DFE:
	call fn_01_7F08	; $8DFE
	pop de	; $8E01
	ld hl,$0024	; $8E02
	add hl,de	; $8E05
	ex de,hl	; $8E06
	ld hl,$8E5B	; $8E07
	call fn_01_7F08	; $8E0A
	ld a,($C1CF)	; $8E0D
	add a,$30	; $8E10
	call fn_01_7EFC	; $8E12
	ld hl,$000A	; $8E15
	add hl,de	; $8E18
	ex de,hl	; $8E19
	push de	; $8E1A
	ld a,($C1D0)	; $8E1B
	add a,$4D	; $8E1E
	call fn_01_7F02	; $8E20
	call fn_01_7F08	; $8E23
	pop de	; $8E26
	ld hl,$0028	; $8E27
	add hl,de	; $8E2A
	ex de,hl	; $8E2B
	ld a,($C1D6)	; $8E2C
	call fn_01_7EFC	; $8E2F
	ld hl,$000A	; $8E32
	add hl,de	; $8E35
	ex de,hl	; $8E36
	ld hl,$C1D1	; $8E37
	call fn_01_7E09	; $8E3A
	ld hl,$C065	; $8E3D
	call fn_01_7F08	; $8E40
	ld hl,$8E62	; $8E43
	jp fn_01_7F08	; $8E46
	.INCBIN "bank03.bin" SKIP $0E49 READ 12	; data, 12 bytes

tbl_03_8E55:
	.dw fn_03_8056
	.INCBIN "bank03.bin" SKIP $0E57 READ 102	; data, 102 bytes
	ld a,$04	; $8EBD
	call play_music	; $8EBF
	ld hl,$C500	; $8EC2
	ld de,$C501	; $8EC5
	ld bc,$00FF	; $8EC8
	ld (hl),$00	; $8ECB
	ldir	; $8ECD
	ld hl,$C5E0	; $8ECF
	ld (hl),$01	; $8ED2
	inc l	; $8ED4
	ld (hl),$02	; $8ED5
	inc l	; $8ED7
	ld (hl),$03	; $8ED8
	inc l	; $8EDA
	ld (hl),$04	; $8EDB
	inc l	; $8EDD
	ld (hl),$05	; $8EDE
	inc l	; $8EE0
	ld (hl),$06	; $8EE1
	inc l	; $8EE3
	ld (hl),$07	; $8EE4
	ld hl,location_people	; $8EE6
	ld de,$C652	; $8EE9
	ld bc,$036B	; $8EEC
	ld (hl),$00	; $8EEF
	ldir	; $8EF1
	ld a,$FF	; $8EF3
	ld ($C0CD),a	; $8EF5
	ld a,$20	; $8EF8
	ld ($C0CE),a	; $8EFA
	ld hl,$C614	; $8EFD
	ld bc,$0E00	; $8F00
	call fn_01_7E57	; $8F03
	ld hl,$C615	; $8F06
	ld bc,$0C20	; $8F09
	call fn_01_7E57	; $8F0C
	ld a,$1E	; $8F0F
	ld ($C622),a	; $8F11
	ld hl,$9C78	; $8F14
	call fn_00_083B	; $8F17
	call fn_03_8A97	; $8F1A
	ld hl,$915B	; $8F1D
	call fn_01_7E4E	; $8F20
	ld hl,$9191	; $8F23
	call fn_01_7E4E	; $8F26
	ld c,$06	; $8F29
	ld de,$0101	; $8F2B
	call fn_03_9104	; $8F2E
	ld (ix+$1A),$00	; $8F31
	ld (ix+$0A),$00	; $8F35
	ld (ix+$0C),$00	; $8F39
	ld hl,$201C	; $8F3D
	call fn_00_06DA	; $8F40

loc_03_8F43:
	ld a,($C614)	; $8F43
	add a,$0B	; $8F46
	ld l,a	; $8F48
	ld h,$15	; $8F49
	ld (text_col),hl	; $8F4B
	call fn_01_7F5C	; $8F4E
	ld hl,$C615	; $8F51
	ld de,$150B	; $8F54
	call fn_01_7EB4	; $8F57
	ld e,(ix+$0A)	; $8F5A
	ld d,(ix+$0C)	; $8F5D
	ld b,$00	; $8F60
	ld a,d	; $8F62
	cp $04	; $8F63
	jr c,loc_03_8F77	; $8F65
	ld a,e	; $8F67
	cp $0B	; $8F68
	jr c,loc_03_8F77	; $8F6A
	inc b	; $8F6C
	ld e,$0B	; $8F6D
	ld hl,$9208	; $8F6F
	inc (ix+$1A)	; $8F72
	jr loc_03_8F7D	; $8F75

loc_03_8F77:
	ld hl,$91EF	; $8F77
	dec (ix+$1A)	; $8F7A

loc_03_8F7D:
	ld (ix+$07),b	; $8F7D
	ld a,e	; $8F80
	add a,a	; $8F81
	add a,a	; $8F82
	add a,a	; $8F83
	add a,a	; $8F84
	add a,$20	; $8F85
	ld e,a	; $8F87
	ld a,d	; $8F88
	add a,a	; $8F89
	add a,a	; $8F8A
	add a,a	; $8F8B
	add a,a	; $8F8C
	add a,$10	; $8F8D
	ld d,a	; $8F8F
	call fn_00_09C7	; $8F90
	call fn_03_9134	; $8F93
	call yield	; $8F96
	ld a,(joy_pressed)	; $8F99
	and $3F	; $8F9C
	ld hl,$8FA6	; $8F9E
	call fn_01_7E18	; $8FA1
	jr loc_03_8F43	; $8FA4
	.INCBIN "bank03.bin" SKIP $0FA6 READ 19	; data, 19 bytes
	dec (ix+$0C)	; $8FB9
	jp p,loc_03_8FC3	; $8FBC
	ld (ix+$0C),$04	; $8FBF

loc_03_8FC3:
	call fn_01_7F14	; $8FC3
	jp loc_03_8F43	; $8FC6
	inc (ix+$0C)	; $8FC9
	ld a,(ix+$0C)	; $8FCC
	cp $05	; $8FCF
	jr c,loc_03_8FC3	; $8FD1
	ld (ix+$0C),$00	; $8FD3
	jr loc_03_8FC3	; $8FD7
	bit 0,(ix+$07)	; $8FD9
	jr z,loc_03_8FE5	; $8FDD
	ld (ix+$0A),$0A	; $8FDF
	jr loc_03_8FC3	; $8FE3

loc_03_8FE5:
	dec (ix+$0A)	; $8FE5
	jp p,loc_03_8FC3	; $8FE8
	ld (ix+$0A),$0C	; $8FEB
	jr loc_03_8FC3	; $8FEF
	bit 0,(ix+$07)	; $8FF1
	jr nz,loc_03_9001	; $8FF5
	inc (ix+$0A)	; $8FF7
	ld a,(ix+$0A)	; $8FFA
	cp $0D	; $8FFD
	jr c,loc_03_8FC3	; $8FFF

loc_03_9001:
	ld (ix+$0A),$00	; $9001
	jr loc_03_8FC3	; $9005
	call fn_01_7F11	; $9007
	ld hl,$C614	; $900A
	ld a,(hl)	; $900D
	or a	; $900E
	jr z,loc_03_9015	; $900F
	dec (hl)	; $9011
	rst $30	; $9012
	ld (hl),$20	; $9013

loc_03_9015:
	jp loc_03_8F43	; $9015
	call fn_01_7F17	; $9018
	bit 0,(ix+$07)	; $901B
	jr nz,loc_03_9041	; $901F
	ld hl,$C614	; $9021
	ld a,(hl)	; $9024
	cp $0C	; $9025
	jr nc,loc_03_903E	; $9027
	inc (hl)	; $9029
	inc a	; $902A
	rst $30	; $902B
	ex de,hl	; $902C
	ld l,(ix+$0C)	; $902D
	ld h,$0D	; $9030
	call fn_01_7EC3	; $9032
	ld a,(ix+$0A)	; $9035
	add a,l	; $9038
	ld hl,$928F	; $9039
	rst $28	; $903C
	ld (de),a	; $903D

loc_03_903E:
	jp loc_03_8F43	; $903E

loc_03_9041:
	ld a,($C614)	; $9041
	or a	; $9044
	jp z,loc_03_8F43	; $9045
	call fn_00_06D4	; $9048
	ld c,$06	; $904B
	ld de,$05FF	; $904D
	call fn_03_9104	; $9050
	ld hl,$C614	; $9053
	ld de,$92CE	; $9056
	ld b,$06	; $9059

loc_03_905B:
	ld a,(de)	; $905B
	cp (hl)	; $905C
	jr nz,loc_03_9072	; $905D
	inc hl	; $905F
	inc de	; $9060
	djnz loc_03_905B	; $9061
	ld bc,$8301	; $9063
	call fn_00_0A22	; $9066
	call wait_flag	; $9069
	ld a,$09	; $906C
	ld (game_mode),a	; $906E
	ret	; $9071

loc_03_9072:
	call fn_03_8A97	; $9072
	ld hl,$91B9	; $9075
	call fn_01_7E4E	; $9078
	ld c,$03	; $907B
	ld de,$0801	; $907D
	call fn_03_9104	; $9080

loc_03_9083:
	ld hl,$140E	; $9083
	ld (text_col),hl	; $9086
	call fn_01_7F5C	; $9089
	ld a,($C622)	; $908C
	ld b,$00	; $908F
	cp $1E	; $9091
	jr nz,loc_03_90A0	; $9093
	dec (ix+$1A)	; $9095
	ld hl,$922D	; $9098
	ld de,$2870	; $909B
	jr loc_03_90AA	; $909E

loc_03_90A0:
	inc b	; $90A0
	inc (ix+$1A)	; $90A1
	ld hl,$9258	; $90A4
	ld de,$3868	; $90A7

loc_03_90AA:
	ld (ix+$07),b	; $90AA
	call fn_00_09C7	; $90AD
	call fn_03_9134	; $90B0
	call yield	; $90B3
	ld a,(joy_pressed)	; $90B6
	ld b,a	; $90B9
	and $03	; $90BA
	jr z,loc_03_90D0	; $90BC
	bit 0,(ix+$07)	; $90BE
	ld a,$1E	; $90C2
	jr nz,loc_03_90C8	; $90C4
	ld a,$1F	; $90C6

loc_03_90C8:
	ld ($C622),a	; $90C8
	call fn_01_7F14	; $90CB
	jr loc_03_9083	; $90CE

loc_03_90D0:
	ld a,b	; $90D0
	and $30	; $90D1
	jr z,loc_03_9083	; $90D3
	call fn_01_7F17	; $90D5
	bit 0,(ix+$07)	; $90D8
	ld hl,$91E3	; $90DC
	jr z,loc_03_90E4	; $90DF
	ld hl,$91E8	; $90E1

loc_03_90E4:
	ld de,$140E	; $90E4
	call fn_01_7EB4	; $90E7
	ld c,$03	; $90EA
	ld de,$09FF	; $90EC
	call fn_03_9104	; $90EF
	call fn_00_0605	; $90F2
	ld bc,$8301	; $90F5
	call fn_00_0A22	; $90F8
	call wait_flag	; $90FB
	ld a,$07	; $90FE
	ld (game_mode),a	; $9100
	ret	; $9103

fn_03_9104:
	ld (ix+$16),c	; $9104
	ld (ix+$1A),d	; $9107
	ld (ix+$1B),e	; $910A
	call task_enter_1	; $910D

loc_03_9110:
	ld c,(ix+$1A)	; $9110
	ld a,$15	; $9113
	ld hl,$85BE	; $9115
	call fn_00_0807	; $9118
	ld a,$15	; $911B
	call fn_01_7F77	; $911D
	call yield	; $9120
	ld a,(ix+$1A)	; $9123
	add a,(ix+$1B)	; $9126
	ld (ix+$1A),a	; $9129
	dec (ix+$16)	; $912C
	jr nz,loc_03_9110	; $912F
	jp task_return_1	; $9131

fn_03_9134:
	ld a,(ix+$1A)	; $9134
	rrca	; $9137
	and $07	; $9138
	ld hl,$914C	; $913A
	rst $30	; $913D
	ld de,$C03B	; $913E
	ld bc,$0008	; $9141
	ldir	; $9144
	ld a,$FF	; $9146
	ld ($C009),a	; $9148
	ret	; $914B
	.INCBIN "bank03.bin" SKIP $114C READ 392	; data, 392 bytes
	ld hl,$9C58	; $92D4
	call fn_00_083B	; $92D7
	call yield	; $92DA
	ld (ix+$1A),$00	; $92DD

loc_03_92E1:
	ld a,(ix+$1A)	; $92E1
	ld hl,$92FE	; $92E4
	call fn_01_7E18	; $92E7

loc_03_92EA:
	ld a,(ix+$1A)	; $92EA
	call fn_03_9903	; $92ED
	call fn_03_99FC	; $92F0
	inc (ix+$1A)	; $92F3
	jr loc_03_92E1	; $92F6
	ld a,$08	; $92F8
	ld (game_mode),a	; $92FA
	ret	; $92FD
	.INCBIN "bank03.bin" SKIP $12FE READ 34	; data, 34 bytes
	ld a,$01	; $9320
	call play_music	; $9322
	jp loc_03_92EA	; $9325
	ld a,$12	; $9328
	ld hl,$A8A9	; $932A
	jp loc_03_936F	; $932D
	ld a,$12	; $9330
	ld hl,$A8A9	; $9332
	jr loc_03_935A	; $9335
	ld a,$13	; $9337
	ld hl,$8000	; $9339
	jr loc_03_935A	; $933C
	ld a,$12	; $933E
	ld hl,$9490	; $9340
	jr loc_03_935A	; $9343
	ld a,$11	; $9345
	ld hl,$A5AB	; $9347
	jr loc_03_935A	; $934A
	ld a,$12	; $934C
	ld hl,$8000	; $934E
	jr loc_03_935A	; $9351
	ld a,$1A	; $9353
	ld hl,$AF91	; $9355
	jr loc_03_935A	; $9358

loc_03_935A:
	ld (ix+$1F),a	; $935A
	ld (ix+$20),l	; $935D
	ld (ix+$21),h	; $9360
	call fn_03_93A7	; $9363
	ld a,(ix+$1F)	; $9366
	ld l,(ix+$20)	; $9369
	ld h,(ix+$21)	; $936C

loc_03_936F:
	call fn_03_938E	; $936F
	ld hl,$D000	; $9372
	ld de,$CA0A	; $9375
	ld a,$0F	; $9378

loc_03_937A:
	ld bc,$002C	; $937A
	ldir	; $937D
	ld bc,$0014	; $937F
	ex de,hl	; $9382
	add hl,bc	; $9383
	ex de,hl	; $9384
	dec a	; $9385
	jr nz,loc_03_937A	; $9386
	call fn_03_93C3	; $9388
	jp loc_03_92EA	; $938B

fn_03_938E:
	ld de,$C043	; $938E
	ld bc,$0010	; $9391
	push af	; $9394
	call fn_00_07EC	; $9395
	pop af	; $9398
	ld de,$0000	; $9399
	push af	; $939C
	call fn_00_0788	; $939D
	pop af	; $93A0
	ld de,$D000	; $93A1
	jp fn_00_071A	; $93A4

fn_03_93A7:
	call task_enter_1	; $93A7
	ld hl,$CA00	; $93AA
	ld de,$CA01	; $93AD
	ld bc,$03BF	; $93B0
	ld (hl),$00	; $93B3
	ldir	; $93B5
	ld bc,$8101	; $93B7
	call fn_00_0A22	; $93BA
	call wait_flag	; $93BD
	jp task_return_1	; $93C0

fn_03_93C3:
	ld hl,$CA00	; $93C3
	ld de,$3800	; $93C6
	ld bc,$03C0	; $93C9
	call fn_00_0463	; $93CC
	ld hl,$C043	; $93CF
	ld bc,$0101	; $93D2
	jp fn_00_0A22	; $93D5
	ld a,$1D	; $93D8
	call fn_00_05F1	; $93DA
	ld a,$11	; $93DD
	ld hl,$BD34	; $93DF
	ld de,$2000	; $93E2
	call fn_00_0788	; $93E5
	ld de,$CC14	; $93E8
	ld bc,$0201	; $93EB
	call fn_01_7F05	; $93EE
	ld (ix+$1B),$0C	; $93F1
	ld (ix+$17),$0C	; $93F5

loc_03_93F9:
	call fn_03_9441	; $93F9
	call yield	; $93FC
	call fn_03_9441	; $93FF
	call yield	; $9402
	dec (ix+$1B)	; $9405
	dec (ix+$17)	; $9408
	jr nz,loc_03_93F9	; $940B
	ld c,$40	; $940D
	ld de,$9441	; $940F
	call spawn_task_c	; $9412
	jp loc_03_92EA	; $9415
	ld a,$1D	; $9418
	call fn_00_05F1	; $941A
	ld a,$40	; $941D
	call fn_00_0B6D	; $941F
	ld (ix+$1B),$00	; $9422
	ld (ix+$17),$0C	; $9426

loc_03_942A:
	call fn_03_9441	; $942A
	call yield	; $942D
	call fn_03_9441	; $9430
	call yield	; $9433
	inc (ix+$1B)	; $9436
	dec (ix+$17)	; $9439
	jr nz,loc_03_942A	; $943C
	jp loc_03_92EA	; $943E

fn_03_9441:
	ld de,$3850	; $9441
	ld a,$11	; $9444
	ld hl,$BD66	; $9446
	call fn_00_09BE	; $9449
	ld a,(ix+$1B)	; $944C
	add a,$30	; $944F
	ld d,a	; $9451
	ld e,$50	; $9452
	ld a,$11	; $9454
	ld hl,$BD5F	; $9456
	jp fn_00_09BE	; $9459
	call fn_03_93A7	; $945C
	ld a,$11	; $945F
	ld hl,$BBE2	; $9461
	call fn_03_938E	; $9464
	ld hl,$D000	; $9467
	ld de,$CA00	; $946A
	ld bc,$0400	; $946D
	ldir	; $9470
	ld hl,$CA00	; $9472
	ld de,$3800	; $9475
	ld bc,$0400	; $9478
	call fn_00_0463	; $947B
	ld hl,$C043	; $947E
	ld bc,$0101	; $9481
	call fn_00_0A22	; $9484
	ld a,$38	; $9487
	call fn_03_9917	; $9489
	call fn_03_99FC	; $948C
	call yield	; $948F
	ld hl,$C623	; $9492
	ld bc,$0800	; $9495
	call fn_01_7E57	; $9498
	ld bc,$0832	; $949B
	call fn_01_7E57	; $949E
	ld bc,$030F	; $94A1
	call fn_01_7E57	; $94A4
	ld (hl),$00	; $94A7

loc_03_94A9:
	ld a,($C636)	; $94A9
	or a	; $94AC
	jr z,loc_03_94F1	; $94AD
	ld hl,$0001	; $94AF
	ld de,$CA46	; $94B2
	ld bc,$0E06	; $94B5
	call fn_01_7E0F	; $94B8
	ld hl,$0001	; $94BB
	ld de,$CA6E	; $94BE
	ld bc,$0E06	; $94C1
	call fn_01_7E0F	; $94C4
	ld (ix+$1B),$46	; $94C7
	ld (ix+$1C),$CD	; $94CB
	ld (ix+$17),$07	; $94CF

loc_03_94D3:
	ld e,(ix+$1B)	; $94D3
	ld d,(ix+$1C)	; $94D6
	ld hl,$FF80	; $94D9
	add hl,de	; $94DC
	ld (ix+$1B),l	; $94DD
	ld (ix+$1C),h	; $94E0
	ld bc,$021A	; $94E3
	call queue_text_copy	; $94E6
	call yield	; $94E9
	dec (ix+$17)	; $94EC
	jr nz,loc_03_94D3	; $94EF

loc_03_94F1:
	ld a,($C636)	; $94F1
	cp $04	; $94F4
	jr z,loc_03_94FC	; $94F6
	cp $06	; $94F8
	jr nz,loc_03_950A	; $94FA

loc_03_94FC:
	ld hl,$C623	; $94FC
	ld b,$08	; $94FF

loc_03_9501:
	bit 7,(hl)	; $9501
	jr nz,loc_03_9507	; $9503
	ld (hl),$00	; $9505

loc_03_9507:
	inc hl	; $9507
	djnz loc_03_9501	; $9508

loc_03_950A:
	ld a,$08	; $950A
	call fn_01_7EDB	; $950C
	ld e,a	; $950F
	ld hl,$C623	; $9510
	rst $28	; $9513
	or a	; $9514
	jr nz,loc_03_950A	; $9515

loc_03_9517:
	ld a,$08	; $9517
	call fn_01_7EDB	; $9519
	cp e	; $951C
	jr z,loc_03_9517	; $951D
	ld d,a	; $951F
	ld hl,$C623	; $9520
	rst $28	; $9523
	or a	; $9524
	jr nz,loc_03_9517	; $9525
	ld a,e	; $9527
	cp d	; $9528
	jr c,loc_03_952D	; $9529
	ld e,d	; $952B
	ld d,a	; $952C

loc_03_952D:
	ld ($C637),de	; $952D
	ld a,($C637)	; $9531
	ld hl,$CA46	; $9534
	ld de,$0200	; $9537
	ld c,$10	; $953A
	call fn_01_7EF9	; $953C
	ld a,($C638)	; $953F
	ld hl,$CA6E	; $9542
	ld de,$0C00	; $9545
	ld c,$60	; $9548
	call fn_01_7EF9	; $954A
	call fn_03_9982	; $954D
	call yield	; $9550
	ld de,$96E7	; $9553
	call spawn_task	; $9556
	ld a,($C636)	; $9559
	ld b,$3C	; $955C
	or a	; $955E
	jr z,loc_03_9569	; $955F
	ld b,$3E	; $9561
	cp $06	; $9563
	jr z,loc_03_9569	; $9565
	ld b,$3D	; $9567

loc_03_9569:
	ld a,b	; $9569
	call fn_03_99B2	; $956A
	ld a,$3F	; $956D
	call fn_03_99B2	; $956F
	ld a,($C637)	; $9572
	add a,$40	; $9575
	call fn_03_99B2	; $9577
	ld a,$48	; $957A
	call fn_03_99B2	; $957C
	ld a,($C638)	; $957F
	add a,$40	; $9582
	call fn_03_99B2	; $9584
	ld a,$49	; $9587
	call fn_03_99B2	; $9589
	call fn_03_992B	; $958C
	call fn_03_99FC	; $958F
	ld a,($C637)	; $9592
	ld hl,$96C8	; $9595
	rst $28	; $9598
	ld hl,$C638	; $9599
	add a,(hl)	; $959C
	add a,a	; $959D
	ld (ix+$1A),a	; $959E
	call fn_03_9917	; $95A1
	call fn_03_99FC	; $95A4
	call fn_03_9982	; $95A7
	call yield	; $95AA
	ld a,$39	; $95AD
	call fn_03_99B2	; $95AF
	ld (ix+$0A),$02	; $95B2
	ld a,$3A	; $95B6
	call fn_03_99B2	; $95B8
	ld (ix+$0E),$05	; $95BB
	ld a,(ix+$1A)	; $95BF
	inc a	; $95C2
	call fn_03_99B2	; $95C3
	push hl	; $95C6
	ld (ix+$0A),$02	; $95C7
	inc (ix+$0C)	; $95CB
	ld a,(ix+$0C)	; $95CE
	ld (ix+$0D),a	; $95D1
	ld a,$3B	; $95D4
	call fn_03_99B2	; $95D6
	pop hl	; $95D9
	call fn_03_99BF	; $95DA
	call fn_03_992B	; $95DD
	ld (ix+$1A),$00	; $95E0

loc_03_95E4:
	ld hl,$1101	; $95E4
	bit 0,(ix+$1A)	; $95E7
	jr z,loc_03_95F0	; $95EB
	ld h,(ix+$0D)	; $95ED

loc_03_95F0:
	ld (text_col),hl	; $95F0
	call fn_01_7F5C	; $95F3
	call yield	; $95F6
	ld a,(joy_pressed)	; $95F9
	bit 0,a	; $95FC
	jr z,loc_03_9609	; $95FE
	call fn_01_7F14	; $9600
	ld (ix+$1A),$00	; $9603
	jr loc_03_95E4	; $9607

loc_03_9609:
	bit 1,a	; $9609
	jr z,loc_03_9616	; $960B
	call fn_01_7F14	; $960D
	ld (ix+$1A),$01	; $9610
	jr loc_03_95E4	; $9614

loc_03_9616:
	and $30	; $9616
	jr z,loc_03_95E4	; $9618
	call fn_01_7F17	; $961A
	bit 0,(ix+$1A)	; $961D
	jr z,loc_03_962C	; $9621
	ld hl,($C637)	; $9623
	ld a,h	; $9626
	ld h,l	; $9627
	ld l,a	; $9628
	ld ($C637),hl	; $9629

loc_03_962C:
	ld bc,($C637)	; $962C
	ld b,$00	; $9630
	ld de,($C638)	; $9632
	ld d,b	; $9636
	ld hl,$C623	; $9637
	add hl,bc	; $963A
	ld (hl),$01	; $963B
	ld hl,$C623	; $963D
	add hl,de	; $9640
	ld (hl),$FF	; $9641
	ld hl,$C62B	; $9643
	add hl,bc	; $9646
	ld a,(hl)	; $9647
	add a,$05	; $9648
	ld (hl),a	; $964A
	ld a,c	; $964B
	add a,a	; $964C
	add a,c	; $964D
	ld e,a	; $964E
	ld hl,$96CF	; $964F
	add hl,de	; $9652
	ld de,$C633	; $9653
	ld b,$03	; $9656

loc_03_9658:
	ld a,(de)	; $9658
	add a,(hl)	; $9659
	ld (de),a	; $965A
	inc hl	; $965B
	inc de	; $965C
	djnz loc_03_9658	; $965D
	ld a,($C637)	; $965F
	ld b,$0B	; $9662
	call fn_03_96B3	; $9664
	ld a,($C638)	; $9667
	ld b,$09	; $966A
	call fn_03_96B3	; $966C
	ld de,$CA58	; $966F
	ld bc,$0E08	; $9672
	call queue_text_copy	; $9675
	call yield	; $9678
	ld hl,$C636	; $967B
	inc (hl)	; $967E
	ld a,(hl)	; $967F
	cp $07	; $9680
	jp c,loc_03_94A9	; $9682
	ld (ix+$1A),$4A	; $9685
	ld (ix+$16),$02	; $9689

loc_03_968D:
	ld a,(ix+$1A)	; $968D
	call fn_03_9917	; $9690
	call fn_03_99FC	; $9693
	inc (ix+$1A)	; $9696
	dec (ix+$16)	; $9699
	jr nz,loc_03_968D	; $969C
	call fn_00_0605	; $969E
	ld bc,$8301	; $96A1
	call fn_00_0A22	; $96A4
	call wait_flag	; $96A7
	call fn_03_9724	; $96AA
	ld a,$10	; $96AD
	ld (game_mode),a	; $96AF
	ret	; $96B2

fn_03_96B3:
	add a,$0C	; $96B3
	ld e,a	; $96B5
	ld a,($C636)	; $96B6
	add a,a	; $96B9
	inc a	; $96BA
	ld d,a	; $96BB
	call fn_01_7EEA	; $96BC
	ex de,hl	; $96BF
	ld (hl),b	; $96C0
	ld de,$0040	; $96C1
	add hl,de	; $96C4
	inc b	; $96C5
	ld (hl),b	; $96C6
	ret	; $96C7
	.INCBIN "bank03.bin" SKIP $16C8 READ 31	; data, 31 bytes
	ld hl,$CA06	; $96E7
	call fn_03_96F6	; $96EA
	ld hl,$CA2E	; $96ED
	call fn_03_96F6	; $96F0
	jp end_task	; $96F3

fn_03_96F6:
	ld (ix+$1A),l	; $96F6
	ld (ix+$1B),h	; $96F9
	call task_enter_1	; $96FC
	ld (ix+$16),$04	; $96FF

loc_03_9703:
	ld e,(ix+$1A)	; $9703
	ld d,(ix+$1B)	; $9706
	ld hl,$0100	; $9709
	add hl,de	; $970C
	ld (ix+$1A),l	; $970D
	ld (ix+$1B),h	; $9710
	ld bc,$0406	; $9713
	call queue_text_copy	; $9716
	call yield	; $9719
	dec (ix+$16)	; $971C
	jr nz,loc_03_9703	; $971F
	jp task_return_1	; $9721

fn_03_9724:
	ld de,$C8FD	; $9724
	ld hl,$9797	; $9727
	ld bc,$0074	; $972A
	ldir	; $972D
	ld de,party	; $972F
	ld hl,$980B	; $9732
	ld bc,$00E8	; $9735
	ldir	; $9738
	ld a,($C637)	; $973A
	ld hl,$98F3	; $973D
	rst $20	; $9740
	ld (party_x),hl	; $9741
	ld hl,$C62B	; $9744
	ld de,karma	; $9747
	ld bc,$0008	; $974A
	ldir	; $974D
	ld a,($C637)	; $974F
	inc a	; $9752
	ld hl,$C734	; $9753
	ld de,$001D	; $9756

loc_03_9759:
	add hl,de	; $9759
	dec a	; $975A
	jr nz,loc_03_9759	; $975B
	ld de,party	; $975D
	ld b,$1D	; $9760
	call fn_01_7F3B	; $9762
	ld hl,party_name	; $9765
	ld bc,$0D00	; $9768
	call fn_01_7E57	; $976B
	ld hl,$C615	; $976E
	ld de,party_name	; $9771
	ld bc,($C614)	; $9774
	ld b,$00	; $9778
	ldir	; $977A
	ld a,($C622)	; $977C
	ld ($C76B),a	; $977F
	ld a,($C633)	; $9782
	ld ($C757),a	; $9785
	ld a,($C634)	; $9788
	ld ($C759),a	; $978B
	ld a,($C635)	; $978E
	ld ($C758),a	; $9791
	jp fn_01_7E1E	; $9794
	.INCBIN "bank03.bin" SKIP $1797 READ 348	; data, 348 bytes

tbl_03_98F3:
	.dw $88E7
	.dw fn_01_6953
	.INCBIN "bank03.bin" SKIP $18F7 READ 12	; data, 12 bytes

fn_03_9903:
	ld (ix+$21),a	; $9903
	call task_enter_1	; $9906
	call fn_03_9964	; $9909
	call yield	; $990C
	ld a,(ix+$21)	; $990F
	call fn_03_99AB	; $9912
	jr loc_03_992E	; $9915

fn_03_9917:
	ld (ix+$21),a	; $9917
	call task_enter_1	; $991A
	call fn_03_9982	; $991D
	call yield	; $9920
	ld a,(ix+$21)	; $9923
	call fn_03_99B2	; $9926
	jr loc_03_992E	; $9929

fn_03_992B:
	call task_enter_1	; $992B

loc_03_992E:
	ld b,(ix+$10)	; $992E
	ld d,b	; $9931
	ld e,$00	; $9932
	call fn_01_7EEA	; $9934
	ld (ix+$1B),e	; $9937
	ld (ix+$1C),d	; $993A
	ld a,$18	; $993D
	sub b	; $993F
	ld (ix+$17),a	; $9940

loc_03_9943:
	ld e,(ix+$1B)	; $9943
	ld d,(ix+$1C)	; $9946
	ld hl,$0040	; $9949
	add hl,de	; $994C
	ld (ix+$1B),l	; $994D
	ld (ix+$1C),h	; $9950
	ld bc,$0120	; $9953
	call queue_text_copy	; $9956
	call yield	; $9959
	dec (ix+$17)	; $995C
	jr nz,loc_03_9943	; $995F
	jp task_return_1	; $9961

fn_03_9964:
	ld hl,$CDC0	; $9964
	ld de,$CDC1	; $9967
	ld bc,$023F	; $996A
	ld (hl),$00	; $996D
	ldir	; $996F
	ld hl,$CDC0	; $9971
	ld de,$3BC0	; $9974
	ld bc,$0240	; $9977
	call fn_00_0463	; $997A
	ld hl,$0F01	; $997D
	jr loc_03_999E	; $9980

fn_03_9982:
	ld hl,$CE00	; $9982
	ld de,$CE01	; $9985
	ld bc,$01FF	; $9988
	ld (hl),$00	; $998B
	ldir	; $998D
	ld hl,$CE00	; $998F
	ld de,$3C00	; $9992
	ld bc,$0200	; $9995
	call fn_00_0463	; $9998
	ld hl,$1001	; $999B

loc_03_999E:
	ld (ix+$0A),l	; $999E
	ld (ix+$0C),h	; $99A1
	ld (ix+$0E),l	; $99A4
	ld (ix+$10),h	; $99A7
	ret	; $99AA

fn_03_99AB:
	ld b,$15	; $99AB
	ld hl,$A6A6	; $99AD
	jr loc_03_99B7	; $99B0

fn_03_99B2:
	ld b,$18	; $99B2
	ld hl,$9E1E	; $99B4

loc_03_99B7:
	ld c,a	; $99B7
	ld (ix+$07),b	; $99B8
	ld a,b	; $99BB
	call fn_00_0807	; $99BC

fn_03_99BF:
	ld e,(ix+$0A)	; $99BF
	ld d,(ix+$0C)	; $99C2
	call fn_01_7EEA	; $99C5

loc_03_99C8:
	ld a,(ix+$07)	; $99C8
	push hl	; $99CB
	call fn_00_0815	; $99CC
	pop hl	; $99CF
	inc hl	; $99D0
	dec a	; $99D1
	jr z,loc_03_99E4	; $99D2
	inc a	; $99D4
	jr z,loc_03_99EF	; $99D5
	add a,$40	; $99D7
	ld (de),a	; $99D9
	inc e	; $99DA
	ld a,$09	; $99DB
	ld (de),a	; $99DD
	inc e	; $99DE
	inc (ix+$0A)	; $99DF
	jr loc_03_99C8	; $99E2

loc_03_99E4:
	inc (ix+$0C)	; $99E4
	ld a,(ix+$0E)	; $99E7
	ld (ix+$0A),a	; $99EA
	jr fn_03_99BF	; $99ED

loc_03_99EF:
	ld a,(ix+$0A)	; $99EF
	ld (text_col),a	; $99F2
	ld a,(ix+$0C)	; $99F5
	ld (text_row),a	; $99F8
	ret	; $99FB

fn_03_99FC:
	call task_enter_1	; $99FC

loc_03_99FF:
	call fn_01_7F5C	; $99FF
	call yield	; $9A02
	ld a,(joy_pressed)	; $9A05
	and $30	; $9A08
	jr z,loc_03_99FF	; $9A0A
	call fn_01_7F14	; $9A0C
	jp task_return_1	; $9A0F
	.INCBIN "bank03.bin" SKIP $1A12 READ 96	; data, 96 bytes

loc_03_9A72:
	call fn_01_7E87	; $9A72
	call fn_01_7EF0	; $9A75
	call fn_01_7EED	; $9A78
	ld hl,$9B83	; $9A7B
	call fn_01_7F0B	; $9A7E
	call fn_00_0458	; $9A81
	ld de,$9A8D	; $9A84
	call spawn_task	; $9A87
	jp fn_01_7E5D	; $9A8A

sub_03_9A8D:
	ld (ix+$1A),$00	; $9A8D
	ld (ix+$1B),$01	; $9A91
	ld hl,$201C	; $9A95
	call fn_00_06DA	; $9A98
	ld hl,$9C18	; $9A9B
	ld bc,$0301	; $9A9E
	call fn_00_0A22	; $9AA1
	call wait_flag	; $9AA4

loc_03_9AA7:
	ld a,(ix+$1A)	; $9AA7
	add a,a	; $9AAA
	add a,$03	; $9AAB
	ld h,a	; $9AAD
	ld l,$0B	; $9AAE
	ld (text_col),hl	; $9AB0
	call fn_01_7F5C	; $9AB3
	call yield	; $9AB6
	ld a,(joy_pressed)	; $9AB9
	and $3F	; $9ABC
	ld hl,$9AC6	; $9ABE
	call fn_01_7E18	; $9AC1
	jr loc_03_9AA7	; $9AC4
	.INCBIN "bank03.bin" SKIP $1AC6 READ 66	; data, 66 bytes

sub_03_9B08:
	ld d,$DD	; $9B08
	ld a,(hl)	; $9B0A
	ld a,(de)	; $9B0B
	cp $0A	; $9B0C
	jp nz,loc_03_9AA7	; $9B0E
	inc (ix+$1B)	; $9B11
	ld a,(ix+$1B)	; $9B14
	cp $18	; $9B17
	jr c,loc_03_9B1F	; $9B19
	ld (ix+$1B),$01	; $9B1B

loc_03_9B1F:
	ld a,(ix+$1B)	; $9B1F
	ld hl,$C065	; $9B22
	ld b,$0A	; $9B25
	call fn_03_9B74	; $9B27
	ld b,$01	; $9B2A
	call fn_03_9B74	; $9B2C
	ld hl,$C065	; $9B2F
	ld de,$1712	; $9B32
	call fn_01_7EB4	; $9B35
	jp loc_03_9AA7	; $9B38

sub_03_9B3B:
	call fn_00_05E3	; $9B3B
	jp loc_03_9AA7	; $9B3E

sub_03_9B41:
	ld a,(ix+$1A)	; $9B41
	or a	; $9B44
	jr nz,loc_03_9B59	; $9B45
	call fn_00_0605	; $9B47
	ld bc,$8301	; $9B4A
	call fn_00_0A22	; $9B4D
	call wait_flag	; $9B50
	ld a,$02	; $9B53
	ld (game_mode),a	; $9B55
	ret	; $9B58

loc_03_9B59:
	cp $0A	; $9B59
	jr z,loc_03_9B67	; $9B5B
	ld hl,$9BF7	; $9B5D
	rst $28	; $9B60
	call play_music	; $9B61
	jp loc_03_9AA7	; $9B64

loc_03_9B67:
	ld a,(ix+$1B)	; $9B67
	ld hl,$9C00	; $9B6A
	rst $28	; $9B6D
	call fn_00_05F1	; $9B6E
	jp loc_03_9AA7	; $9B71

fn_03_9B74:
	ld c,$FF	; $9B74

loc_03_9B76:
	sub b	; $9B76
	inc c	; $9B77
	jr nc,loc_03_9B76	; $9B78
	add a,b	; $9B7A
	push af	; $9B7B
	ld a,c	; $9B7C
	add a,$30	; $9B7D
	ld (hl),a	; $9B7F
	inc l	; $9B80
	pop af	; $9B81
	ret	; $9B82
	.INCBIN "bank03.bin" SKIP $1B83 READ 261	; data, 261 bytes

tbl_03_9C88:
	.dw fn_03_9C8E
	.dw fn_03_9C9E
	.dw fn_03_9CAE

fn_03_9C8E:
	nop	; $9C8E
	nop	; $9C8F
	ccf	; $9C90
	jr nc,loc_03_9CBD	; $9C91
	ccf	; $9C93
	ccf	; $9C94
	jr nz,loc_03_9CB7	; $9C95
	jr nz,loc_03_9CCC	; $9C97
	inc sp	; $9C99
	inc sp	; $9C9A
	inc sp	; $9C9B
	inc sp	; $9C9C
	inc sp	; $9C9D

fn_03_9C9E:
	nop	; $9C9E
	nop	; $9C9F
	ccf	; $9CA0
	jr nc,$2A	; $9CA1
	jr nz,loc_03_9CE4	; $9CA3
	ccf	; $9CA5
	ccf	; $9CA6
	jr nz,loc_03_9CDC	; $9CA7
	inc sp	; $9CA9
	inc sp	; $9CAA
	inc sp	; $9CAB
	inc sp	; $9CAC
	inc sp	; $9CAD

fn_03_9CAE:
	nop	; $9CAE
	nop	; $9CAF
	ccf	; $9CB0
	jr nc,loc_03_9CDD	; $9CB1
	jr nz,loc_03_9CD5	; $9CB3
	jr nz,loc_03_9CF6	; $9CB5

loc_03_9CB7:
	ccf	; $9CB7
	inc sp	; $9CB8
	inc sp	; $9CB9
	inc sp	; $9CBA
	inc sp	; $9CBB
	inc sp	; $9CBC

loc_03_9CBD:
	inc sp	; $9CBD
	nop	; $9CBE
	nop	; $9CBF
	ccf	; $9CC0
	jr nc,loc_03_9CF8	; $9CC1
	inc a	; $9CC3
	inc sp	; $9CC4
	inc bc	; $9CC5
	rlca	; $9CC6
	inc b	; $9CC7
	ex af,af'	; $9CC8
	ld a,(bc)	; $9CC9
	inc c	; $9CCA
	inc d	; $9CCB

loc_03_9CCC:
	ld hl,($002F)	; $9CCC
	nop	; $9CCF
	ccf	; $9CD0
	jr nc,loc_03_9CD3	; $9CD1

loc_03_9CD3:
	nop	; $9CD3
	nop	; $9CD4

loc_03_9CD5:
	nop	; $9CD5
	nop	; $9CD6
	nop	; $9CD7
	nop	; $9CD8
	nop	; $9CD9
	nop	; $9CDA
	nop	; $9CDB

loc_03_9CDC:
	nop	; $9CDC

loc_03_9CDD:
	nop	; $9CDD
	nop	; $9CDE
	nop	; $9CDF
	ccf	; $9CE0
	jr nc,loc_03_9CF8	; $9CE1
	ccf	; $9CE3

loc_03_9CE4:
	nop	; $9CE4
	ld hl,($0804)	; $9CE5
	add hl,de	; $9CE8
	ld c,$0A	; $9CE9
	nop	; $9CEB
	nop	; $9CEC
	nop	; $9CED
	nop	; $9CEE
	nop	; $9CEF
	ccf	; $9CF0
	jr nc,$38	; $9CF1
	inc a	; $9CF3
	ld (hl),$12	; $9CF4

loc_03_9CF6:
	ld d,$10	; $9CF6

loc_03_9CF8:
	inc d	; $9CF8
	add hl,de	; $9CF9
	jr loc_03_9D10	; $9CFA
	.INCBIN "bank03.bin" SKIP $1CFC READ 2	; data, 2 bytes

loc_03_9CFE:
	call fn_01_7E84	; $9CFE
	ld de,$9D1E	; $9D01
	call spawn_task	; $9D04
	jp fn_01_7E5D	; $9D07

loc_03_9D0A:
	call fn_01_7E84	; $9D0A
	ld de,$9D16	; $9D0D

loc_03_9D10:
	call spawn_task	; $9D10
	jp fn_01_7E5D	; $9D13
	call fn_00_0605	; $9D16
	call fn_01_7EA5	; $9D19
	jr loc_03_9D24	; $9D1C
	call fn_00_0605	; $9D1E
	call fn_01_7EA2	; $9D21

loc_03_9D24:
	call fn_01_7F4D	; $9D24
	call fn_01_7F29	; $9D27
	call fn_01_7EAB	; $9D2A
	ld a,$02	; $9D2D
	call play_music	; $9D2F
	ld a,$FF	; $9D32
	ld ($C9BA),a	; $9D34

loc_03_9D37:
	xor a	; $9D37
	ld ($C97F),a	; $9D38

combat_member_turn:
	call fn_01_7E8A	; $9D3B
	ld a,(iy+$02)	; $9D3E
	or a	; $9D41
	jr z,loc_03_9DA0	; $9D42
	ld bc,($C97F)	; $9D44
	call fn_01_7ECF	; $9D48
	jr c,loc_03_9DA0	; $9D4B
	call fn_03_9DE3	; $9D4D
	call fn_01_7F6E	; $9D50
	ld bc,($C97F)	; $9D53
	call fn_01_7ECC	; $9D57
	ld hl,$AC4C	; $9D5A
	call fn_01_7F62	; $9D5D
	call fn_03_9DF0	; $9D60
	ld a,(iy+$0B)	; $9D63
	add a,$25	; $9D66
	call fn_01_7F6B	; $9D68
	call fn_01_7F6E	; $9D6B

combat_member_prompt:
	call fn_01_7E8A	; $9D6E
	ld l,(iy+$00)	; $9D71
	ld h,(iy+$01)	; $9D74
	ld (ix+$1E),l	; $9D77
	ld (ix+$1F),h	; $9D7A
	call fn_03_9DF0	; $9D7D
	ld a,(iy+$0B)	; $9D80
	ld (ix+$07),a	; $9D83
	ld a,$7F	; $9D86
	call fn_01_7E33	; $9D88

loc_03_9D8B:
	call fn_01_7F56	; $9D8B
	ld a,(joy_pressed)	; $9D8E
	ld hl,$A2F5	; $9D91
	call fn_01_7E18	; $9D94
	and $20	; $9D97
	jr z,loc_03_9D8B	; $9D99
	call combat_menu	; $9D9B
	jr c,combat_member_prompt	; $9D9E

loc_03_9DA0:
	call fn_03_9DFA	; $9DA0
	call fn_03_9FD8	; $9DA3
	jr nc,loc_03_9DBF	; $9DA6
	ld hl,$C97F	; $9DA8
	inc (hl)	; $9DAB
	ld a,(hl)	; $9DAC
	ld hl,party_size	; $9DAD
	cp (hl)	; $9DB0
	jp c,combat_member_turn	; $9DB1
	call fn_03_A493	; $9DB4
	call fn_03_9FD8	; $9DB7
	jr nc,loc_03_9DBF	; $9DBA
	jp loc_03_9D37	; $9DBC

loc_03_9DBF:
	call fn_01_7F4D	; $9DBF
	call yield	; $9DC2
	call fn_03_9FC8	; $9DC5
	call fn_03_9E66	; $9DC8
	call fn_00_0605	; $9DCB
	call fn_01_7EA2	; $9DCE
	ld a,($C975)	; $9DD1
	add a,$11	; $9DD4
	cp $15	; $9DD6
	jr nz,loc_03_9DDF	; $9DD8
	ld a,($C9B7)	; $9DDA
	and $7F	; $9DDD

loc_03_9DDF:
	ld (game_mode),a	; $9DDF
	ret	; $9DE2

fn_03_9DE3:
	call fn_01_7F29	; $9DE3
	ld bc,($C97F)	; $9DE6
	call fn_01_7E66	; $9DEA
	jp wait_vblank	; $9DED

fn_03_9DF0:
	push bc	; $9DF0
	ld bc,($C97F)	; $9DF1
	call fn_01_7E96	; $9DF5
	pop bc	; $9DF8
	ret	; $9DF9

fn_03_9DFA:
	call fn_01_7E8A	; $9DFA
	ld a,(iy+$02)	; $9DFD
	or a	; $9E00
	ret z	; $9E01
	ld bc,($C97F)	; $9E02
	call fn_01_7ECF	; $9E06
	call nc,fn_01_7E54	; $9E09
	ld bc,($C97F)	; $9E0C
	call fn_01_7ED2	; $9E10
	jr nc,loc_03_9E1D	; $9E13
	call fn_01_7E8A	; $9E15
	ld (iy+$02),$00	; $9E18
	ret	; $9E1C

loc_03_9E1D:
	ld bc,($C97F)	; $9E1D
	call fn_01_7ECF	; $9E21
	jr c,loc_03_9E39	; $9E24
	ld a,($C976)	; $9E26
	cp $51	; $9E29
	jr nz,loc_03_9E39	; $9E2B
	call fn_01_7ED8	; $9E2D
	and $01	; $9E30
	jr z,loc_03_9E39	; $9E32
	ld hl,$C97F	; $9E34
	dec (hl)	; $9E37
	ret	; $9E38

loc_03_9E39:
	call fn_03_9DF0	; $9E39
	ld a,(iy+$1C)	; $9E3C
	cp $53	; $9E3F
	jr nz,loc_03_9E61	; $9E41
	call fn_01_7ED8	; $9E43
	and $07	; $9E46
	jr nz,loc_03_9E61	; $9E48
	ld (iy+$1C),$47	; $9E4A
	call fn_01_7E8A	; $9E4E
	ld bc,($C97F)	; $9E51
	call fn_01_7ED5	; $9E55
	ld (iy+$02),a	; $9E58
	ld (iy+$03),a	; $9E5B
	call fn_01_7F29	; $9E5E

loc_03_9E61:
	ld a,$01	; $9E61
	jp fn_01_7F32	; $9E63

fn_03_9E66:
	ld a,(map_mode)	; $9E66
	cp $05	; $9E69
	jp nz,loc_03_9F02	; $9E6B
	call vec_print_inline	; $9E6E
	.INCBIN "bank03.bin" SKIP $1E71 READ 13	; text, 13 bytes
	ld a,($C984)	; $9E7E
	or a	; $9E81
	jp p,loc_03_9EB8	; $9E82
	ld a,($C9BA)	; $9E85
	ld hl,$9E8E	; $9E88
	jp fn_00_0018	; $9E8B

tbl_03_9E8E:
	.dw fn_03_9E96
	.dw fn_03_9E9D
	.dw fn_03_9EA4
	.dw fn_03_9EAE

fn_03_9E96:
	call fn_03_9FFD	; $9E96
	ret z	; $9E99
	jp $A006	; $9E9A

fn_03_9E9D:
	call $A000	; $9E9D
	ret z	; $9EA0
	jp $A003	; $9EA1

fn_03_9EA4:
	call $A003	; $9EA4
	call $A003	; $9EA7
	ret z	; $9EAA
	jp $A006	; $9EAB

fn_03_9EAE:
	call $A006	; $9EAE
	call $A006	; $9EB1
	ret z	; $9EB4
	jp $A003	; $9EB5

loc_03_9EB8:
	add a,a	; $9EB8
	add a,a	; $9EB9
	ld hl,$C9BA	; $9EBA
	add a,(hl)	; $9EBD
	ld b,a	; $9EBE
	add a,a	; $9EBF
	add a,a	; $9EC0
	add a,b	; $9EC1
	ld hl,$A037	; $9EC2
	rst $30	; $9EC5
	ld e,(hl)	; $9EC6
	inc hl	; $9EC7
	ld d,(hl)	; $9EC8
	inc hl	; $9EC9
	ld (party_x),de	; $9ECA
	ld e,(hl)	; $9ECE
	inc hl	; $9ECF
	ld d,(hl)	; $9ED0
	inc hl	; $9ED1
	ld (world_return_x),de	; $9ED2
	ld a,(hl)	; $9ED6
	ld hl,location	; $9ED7
	cp (hl)	; $9EDA
	ld (hl),a	; $9EDB
	ret z	; $9EDC
	call vec_print_inline	; $9EDD
	.INCBIN "bank03.bin" SKIP $1EE0 READ 14	; text, 14 bytes
	ld a,(location)	; $9EEE
	add a,$7D	; $9EF1
	call fn_01_7F6B	; $9EF3
	call fn_01_7F6E	; $9EF6
	call fn_01_7E48	; $9EF9
	ld a,$07	; $9EFC
	ld ($C970),a	; $9EFE
	ret	; $9F01

loc_03_9F02:
	ld iy,party_objects	; $9F02
	ld de,$0005	; $9F06
	ld b,$08	; $9F09

loc_03_9F0B:
	ld a,(iy+$02)	; $9F0B
	or a	; $9F0E
	jr nz,loc_03_9F47	; $9F0F
	add iy,de	; $9F11
	djnz loc_03_9F0B	; $9F13
	ld a,($C97D)	; $9F15
	call fn_01_7E75	; $9F18
	jr c,loc_03_9F2B	; $9F1B
	ld a,$02	; $9F1D
	ld c,$01	; $9F1F
	call bank01_vectors	; $9F21
	ld a,$02	; $9F24
	ld c,$03	; $9F26
	jp bank01_vectors	; $9F28

loc_03_9F2B:
	call vec_print_inline	; $9F2B
	.INCBIN "bank03.bin" SKIP $1F2E READ 18	; text, 18 bytes
	ld a,$02	; $9F40
	ld c,$02	; $9F42
	jp fn_01_7F2F	; $9F44

loc_03_9F47:
	call vec_print_inline	; $9F47
	.INCBIN "bank03.bin" SKIP $1F4A READ 11	; text, 11 bytes
	ld a,($C97D)	; $9F55
	call fn_01_7E75	; $9F58
	jr nc,loc_03_9F67	; $9F5B
	call fn_01_7ED8	; $9F5D
	and $01	; $9F60
	ld c,$02	; $9F62
	call bank01_vectors	; $9F64

loc_03_9F67:
	ld a,($C975)	; $9F67
	cp $04	; $9F6A
	ret z	; $9F6C
	ld a,($C975)	; $9F6D
	cp $01	; $9F70
	ret z	; $9F72
	ld a,($C97D)	; $9F73
	cp $80	; $9F76
	jp z,loc_03_BA0B	; $9F78
	ld a,($C975)	; $9F7B
	cp $02	; $9F7E
	jr nz,loc_03_9F9A	; $9F80
	ld a,($C97D)	; $9F82
	cp $94	; $9F85
	ret z	; $9F87
	cp $B4	; $9F88
	ret z	; $9F8A
	cp $A0	; $9F8B
	ret z	; $9F8D
	ld hl,($C97A)	; $9F8E
	call fn_03_A029	; $9F91
	ld a,(hl)	; $9F94
	or a	; $9F95
	ret nz	; $9F96
	ld (hl),$40	; $9F97
	ret	; $9F99

loc_03_9F9A:
	ld a,($C97D)	; $9F9A
	cp $90	; $9F9D
	jr nc,loc_03_9FAF	; $9F9F
	cp $20	; $9FA1
	jr c,loc_03_9FA9	; $9FA3
	cp $2F	; $9FA5
	jr c,loc_03_9FAF	; $9FA7

loc_03_9FA9:
	cp $50	; $9FA9
	ret c	; $9FAB
	cp $5F	; $9FAC
	ret nc	; $9FAE

loc_03_9FAF:
	cp $94	; $9FAF
	ret z	; $9FB1
	cp $A0	; $9FB2
	ret z	; $9FB4
	cp $B8	; $9FB5
	ret z	; $9FB7
	cp $B4	; $9FB8
	ret z	; $9FBA
	cp $DC	; $9FBB
	ret z	; $9FBD
	ld a,($C97C)	; $9FBE
	call fn_01_7E99	; $9FC1
	ret c	; $9FC4
	jp loc_03_BA4C	; $9FC5

fn_03_9FC8:
	ld a,(party_size)	; $9FC8
	ld b,a	; $9FCB
	ld c,$00	; $9FCC

loc_03_9FCE:
	call fn_01_7ED2	; $9FCE
	ret nc	; $9FD1
	inc c	; $9FD2
	djnz loc_03_9FCE	; $9FD3
	jp fn_01_7EC9	; $9FD5

fn_03_9FD8:
	ld hl,$C8BB	; $9FD8
	ld de,$0005	; $9FDB
	ld b,$08	; $9FDE
	xor a	; $9FE0

loc_03_9FE1:
	or (hl)	; $9FE1
	add hl,de	; $9FE2
	djnz loc_03_9FE1	; $9FE3
	ret z	; $9FE5
	ld a,(map_mode)	; $9FE6
	cp $05	; $9FE9
	scf	; $9FEB
	ret z	; $9FEC
	ld hl,$C83E	; $9FED
	ld de,$0008	; $9FF0
	ld b,$10	; $9FF3
	xor a	; $9FF5

loc_03_9FF6:
	or (hl)	; $9FF6
	add hl,de	; $9FF7
	djnz loc_03_9FF6	; $9FF8
	ret z	; $9FFA
	scf	; $9FFB
	ret	; $9FFC

fn_03_9FFD:
	ld a,$00	; $9FFD
	ld bc,$013E	; $9FFF
	ld bc,$023E	; $A002
	ld bc,$033E	; $A005
	call fn_01_7E39	; $A008
	ld a,(party_x)	; $A00B
	add a,l	; $A00E
	and $0F	; $A00F
	ld l,a	; $A011
	ld a,(party_y)	; $A012
	add a,h	; $A015
	and $0F	; $A016
	ld h,a	; $A018
	ld (party_x),hl	; $A019
	call fn_03_A022	; $A01C
	cp $C0	; $A01F
	ret	; $A021

fn_03_A022:
	push hl	; $A022
	call fn_03_A029	; $A023
	ld a,(hl)	; $A026
	pop hl	; $A027
	ret	; $A028

fn_03_A029:
	ld a,h	; $A029
	add a,a	; $A02A
	add a,a	; $A02B
	add a,a	; $A02C
	add a,a	; $A02D
	or l	; $A02E
	ld l,a	; $A02F
	ld a,($C970)	; $A030
	add a,$D0	; $A033
	ld h,a	; $A035
	ret	; $A036
	.INCBIN "bank03.bin" SKIP $2037 READ 60	; data, 60 bytes

combat_menu:
	call task_enter_1	; $A073
	call fn_01_7F17	; $A076
	ld hl,$AC70	; $A079
	call fn_01_7F62	; $A07C
	ld hl,$0B0A	; $A07F
	call fn_00_06DA	; $A082
	ld hl,$ACBD	; $A085
	call fn_01_7E60	; $A088
	ld (ix+$1A),$00	; $A08B

loc_03_A08F:
	call fn_01_7F56	; $A08F
	ld c,$06	; $A092
	ld de,$0102	; $A094
	call fn_01_7EB1	; $A097
	jr z,loc_03_A08F	; $A09A
	jr c,loc_03_A0C0	; $A09C
	ld a,(ix+$1A)	; $A09E
	ld hl,$ACB1	; $A0A1
	rst $20	; $A0A4
	call fn_01_7F62	; $A0A5
	call fn_01_7F6E	; $A0A8
	ld a,(ix+$1A)	; $A0AB
	ld hl,$A0B4	; $A0AE
	jp fn_00_0018	; $A0B1

tbl_03_A0B4:
	.dw combat_attack
	.dw combat_cast_spell
	.dw combat_get_chest
	.dw combat_ready_weapon
	.dw combat_use_item
	.dw combat_status

loc_03_A0C0:
	ld hl,$AC79	; $A0C0
	call fn_01_7F62	; $A0C3

loc_03_A0C6:
	scf	; $A0C6
	jr loc_03_A0CD	; $A0C7

loc_03_A0C9:
	call fn_01_7F6E	; $A0C9

loc_03_A0CC:
	or a	; $A0CC

loc_03_A0CD:
	push af	; $A0CD
	call fn_00_06D4	; $A0CE
	call fn_03_9DE3	; $A0D1
	pop af	; $A0D4
	jp task_return_1	; $A0D5

combat_attack:
	call fn_03_A24F	; $A0D8
	jp c,loc_03_A0C6	; $A0DB
	call fn_01_7E06	; $A0DE
	jp c,loc_03_A0C0	; $A0E1
	ld a,(ix+$07)	; $A0E4
	ld hl,$A292	; $A0E7
	rst $28	; $A0EA
	ld (ix+$1B),a	; $A0EB
	or a	; $A0EE
	jr z,loc_03_A100	; $A0EF
	ld a,(ix+$07)	; $A0F1
	cp $02	; $A0F4
	jr nz,loc_03_A14F	; $A0F6
	ld hl,$A2A2	; $A0F8
	call target_in_pattern	; $A0FB
	jr c,loc_03_A14F	; $A0FE

loc_03_A100:
	ld a,(ix+$07)	; $A100
	cp $0A	; $A103
	ld hl,$A2A2	; $A105
	jr nz,loc_03_A10D	; $A108
	ld hl,$A2B3	; $A10A

loc_03_A10D:
	call target_in_pattern	; $A10D
	jr nc,loc_03_A120	; $A110
	ld a,$19	; $A112
	call fn_00_05F1	; $A114
	ld hl,$ACA8	; $A117
	call fn_01_7F62	; $A11A
	jp loc_03_A0C9	; $A11D

loc_03_A120:
	ld a,$06	; $A120
	call fn_01_7EF6	; $A122
	ld (ix+$09),$08	; $A125
	ld l,(ix+$20)	; $A129
	ld h,(ix+$21)	; $A12C
	call fn_03_A477	; $A12F
	ld (ix+$0B),l	; $A132
	ld (ix+$0D),h	; $A135
	ld a,$18	; $A138
	call fn_00_05F1	; $A13A
	ld (ix+$16),$05	; $A13D

loc_03_A141:
	call fn_03_A482	; $A141
	call fn_01_7F59	; $A144
	dec (ix+$16)	; $A147
	jr nz,loc_03_A141	; $A14A
	jp loc_03_A1BA	; $A14C

loc_03_A14F:
	ld a,(ix+$07)	; $A14F
	cp $02	; $A152
	jr z,loc_03_A15A	; $A154
	cp $09	; $A156
	jr nz,loc_03_A172	; $A158

loc_03_A15A:
	ld hl,weapons_owned	; $A15A
	rst $30	; $A15D
	ld a,(hl)	; $A15E
	or a	; $A15F
	jr z,loc_03_A165	; $A160
	dec (hl)	; $A162
	jr loc_03_A172	; $A163

loc_03_A165:
	call fn_03_9DF0	; $A165
	ld (iy+$0B),$00	; $A168
	ld hl,$AC9D	; $A16C
	call fn_01_7F62	; $A16F

loc_03_A172:
	ld a,(ix+$07)	; $A172
	cp $0E	; $A175
	ld a,$06	; $A177
	jr nz,loc_03_A17D	; $A179
	ld a,$09	; $A17B

loc_03_A17D:
	ld l,(ix+$1E)	; $A17D
	ld h,(ix+$1F)	; $A180
	ld e,(ix+$20)	; $A183
	ld d,(ix+$21)	; $A186
	call fn_01_7EBD	; $A189
	ld (ix+$16),$00	; $A18C

loc_03_A190:
	inc (ix+$16)	; $A190
	call fn_01_7EBA	; $A193
	call fn_01_7E24	; $A196
	jr nc,loc_03_A1B7	; $A199
	call fn_03_A460	; $A19B
	call fn_01_7EB7	; $A19E
	jr c,loc_03_A1AB	; $A1A1
	ld a,(ix+$16)	; $A1A3
	cp (ix+$1B)	; $A1A6
	jr c,loc_03_A190	; $A1A9

loc_03_A1AB:
	ld hl,$ACA8	; $A1AB
	call fn_01_7F62	; $A1AE
	call fn_03_A219	; $A1B1
	jp loc_03_A0C9	; $A1B4

loc_03_A1B7:
	ld (ix+$08),a	; $A1B7

loc_03_A1BA:
	ld a,(location)	; $A1BA
	cp $19	; $A1BD
	jr nz,loc_03_A1C8	; $A1BF
	ld a,(ix+$07)	; $A1C1
	cp $0B	; $A1C4
	jr c,loc_03_A20D	; $A1C6

loc_03_A1C8:
	call fn_03_9DF0	; $A1C8
	ld a,(iy+$07)	; $A1CB
	cp $28	; $A1CE
	jr nc,loc_03_A1DB	; $A1D0
	add a,$80	; $A1D2
	ld b,a	; $A1D4
	call fn_01_7ED8	; $A1D5
	cp b	; $A1D8
	jr nc,loc_03_A20D	; $A1D9

loc_03_A1DB:
	ld hl,$ACAD	; $A1DB
	call fn_01_7F62	; $A1DE
	call fn_01_7F1A	; $A1E1
	call fn_03_9DF0	; $A1E4
	ld a,(iy+$0B)	; $A1E7
	ld hl,$AC34	; $A1EA
	rst $28	; $A1ED
	add a,(iy+$06)	; $A1EE
	jr nc,loc_03_A1F5	; $A1F1
	ld a,$FF	; $A1F3

loc_03_A1F5:
	push af	; $A1F5
	ld a,($C97F)	; $A1F6
	ld b,a	; $A1F9
	ld c,(ix+$08)	; $A1FA
	pop af	; $A1FD
	call fn_01_7EDB	; $A1FE
	call fn_01_7E36	; $A201
	call fn_01_7F4D	; $A204
	call fn_03_A221	; $A207
	jp loc_03_A0C9	; $A20A

loc_03_A20D:
	ld hl,$ACA8	; $A20D
	call fn_01_7F62	; $A210
	call fn_03_A21C	; $A213
	jp loc_03_A0C9	; $A216

fn_03_A219:
	call fn_00_0BC9	; $A219

fn_03_A21C:
	ld a,$19	; $A21C
	call fn_00_05F1	; $A21E

fn_03_A221:
	ld a,(ix+$07)	; $A221
	cp $0B	; $A224
	jr z,loc_03_A23B	; $A226
	cp $09	; $A228
	ret nz	; $A22A
	call fn_03_A43E	; $A22B
	call fn_03_A469	; $A22E
	ld a,(hl)	; $A231
	cp $03	; $A232
	ret c	; $A234
	ld (hl),$46	; $A235
	inc l	; $A237
	ld (hl),$82	; $A238
	ret	; $A23A

loc_03_A23B:
	call task_enter_2	; $A23B

loc_03_A23E:
	call fn_00_0BC9	; $A23E
	call fn_03_A482	; $A241
	call fn_01_7F59	; $A244
	dec (ix+$16)	; $A247
	jr nz,loc_03_A23E	; $A24A
	jp task_return_2	; $A24C

fn_03_A24F:
	ld hl,$C83E	; $A24F
	ld de,$0008	; $A252
	ld b,$10	; $A255
	xor a	; $A257

loc_03_A258:
	or (hl)	; $A258
	add hl,de	; $A259
	djnz loc_03_A258	; $A25A
	ret nz	; $A25C
	call fn_01_7F11	; $A25D
	call vec_print_inline	; $A260
	.INCBIN "bank03.bin" SKIP $2263 READ 20	; text, 20 bytes
	scf	; $A277
	ret	; $A278

target_in_pattern:
	ld a,(hl)	; $A279
	inc hl	; $A27A
	cp $80	; $A27B
	scf	; $A27D
	ret z	; $A27E
	add a,(ix+$1E)	; $A27F
	cp (ix+$20)	; $A282
	jr nz,loc_03_A28F	; $A285
	ld a,(hl)	; $A287
	add a,(ix+$1F)	; $A288
	cp (ix+$21)	; $A28B
	ret z	; $A28E

loc_03_A28F:
	inc hl	; $A28F
	jr target_in_pattern	; $A290

weapon_range:
	.INCBIN "bank03.bin" SKIP $2292 READ 66	; data, 66 bytes

combat_cast_spell:
	call fn_01_7E1B	; $A2D4
	jp loc_03_A0CD	; $A2D7

combat_get_chest:
	call fn_01_7F29	; $A2DA
	call fn_01_7F20	; $A2DD
	jp loc_03_A0CC	; $A2E0

combat_ready_weapon:
	call fn_01_7EDE	; $A2E3
	jp loc_03_A0CD	; $A2E6

combat_use_item:
	call fn_01_7E2D	; $A2E9
	jp loc_03_A0CC	; $A2EC

combat_status:
	call fn_01_7E2A	; $A2EF
	jp loc_03_A0CC	; $A2F2
	.INCBIN "bank03.bin" SKIP $22F5 READ 16	; data, 16 bytes
	call fn_01_7E27	; $A305
	jp loc_03_9DA0	; $A308
	ld a,$01	; $A30B
	ld bc,$033E	; $A30D
	ld bc,$003E	; $A310
	ld bc,$023E	; $A313
	ld (entry_direction),a	; $A316
	call fn_01_7F68	; $A319
	call fn_01_7E8A	; $A31C
	ld a,(entry_direction)	; $A31F
	call fn_01_7E39	; $A322
	ld a,(iy+$00)	; $A325
	add a,l	; $A328
	ld l,a	; $A329
	cp $0B	; $A32A
	jp nc,loc_03_A3B7	; $A32C
	ld a,(iy+$01)	; $A32F
	add a,h	; $A332
	ld h,a	; $A333
	cp $0B	; $A334
	jp nc,loc_03_A3B7	; $A336
	call fn_03_A457	; $A339
	call fn_01_7E99	; $A33C
	jr nc,loc_03_A34C	; $A33F
	ld a,$20	; $A341
	call fn_01_7E33	; $A343
	call fn_01_7E7E	; $A346
	jp loc_03_9DA0	; $A349

loc_03_A34C:
	call fn_01_7F47	; $A34C
	jr nc,loc_03_A35C	; $A34F
	ld a,$20	; $A351
	call fn_01_7E33	; $A353
	call fn_01_7F1D	; $A356
	jp loc_03_9DA0	; $A359

loc_03_A35C:
	ld (iy+$00),l	; $A35C
	ld (iy+$01),h	; $A35F
	ld a,$0A	; $A362
	call fn_00_05F1	; $A364
	call fn_01_7F6E	; $A367
	ld a,(map_mode)	; $A36A
	cp $05	; $A36D
	jp nz,loc_03_9DA0	; $A36F
	ld iy,$C8E1	; $A372
	ld de,$0007	; $A376
	ld b,$04	; $A379

loc_03_A37B:
	ld a,(iy+$00)	; $A37B
	ld c,a	; $A37E
	or a	; $A37F
	jr z,loc_03_A3B0	; $A380
	ld a,l	; $A382
	cp (iy+$02)	; $A383
	jr nz,loc_03_A3B0	; $A386
	ld a,h	; $A388
	cp (iy+$01)	; $A389
	jr nz,loc_03_A3B0	; $A38C
	push hl	; $A38E
	ld l,(iy+$04)	; $A38F
	ld h,(iy+$03)	; $A392
	ld a,l	; $A395
	or h	; $A396
	jr z,loc_03_A39F	; $A397
	call fn_03_A469	; $A399
	call fn_01_7EE4	; $A39C

loc_03_A39F:
	ld l,(iy+$06)	; $A39F
	ld h,(iy+$05)	; $A3A2
	ld a,l	; $A3A5
	or h	; $A3A6
	jr z,loc_03_A3AF	; $A3A7
	call fn_03_A469	; $A3A9
	call fn_01_7EE4	; $A3AC

loc_03_A3AF:
	pop hl	; $A3AF

loc_03_A3B0:
	add iy,de	; $A3B0
	djnz loc_03_A37B	; $A3B2
	jp loc_03_9DA0	; $A3B4

loc_03_A3B7:
	ld a,(map_mode)	; $A3B7
	cp $05	; $A3BA
	jr nz,loc_03_A3DB	; $A3BC
	ld hl,$C9BA	; $A3BE
	ld a,(hl)	; $A3C1
	or a	; $A3C2
	ld a,(entry_direction)	; $A3C3
	jp m,loc_03_A3DA	; $A3C6
	cp (hl)	; $A3C9
	jr z,loc_03_A3DA	; $A3CA
	ld a,$10	; $A3CC
	call fn_00_05F1	; $A3CE
	ld hl,$AC83	; $A3D1
	call fn_01_7F62	; $A3D4
	jp loc_03_9DA0	; $A3D7

loc_03_A3DA:
	ld (hl),a	; $A3DA

loc_03_A3DB:
	call fn_01_7E8A	; $A3DB
	ld (iy+$00),$80	; $A3DE
	ld (iy+$01),$80	; $A3E2
	ld (iy+$02),$00	; $A3E6
	ld a,$1B	; $A3EA
	call fn_00_05F1	; $A3EC
	ld a,(map_mode)	; $A3EF
	cp $03	; $A3F2
	jr nz,loc_03_A424	; $A3F4
	ld a,($C97D)	; $A3F6
	call fn_01_7E75	; $A3F9
	jr nc,loc_03_A424	; $A3FC
	push iy	; $A3FE
	call fn_03_9DF0	; $A400
	ld l,(iy+$02)	; $A403
	ld h,(iy+$03)	; $A406
	ld e,(iy+$00)	; $A409
	ld d,(iy+$01)	; $A40C
	or a	; $A40F
	sbc hl,de	; $A410
	pop iy	; $A412
	jr nz,loc_03_A424	; $A414
	ld a,$02	; $A416
	ld c,$02	; $A418
	call fn_01_7F2F	; $A41A
	ld a,$02	; $A41D
	ld c,$04	; $A41F
	call fn_01_7F2F	; $A421

loc_03_A424:
	call fn_01_7F6E	; $A424
	jp loc_03_9DA0	; $A427

combat_strength_index:
	cp $80	; $A42A
	jr c,loc_03_A439	; $A42C
	rrca	; $A42E
	and $3F	; $A42F
	cp $08	; $A431
	ret c	; $A433
	srl a	; $A434
	add a,$04	; $A436
	ret	; $A438

loc_03_A439:
	and $1F	; $A439
	add a,$24	; $A43B
	ret	; $A43D

fn_03_A43E:
	ld a,(ix+$0D)	; $A43E
	add a,$06	; $A441
	ld l,a	; $A443
	ld h,$0C	; $A444
	call fn_01_7E3C	; $A446
	ld b,l	; $A449
	ld a,(ix+$0B)	; $A44A
	add a,$06	; $A44D
	ld l,a	; $A44F
	ld h,$0C	; $A450
	call fn_01_7E3C	; $A452
	ld h,b	; $A455
	ret	; $A456

fn_03_A457:
	push hl	; $A457
	call fn_03_A46F	; $A458
	ld h,$D9	; $A45B
	ld a,(hl)	; $A45D
	pop hl	; $A45E
	ret	; $A45F

fn_03_A460:
	push hl	; $A460
	call fn_03_A46F	; $A461
	ld h,$D8	; $A464
	ld a,(hl)	; $A466
	pop hl	; $A467
	ret	; $A468

fn_03_A469:
	call fn_03_A46F	; $A469
	ld h,$D8	; $A46C
	ret	; $A46E

fn_03_A46F:
	ld a,h	; $A46F
	call fn_03_B856	; $A470
	add a,l	; $A473
	add a,a	; $A474
	ld l,a	; $A475
	ret	; $A476

fn_03_A477:
	ld a,l	; $A477
	call fn_03_B860	; $A478
	ld l,a	; $A47B
	ld a,h	; $A47C
	call fn_03_B860	; $A47D
	ld h,a	; $A480
	ret	; $A481

fn_03_A482:
	ld a,(ix+$09)	; $A482
	xor $01	; $A485
	ld (ix+$09),a	; $A487

fn_03_A48A:
	ld e,(ix+$0B)	; $A48A
	ld d,(ix+$0D)	; $A48D
	jp fn_01_7F26	; $A490

fn_03_A493:
	call task_enter_2	; $A493
	call fn_01_7F29	; $A496
	call fn_01_7F4D	; $A499
	call yield	; $A49C
	ld (ix+$16),$10	; $A49F
	ld (ix+$1B),$00	; $A4A3

loc_03_A4A7:
	ld c,(ix+$1B)	; $A4A7
	call fn_01_7E8D	; $A4AA
	ld a,(iy+$05)	; $A4AD
	or a	; $A4B0
	jp z,loc_03_A560	; $A4B1
	ld a,(iy+$07)	; $A4B4
	or a	; $A4B7
	jr z,loc_03_A4C2	; $A4B8
	call fn_01_7ED8	; $A4BA
	and $07	; $A4BD
	jp nz,loc_03_A560	; $A4BF

loc_03_A4C2:
	ld (iy+$07),$00	; $A4C2
	ld a,(iy+$05)	; $A4C6
	cp $EC	; $A4C9
	jr nz,loc_03_A4DC	; $A4CB
	ld a,$4E	; $A4CD
	ld ($C976),a	; $A4CF
	ld a,$02	; $A4D2
	ld ($C977),a	; $A4D4
	call fn_01_7F29	; $A4D7
	jr loc_03_A4FE	; $A4DA

loc_03_A4DC:
	cp $DC	; $A4DC
	jr nz,loc_03_A4FE	; $A4DE
	call fn_01_7ED8	; $A4E0
	and $07	; $A4E3
	jr nz,loc_03_A4FE	; $A4E5

loc_03_A4E7:
	ld a,$0B	; $A4E7
	call fn_01_7EDB	; $A4E9
	ld l,a	; $A4EC
	ld a,$0B	; $A4ED
	call fn_01_7EDB	; $A4EF
	ld h,a	; $A4F2
	call fn_03_A9E2	; $A4F3
	jr c,loc_03_A4E7	; $A4F6
	ld (iy+$00),l	; $A4F8
	ld (iy+$01),h	; $A4FB

loc_03_A4FE:
	ld l,(iy+$00)	; $A4FE
	ld h,(iy+$01)	; $A501
	ld (ix+$1C),$FF	; $A504
	ld (ix+$1D),$FF	; $A508
	ld a,(party_size)	; $A50C
	ld b,a	; $A50F
	ld c,$00	; $A510

loc_03_A512:
	call fn_01_7ED2	; $A512
	jr c,loc_03_A53D	; $A515
	call fn_01_7E90	; $A517
	ld a,(iy+$02)	; $A51A
	or a	; $A51D
	jr z,loc_03_A53D	; $A51E
	ld a,l	; $A520
	sub (iy+$00)	; $A521
	jr nc,loc_03_A528	; $A524
	neg	; $A526

loc_03_A528:
	ld d,a	; $A528
	ld a,h	; $A529
	sub (iy+$01)	; $A52A
	jr nc,loc_03_A531	; $A52D
	neg	; $A52F

loc_03_A531:
	add a,d	; $A531
	cp (ix+$1D)	; $A532
	jr nc,loc_03_A53D	; $A535
	ld (ix+$1D),a	; $A537
	ld (ix+$1C),c	; $A53A

loc_03_A53D:
	inc c	; $A53D
	djnz loc_03_A512	; $A53E
	ld a,(ix+$1C)	; $A540
	cp $FF	; $A543
	call nz,fn_03_A5BC	; $A545
	ld c,(ix+$1B)	; $A548
	call fn_01_7E8D	; $A54B
	ld l,(iy+$00)	; $A54E
	ld h,(iy+$01)	; $A551
	call fn_03_A460	; $A554
	ld hl,$A575	; $A557
	call fn_01_7E18	; $A55A

loc_03_A55D:
	call yield	; $A55D

loc_03_A560:
	inc (ix+$1B)	; $A560
	dec (ix+$16)	; $A563
	jp nz,loc_03_A4A7	; $A566
	call fn_01_7EE1	; $A569
	call fn_01_7E81	; $A56C
	call fn_01_7F23	; $A56F
	jp task_return_2	; $A572
	.INCBIN "bank03.bin" SKIP $2575 READ 13	; data, 13 bytes

sub_03_A582:
	ld a,(iy+$05)	; $A582
	cp $E8	; $A585
	jr z,loc_03_A55D	; $A587
	cp $F0	; $A589
	jr nc,loc_03_A55D	; $A58B
	ld a,$14	; $A58D
	call fn_00_05F1	; $A58F
	ld b,$FF	; $A592
	ld c,(ix+$1B)	; $A594
	call fn_01_7ED8	; $A597
	and $7F	; $A59A
	call fn_01_7E36	; $A59C
	jr loc_03_A55D	; $A59F

sub_03_A5A1:
	ld a,(iy+$05)	; $A5A1
	call fn_01_7F4A	; $A5A4
	jr z,loc_03_A55D	; $A5A7
	call fn_01_7ED8	; $A5A9
	cp (iy+$04)	; $A5AC
	jr c,loc_03_A55D	; $A5AF
	ld a,$14	; $A5B1
	call fn_00_05F1	; $A5B3
	ld (iy+$07),$FF	; $A5B6
	jr loc_03_A55D	; $A5BA

fn_03_A5BC:
	call task_enter_3	; $A5BC
	ld c,(ix+$1C)	; $A5BF
	call fn_01_7E90	; $A5C2
	ld l,(iy+$00)	; $A5C5
	ld h,(iy+$01)	; $A5C8
	ld c,(ix+$1B)	; $A5CB
	call fn_01_7E8D	; $A5CE
	ld a,l	; $A5D1
	sub (iy+$00)	; $A5D2
	call fn_03_ABF8	; $A5D5
	ld (ix+$20),a	; $A5D8
	ld a,h	; $A5DB
	sub (iy+$01)	; $A5DC
	call fn_03_ABF8	; $A5DF
	ld (ix+$21),a	; $A5E2
	ld a,(iy+$05)	; $A5E5
	cp $AC	; $A5E8
	jr nz,loc_03_A5FB	; $A5EA
	ld (iy+$06),$3C	; $A5EC
	ld a,(ix+$1D)	; $A5F0
	cp $05	; $A5F3
	jr nc,loc_03_A625	; $A5F5
	ld (iy+$06),$AC	; $A5F7

loc_03_A5FB:
	call fn_01_7ED8	; $A5FB
	and $03	; $A5FE
	jr nz,loc_03_A625	; $A600
	ld a,(iy+$05)	; $A602
	call fn_03_AAC7	; $A605
	jr c,loc_03_A625	; $A608
	ld a,d	; $A60A
	cp $4E	; $A60B
	jr nz,loc_03_A616	; $A60D
	ld a,($C976)	; $A60F
	cp $4E	; $A612
	jr z,loc_03_A625	; $A614

loc_03_A616:
	ld b,(ix+$1B)	; $A616
	ld c,(ix+$1C)	; $A619
	call fn_03_A888	; $A61C
	call fn_01_7F4D	; $A61F
	jp loc_03_A74B	; $A622

loc_03_A625:
	ld a,(iy+$05)	; $A625
	cp $FC	; $A628
	jr z,loc_03_A630	; $A62A
	cp $B0	; $A62C
	jr nz,loc_03_A684	; $A62E

loc_03_A630:
	ld a,($C976)	; $A630
	cp $4E	; $A633
	jr z,loc_03_A684	; $A635
	call fn_01_7ED8	; $A637
	and $03	; $A63A
	jr nz,loc_03_A684	; $A63C
	call fn_01_7F4D	; $A63E
	ld a,$15	; $A641
	call fn_00_05F1	; $A643
	call fn_01_7E69	; $A646
	call fn_01_7E69	; $A649
	ld a,(party_size)	; $A64C
	ld b,a	; $A64F
	ld c,$00	; $A650

loc_03_A652:
	call fn_01_7E90	; $A652
	ld a,(iy+$02)	; $A655
	or a	; $A658
	jr z,loc_03_A67B	; $A659
	call fn_01_7E96	; $A65B
	ld a,(iy+$1C)	; $A65E
	cp $47	; $A661
	jr nz,loc_03_A67B	; $A663
	call fn_01_7ED8	; $A665
	and $01	; $A668
	jr z,loc_03_A67B	; $A66A
	ld (iy+$1C),$53	; $A66C
	call fn_01_7E90	; $A670
	ld (iy+$02),$38	; $A673
	ld (iy+$03),$38	; $A677

loc_03_A67B:
	inc c	; $A67B
	djnz loc_03_A652	; $A67C
	call fn_01_7F29	; $A67E
	jp loc_03_A74B	; $A681

loc_03_A684:
	ld a,(iy+$04)	; $A684
	cp $18	; $A687
	jr nc,loc_03_A69D	; $A689
	ld a,(ix+$20)	; $A68B
	neg	; $A68E
	ld (ix+$20),a	; $A690
	ld a,(ix+$21)	; $A693
	neg	; $A696
	ld (ix+$21),a	; $A698
	jr loc_03_A6B7	; $A69B

loc_03_A69D:
	ld a,(ix+$1D)	; $A69D
	cp $02	; $A6A0
	jr nc,loc_03_A6B7	; $A6A2
	ld a,(iy+$05)	; $A6A4
	cp $A8	; $A6A7
	call z,fn_03_AB38	; $A6A9
	ld a,(iy+$05)	; $A6AC
	cp $C8	; $A6AF
	call z,fn_03_AB54	; $A6B1
	jp loc_03_A74E	; $A6B4

loc_03_A6B7:
	ld a,(iy+$05)	; $A6B7
	cp $AC	; $A6BA
	jp z,loc_03_A74B	; $A6BC
	cp $B0	; $A6BF
	jp z,loc_03_A74B	; $A6C1
	ld (ix+$17),$02	; $A6C4

loc_03_A6C8:
	call fn_01_7ED8	; $A6C8
	and $01	; $A6CB
	jr z,loc_03_A6E1	; $A6CD
	ld a,(ix+$20)	; $A6CF
	or a	; $A6D2
	jr z,loc_03_A6E7	; $A6D3

loc_03_A6D5:
	ld a,(iy+$00)	; $A6D5
	add a,(ix+$20)	; $A6D8
	ld l,a	; $A6DB
	ld h,(iy+$01)	; $A6DC
	jr loc_03_A6F1	; $A6DF

loc_03_A6E1:
	ld a,(ix+$21)	; $A6E1
	or a	; $A6E4
	jr z,loc_03_A6D5	; $A6E5

loc_03_A6E7:
	ld a,(iy+$01)	; $A6E7
	add a,(ix+$21)	; $A6EA
	ld h,a	; $A6ED
	ld l,(iy+$00)	; $A6EE

loc_03_A6F1:
	ld a,($C976)	; $A6F1
	cp $4A	; $A6F4
	jr nz,loc_03_A6FE	; $A6F6
	call fn_01_7E24	; $A6F8
	jp nc,loc_03_A794	; $A6FB

loc_03_A6FE:
	ld c,(ix+$1B)	; $A6FE
	call fn_03_A9E2	; $A701
	jr nc,loc_03_A727	; $A704
	call fn_01_7ED8	; $A706
	call fn_03_ABF8	; $A709
	ld (ix+$20),a	; $A70C
	call fn_01_7ED8	; $A70F
	call fn_03_ABF8	; $A712
	ld (ix+$21),a	; $A715
	dec (ix+$17)	; $A718
	jr nz,loc_03_A6C8	; $A71B
	ld a,(ix+$1D)	; $A71D
	cp $02	; $A720
	jp c,loc_03_A74E	; $A722
	jr loc_03_A74B	; $A725

loc_03_A727:
	ld e,(iy+$00)	; $A727
	ld d,(iy+$01)	; $A72A
	ld (iy+$02),e	; $A72D
	ld (iy+$03),d	; $A730
	ld (iy+$00),l	; $A733
	ld (iy+$01),h	; $A736
	ld a,l	; $A739
	cp $0B	; $A73A
	jr nc,loc_03_A748	; $A73C
	ld a,h	; $A73E
	cp $0B	; $A73F
	jr nc,loc_03_A748	; $A741
	call fn_01_7F4D	; $A743
	jr loc_03_A74B	; $A746

loc_03_A748:
	call fn_03_AB6B	; $A748

loc_03_A74B:
	jp task_return_3	; $A74B

loc_03_A74E:
	ld c,(ix+$1C)	; $A74E
	call fn_01_7E90	; $A751
	ld l,(iy+$00)	; $A754
	ld h,(iy+$01)	; $A757
	ld bc,$1806	; $A75A
	call fn_03_A7B0	; $A75D
	ld a,($C976)	; $A760
	cp $50	; $A763
	jr nz,loc_03_A76E	; $A765
	call fn_01_7ED8	; $A767
	and $01	; $A76A
	jr z,loc_03_A791	; $A76C

loc_03_A76E:
	call fn_01_7ED8	; $A76E
	ld b,a	; $A771
	ld c,(ix+$1C)	; $A772
	call fn_01_7E96	; $A775
	ld a,(iy+$0C)	; $A778
	ld hl,$AC44	; $A77B
	rst $28	; $A77E
	cp b	; $A77F
	jr nc,loc_03_A791	; $A780
	ld c,(ix+$1C)	; $A782
	call fn_03_ABB7	; $A785
	ld b,(ix+$1B)	; $A788
	ld c,(ix+$1C)	; $A78B
	call monster_hits_member	; $A78E

loc_03_A791:
	jp task_return_3	; $A791

loc_03_A794:
	ld (ix+$22),a	; $A794
	ld bc,$1208	; $A797
	call fn_03_A7B0	; $A79A
	call fn_01_7ED8	; $A79D
	and $3F	; $A7A0
	ld b,$FF	; $A7A2
	ld c,(ix+$22)	; $A7A4
	call fn_01_7E36	; $A7A7
	call fn_01_7F4D	; $A7AA
	jp task_return_3	; $A7AD

fn_03_A7B0:
	ld a,b	; $A7B0
	call fn_00_05F1	; $A7B1
	ld a,c	; $A7B4
	call fn_01_7EF6	; $A7B5
	call fn_03_A477	; $A7B8
	ld (ix+$0B),l	; $A7BB
	ld (ix+$0D),h	; $A7BE
	ld (ix+$09),$08	; $A7C1
	call task_enter_4	; $A7C5
	ld (ix+$19),$05	; $A7C8

loc_03_A7CC:
	call fn_03_A482	; $A7CC
	call fn_01_7F59	; $A7CF
	dec (ix+$19)	; $A7D2
	jr nz,loc_03_A7CC	; $A7D5
	jp task_return_4	; $A7D7

member_at_position:
	push bc	; $A7DA
	push iy	; $A7DB
	ld a,(party_size)	; $A7DD
	ld b,a	; $A7E0
	ld c,$00	; $A7E1

loc_03_A7E3:
	call fn_01_7ED2	; $A7E3
	jr c,loc_03_A7FA	; $A7E6
	call fn_01_7E90	; $A7E8
	ld a,l	; $A7EB
	cp (iy+$00)	; $A7EC
	jr nz,loc_03_A7FA	; $A7EF
	ld a,h	; $A7F1
	cp (iy+$01)	; $A7F2
	jr nz,loc_03_A7FA	; $A7F5
	ld a,c	; $A7F7
	jr loc_03_A7FE	; $A7F8

loc_03_A7FA:
	inc c	; $A7FA
	djnz loc_03_A7E3	; $A7FB
	scf	; $A7FD

loc_03_A7FE:
	pop iy	; $A7FE
	pop bc	; $A800
	ret	; $A801

monster_hits_member:
	push de	; $A802
	push hl	; $A803
	push iy	; $A804
	ld a,$12	; $A806
	call fn_00_05F1	; $A808
	call fn_01_7E66	; $A80B
	call wait_vblank	; $A80E
	call fn_01_7F4D	; $A811
	push bc	; $A814
	ld c,b	; $A815
	call fn_01_7E8D	; $A816
	ld a,(iy+$05)	; $A819
	call combat_strength_index	; $A81C
	ld hl,$AC00	; $A81F
	rst $28	; $A822
	srl a	; $A823
	call fn_01_7EDB	; $A825
	ld d,a	; $A828
	rrca	; $A829
	rrca	; $A82A
	rrca	; $A82B
	rrca	; $A82C
	and $0F	; $A82D
	ld l,a	; $A82F
	ld h,$0A	; $A830
	call fn_01_7EC3	; $A832
	ld e,l	; $A835
	ld l,d	; $A836
	ld h,$0A	; $A837
	call fn_01_7E3C	; $A839
	ld a,h	; $A83C
	add a,e	; $A83D
	ld l,a	; $A83E
	ld h,$00	; $A83F
	pop bc	; $A841
	call fn_01_7E63	; $A842
	call fn_01_7F38	; $A845
	jr nc,loc_03_A872	; $A848
	call fn_01_7E96	; $A84A
	push iy	; $A84D
	pop hl	; $A84F
	ld de,$000D	; $A850
	add hl,de	; $A853
	call fn_01_7F62	; $A854
	call vec_print_inline	; $A857
	.INCBIN "bank03.bin" SKIP $285A READ 13	; text, 13 bytes
	ld a,$01	; $A867
	ld c,$04	; $A869
	call bank01_vectors	; $A86B
	call fn_01_7EC6	; $A86E
	scf	; $A871

loc_03_A872:
	pop iy	; $A872
	pop hl	; $A874
	pop de	; $A875
	ret	; $A876

fn_03_A877:
	ld a,$12	; $A877
	call fn_00_05F1	; $A879
	call fn_01_7E66	; $A87C
	call wait_vblank	; $A87F
	call fn_01_7F4D	; $A882
	jp fn_01_7E63	; $A885

fn_03_A888:
	call task_enter_4	; $A888
	ld (ix+$22),d	; $A88B
	ld (ix+$23),e	; $A88E
	call fn_01_7E90	; $A891
	ld e,(iy+$00)	; $A894
	ld d,(iy+$01)	; $A897
	ld c,b	; $A89A
	call fn_01_7E8D	; $A89B
	ld l,(iy+$00)	; $A89E
	ld h,(iy+$01)	; $A8A1
	ld a,(ix+$23)	; $A8A4
	call fn_01_7EBD	; $A8A7

loc_03_A8AA:
	call fn_00_0BA4	; $A8AA
	call fn_03_A482	; $A8AD
	call fn_01_7F59	; $A8B0
	call fn_03_A43E	; $A8B3
	call member_at_position	; $A8B6
	jr nc,loc_03_A8DA	; $A8B9
	call fn_03_A460	; $A8BB
	call fn_01_7EB7	; $A8BE
	jr nc,loc_03_A8AA	; $A8C1
	ld a,(ix+$22)	; $A8C3
	cp $4C	; $A8C6
	jr nz,loc_03_A90A	; $A8C8
	call fn_00_0BC9	; $A8CA
	call fn_03_A43E	; $A8CD
	call fn_03_A469	; $A8D0
	ld (hl),$4C	; $A8D3
	inc l	; $A8D5
	ld (hl),$89	; $A8D6
	jr loc_03_A90A	; $A8D8

loc_03_A8DA:
	ld (ix+$08),a	; $A8DA
	ld c,a	; $A8DD
	call fn_01_7F6E	; $A8DE
	call fn_01_7ECC	; $A8E1
	ld a,$20	; $A8E4
	call fn_01_7E33	; $A8E6
	ld a,(ix+$22)	; $A8E9
	ld hl,$A910	; $A8EC
	call fn_01_7E18	; $A8EF
	call vec_print_inline	; $A8F2
	.INCBIN "bank03.bin" SKIP $28F5 READ 6	; text, 6 bytes

loc_03_A8FB:
	ld c,(ix+$08)	; $A8FB
	call fn_03_ABB7	; $A8FE
	ld b,(ix+$1B)	; $A901
	ld c,(ix+$08)	; $A904
	call monster_hits_member	; $A907

loc_03_A90A:
	call fn_01_7F29	; $A90A
	jp task_return_4	; $A90D
	.INCBIN "bank03.bin" SKIP $2910 READ 19	; data, 19 bytes
	call vec_print_inline	; $A923
	.INCBIN "bank03.bin" SKIP $2926 READ 11	; text, 11 bytes
	ld c,(ix+$08)	; $A931
	call fn_03_A877	; $A934
	call fn_01_7ED8	; $A937
	and $01	; $A93A
	jp z,loc_03_A9D3	; $A93C
	call fn_01_7E96	; $A93F
	ld a,(iy+$1C)	; $A942
	cp $47	; $A945
	jp nz,loc_03_A9D3	; $A947
	ld (iy+$1C),$50	; $A94A
	jp loc_03_A90A	; $A94E
	call vec_print_inline	; $A951
	.INCBIN "bank03.bin" SKIP $2954 READ 14	; text, 14 bytes
	jp loc_03_A8FB	; $A962
	call vec_print_inline	; $A965
	.INCBIN "bank03.bin" SKIP $2968 READ 12	; text, 12 bytes
	jp loc_03_A8FB	; $A974
	call vec_print_inline	; $A977
	.INCBIN "bank03.bin" SKIP $297A READ 8	; text, 8 bytes
	ld c,(ix+$08)	; $A982
	call fn_03_A877	; $A985
	call fn_01_7ED8	; $A988
	and $01	; $A98B
	jr z,loc_03_A9D3	; $A98D
	call fn_01_7E96	; $A98F
	ld a,(iy+$1C)	; $A992
	cp $47	; $A995
	jr nz,loc_03_A9D3	; $A997
	ld (iy+$1C),$53	; $A999
	ld c,(ix+$08)	; $A99D
	call fn_01_7E90	; $A9A0
	ld (iy+$02),$38	; $A9A3
	ld (iy+$03),$38	; $A9A7
	jp loc_03_A90A	; $A9AB
	.INCBIN "bank03.bin" SKIP $29AE READ 17	; data, 17 bytes
	call vec_print_inline	; $A9BF
	.INCBIN "bank03.bin" SKIP $29C2 READ 14	; text, 14 bytes
	jp loc_03_A8FB	; $A9D0

loc_03_A9D3:
	call vec_print_inline	; $A9D3
	.INCBIN "bank03.bin" SKIP $29D6 READ 9	; text, 9 bytes
	jp loc_03_A90A	; $A9DF

fn_03_A9E2:
	push bc	; $A9E2
	push de	; $A9E3
	push hl	; $A9E4
	push iy	; $A9E5
	ld (ix+$22),c	; $A9E7
	call fn_01_7E8D	; $A9EA
	ld a,l	; $A9ED
	cp $0B	; $A9EE
	jr nc,loc_03_A9F7	; $A9F0
	ld a,h	; $A9F2
	cp $0B	; $A9F3
	jr c,loc_03_AA00	; $A9F5

loc_03_A9F7:
	ld a,(iy+$04)	; $A9F7
	cp $18	; $A9FA
	ccf	; $A9FC
	jp loc_03_AA8D	; $A9FD

loc_03_AA00:
	call fn_03_A460	; $AA00
	ld (ix+$23),a	; $AA03
	call fn_01_7F47	; $AA06
	jp c,loc_03_AA8D	; $AA09
	ld a,l	; $AA0C
	cp (iy+$00)	; $AA0D
	jr nz,loc_03_AA1F	; $AA10
	ld a,h	; $AA12
	cp (iy+$01)	; $AA13
	jr nz,loc_03_AA1F	; $AA16
	call fn_01_7ED8	; $AA18
	and $03	; $AA1B
	jr nz,loc_03_AA8C	; $AA1D

loc_03_AA1F:
	ld iy,room_monsters	; $AA1F
	ld de,$0008	; $AA23
	ld b,$10	; $AA26

loc_03_AA28:
	ld a,(iy+$05)	; $AA28
	or a	; $AA2B
	jr z,loc_03_AA3A	; $AA2C
	ld a,l	; $AA2E
	cp (iy+$00)	; $AA2F
	jr nz,loc_03_AA3A	; $AA32
	ld a,h	; $AA34
	cp (iy+$01)	; $AA35
	jr z,loc_03_AA8C	; $AA38

loc_03_AA3A:
	add iy,de	; $AA3A
	djnz loc_03_AA28	; $AA3C
	ld iy,party_objects	; $AA3E
	ld de,$0005	; $AA42
	ld b,$08	; $AA45

loc_03_AA47:
	ld a,(iy+$02)	; $AA47
	or a	; $AA4A
	jr z,loc_03_AA59	; $AA4B
	ld a,l	; $AA4D
	cp (iy+$00)	; $AA4E
	jr nz,loc_03_AA59	; $AA51
	ld a,h	; $AA53
	cp (iy+$01)	; $AA54
	jr z,loc_03_AA8C	; $AA57

loc_03_AA59:
	add iy,de	; $AA59
	djnz loc_03_AA47	; $AA5B
	ld c,(ix+$22)	; $AA5D
	call fn_01_7E8D	; $AA60
	ld a,(iy+$05)	; $AA63
	cp $80	; $AA66
	jr nc,loc_03_AA72	; $AA68
	ld a,(ix+$23)	; $AA6A
	call fn_01_7E99	; $AA6D
	jr loc_03_AA8D	; $AA70

loc_03_AA72:
	cp $8E	; $AA72
	jr nc,loc_03_AA7E	; $AA74
	ld a,(ix+$23)	; $AA76
	cp $03	; $AA79
	ccf	; $AA7B
	jr loc_03_AA8D	; $AA7C

loc_03_AA7E:
	ld hl,$AA93	; $AA7E
	call fn_01_7E18	; $AA81
	ld a,(ix+$23)	; $AA84
	call fn_01_7E99	; $AA87
	jr loc_03_AA8D	; $AA8A

loc_03_AA8C:
	scf	; $AA8C

loc_03_AA8D:
	pop iy	; $AA8D
	pop hl	; $AA8F
	pop de	; $AA90
	pop bc	; $AA91
	ret	; $AA92
	.INCBIN "bank03.bin" SKIP $2A93 READ 25	; data, 25 bytes
	ld a,(ix+$23)	; $AAAC
	cp $03	; $AAAF
	ccf	; $AAB1
	jr nc,loc_03_AA8D	; $AAB2
	call fn_01_7E99	; $AAB4
	jr loc_03_AA8D	; $AAB7
	ld a,(ix+$23)	; $AAB9
	cp $03	; $AABC
	jr c,loc_03_AA8D	; $AABE
	cp $45	; $AAC0
	jr z,loc_03_AA8C	; $AAC2
	or a	; $AAC4
	jr loc_03_AA8D	; $AAC5

fn_03_AAC7:
	push bc	; $AAC7
	push hl	; $AAC8
	cp $B0	; $AAC9
	jr z,loc_03_AAE9	; $AACB
	cp $FC	; $AACD
	jr z,loc_03_AAE9	; $AACF
	ld hl,$AAF7	; $AAD1
	ld b,$13	; $AAD4

loc_03_AAD6:
	cp (hl)	; $AAD6
	inc hl	; $AAD7
	jr z,loc_03_AAE4	; $AAD8
	inc hl	; $AADA
	inc hl	; $AADB
	djnz loc_03_AAD6	; $AADC
	ld de,$0000	; $AADE
	scf	; $AAE1
	jr loc_03_AAF4	; $AAE2

loc_03_AAE4:
	ld e,(hl)	; $AAE4
	inc hl	; $AAE5
	ld d,(hl)	; $AAE6
	jr loc_03_AAF3	; $AAE7

loc_03_AAE9:
	call fn_01_7ED8	; $AAE9
	and $03	; $AAEC
	ld hl,$AB30	; $AAEE
	rst $20	; $AAF1
	ex de,hl	; $AAF2

loc_03_AAF3:
	or a	; $AAF3

loc_03_AAF4:
	pop hl	; $AAF4
	pop bc	; $AAF5
	ret	; $AAF6
	.INCBIN "bank03.bin" SKIP $2AF7 READ 65	; data, 65 bytes

fn_03_AB38:
	push de	; $AB38
	push hl	; $AB39
	call fn_01_7F11	; $AB3A
	ld hl,(food)	; $AB3D
	ld de,$0019	; $AB40
	or a	; $AB43
	sbc hl,de	; $AB44
	jr nc,loc_03_AB4B	; $AB46
	ld hl,$0000	; $AB48

loc_03_AB4B:
	ld (food),hl	; $AB4B
	call fn_01_7F29	; $AB4E
	pop hl	; $AB51
	pop de	; $AB52
	ret	; $AB53

fn_03_AB54:
	call fn_01_7ED8	; $AB54
	and $03	; $AB57
	ret nz	; $AB59
	push hl	; $AB5A
	call fn_01_7F11	; $AB5B
	call fn_01_7ED8	; $AB5E
	and $3F	; $AB61
	ld l,a	; $AB63
	ld h,$00	; $AB64
	call fn_01_7F35	; $AB66
	pop hl	; $AB69
	ret	; $AB6A

fn_03_AB6B:
	push bc	; $AB6B
	push de	; $AB6C
	push hl	; $AB6D
	push iy	; $AB6E
	call fn_01_7F6E	; $AB70
	ld c,(ix+$1B)	; $AB73
	call fn_01_7E8D	; $AB76
	ld a,(iy+$05)	; $AB79
	call fn_01_7EC0	; $AB7C
	call fn_01_7F62	; $AB7F
	call vec_print_inline	; $AB82
	.INCBIN "bank03.bin" SKIP $2B85 READ 9	; text, 9 bytes
	ld a,(iy+$05)	; $AB8E
	call fn_01_7E75	; $AB91
	jr c,loc_03_ABA4	; $AB94
	ld a,$01	; $AB96
	ld c,$01	; $AB98
	call bank01_vectors	; $AB9A
	ld a,$01	; $AB9D
	ld c,$03	; $AB9F
	call bank01_vectors	; $ABA1

loc_03_ABA4:
	ld a,$19	; $ABA4
	call fn_00_05F1	; $ABA6
	ld (iy+$05),$00	; $ABA9
	ld (iy+$04),$00	; $ABAD
	pop iy	; $ABB1
	pop hl	; $ABB3
	pop de	; $ABB4
	pop bc	; $ABB5
	ret	; $ABB6

fn_03_ABB7:
	pop hl	; $ABB7
	ld (ix+$14),l	; $ABB8
	ld (ix+$15),h	; $ABBB
	call fn_01_7E90	; $ABBE
	ld l,(iy+$00)	; $ABC1
	ld h,(iy+$01)	; $ABC4
	call fn_03_A477	; $ABC7
	ld (ix+$0B),l	; $ABCA
	ld (ix+$0D),h	; $ABCD
	ld a,$04	; $ABD0
	call fn_03_A48A	; $ABD2
	call fn_01_7F4D	; $ABD5
	call yield	; $ABD8
	ld a,$05	; $ABDB
	call fn_03_A48A	; $ABDD
	call fn_01_7F4D	; $ABE0
	call yield	; $ABE3
	ld a,$06	; $ABE6
	call fn_03_A48A	; $ABE8
	call fn_01_7F4D	; $ABEB
	call yield	; $ABEE
	ld l,(ix+$14)	; $ABF1
	ld h,(ix+$15)	; $ABF4
	jp (hl)	; $ABF7

fn_03_ABF8:
	or a	; $ABF8
	ret z	; $ABF9
	ld a,$01	; $ABFA
	ret p	; $ABFC
	ld a,$FF	; $ABFD
	ret	; $ABFF

combat_strength:
	.INCBIN "bank03.bin" SKIP $2C00 READ 177	; data, 177 bytes

tbl_03_ACB1:
	.dw $ACBF
	.dw $ACC8
	.dw $ACD5
	.dw $ACE1
	.dw $ACF0
	.dw $ACFB
	.dw fn_00_0213
	.dw fn_01_7441
	.INCBIN "bank03.bin" SKIP $2CC1 READ 7	; data, 7 bytes

fn_03_ACC8:
	.INCBIN "bank03.bin" SKIP $2CC8 READ 13	; data, 13 bytes

fn_03_ACD5:
	.INCBIN "bank03.bin" SKIP $2CD5 READ 12	; data, 12 bytes

fn_03_ACE1:
	.INCBIN "bank03.bin" SKIP $2CE1 READ 15	; data, 15 bytes

fn_03_ACF0:
	.INCBIN "bank03.bin" SKIP $2CF0 READ 11	; data, 11 bytes

fn_03_ACFB:
	.INCBIN "bank03.bin" SKIP $2CFB READ 8	; data, 8 bytes

loc_03_AD03:
	call fn_01_7E84	; $AD03
	ld de,$AD0F	; $AD06
	call spawn_task	; $AD09
	jp fn_01_7E5D	; $AD0C
	call fn_00_0605	; $AD0F
	call fn_01_7EA2	; $AD12
	ld a,($C9B7)	; $AD15
	and $7F	; $AD18
	cp $13	; $AD1A
	ld a,$0D	; $AD1C
	jr nz,loc_03_AD21	; $AD1E
	inc a	; $AD20

loc_03_AD21:
	call fn_01_7E9C	; $AD21
	ld hl,$C83E	; $AD24
	ld de,$0008	; $AD27
	ld bc,$1000	; $AD2A

loc_03_AD2D:
	ld (hl),c	; $AD2D
	add hl,de	; $AD2E
	djnz loc_03_AD2D	; $AD2F
	ld iy,party_objects	; $AD31
	ld de,$0005	; $AD35
	ld a,(party_size)	; $AD38
	ld b,a	; $AD3B
	ld c,$00	; $AD3C

loc_03_AD3E:
	call fn_01_7ED2	; $AD3E
	ld a,$38	; $AD41
	jr nc,loc_03_AD46	; $AD43
	xor a	; $AD45

loc_03_AD46:
	ld (iy+$02),a	; $AD46
	ld (iy+$03),a	; $AD49
	add iy,de	; $AD4C
	inc c	; $AD4E
	djnz loc_03_AD3E	; $AD4F
	call fn_01_7F4D	; $AD51
	call fn_01_7F29	; $AD54
	call fn_01_7EAB	; $AD57
	ld a,$09	; $AD5A
	call play_music	; $AD5C
	call vec_print_inline	; $AD5F
	.INCBIN "bank03.bin" SKIP $2D62 READ 8	; text, 8 bytes
	ld (ix+$16),$03	; $AD6A

loc_03_AD6E:
	ld a,$46	; $AD6E
	call fn_01_7F53	; $AD70
	ld a,$2E	; $AD73
	call fn_01_7E33	; $AD75
	dec (ix+$16)	; $AD78
	jr nz,loc_03_AD6E	; $AD7B
	call fn_01_7F6E	; $AD7D
	call fn_01_7ED8	; $AD80
	and $07	; $AD83
	jp z,loc_03_AE0C	; $AD85
	ld hl,($C8FD)	; $AD88
	ld de,$0064	; $AD8B
	call fn_03_B7EA	; $AD8E
	ex de,hl	; $AD91
	ld hl,($C965)	; $AD92
	or a	; $AD95
	sbc hl,de	; $AD96
	ex de,hl	; $AD98
	ld ($C965),hl	; $AD99
	jr nz,loc_03_ADB0	; $AD9C
	call vec_print_inline	; $AD9E
	.INCBIN "bank03.bin" SKIP $2DA1 READ 12	; text, 12 bytes
	jp loc_03_ADFA	; $ADAD

loc_03_ADB0:
	ld iy,party	; $ADB0
	ld de,$001D	; $ADB4
	ld a,(party_size)	; $ADB7
	ld b,a	; $ADBA
	ld c,$00	; $ADBB

loc_03_ADBD:
	call fn_01_7ED2	; $ADBD
	jr c,loc_03_ADDE	; $ADC0
	ld a,(iy+$1C)	; $ADC2
	cp $53	; $ADC5
	jr nz,loc_03_ADCD	; $ADC7
	ld (iy+$1C),$47	; $ADC9

loc_03_ADCD:
	call fn_01_7ED8	; $ADCD
	and $77	; $ADD0
	add a,$63	; $ADD2
	ld l,a	; $ADD4
	ld h,$00	; $ADD5
	call fn_01_7E03	; $ADD7
	ld (iy+$09),$63	; $ADDA

loc_03_ADDE:
	add iy,de	; $ADDE
	inc c	; $ADE0
	djnz loc_03_ADBD	; $ADE1
	call fn_01_7EE1	; $ADE3
	call vec_print_inline	; $ADE6
	.INCBIN "bank03.bin" SKIP $2DE9 READ 17	; text, 17 bytes

loc_03_ADFA:
	call fn_01_7F29	; $ADFA
	call fn_00_0605	; $ADFD
	call fn_01_7EA2	; $AE00
	ld a,($C9B7)	; $AE03
	and $7F	; $AE06
	ld (game_mode),a	; $AE08
	ret	; $AE0B

loc_03_AE0C:
	call vec_print_inline	; $AE0C
	.INCBIN "bank03.bin" SKIP $2E0F READ 11	; text, 11 bytes
	ld hl,(party_x)	; $AE1A
	ld ($C97A),hl	; $AE1D
	call fn_01_7ED8	; $AE20
	and $07	; $AE23
	ld hl,$AE4F	; $AE25
	rst $28	; $AE28
	ld ($C97D),a	; $AE29
	ld ($C97E),a	; $AE2C
	ld iy,party	; $AE2F
	ld de,$001D	; $AE33
	ld a,(party_size)	; $AE36
	ld b,a	; $AE39

loc_03_AE3A:
	ld a,(iy+$1C)	; $AE3A
	cp $47	; $AE3D
	jr nz,loc_03_AE45	; $AE3F
	ld (iy+$1C),$53	; $AE41

loc_03_AE45:
	add iy,de	; $AE45
	djnz loc_03_AE3A	; $AE47
	call fn_01_7F29	; $AE49
	jp fn_01_7E30	; $AE4C
	.INCBIN "bank03.bin" SKIP $2E4F READ 8	; data, 8 bytes

loc_03_AE57:
	call fn_01_7E84	; $AE57
	ld de,$AE63	; $AE5A
	call spawn_task	; $AE5D
	jp fn_01_7E5D	; $AE60
	ld a,$1A	; $AE63
	ld (message_bank),a	; $AE65
	ld hl,$8096	; $AE68
	ld (message_table),hl	; $AE6B
	ld hl,$0B0A	; $AE6E
	call fn_00_06DA	; $AE71
	ld a,(location)	; $AE74
	add a,$E7	; $AE77
	ld b,a	; $AE79
	ld a,$80	; $AE7A

loc_03_AE7C:
	rlca	; $AE7C
	djnz loc_03_AE7C	; $AE7D
	ld hl,$C956	; $AE7F
	and (hl)	; $AE82
	jr nz,loc_03_AE8D	; $AE83
	ld a,$20	; $AE85
	call fn_01_7F65	; $AE87
	jp loc_03_B02A	; $AE8A

loc_03_AE8D:
	ld a,$17	; $AE8D
	call fn_01_7E9C	; $AE8F
	call fn_01_7F4D	; $AE92
	call fn_01_7F29	; $AE95
	call fn_01_7EAB	; $AE98
	ld a,$03	; $AE9B
	call fn_00_05F1	; $AE9D
	ld a,$21	; $AEA0
	call fn_01_7F65	; $AEA2
	call fn_03_B0AE	; $AEA5
	ld (ix+$1A),$00	; $AEA8

loc_03_AEAC:
	call fn_01_7F56	; $AEAC
	ld c,$08	; $AEAF
	ld de,$0102	; $AEB1
	call fn_01_7EB1	; $AEB4
	jr z,loc_03_AEAC	; $AEB7
	jp c,loc_03_B027	; $AEB9
	ld a,(ix+$1A)	; $AEBC
	ld (ix+$1B),a	; $AEBF
	add a,$97	; $AEC2
	call fn_01_7F6B	; $AEC4
	call fn_01_7F71	; $AEC7
	ld a,$22	; $AECA
	call fn_01_7F65	; $AECC
	ld hl,$B0CA	; $AECF
	call fn_01_7E60	; $AED2
	ld (ix+$1A),$00	; $AED5

loc_03_AED9:
	call fn_01_7F56	; $AED9
	ld c,$04	; $AEDC
	ld de,$0102	; $AEDE
	call fn_01_7EB1	; $AEE1
	jr z,loc_03_AED9	; $AEE4
	jp c,loc_03_B027	; $AEE6
	ld a,(ix+$1A)	; $AEE9
	ld (ix+$1C),a	; $AEEC
	add a,$30	; $AEEF
	call fn_01_7E33	; $AEF1
	call fn_01_7F71	; $AEF4
	call fn_01_7F29	; $AEF7
	ld a,(ix+$1C)	; $AEFA
	or a	; $AEFD
	jr z,loc_03_AF0A	; $AEFE
	ld a,(location)	; $AF00
	sub $1A	; $AF03
	cp (ix+$1B)	; $AF05
	jr z,loc_03_AF12	; $AF08

loc_03_AF0A:
	ld a,$23	; $AF0A
	call fn_01_7F65	; $AF0C
	jp loc_03_B02A	; $AF0F

loc_03_AF12:
	ld hl,($C8FD)	; $AF12
	ld de,$0064	; $AF15
	call fn_03_B7EA	; $AF18
	ex de,hl	; $AF1B
	ld hl,($C969)	; $AF1C
	or a	; $AF1F
	sbc hl,de	; $AF20
	ld ($C969),de	; $AF22
	jr nz,loc_03_AF30	; $AF26
	ld a,$24	; $AF28
	call fn_01_7F65	; $AF2A
	jp loc_03_B02A	; $AF2D

loc_03_AF30:
	ld a,$25	; $AF30
	call fn_01_7F65	; $AF32
	call fn_01_7F6E	; $AF35
	ld a,(ix+$1C)	; $AF38
	ld (ix+$16),a	; $AF3B

loc_03_AF3E:
	ld (ix+$17),$10	; $AF3E

loc_03_AF42:
	ld a,$08	; $AF42
	call fn_01_7F53	; $AF44
	ld a,$2E	; $AF47
	call fn_01_7E33	; $AF49
	dec (ix+$17)	; $AF4C
	jr nz,loc_03_AF42	; $AF4F
	call fn_01_7F6E	; $AF51
	ld a,$26	; $AF54
	call fn_01_7F65	; $AF56
	ld a,$05	; $AF59
	call fn_01_7E72	; $AF5B

loc_03_AF5E:
	call fn_01_7F56	; $AF5E
	call fn_01_7E6F	; $AF61
	jr z,loc_03_AF5E	; $AF64
	call fn_01_7F29	; $AF66
	jr c,loc_03_AF7D	; $AF69
	call fn_01_7F6E	; $AF6B
	ld a,(ix+$1B)	; $AF6E
	ld hl,$B0DB	; $AF71
	rst $20	; $AF74
	ld de,$C065	; $AF75
	call fn_01_7F2C	; $AF78
	jr nc,loc_03_AF8C	; $AF7B

loc_03_AF7D:
	ld a,$27	; $AF7D
	call fn_01_7F65	; $AF7F
	ld a,$03	; $AF82
	ld c,$06	; $AF84
	call fn_01_7F2F	; $AF86
	jp loc_03_B02A	; $AF89

loc_03_AF8C:
	dec (ix+$16)	; $AF8C
	jr nz,loc_03_AF3E	; $AF8F
	call fn_01_7F6E	; $AF91
	ld a,(ix+$1C)	; $AF94
	cp $03	; $AF97
	jr nz,loc_03_AFA6	; $AF99
	ld a,(ix+$1B)	; $AF9B
	ld hl,karma	; $AF9E
	rst $28	; $AFA1
	cp $63	; $AFA2
	jr z,loc_03_AFD3	; $AFA4

loc_03_AFA6:
	ld a,$28	; $AFA6
	call fn_01_7F65	; $AFA8
	call fn_01_7F50	; $AFAB
	call fn_01_7F71	; $AFAE
	ld a,(ix+$1C)	; $AFB1
	ld b,a	; $AFB4
	add a,a	; $AFB5
	add a,b	; $AFB6
	ld c,$06	; $AFB7
	call bank01_vectors	; $AFB9
	ld a,(ix+$1B)	; $AFBC
	ld b,a	; $AFBF
	add a,a	; $AFC0
	add a,b	; $AFC1
	add a,(ix+$1C)	; $AFC2
	add a,$07	; $AFC5
	call fn_01_7F65	; $AFC7
	call fn_01_7F50	; $AFCA
	call fn_01_7F6E	; $AFCD
	jp loc_03_B02A	; $AFD0

loc_03_AFD3:
	ld a,$29	; $AFD3
	call fn_01_7F65	; $AFD5
	ld a,(ix+$1B)	; $AFD8
	add a,$97	; $AFDB
	call fn_01_7F6B	; $AFDD
	ld a,$15	; $AFE0
	call fn_00_05F1	; $AFE2
	call wait_vblank	; $AFE5
	call fn_01_7E69	; $AFE8
	call wait_vblank	; $AFEB
	call fn_01_7E69	; $AFEE
	call wait_vblank	; $AFF1
	ld a,(ix+$1B)	; $AFF4
	ld hl,karma	; $AFF7
	rst $30	; $AFFA
	ld (hl),$00	; $AFFB
	call fn_01_7F29	; $AFFD
	call fn_01_7F50	; $B000
	call fn_01_7F71	; $B003
	call fn_03_B039	; $B006
	ld a,(ix+$1B)	; $B009
	call fn_03_B085	; $B00C
	ld a,$2A	; $B00F
	call fn_01_7F65	; $B011

loc_03_B014:
	call fn_01_7F5C	; $B014
	call yield	; $B017
	ld a,(joy_pressed)	; $B01A
	and $30	; $B01D
	jr z,loc_03_B014	; $B01F
	call fn_01_7F6E	; $B021
	jp loc_03_B02A	; $B024

loc_03_B027:
	call fn_01_7F29	; $B027

loc_03_B02A:
	call fn_01_7F6E	; $B02A
	call fn_00_0605	; $B02D
	call fn_01_7EA2	; $B030
	ld a,$11	; $B033
	ld (game_mode),a	; $B035
	ret	; $B038

fn_03_B039:
	ld de,$0020	; $B039
	ld b,$0C	; $B03C

loc_03_B03E:
	push bc	; $B03E
	call wait_vblank	; $B03F
	ld b,$0B	; $B042

loc_03_B044:
	push bc	; $B044
	ld hl,$0220	; $B045
	ld b,$10	; $B048

loc_03_B04A:
	rst $10	; $B04A
	xor a	; $B04B
	call fn_00_0262	; $B04C
	out ($BE),a	; $B04F
	call fn_00_0262	; $B051
	out ($BE),a	; $B054
	call fn_00_0262	; $B056
	out ($BE),a	; $B059
	call fn_00_0262	; $B05B
	out ($BE),a	; $B05E
	ex de,hl	; $B060
	add hl,de	; $B061
	ex de,hl	; $B062
	djnz loc_03_B04A	; $B063
	rst $10	; $B065
	ld hl,$B081	; $B066
	ld bc,$04BE	; $B069

loc_03_B06C:
	outi	; $B06C
	jr nz,loc_03_B06C	; $B06E
	ld hl,$DE30	; $B070
	add hl,de	; $B073
	ex de,hl	; $B074
	pop bc	; $B075
	djnz loc_03_B044	; $B076
	ld hl,$FDF4	; $B078
	add hl,de	; $B07B
	ex de,hl	; $B07C
	pop bc	; $B07D
	djnz loc_03_B03E	; $B07E
	ret	; $B080
	.INCBIN "bank03.bin" SKIP $3081 READ 4	; data, 4 bytes

fn_03_B085:
	push af	; $B085
	ld a,$10	; $B086
	ld hl,$8923	; $B088
	ld de,$0020	; $B08B
	call fn_01_7E78	; $B08E
	pop af	; $B091
	ld hl,$B09E	; $B092
	rst $20	; $B095
	ld a,$10	; $B096
	ld de,$0DA0	; $B098
	jp fn_01_7E7B	; $B09B

tbl_03_B09E:
	.dw fn_03_8A48
	.dw fn_03_8A65
	.dw fn_03_8A82
	.dw fn_03_8A48
	.dw fn_03_8A65
	.dw fn_03_8A48
	.dw fn_03_8A9F
	.INCBIN "bank03.bin" SKIP $30AC READ 2	; data, 2 bytes

fn_03_B0AE:
	call fn_01_7E21	; $B0AE
	ld de,$0213	; $B0B1
	ld bc,$0897	; $B0B4

loc_03_B0B7:
	push bc	; $B0B7
	push de	; $B0B8
	ld a,c	; $B0B9
	call fn_01_7F02	; $B0BA
	call fn_01_7F0E	; $B0BD
	pop de	; $B0C0
	pop bc	; $B0C1
	inc d	; $B0C2
	inc d	; $B0C3
	inc c	; $B0C4
	djnz loc_03_B0B7	; $B0C5
	jp fn_01_7F41	; $B0C7
	.INCBIN "bank03.bin" SKIP $30CA READ 17	; data, 17 bytes

tbl_03_B0DB:
	.dw $B0EB
	.dw fn_03_B0EF
	.dw fn_03_B0F2
	.dw fn_03_B0F5
	.dw fn_03_B0F9
	.dw fn_03_B0FD
	.dw fn_03_B102
	.dw fn_03_B105
	.dw fn_01_6861
	.dw fn_00_006D

fn_03_B0EF:
	ld l,l	; $B0EF
	ld (hl),l	; $B0F0
	nop	; $B0F1

fn_03_B0F2:
	ld (hl),d	; $B0F2
	ld h,c	; $B0F3
	nop	; $B0F4

fn_03_B0F5:
	ld h,d	; $B0F5
	ld h,l	; $B0F6
	ld l,b	; $B0F7
	nop	; $B0F8

fn_03_B0F9:
	ld h,e	; $B0F9
	ld h,c	; $B0FA
	ld l,b	; $B0FB
	nop	; $B0FC

fn_03_B0FD:
	ld (hl),e	; $B0FD
	ld (hl),l	; $B0FE
	ld l,l	; $B0FF
	ld l,l	; $B100
	nop	; $B101

fn_03_B102:
	ld l,a	; $B102
	ld l,l	; $B103
	nop	; $B104

fn_03_B105:
	ld l,h	; $B105
	ld (hl),l	; $B106
	ld l,l	; $B107
	nop	; $B108

loc_03_B109:
	call fn_01_7E84	; $B109
	ld de,$B127	; $B10C
	call spawn_task	; $B10F
	jp fn_01_7E5D	; $B112

loc_03_B115:
	call fn_01_7E84	; $B115
	ld de,$B7CF	; $B118
	call spawn_task	; $B11B
	ld de,$B63F	; $B11E
	call spawn_task	; $B121
	jp fn_01_7E5A	; $B124

sub_03_B127:
	ld a,$1A	; $B127
	ld (message_bank),a	; $B129
	ld hl,$8036	; $B12C
	ld (message_table),hl	; $B12F
	ld hl,$0B0A	; $B132
	call fn_00_06DA	; $B135
	ld a,$02	; $B138
	ld ($C00C),a	; $B13A
	call fn_00_0605	; $B13D
	call fn_03_B55B	; $B140
	ld hl,$C76D	; $B143
	ld a,(hl)	; $B146
	cp $44	; $B147
	jr nz,loc_03_B158	; $B149
	ld (hl),$47	; $B14B
	ld a,$15	; $B14D
	call fn_00_05F1	; $B14F
	call fn_01_7E69	; $B152
	call fn_01_7E69	; $B155

loc_03_B158:
	ld hl,party_size	; $B158
	ld a,(hl)	; $B15B
	ld ($C64D),a	; $B15C
	ld (hl),$01	; $B15F
	call fn_01_7F29	; $B161
	ld a,$1E	; $B164
	call sleep	; $B166
	call fn_00_05E3	; $B169
	call fn_01_7F6E	; $B16C
	ld a,$1C	; $B16F
	call fn_01_7F65	; $B171
	call fn_03_B476	; $B174
	ld a,($C951)	; $B177
	and $E0	; $B17A
	cp $E0	; $B17C
	jr z,loc_03_B190	; $B17E
	call fn_01_7F11	; $B180
	ld a,$1D	; $B183
	call fn_01_7F65	; $B185
	call fn_01_7F6E	; $B188
	ld a,$0C	; $B18B
	jp loc_03_B41C	; $B18D

loc_03_B190:
	ld a,$1E	; $B190
	call fn_01_7F65	; $B192
	call fn_03_B476	; $B195
	ld a,$1F	; $B198
	ld hl,$B5BC	; $B19A
	call fn_03_B3BA	; $B19D
	jp c,loc_03_B40F	; $B1A0
	ld a,($C64D)	; $B1A3
	cp $08	; $B1A6
	jr z,loc_03_B1B5	; $B1A8
	ld a,$21	; $B1AA
	call fn_01_7F65	; $B1AC
	call fn_03_B476	; $B1AF
	jp loc_03_B40F	; $B1B2

loc_03_B1B5:
	ld hl,karma	; $B1B5
	ld b,$08	; $B1B8
	xor a	; $B1BA

loc_03_B1BB:
	or (hl)	; $B1BB
	inc hl	; $B1BC
	djnz loc_03_B1BB	; $B1BD
	jr z,loc_03_B1CC	; $B1BF
	ld a,$22	; $B1C1
	call fn_01_7F65	; $B1C3
	call fn_01_7F6E	; $B1C6
	jp loc_03_B40F	; $B1C9

loc_03_B1CC:
	ld a,$23	; $B1CC
	call fn_01_7F65	; $B1CE
	call fn_03_B476	; $B1D1
	ld hl,$9CCE	; $B1D4
	call fn_00_0830	; $B1D7
	call wait_vblank	; $B1DA
	ld a,$13	; $B1DD
	ld hl,$8BEE	; $B1DF
	ld de,$0020	; $B1E2
	call fn_01_7E78	; $B1E5
	ld (ix+$16),$0B	; $B1E8
	ld (ix+$17),$00	; $B1EC

loc_03_B1F0:
	ld a,$24	; $B1F0
	call fn_01_7F65	; $B1F2
	call fn_01_7F6E	; $B1F5
	ld a,(ix+$17)	; $B1F8
	push af	; $B1FB
	ld hl,$B5CF	; $B1FC
	rst $20	; $B1FF
	pop af	; $B200
	call fn_03_B3BA	; $B201
	jr nc,loc_03_B217	; $B204
	call fn_01_7F6E	; $B206
	ld a,$25	; $B209
	call fn_01_7F65	; $B20B
	call fn_01_7F6E	; $B20E
	ld a,(ix+$17)	; $B211
	jp loc_03_B41C	; $B214

loc_03_B217:
	ld hl,$C023	; $B217
	ld de,$C043	; $B21A
	ld bc,$0010	; $B21D
	ldir	; $B220
	ld a,(ix+$17)	; $B222
	ld hl,$C048	; $B225
	rst $30	; $B228
	ld (hl),$3F	; $B229
	ld hl,$C043	; $B22B
	ld bc,$0101	; $B22E
	call fn_00_0A22	; $B231
	call wait_flag	; $B234
	ld a,(ix+$17)	; $B237
	cp $07	; $B23A
	jr nz,loc_03_B246	; $B23C
	ld a,$26	; $B23E
	call fn_01_7F65	; $B240
	call fn_03_B476	; $B243

loc_03_B246:
	inc (ix+$17)	; $B246
	dec (ix+$16)	; $B249
	jp nz,loc_03_B1F0	; $B24C
	ld a,$27	; $B24F
	call fn_01_7F65	; $B251
	ld a,$1C	; $B254
	call fn_00_05F1	; $B256
	call fn_01_7F44	; $B259
	ld a,$1C	; $B25C
	call fn_00_05F1	; $B25E
	call fn_01_7F44	; $B261
	call fn_03_B476	; $B264
	ld a,$28	; $B267
	call fn_01_7F65	; $B269
	call fn_03_B476	; $B26C
	ld a,$29	; $B26F
	call fn_01_7F65	; $B271
	call fn_03_B476	; $B274
	ld (ix+$16),$03	; $B277

loc_03_B27B:
	ld a,(ix+$16)	; $B27B
	cp $03	; $B27E
	ld a,$2A	; $B280
	jr z,loc_03_B285	; $B282
	inc a	; $B284

loc_03_B285:
	call fn_01_7F65	; $B285
	ld a,$2C	; $B288
	call fn_01_7F65	; $B28A
	ld a,$10	; $B28D
	call fn_01_7E72	; $B28F

loc_03_B292:
	call fn_03_B48C	; $B292
	call fn_01_7E6F	; $B295
	jr z,loc_03_B292	; $B298
	call fn_01_7F29	; $B29A
	call fn_01_7F6E	; $B29D
	jr c,loc_03_B2B0	; $B2A0
	call fn_01_7F6E	; $B2A2
	ld hl,$B5C6	; $B2A5
	ld de,$C065	; $B2A8
	call fn_01_7F2C	; $B2AB
	jr nc,loc_03_B2CD	; $B2AE

loc_03_B2B0:
	dec (ix+$16)	; $B2B0
	jr z,loc_03_B2C0	; $B2B3
	ld a,$2E	; $B2B5
	call fn_01_7F65	; $B2B7
	call fn_03_B476	; $B2BA
	jp loc_03_B27B	; $B2BD

loc_03_B2C0:
	ld a,$2F	; $B2C0
	call fn_01_7F65	; $B2C2
	call fn_01_7F6E	; $B2C5
	ld a,$0B	; $B2C8
	jp loc_03_B41C	; $B2CA

loc_03_B2CD:
	ld a,($C0CD)	; $B2CD
	or a	; $B2D0
	jp m,loc_03_B2F1	; $B2D1
	ld hl,$B3B4	; $B2D4
	rst $20	; $B2D7
	ld a,$1D	; $B2D8
	call fn_00_0C43	; $B2DA
	ld hl,(text_col)	; $B2DD
	push hl	; $B2E0
	ld hl,$0004	; $B2E1
	ld (text_col),hl	; $B2E4
	call vec_print_inline	; $B2E7
	.INCBIN "bank03.bin" SKIP $32EA READ 3	; text, 3 bytes
	pop hl	; $B2ED
	ld (text_col),hl	; $B2EE

loc_03_B2F1:
	ld a,$1E	; $B2F1
	call sleep	; $B2F3
	ld a,$1C	; $B2F6
	call fn_00_05F1	; $B2F8
	call fn_01_7F44	; $B2FB
	ld a,$1C	; $B2FE
	call fn_00_05F1	; $B300
	call fn_01_7F44	; $B303
	ld a,$1C	; $B306
	call fn_00_05F1	; $B308
	call fn_01_7F44	; $B30B
	ld a,$1E	; $B30E
	call sleep	; $B310
	ld a,$17	; $B313
	call fn_00_05F1	; $B315
	call fn_03_B492	; $B318
	ld hl,$9CBE	; $B31B
	call fn_00_0830	; $B31E
	call wait_vblank	; $B321
	ld a,$10	; $B324
	ld hl,$9E59	; $B326
	ld de,$0020	; $B329
	call fn_01_7E7B	; $B32C
	ld a,$1E	; $B32F
	call sleep	; $B331
	ld a,$0C	; $B334
	call fn_01_7F65	; $B336
	call fn_03_B476	; $B339
	ld a,$0D	; $B33C
	call fn_01_7F65	; $B33E
	call fn_03_B476	; $B341
	ld a,$0E	; $B344
	call fn_01_7F65	; $B346
	call fn_03_B476	; $B349
	ld a,$0F	; $B34C
	call fn_01_7F65	; $B34E
	call fn_03_B476	; $B351
	ld a,$10	; $B354
	call fn_01_7F65	; $B356
	call fn_03_B476	; $B359
	ld a,$11	; $B35C
	call fn_01_7F65	; $B35E
	call fn_03_B476	; $B361
	call fn_03_B55B	; $B364
	ld a,$12	; $B367
	call fn_01_7F65	; $B369
	call fn_03_B476	; $B36C
	ld hl,$9CDE	; $B36F
	call fn_00_0830	; $B372
	call wait_vblank	; $B375
	ld a,$15	; $B378
	ld hl,$94C1	; $B37A
	ld de,$0020	; $B37D
	call fn_01_7E7B	; $B380
	ld a,$13	; $B383
	call fn_01_7F65	; $B385
	call fn_03_B476	; $B388
	ld a,$14	; $B38B
	call fn_01_7F65	; $B38D
	call fn_03_B476	; $B390
	ld a,$15	; $B393
	call fn_01_7F65	; $B395
	call fn_03_B476	; $B398
	ld a,$16	; $B39B
	call fn_01_7F65	; $B39D
	ld hl,move_count	; $B3A0
	call fn_01_7E09	; $B3A3
	call fn_01_7F62	; $B3A6
	ld a,$17	; $B3A9
	call fn_01_7F65	; $B3AB
	ld a,$19	; $B3AE
	ld (game_mode),a	; $B3B0
	ret	; $B3B3

tbl_03_B3B4:
	.dw fn_03_8056
	.INCBIN "bank03.bin" SKIP $33B6 READ 4	; data, 4 bytes

fn_03_B3BA:
	ld (ix+$1C),a	; $B3BA
	ld (ix+$1D),$03	; $B3BD
	ld (ix+$1E),l	; $B3C1
	ld (ix+$1F),h	; $B3C4
	call task_enter_2	; $B3C7

loc_03_B3CA:
	ld a,(ix+$1C)	; $B3CA
	call fn_01_7F65	; $B3CD
	call fn_01_7F6E	; $B3D0
	ld a,$10	; $B3D3
	call fn_01_7E72	; $B3D5

loc_03_B3D8:
	call fn_03_B48C	; $B3D8
	call fn_01_7E6F	; $B3DB
	jr z,loc_03_B3D8	; $B3DE
	call fn_01_7F29	; $B3E0
	call fn_01_7F6E	; $B3E3
	jr c,loc_03_B3F9	; $B3E6
	call fn_01_7F6E	; $B3E8
	ld hl,$C065	; $B3EB
	ld e,(ix+$1E)	; $B3EE
	ld d,(ix+$1F)	; $B3F1
	call fn_01_7F2C	; $B3F4
	jr nc,loc_03_B40C	; $B3F7

loc_03_B3F9:
	dec (ix+$1D)	; $B3F9
	scf	; $B3FC
	jr z,loc_03_B40C	; $B3FD
	call fn_01_7F11	; $B3FF
	ld a,$18	; $B402
	call fn_01_7F65	; $B404
	call fn_03_B476	; $B407
	jr loc_03_B3CA	; $B40A

loc_03_B40C:
	jp task_return_2	; $B40C

loc_03_B40F:
	call fn_01_7F11	; $B40F
	ld a,$0B	; $B412
	call fn_01_7F65	; $B414
	call fn_01_7F6E	; $B417
	ld a,$0C	; $B41A

loc_03_B41C:
	ld hl,$B45C	; $B41C
	rst $20	; $B41F
	ld (party_x),hl	; $B420
	call fn_01_7F6E	; $B423
	call fn_01_7EA8	; $B426
	ld hl,$9CBE	; $B429
	call fn_00_0830	; $B42C
	call wait_vblank	; $B42F
	ld a,($C64D)	; $B432
	ld (party_size),a	; $B435
	xor a	; $B438
	ld (location),a	; $B439
	ld a,$11	; $B43C
	ld (game_mode),a	; $B43E
	xor a	; $B441
	ld ($C977),a	; $B442
	ld ($C976),a	; $B445
	ld a,$FF	; $B448
	ld ($C98F),a	; $B44A
	call fn_01_7F29	; $B44D
	ld a,$FF	; $B450
	ld ($C970),a	; $B452
	ld (map_mode),a	; $B455
	call fn_01_7E9F	; $B458
	ret	; $B45B

tbl_03_B45C:
	.dw $88E7
	.dw fn_01_6953
	.INCBIN "bank03.bin" SKIP $3460 READ 22	; data, 22 bytes

fn_03_B476:
	call task_enter_4	; $B476

loc_03_B479:
	call fn_03_B48C	; $B479
	ld a,(joy_pressed)	; $B47C
	and $30	; $B47F
	jr z,loc_03_B479	; $B481
	call fn_01_7F14	; $B483
	call fn_01_7F71	; $B486
	jp task_return_4	; $B489

fn_03_B48C:
	call fn_01_7F5C	; $B48C
	jp yield	; $B48F

fn_03_B492:
	call task_enter_2	; $B492
	ld (ix+$0A),$42	; $B495
	ld (ix+$0B),$43	; $B499
	ld (ix+$0C),$41	; $B49D
	ld (ix+$0D),$40	; $B4A1
	ld (ix+$16),$21	; $B4A5

loc_03_B4A9:
	call fn_01_7F59	; $B4A9
	ld a,(ix+$0A)	; $B4AC
	inc (ix+$0A)	; $B4AF
	inc (ix+$0A)	; $B4B2
	call fn_03_B4FB	; $B4B5
	call fn_01_7F59	; $B4B8
	ld a,(ix+$0C)	; $B4BB
	dec (ix+$0C)	; $B4BE
	dec (ix+$0C)	; $B4C1
	call fn_03_B4FB	; $B4C4
	call fn_01_7F59	; $B4C7
	ld a,(ix+$16)	; $B4CA
	cp $20	; $B4CD
	jr nc,loc_03_B4DD	; $B4CF
	ld a,(ix+$0B)	; $B4D1
	inc (ix+$0B)	; $B4D4
	inc (ix+$0B)	; $B4D7
	call fn_03_B4FB	; $B4DA

loc_03_B4DD:
	call fn_01_7F59	; $B4DD
	ld a,(ix+$16)	; $B4E0
	cp $20	; $B4E3
	jr nc,loc_03_B4F3	; $B4E5
	ld a,(ix+$0D)	; $B4E7
	dec (ix+$0D)	; $B4EA
	dec (ix+$0D)	; $B4ED
	call fn_03_B4FB	; $B4F0

loc_03_B4F3:
	dec (ix+$16)	; $B4F3
	jr nz,loc_03_B4A9	; $B4F6
	jp task_return_2	; $B4F8

fn_03_B4FB:
	ld b,a	; $B4FB
	and $07	; $B4FC
	ld hl,$B531	; $B4FE
	rst $28	; $B501
	ld c,a	; $B502
	ld a,b	; $B503
	and $F8	; $B504
	rrca	; $B506
	rrca	; $B507
	rrca	; $B508
	ld hl,$B539	; $B509
	rst $20	; $B50C
	ld b,$0B	; $B50D

loc_03_B50F:
	push bc	; $B50F
	rst $08	; $B510
	ld de,$C1C0	; $B511
	ld b,$30	; $B514

loc_03_B516:
	in a,($BE)	; $B516
	and c	; $B518
	ld (de),a	; $B519
	inc e	; $B51A
	djnz loc_03_B516	; $B51B
	ex de,hl	; $B51D
	rst $10	; $B51E
	ld hl,$C1C0	; $B51F
	ld bc,$30BE	; $B522

loc_03_B525:
	outi	; $B525
	jr nz,loc_03_B525	; $B527
	ld hl,$0030	; $B529
	add hl,de	; $B52C
	pop bc	; $B52D
	djnz loc_03_B50F	; $B52E
	ret	; $B530
	.INCBIN "bank03.bin" SKIP $3531 READ 42	; data, 42 bytes

fn_03_B55B:
	ld hl,$0020	; $B55B
	ld de,$022C	; $B55E
	ld b,$21	; $B561

loc_03_B563:
	call wait_vblank	; $B563
	ld c,$02	; $B566

loc_03_B568:
	push bc	; $B568
	push de	; $B569
	push hl	; $B56A
	ld b,$10	; $B56B

loc_03_B56D:
	push bc	; $B56D
	ld b,$02	; $B56E

loc_03_B570:
	rst $10	; $B570
	ex de,hl	; $B571
	xor a	; $B572
	call fn_00_0262	; $B573
	out ($BE),a	; $B576
	call fn_00_0262	; $B578
	out ($BE),a	; $B57B
	call fn_00_0262	; $B57D
	out ($BE),a	; $B580
	call fn_00_0262	; $B582
	out ($BE),a	; $B585
	djnz loc_03_B570	; $B587
	ld bc,$0220	; $B589
	add hl,bc	; $B58C
	ex de,hl	; $B58D
	add hl,bc	; $B58E
	ex de,hl	; $B58F
	pop bc	; $B590
	djnz loc_03_B56D	; $B591
	ld b,$02	; $B593

loc_03_B595:
	push bc	; $B595
	rst $10	; $B596
	ex de,hl	; $B597
	ld hl,$B5B8	; $B598
	ld bc,$04BE	; $B59B

loc_03_B59E:
	outi	; $B59E
	jr nz,loc_03_B59E	; $B5A0
	pop bc	; $B5A2
	djnz loc_03_B595	; $B5A3
	pop hl	; $B5A5
	pop de	; $B5A6
	ld bc,$0008	; $B5A7
	add hl,bc	; $B5AA
	ex de,hl	; $B5AB
	ld bc,$FFF8	; $B5AC
	add hl,bc	; $B5AF
	ex de,hl	; $B5B0
	pop bc	; $B5B1
	dec c	; $B5B2
	jr nz,loc_03_B568	; $B5B3
	djnz loc_03_B563	; $B5B5
	ret	; $B5B7
	.INCBIN "bank03.bin" SKIP $35B8 READ 23	; data, 23 bytes

tbl_03_B5CF:
	.dw fn_03_B5E5
	.dw fn_03_B5ED
	.dw fn_03_B5F8
	.dw fn_03_B5FE
	.dw fn_03_B606
	.dw fn_03_B610
	.dw fn_03_B616
	.dw fn_03_B623
	.dw fn_03_B62C
	.dw fn_03_B632
	.dw fn_03_B637

fn_03_B5E5:
	ld c,b	; $B5E5
	ld l,a	; $B5E6
	ld l,(hl)	; $B5E7
	ld h,l	; $B5E8
	ld (hl),e	; $B5E9
	ld (hl),h	; $B5EA
	ld a,c	; $B5EB
	nop	; $B5EC

fn_03_B5ED:
	ld b,e	; $B5ED
	ld l,a	; $B5EE
	ld l,l	; $B5EF
	ld (hl),b	; $B5F0
	ld h,c	; $B5F1
	ld (hl),e	; $B5F2
	ld (hl),e	; $B5F3
	ld l,c	; $B5F4
	ld l,a	; $B5F5
	ld l,(hl)	; $B5F6
	nop	; $B5F7

fn_03_B5F8:
	ld d,(hl)	; $B5F8
	ld h,c	; $B5F9
	ld l,h	; $B5FA
	ld l,a	; $B5FB
	ld (hl),d	; $B5FC
	nop	; $B5FD

fn_03_B5FE:
	ld c,d	; $B5FE
	ld (hl),l	; $B5FF
	ld (hl),e	; $B600
	ld (hl),h	; $B601
	ld l,c	; $B602
	ld h,e	; $B603
	ld h,l	; $B604
	nop	; $B605

fn_03_B606:
	ld d,e	; $B606
	ld h,c	; $B607
	ld h,e	; $B608
	ld (hl),d	; $B609
	ld l,c	; $B60A
	ld h,(hl)	; $B60B
	ld l,c	; $B60C
	ld h,e	; $B60D
	ld h,l	; $B60E
	nop	; $B60F

fn_03_B610:
	ld c,b	; $B610
	ld l,a	; $B611
	ld l,(hl)	; $B612
	ld l,a	; $B613
	ld (hl),d	; $B614
	nop	; $B615

fn_03_B616:
	ld d,e	; $B616
	ld (hl),b	; $B617
	ld l,c	; $B618
	ld (hl),d	; $B619
	ld l,c	; $B61A
	ld (hl),h	; $B61B
	ld (hl),l	; $B61C
	ld h,c	; $B61D
	ld l,h	; $B61E
	ld l,c	; $B61F
	ld (hl),h	; $B620
	ld a,c	; $B621
	nop	; $B622

fn_03_B623:
	ld c,b	; $B623
	ld (hl),l	; $B624
	ld l,l	; $B625
	ld l,c	; $B626
	ld l,h	; $B627
	ld l,c	; $B628
	ld (hl),h	; $B629
	ld a,c	; $B62A
	nop	; $B62B

fn_03_B62C:
	ld d,h	; $B62C
	ld (hl),d	; $B62D
	ld (hl),l	; $B62E
	ld (hl),h	; $B62F
	ld l,b	; $B630
	nop	; $B631

fn_03_B632:
	ld c,h	; $B632
	ld l,a	; $B633
	halt	; $B634
	ld h,l	; $B635
	nop	; $B636

fn_03_B637:
	ld b,e	; $B637
	ld l,a	; $B638
	ld (hl),l	; $B639
	ld (hl),d	; $B63A
	ld h,c	; $B63B
	ld h,a	; $B63C
	ld h,l	; $B63D
	nop	; $B63E
	ld a,$3C	; $B63F
	call sleep	; $B641
	ld a,$01	; $B644
	call play_music	; $B646
	xor a	; $B649
	ld ($DD00),a	; $B64A

loc_03_B64D:
	call yield	; $B64D
	ld a,($DD00)	; $B650
	or a	; $B653
	jr z,loc_03_B64D	; $B654
	ld a,($C76C)	; $B656
	call fn_03_B6BF	; $B659
	ld a,$10	; $B65C
	call fn_03_B6BF	; $B65E
	ld (ix+$16),$00	; $B661

loc_03_B665:
	ld a,(ix+$16)	; $B665
	ld hl,$C76C	; $B668
	cp (hl)	; $B66B
	jr z,loc_03_B673	; $B66C
	add a,$08	; $B66E
	call fn_03_B6BF	; $B670

loc_03_B673:
	inc (ix+$16)	; $B673
	ld a,(ix+$16)	; $B676
	cp $08	; $B679
	jr c,loc_03_B665	; $B67B
	ld a,$15	; $B67D
	call fn_03_B6BF	; $B67F
	ld a,$11	; $B682
	call fn_03_B6BF	; $B684
	ld a,$12	; $B687
	call fn_03_B6BF	; $B689
	ld a,$13	; $B68C
	call fn_03_B6BF	; $B68E
	ld a,$16	; $B691
	call fn_03_B6BF	; $B693
	ld a,$14	; $B696
	call fn_03_B6BF	; $B698
	ld a,$17	; $B69B
	call fn_03_B6BF	; $B69D
	ld a,$18	; $B6A0
	call fn_03_B6BF	; $B6A2
	call fn_03_B55B	; $B6A5
	ld hl,$9CDE	; $B6A8
	call fn_00_0830	; $B6AB
	call yield	; $B6AE
	ld a,$15	; $B6B1
	ld hl,$94C1	; $B6B3
	ld de,$0020	; $B6B6
	call fn_01_7E7B	; $B6B9

loc_03_B6BC:
	jp loc_03_B6BC	; $B6BC

fn_03_B6BF:
	ld (ix+$09),a	; $B6BF
	call task_enter_1	; $B6C2
	call fn_03_B709	; $B6C5
	ld a,$1F	; $B6C8
	ld hl,$AA00	; $B6CA
	ld c,(ix+$09)	; $B6CD
	call fn_00_0807	; $B6D0
	ld a,$1F	; $B6D3
	ld de,$D000	; $B6D5
	call fn_00_071A	; $B6D8
	ld hl,$D000	; $B6DB
	ld de,$D400	; $B6DE
	call fn_00_0723	; $B6E1
	ld hl,$D400	; $B6E4
	ld de,$D800	; $B6E7
	call fn_00_0723	; $B6EA
	call fn_01_7E12	; $B6ED
	call fn_01_7E45	; $B6F0
	ld hl,$9CEE	; $B6F3
	call fn_00_0830	; $B6F6
	call fn_03_B77D	; $B6F9
	ld a,$B4	; $B6FC
	call sleep	; $B6FE
	ld a,$B4	; $B701
	call sleep	; $B703
	jp task_return_1	; $B706

fn_03_B709:
	call task_enter_2	; $B709
	ld hl,$2220	; $B70C
	ld b,$0B	; $B70F

loc_03_B711:
	push bc	; $B711
	rst $08	; $B712
	ld de,$C1C0	; $B713
	ld bc,$30F0	; $B716

loc_03_B719:
	in a,($BE)	; $B719
	and c	; $B71B
	ld (de),a	; $B71C
	inc e	; $B71D
	djnz loc_03_B719	; $B71E
	ex de,hl	; $B720
	rst $10	; $B721
	ld hl,$C1C0	; $B722
	ld bc,$30BE	; $B725

loc_03_B728:
	outi	; $B728
	jr nz,loc_03_B728	; $B72A
	ld hl,$0030	; $B72C
	add hl,de	; $B72F
	pop bc	; $B730
	djnz loc_03_B711	; $B731
	ld hl,$B779	; $B733
	ld bc,$04BE	; $B736

loc_03_B739:
	outi	; $B739
	jr nz,loc_03_B739	; $B73B
	ld (ix+$17),$11	; $B73D

loc_03_B741:
	ld hl,$CA44	; $B741
	ld de,$CA42	; $B744
	ld a,$11	; $B747

loc_03_B749:
	ex af,af'	; $B749
	ld bc,$0020	; $B74A
	ldir	; $B74D
	xor a	; $B74F
	ld (de),a	; $B750
	inc e	; $B751
	ld (de),a	; $B752
	dec e	; $B753
	ld bc,$0020	; $B754
	add hl,bc	; $B757
	ex de,hl	; $B758
	add hl,bc	; $B759
	ex de,hl	; $B75A
	ex af,af'	; $B75B
	dec a	; $B75C
	jr nz,loc_03_B749	; $B75D
	ld hl,$0529	; $B75F
	ld ($CE62),hl	; $B762
	ld de,$CA42	; $B765
	ld bc,$1111	; $B768
	call queue_text_copy	; $B76B
	call yield	; $B76E
	dec (ix+$17)	; $B771
	jr nz,loc_03_B741	; $B774
	jp task_return_2	; $B776
	.INCBIN "bank03.bin" SKIP $3779 READ 4	; data, 4 bytes

fn_03_B77D:
	call task_enter_2	; $B77D
	ld hl,$B9AC	; $B780
	call fn_01_7EAE	; $B783
	ld (ix+$1A),$62	; $B786
	ld (ix+$1B),$38	; $B78A
	ld (ix+$1C),$02	; $B78E
	ld (ix+$17),$11	; $B792

loc_03_B796:
	ld hl,$CA42	; $B796
	ld e,(ix+$1A)	; $B799
	ld d,(ix+$1B)	; $B79C
	ld b,$11	; $B79F

loc_03_B7A1:
	push bc	; $B7A1
	push de	; $B7A2
	push hl	; $B7A3
	ld c,(ix+$1C)	; $B7A4
	ld b,$00	; $B7A7
	call fn_00_0463	; $B7A9
	pop hl	; $B7AC
	pop de	; $B7AD
	ld bc,$0040	; $B7AE
	add hl,bc	; $B7B1
	ex de,hl	; $B7B2
	add hl,bc	; $B7B3
	ex de,hl	; $B7B4
	pop bc	; $B7B5
	djnz loc_03_B7A1	; $B7B6
	call yield	; $B7B8
	dec (ix+$1A)	; $B7BB
	dec (ix+$1A)	; $B7BE
	inc (ix+$1C)	; $B7C1
	inc (ix+$1C)	; $B7C4
	dec (ix+$17)	; $B7C7
	jr nz,loc_03_B796	; $B7CA
	jp task_return_2	; $B7CC

sub_03_B7CF:
	ld a,$05	; $B7CF
	ld hl,$B8E4	; $B7D1
	ld de,$3E80	; $B7D4
	call fn_00_0788	; $B7D7

loc_03_B7DA:
	ld a,$05	; $B7DA
	ld hl,$B8F0	; $B7DC
	ld de,$0888	; $B7DF
	call fn_00_09BE	; $B7E2
	call yield	; $B7E5
	jr loc_03_B7DA	; $B7E8

fn_03_B7EA:
	ld a,d	; $B7EA
	or e	; $B7EB
	jr z,loc_03_B801	; $B7EC
	push bc	; $B7EE
	ld a,h	; $B7EF
	ld b,l	; $B7F0
	ld hl,$0000	; $B7F1
	call fn_03_B805	; $B7F4
	ld c,a	; $B7F7
	ld a,b	; $B7F8
	call fn_03_B805	; $B7F9
	ex de,hl	; $B7FC
	ld h,c	; $B7FD
	ld l,a	; $B7FE
	pop bc	; $B7FF
	ret	; $B800

loc_03_B801:
	ld hl,$0000	; $B801
	ret	; $B804

fn_03_B805:
	add a,a	; $B805
	adc hl,hl	; $B806
	inc a	; $B808
	sbc hl,de	; $B809
	jr nc,loc_03_B80F	; $B80B
	add hl,de	; $B80D
	dec a	; $B80E

loc_03_B80F:
	add a,a	; $B80F
	adc hl,hl	; $B810
	inc a	; $B812
	sbc hl,de	; $B813
	jr nc,loc_03_B819	; $B815
	add hl,de	; $B817
	dec a	; $B818

loc_03_B819:
	add a,a	; $B819
	adc hl,hl	; $B81A
	inc a	; $B81C
	sbc hl,de	; $B81D
	jr nc,loc_03_B823	; $B81F
	add hl,de	; $B821
	dec a	; $B822

loc_03_B823:
	add a,a	; $B823
	adc hl,hl	; $B824
	inc a	; $B826
	sbc hl,de	; $B827
	jr nc,loc_03_B82D	; $B829
	add hl,de	; $B82B
	dec a	; $B82C

loc_03_B82D:
	add a,a	; $B82D
	adc hl,hl	; $B82E
	inc a	; $B830
	sbc hl,de	; $B831
	jr nc,loc_03_B837	; $B833
	add hl,de	; $B835
	dec a	; $B836

loc_03_B837:
	add a,a	; $B837
	adc hl,hl	; $B838
	inc a	; $B83A
	sbc hl,de	; $B83B
	jr nc,loc_03_B841	; $B83D
	add hl,de	; $B83F
	dec a	; $B840

loc_03_B841:
	add a,a	; $B841
	adc hl,hl	; $B842
	inc a	; $B844
	sbc hl,de	; $B845
	jr nc,loc_03_B84B	; $B847
	add hl,de	; $B849
	dec a	; $B84A

loc_03_B84B:
	add a,a	; $B84B
	adc hl,hl	; $B84C
	inc a	; $B84E
	sbc hl,de	; $B84F
	jr nc,loc_03_B855	; $B851
	add hl,de	; $B853
	dec a	; $B854

loc_03_B855:
	ret	; $B855

fn_03_B856:
	push bc	; $B856
	ld b,a	; $B857
	add a,a	; $B858
	ld c,a	; $B859
	add a,a	; $B85A
	add a,a	; $B85B
	add a,b	; $B85C
	add a,c	; $B85D
	pop bc	; $B85E
	ret	; $B85F

fn_03_B860:
	push bc	; $B860
	add a,a	; $B861
	add a,a	; $B862
	ld b,a	; $B863
	add a,a	; $B864
	add a,b	; $B865
	pop bc	; $B866
	ret	; $B867

fn_03_B868:
	push bc	; $B868
	push de	; $B869
	push hl	; $B86A
	dec b	; $B86B

loc_03_B86C:
	ld a,(hl)	; $B86C
	or a	; $B86D
	jr z,loc_03_B876	; $B86E
	ld de,$0008	; $B870
	add hl,de	; $B873
	jr loc_03_B88E	; $B874

loc_03_B876:
	push bc	; $B876
	push hl	; $B877
	ld d,h	; $B878
	ld e,l	; $B879
	ld a,b	; $B87A
	ld bc,$0008	; $B87B
	add hl,bc	; $B87E
	add a,a	; $B87F
	add a,a	; $B880
	add a,a	; $B881
	ld c,a	; $B882
	ldir	; $B883
	xor a	; $B885
	ld b,$08	; $B886

loc_03_B888:
	ld (de),a	; $B888
	inc de	; $B889
	djnz loc_03_B888	; $B88A
	pop hl	; $B88C
	pop bc	; $B88D

loc_03_B88E:
	djnz loc_03_B86C	; $B88E
	pop hl	; $B890
	pop de	; $B891
	pop bc	; $B892
	ret	; $B893

fn_03_B894:
	ld hl,(party_x)	; $B894
	ld (iy+$01),l	; $B897
	ld (iy+$02),h	; $B89A
	ld a,($C959)	; $B89D
	ld (iy+$03),a	; $B8A0
	ld (iy+$00),a	; $B8A3
	ret	; $B8A6

fn_03_B8A7:
	push de	; $B8A7
	ld de,$0008	; $B8A8

loc_03_B8AB:
	call fn_03_B8B7	; $B8AB
	jr nc,loc_03_B8B5	; $B8AE
	add iy,de	; $B8B0
	djnz loc_03_B8AB	; $B8B2
	scf	; $B8B4

loc_03_B8B5:
	pop de	; $B8B5
	ret	; $B8B6

fn_03_B8B7:
	ld a,(party_x)	; $B8B7
	sub (iy+$01)	; $B8BA
	add a,$05	; $B8BD
	cp $0B	; $B8BF
	ret nc	; $B8C1
	ld a,(party_y)	; $B8C2
	sub (iy+$02)	; $B8C5
	add a,$05	; $B8C8
	cp $0B	; $B8CA
	ret	; $B8CC

fn_03_B8CD:
	push de	; $B8CD
	ld de,$0008	; $B8CE

loc_03_B8D1:
	call fn_03_B8DD	; $B8D1
	jr nz,loc_03_B8DB	; $B8D4
	add iy,de	; $B8D6
	djnz loc_03_B8D1	; $B8D8
	xor a	; $B8DA

loc_03_B8DB:
	pop de	; $B8DB
	ret	; $B8DC

fn_03_B8DD:
	ld a,(party_x)	; $B8DD
	cp (iy+$01)	; $B8E0
	ret nz	; $B8E3
	ld a,(party_y)	; $B8E4
	cp (iy+$02)	; $B8E7
	ret	; $B8EA

fn_03_B8EB:
	ld hl,$C699	; $B8EB
	ld b,$05	; $B8EE
	call fn_03_B868	; $B8F0
	ld a,($C6B9)	; $B8F3
	or a	; $B8F6
	jr z,loc_03_B937	; $B8F7
	ld iy,$C699	; $B8F9
	ld de,$0008	; $B8FD
	ld b,$05	; $B900

loc_03_B902:
	call fn_03_B94C	; $B902
	jr z,loc_03_B90C	; $B905
	call fn_03_B8B7	; $B907
	jr nc,loc_03_B92B	; $B90A

loc_03_B90C:
	add iy,de	; $B90C
	djnz loc_03_B902	; $B90E
	ld iy,$C699	; $B910
	ld de,$0008	; $B914
	ld b,$05	; $B917

loc_03_B919:
	call fn_03_B94C	; $B919
	jr z,loc_03_B923	; $B91C
	call fn_03_B8DD	; $B91E
	jr nz,loc_03_B92B	; $B921

loc_03_B923:
	add iy,de	; $B923
	djnz loc_03_B919	; $B925
	ld iy,$C699	; $B927

loc_03_B92B:
	ld (iy+$00),$00	; $B92B
	ld hl,$C699	; $B92F
	ld b,$05	; $B932
	call fn_03_B868	; $B934

loc_03_B937:
	ld iy,$C6B9	; $B937

loc_03_B93B:
	ld (iy+$03),$00	; $B93B
	ld (iy+$00),$00	; $B93F
	ld (iy+$06),$00	; $B943
	ld (iy+$07),$00	; $B947
	ret	; $B94B

fn_03_B94C:
	ld a,($C973)	; $B94C
	cp (iy+$01)	; $B94F
	ret nz	; $B952
	ld a,($C974)	; $B953
	cp (iy+$02)	; $B956
	ret	; $B959

fn_03_B95A:
	ld hl,$C6C1	; $B95A
	ld b,$05	; $B95D
	call fn_03_B868	; $B95F
	ld a,($C6E1)	; $B962
	or a	; $B965
	jr z,loc_03_B98E	; $B966
	ld iy,$C6C1	; $B968
	ld b,$05	; $B96C
	call fn_03_B8A7	; $B96E
	jr nc,loc_03_B982	; $B971
	ld iy,$C6C1	; $B973
	ld b,$05	; $B977
	call fn_03_B8CD	; $B979
	jr nz,loc_03_B982	; $B97C
	ld iy,$C6C1	; $B97E

loc_03_B982:
	ld (iy+$00),$00	; $B982
	ld hl,$C6C1	; $B986
	ld b,$05	; $B989
	call fn_03_B868	; $B98B

loc_03_B98E:
	ld iy,$C6E1	; $B98E
	jr loc_03_B93B	; $B992

fn_03_B994:
	ld hl,$C6E9	; $B994
	ld b,$0D	; $B997
	call fn_03_B868	; $B999
	ld a,($C749)	; $B99C
	or a	; $B99F
	jr z,loc_03_B9C8	; $B9A0
	ld iy,$C6E9	; $B9A2
	ld b,$0D	; $B9A6
	call fn_03_B8A7	; $B9A8
	jr nc,loc_03_B9BC	; $B9AB
	ld iy,$C6E9	; $B9AD
	ld b,$0D	; $B9B1
	call fn_03_B8CD	; $B9B3
	jr nz,loc_03_B9BC	; $B9B6
	ld iy,$C6E9	; $B9B8

loc_03_B9BC:
	ld (iy+$00),$00	; $B9BC
	ld hl,$C6E9	; $B9C0
	ld b,$0D	; $B9C3
	call fn_03_B868	; $B9C5

loc_03_B9C8:
	ld iy,$C749	; $B9C8
	jp loc_03_B93B	; $B9CC

loc_03_B9CF:
	push bc	; $B9CF
	push de	; $B9D0
	push hl	; $B9D1
	push iy	; $B9D2
	call fn_03_B8EB	; $B9D4
	call fn_03_B95A	; $B9D7
	pop iy	; $B9DA
	pop hl	; $B9DC
	pop de	; $B9DD
	pop bc	; $B9DE
	ret	; $B9DF

loc_03_B9E0:
	push bc	; $B9E0
	push de	; $B9E1
	push hl	; $B9E2
	ld hl,$B9F2	; $B9E3
	ld de,$C691	; $B9E6
	ld bc,$0008	; $B9E9
	ldir	; $B9EC
	pop hl	; $B9EE
	pop de	; $B9EF
	pop bc	; $B9F0
	ret	; $B9F1
	.INCBIN "bank03.bin" SKIP $39F2 READ 8	; data, 8 bytes

loc_03_B9FA:
	push hl	; $B9FA
	ld a,$18	; $B9FB
	ld ($C691),a	; $B9FD
	ld ($C694),a	; $BA00
	ld hl,(party_x)	; $BA03
	ld ($C692),hl	; $BA06
	pop hl	; $BA09
	ret	; $BA0A

loc_03_BA0B:
	push bc	; $BA0B
	push de	; $BA0C
	push hl	; $BA0D
	push iy	; $BA0E
	call fn_03_B8EB	; $BA10
	ld hl,($C97A)	; $BA13
	ld (iy+$01),l	; $BA16
	ld (iy+$02),h	; $BA19
	ld (iy+$03),$10	; $BA1C
	ld (iy+$00),$10	; $BA20
	pop iy	; $BA24
	pop hl	; $BA26
	pop de	; $BA27
	pop bc	; $BA28
	ret	; $BA29

loc_03_BA2A:
	push bc	; $BA2A
	push de	; $BA2B
	push hl	; $BA2C
	push iy	; $BA2D
	call fn_03_B8EB	; $BA2F
	call fn_03_B894	; $BA32
	pop iy	; $BA35
	pop hl	; $BA37
	pop de	; $BA38
	pop bc	; $BA39
	ret	; $BA3A

loc_03_BA3B:
	push bc	; $BA3B
	push de	; $BA3C
	push hl	; $BA3D
	push iy	; $BA3E
	call fn_03_B95A	; $BA40
	call fn_03_B894	; $BA43
	pop iy	; $BA46
	pop hl	; $BA48
	pop de	; $BA49
	pop bc	; $BA4A
	ret	; $BA4B

loc_03_BA4C:
	push bc	; $BA4C
	push de	; $BA4D
	push hl	; $BA4E
	push iy	; $BA4F
	call fn_03_B994	; $BA51
	ld hl,($C97A)	; $BA54
	ld (iy+$01),l	; $BA57
	ld (iy+$02),h	; $BA5A
	ld (iy+$03),$3C	; $BA5D
	ld (iy+$00),$3C	; $BA61
	pop iy	; $BA65
	pop hl	; $BA67
	pop de	; $BA68
	pop bc	; $BA69
	ret	; $BA6A
	.INCBIN "bank03.bin" SKIP $3A6B READ 1429	; data, 1429 bytes
