; Bank 2 (ROM $08000-$0BFFF), mapped at $8000
.BANK 2 SLOT 2
.ORG $0000


loc_02_8000:
	call task_enter_1	; $8000
	call fn_01_40C9	; $8003
	call print_inline	; $8006
	.INCBIN "bank02.bin" SKIP $0009 READ 9	; text, 9 bytes
	ld hl,$9592	; $8012
	call fn_00_2E7F	; $8015
	ld hl,$0B0A	; $8018
	call fn_00_06DA	; $801B
	ld (ix+$1A),$00	; $801E

loc_02_8022:
	call frame_tick	; $8022
	ld c,$08	; $8025
	ld de,$0102	; $8027
	call fn_01_49F9	; $802A
	jr z,loc_02_8022	; $802D
	jp c,loc_02_8081	; $802F
	ld hl,$9582	; $8032
	call fn_02_806B	; $8035
	ld a,(map_mode)	; $8038
	cp $02	; $803B
	ld hl,$804B	; $803D
	jr nz,loc_02_8045	; $8040
	ld hl,$805B	; $8042

loc_02_8045:
	ld a,(ix+$1A)	; $8045
	jp fn_00_0018	; $8048
	.INCBIN "bank02.bin" SKIP $004B READ 32	; data, 32 bytes

fn_02_806B:
	ld a,(ix+$1A)	; $806B
	rst $20	; $806E
	jp print_text_hl	; $806F

loc_02_8072:
	call fn_01_46DF	; $8072
	jr loc_02_8084	; $8075

loc_02_8077:
	call fn_01_473F	; $8077
	jr loc_02_8084	; $807A

loc_02_807C:
	call fn_01_46F1	; $807C
	jr loc_02_8084	; $807F

loc_02_8081:
	call fn_01_472D	; $8081

loc_02_8084:
	call fn_01_4324	; $8084
	call wait_vblank	; $8087
	call fn_00_06D4	; $808A
	or a	; $808D
	jp task_return_1	; $808E
	call fn_01_4324	; $8091
	call print_newline	; $8094
	call fn_01_4C28	; $8097

loc_02_809A:
	call frame_tick	; $809A
	call fn_01_4C3C	; $809D
	jr z,loc_02_809A	; $80A0
	jp c,loc_02_8081	; $80A2
	ld a,(map_mode)	; $80A5
	cp $01	; $80A8
	jp nz,loc_02_810D	; $80AA
	ld a,(ix+$1A)	; $80AD
	call fn_01_40B9	; $80B0
	ex de,hl	; $80B3
	ld hl,(party_x)	; $80B4
	call fn_02_8101	; $80B7
	jr nc,loc_02_810D	; $80BA
	push hl	; $80BC
	call fn_02_B5A4	; $80BD
	ld a,(hl)	; $80C0
	pop hl	; $80C1
	cp $60	; $80C2
	jr c,loc_02_80E6	; $80C4
	cp $7F	; $80C6
	jr nc,loc_02_80E6	; $80C8
	call fn_02_8101	; $80CA
	jp nc,loc_02_810D	; $80CD
	call fn_01_42E3	; $80D0
	jp c,loc_02_810D	; $80D3
	call fn_01_48F7	; $80D6
	ld a,(iy+$03)	; $80D9
	cp $52	; $80DC
	jp nz,loc_02_810D	; $80DE
	ld a,h	; $80E1
	sub d	; $80E2
	jp loc_02_9652	; $80E3

loc_02_80E6:
	call fn_01_42E3	; $80E6
	jp c,loc_02_810D	; $80E9
	call fn_01_48F7	; $80EC
	ld a,(iy+$07)	; $80EF
	or a	; $80F2
	jp z,loc_02_810D	; $80F3
	ld a,(iy+$03)	; $80F6
	cp $5E	; $80F9
	jp z,loc_02_812B	; $80FB
	jp loc_01_6DCE	; $80FE

fn_02_8101:
	ld a,l	; $8101
	add a,e	; $8102
	ld l,a	; $8103
	cp $20	; $8104
	ret nc	; $8106
	ld a,h	; $8107
	add a,d	; $8108
	ld h,a	; $8109
	cp $20	; $810A
	ret	; $810C

loc_02_810D:
	call fn_01_40D1	; $810D
	call print_inline	; $8110
	.INCBIN "bank02.bin" SKIP $0113 READ 21	; text, 21 bytes
	jp loc_02_8084	; $8128

loc_02_812B:
	ld a,$1A	; $812B
	ld (message_bank),a	; $812D
	ld hl,$80EC	; $8130
	ld (message_table),hl	; $8133
	ld a,$07	; $8136
	call play_music_later	; $8138
	ld a,($C963)	; $813B
	or a	; $813E
	jr nz,loc_02_815B	; $813F
	ld a,$FF	; $8141
	ld ($C963),a	; $8143
	ld a,$2D	; $8146
	call print_message	; $8148
	call print_avatar_name	; $814B
	ld a,$2E	; $814E
	call fn_02_8453	; $8150
	ld a,$2F	; $8153
	call print_message	; $8155
	jp loc_02_81B5	; $8158

loc_02_815B:
	ld a,($C76D)	; $815B
	cp $44	; $815E
	jr nz,loc_02_8184	; $8160
	call print_newline	; $8162
	ld a,$47	; $8165
	ld ($C76D),a	; $8167
	call fn_01_548D	; $816A
	call print_avatar_name	; $816D
	ld a,$30	; $8170
	call print_message	; $8172
	call fn_02_8411	; $8175
	call fn_02_8296	; $8178
	call fn_01_4324	; $817B
	call wait_button	; $817E
	call print_newline	; $8181

loc_02_8184:
	ld a,$31	; $8184
	call print_message	; $8186
	call print_avatar_name	; $8189
	ld a,(party_size)	; $818C
	dec a	; $818F
	jr z,loc_02_81AC	; $8190
	dec a	; $8192
	jr z,loc_02_819C	; $8193
	ld a,$32	; $8195
	call print_message	; $8197
	jr loc_02_81AC	; $819A

loc_02_819C:
	ld a,$33	; $819C
	call print_message	; $819E
	ld hl,$C77B	; $81A1
	call print_text_hl	; $81A4
	ld a,$34	; $81A7
	call print_message	; $81A9

loc_02_81AC:
	call wait_button	; $81AC
	call print_newline	; $81AF
	call fn_02_82BD	; $81B2

loc_02_81B5:
	ld (ix+$1B),$00	; $81B5
	ld (ix+$1C),$00	; $81B9

loc_02_81BD:
	call fn_00_36A3	; $81BD

loc_02_81C0:
	call frame_tick	; $81C0
	ld a,(joy_pressed)	; $81C3
	and $7F	; $81C6
	ld hl,$81D0	; $81C8
	call switch_on_a	; $81CB
	jr loc_02_81C0	; $81CE

tbl_02_81D0:
	.db $01
	.dw loc_02_81DD
	.db $02
	.dw loc_02_8204
	.db $10
	.dw loc_02_8231
	.db $20
	.dw loc_02_823F
	.db $FF

loc_02_81DD:
	ld a,(ix+$1B)	; $81DD
	add a,(ix+$1C)	; $81E0
	jr z,loc_02_81C0	; $81E3
	call fn_01_40D9	; $81E5
	ld a,(ix+$1C)	; $81E8
	or a	; $81EB
	jr z,loc_02_81FF	; $81EC
	call fn_02_840B	; $81EE
	call fn_01_49E0	; $81F1
	dec (ix+$1C)	; $81F4
	call fn_02_840B	; $81F7
	call fn_01_49D5	; $81FA
	jr loc_02_81C0	; $81FD

loc_02_81FF:
	dec (ix+$1B)	; $81FF
	jr loc_02_81BD	; $8202

loc_02_8204:
	ld a,(ix+$1B)	; $8204
	add a,(ix+$1C)	; $8207
	ld hl,$C5E1	; $820A
	rst $28	; $820D
	or a	; $820E
	jr z,loc_02_81C0	; $820F
	call fn_01_40D9	; $8211
	ld a,(ix+$1C)	; $8214
	cp $07	; $8217
	jr nc,loc_02_822C	; $8219
	call fn_02_840B	; $821B
	call fn_01_49E0	; $821E
	inc (ix+$1C)	; $8221
	call fn_02_840B	; $8224
	call fn_01_49D5	; $8227
	jr loc_02_81C0	; $822A

loc_02_822C:
	inc (ix+$1B)	; $822C
	jr loc_02_81BD	; $822F

loc_02_8231:
	ld a,(ix+$1B)	; $8231
	add a,(ix+$1C)	; $8234
	jr z,loc_02_81C0	; $8237
	call fn_01_40D9	; $8239
	jp loc_02_81B5	; $823C

loc_02_823F:
	call fn_01_40C9	; $823F
	ld a,(ix+$1B)	; $8242
	add a,(ix+$1C)	; $8245
	ld hl,$C5E0	; $8248
	rst $28	; $824B
	ld (ix+$1D),a	; $824C
	add a,$3B	; $824F
	call print_message	; $8251
	call fn_01_54CB	; $8254
	ld a,(ix+$1D)	; $8257
	cp $01	; $825A
	jp z,loc_02_8359	; $825C
	cp $05	; $825F
	jp z,loc_02_8375	; $8261
	cp $06	; $8264
	jp z,loc_02_83A3	; $8266
	dec a	; $8269
	call fn_02_8453	; $826A
	ld de,$8422	; $826D

loc_02_8270:
	ld a,(de)	; $8270
	inc de	; $8271
	or a	; $8272
	jp z,loc_02_828E	; $8273
	cp (ix+$1D)	; $8276
	jr nz,loc_02_828B	; $8279
	ld a,(de)	; $827B
	ld b,a	; $827C
	ld hl,$C5E0	; $827D

loc_02_8280:
	ld a,(hl)	; $8280
	or a	; $8281
	jr z,loc_02_828A	; $8282
	cp b	; $8284
	jr z,loc_02_828B	; $8285
	inc hl	; $8287
	jr loc_02_8280	; $8288

loc_02_828A:
	ld (hl),b	; $828A

loc_02_828B:
	inc de	; $828B
	jr loc_02_8270	; $828C

loc_02_828E:
	ld a,$35	; $828E
	call print_message	; $8290
	jp loc_02_81BD	; $8293

fn_02_8296:
	ld iy,party	; $8296
	ld de,$001D	; $829A
	ld a,(party_size)	; $829D
	ld b,a	; $82A0
	ld a,$44	; $82A1

loc_02_82A3:
	cp (iy+$1C)	; $82A3
	jr z,loc_02_82B8	; $82A6
	ld (iy+$1C),$47	; $82A8
	ld l,(iy+$02)	; $82AC
	ld h,(iy+$03)	; $82AF
	ld (iy+$00),l	; $82B2
	ld (iy+$01),h	; $82B5

loc_02_82B8:
	add iy,de	; $82B8
	djnz loc_02_82A3	; $82BA
	ret	; $82BC

fn_02_82BD:
	ld iy,party	; $82BD
	ld bc,(party_size)	; $82C1

loc_02_82C5:
	ld e,(iy+$04)	; $82C5
	ld d,(iy+$05)	; $82C8
	ld hl,$E700	; $82CB
	ld b,$08	; $82CE

loc_02_82D0:
	push hl	; $82D0
	add hl,de	; $82D1
	pop hl	; $82D2
	jr c,loc_02_82DD	; $82D3
	sra h	; $82D5
	rr l	; $82D7
	djnz loc_02_82D0	; $82D9
	jr loc_02_833E	; $82DB

loc_02_82DD:
	ld a,(iy+$0A)	; $82DD
	cp b	; $82E0
	jr nc,loc_02_833E	; $82E1
	ld (iy+$0A),b	; $82E3
	ld l,b	; $82E6
	ld h,$64	; $82E7
	call multiply_8x8	; $82E9
	ld (iy+$00),l	; $82EC
	ld (iy+$01),h	; $82EF
	ld (iy+$02),l	; $82F2
	ld (iy+$03),h	; $82F5
	ld (iy+$1C),$47	; $82F8
	ld d,(iy+$06)	; $82FC
	call fn_02_834C	; $82FF
	ld (iy+$06),a	; $8302
	ld d,(iy+$07)	; $8305
	call fn_02_834C	; $8308
	ld (iy+$07),a	; $830B
	ld d,(iy+$08)	; $830E
	call fn_02_834C	; $8311
	ld (iy+$08),a	; $8314
	call fn_01_548D	; $8317
	push iy	; $831A
	pop hl	; $831C
	ld de,$000D	; $831D
	add hl,de	; $8320
	call print_text_hl	; $8321
	ld a,$2B	; $8324
	call print_message	; $8326
	ld a,(iy+$0A)	; $8329
	add a,$30	; $832C
	call print_char	; $832E
	call print_inline	; $8331
	.INCBIN "bank02.bin" SKIP $0334 READ 4	; text, 4 bytes
	call fn_02_8411	; $8338
	call fn_01_4324	; $833B

loc_02_833E:
	ld de,$001D	; $833E
	add iy,de	; $8341
	dec c	; $8343
	jp nz,loc_02_82C5	; $8344
	ld a,$2C	; $8347
	jp print_message	; $8349

fn_02_834C:
	call random	; $834C
	and $07	; $834F
	inc a	; $8351
	add a,d	; $8352
	cp $33	; $8353
	ret c	; $8355
	ld a,$32	; $8356
	ret	; $8358

loc_02_8359:
	ld a,$36	; $8359
	call print_message	; $835B
	ld a,(party_size)	; $835E
	cp $02	; $8361
	ld a,$73	; $8363
	call nc,print_char	; $8365
	ld a,$37	; $8368
	call print_message	; $836A
	ld a,$06	; $836D
	call play_music_later	; $836F
	jp loc_02_8084	; $8372

loc_02_8375:
	ld a,$28	; $8375
	call print_message	; $8377
	call fn_01_4B5E	; $837A

loc_02_837D:
	call frame_tick	; $837D
	call fn_01_4B69	; $8380
	jr c,loc_02_837D	; $8383
	bit 0,(ix+$07)	; $8385
	jr nz,loc_02_8392	; $8389
	ld a,$29	; $838B
	call print_message	; $838D
	jr loc_02_839D	; $8390

loc_02_8392:
	ld a,$2A	; $8392
	call print_message	; $8394
	call fn_02_8411	; $8397
	call fn_02_8296	; $839A

loc_02_839D:
	call print_newline	; $839D
	jp loc_02_828E	; $83A0

loc_02_83A3:
	ld a,$1E	; $83A3
	call print_message	; $83A5
	ld hl,$8407	; $83A8
	call fn_02_BBC6	; $83AB
	ld a,$1F	; $83AE
	jr c,loc_02_8401	; $83B0
	ld a,(party_size)	; $83B2
	cp $01	; $83B5
	ld a,$20	; $83B7
	jr z,loc_02_8401	; $83B9
	ld a,($C956)	; $83BB
	or a	; $83BE
	ld a,$21	; $83BF
	jr z,loc_02_8401	; $83C1
	ld hl,karma	; $83C3
	ld b,$08	; $83C6

loc_02_83C8:
	ld a,(hl)	; $83C8
	or a	; $83C9
	jr z,loc_02_83D3	; $83CA
	inc hl	; $83CC
	djnz loc_02_83C8	; $83CD
	ld a,$22	; $83CF
	jr loc_02_8401	; $83D1

loc_02_83D3:
	ld a,($C955)	; $83D3
	or a	; $83D6
	ld a,$23	; $83D7
	jr z,loc_02_8401	; $83D9
	ld hl,karma	; $83DB
	xor a	; $83DE
	ld b,$08	; $83DF

loc_02_83E1:
	or (hl)	; $83E1
	inc hl	; $83E2
	djnz loc_02_83E1	; $83E3
	ld a,$24	; $83E5
	jr nz,loc_02_8401	; $83E7
	ld a,($C951)	; $83E9
	and $1C	; $83EC
	cp $1C	; $83EE
	ld a,$25	; $83F0
	jr nz,loc_02_8401	; $83F2
	ld a,($C951)	; $83F4
	and $E0	; $83F7
	cp $E0	; $83F9
	ld a,$26	; $83FB
	jr nz,loc_02_8401	; $83FD
	ld a,$27	; $83FF

loc_02_8401:
	call fn_02_8453	; $8401
	jp loc_02_828E	; $8404
	.INCBIN "bank02.bin" SKIP $0407 READ 4	; data, 4 bytes

fn_02_840B:
	ld a,(ix+$1C)	; $840B
	add a,a	; $840E
	inc a	; $840F
	ret	; $8410

fn_02_8411:
	ld a,$15	; $8411
	call fn_00_05F1	; $8413
	call wait_vblank	; $8416
	call fn_01_41A9	; $8419
	call wait_vblank	; $841C
	jp fn_01_41A9	; $841F
	.INCBIN "bank02.bin" SKIP $0422 READ 49	; data, 49 bytes

fn_02_8453:
	call task_enter_3	; $8453
	ld c,a	; $8456
	ld a,(message_bank)	; $8457
	ld hl,(message_table)	; $845A
	call fn_00_0807	; $845D
	ld (ix+$0E),l	; $8460
	ld (ix+$0F),h	; $8463

loc_02_8466:
	ld a,(message_bank)	; $8466
	ld l,(ix+$0E)	; $8469
	ld h,(ix+$0F)	; $846C
	call fn_00_0815	; $846F
	or a	; $8472
	jp z,task_return_3	; $8473
	ld a,(message_bank)	; $8476
	call fn_01_5597	; $8479
	ld (ix+$0E),l	; $847C
	ld (ix+$0F),h	; $847F
	call wait_button	; $8482
	call fn_01_54CB	; $8485
	jr loc_02_8466	; $8488
	call fn_01_4324	; $848A
	call print_newline	; $848D
	call fn_02_957C	; $8490
	jp z,loc_02_8077	; $8493
	call fn_01_4C28	; $8496

loc_02_8499:
	call frame_tick	; $8499
	call fn_01_4C3C	; $849C
	jr z,loc_02_8499	; $849F
	jp c,loc_02_8081	; $84A1
	ld a,(ix+$1A)	; $84A4
	call fn_01_40B9	; $84A7
	ld a,(party_x)	; $84AA
	add a,l	; $84AD
	ld l,a	; $84AE
	ld a,(party_y)	; $84AF
	add a,h	; $84B2
	ld h,a	; $84B3
	call fn_01_42E3	; $84B4
	jr c,loc_02_8504	; $84B7
	call fn_01_48F7	; $84B9
	ld a,(iy+$03)	; $84BC
	ld hl,$85CD	; $84BF
	ld bc,$0005	; $84C2
	cpir	; $84C5
	jr z,loc_02_8504	; $84C7
	ld a,(map_mode)	; $84C9
	cp $01	; $84CC
	jr nz,loc_02_84F6	; $84CE
	push iy	; $84D0
	ld iy,location_people	; $84D2
	ld de,$0008	; $84D6
	ld b,$20	; $84D9

loc_02_84DB:
	ld a,(iy+$03)	; $84DB
	cp $50	; $84DE
	jr z,loc_02_84E6	; $84E0
	cp $5E	; $84E2
	jr nz,loc_02_84EE	; $84E4

loc_02_84E6:
	ld (iy+$06),$FF	; $84E6
	ld (iy+$07),$00	; $84EA

loc_02_84EE:
	add iy,de	; $84EE
	djnz loc_02_84DB	; $84F0
	pop iy	; $84F2
	jr loc_02_84FE	; $84F4

loc_02_84F6:
	ld a,(iy+$03)	; $84F6
	call fn_01_430A	; $84F9
	jr c,loc_02_8501	; $84FC

loc_02_84FE:
	call fn_02_85EE	; $84FE

loc_02_8501:
	jp loc_00_3BD9	; $8501

loc_02_8504:
	ld a,(map_mode)	; $8504
	or a	; $8507
	jp nz,loc_02_85D2	; $8508
	ld a,($C959)	; $850B
	ld hl,$8517	; $850E
	call switch_on_a	; $8511
	jp loc_02_85D2	; $8514

tbl_02_8517:
	.db $10
	.dw loc_02_8524
	.db $11
	.dw loc_02_852D
	.db $12
	.dw loc_02_8524
	.db $13
	.dw loc_02_852D
	.db $FF

loc_02_8524:
	bit 0,(ix+$1A)	; $8524
	jp z,loc_02_85D2	; $8528
	jr loc_02_8534	; $852B

loc_02_852D:
	bit 0,(ix+$1A)	; $852D
	jp nz,loc_02_85D2	; $8531

loc_02_8534:
	call print_inline	; $8534
	.INCBIN "bank02.bin" SKIP $0537 READ 14	; text, 14 bytes
	ld a,(ix+$1A)	; $8545
	call fn_01_40B9	; $8548
	ex de,hl	; $854B
	ld hl,$0505	; $854C
	ld a,l	; $854F
	add a,e	; $8550
	ld e,a	; $8551
	ld a,h	; $8552
	add a,d	; $8553
	ld d,a	; $8554
	ld a,$07	; $8555
	call fn_00_38FC	; $8557
	ld a,$1C	; $855A
	call fn_00_05F1	; $855C
	ld (ix+$16),$04	; $855F

loc_02_8563:
	ld (ix+$17),$03	; $8563

loc_02_8567:
	call fn_00_3945	; $8567
	dec (ix+$17)	; $856A
	jr nz,loc_02_8567	; $856D
	ld a,(party_x)	; $856F
	add a,l	; $8572
	add a,$FB	; $8573
	ld l,a	; $8575
	ld a,(party_y)	; $8576
	add a,h	; $8579
	add a,$FB	; $857A
	ld h,a	; $857C
	ld iy,location_people	; $857D
	ld de,$0008	; $8581
	ld b,$20	; $8584

loc_02_8586:
	ld a,(iy+$03)	; $8586
	or a	; $8589
	jr z,loc_02_85A0	; $858A
	cp $8C	; $858C
	jr z,loc_02_85A0	; $858E
	cp $18	; $8590
	jr z,loc_02_85A0	; $8592
	ld a,(iy+$01)	; $8594
	cp l	; $8597
	jr nz,loc_02_85A0	; $8598
	ld a,(iy+$02)	; $859A
	cp h	; $859D
	jr z,loc_02_85AC	; $859E

loc_02_85A0:
	add iy,de	; $85A0
	djnz loc_02_8586	; $85A2
	dec (ix+$16)	; $85A4
	jr nz,loc_02_8563	; $85A7
	jp loc_02_8084	; $85A9

loc_02_85AC:
	ld a,$20	; $85AC
	sub b	; $85AE
	push af	; $85AF
	call fn_01_48F7	; $85B0
	pop af	; $85B3
	cp $08	; $85B4
	jr nc,loc_02_85BF	; $85B6
	call random	; $85B8
	and $03	; $85BB
	jr nz,loc_02_85C7	; $85BD

loc_02_85BF:
	ld (iy+$03),$00	; $85BF
	ld (iy+$00),$00	; $85C3

loc_02_85C7:
	call fn_00_3A91	; $85C7
	jp loc_02_8084	; $85CA
	.INCBIN "bank02.bin" SKIP $05CD READ 5	; data, 5 bytes

loc_02_85D2:
	call fn_01_40D1	; $85D2
	call print_inline	; $85D5
	.INCBIN "bank02.bin" SKIP $05D8 READ 19	; text, 19 bytes
	jp loc_02_8084	; $85EB

fn_02_85EE:
	ld a,$05	; $85EE
	ld c,$01	; $85F0
	call karma_lose	; $85F2
	ld a,$05	; $85F5
	ld c,$03	; $85F7
	call karma_lose	; $85F9
	ld a,$05	; $85FC
	ld c,$05	; $85FE
	jp karma_lose	; $8600
	call fn_01_4324	; $8603
	call print_newline	; $8606
	call fn_01_4C28	; $8609

loc_02_860C:
	call frame_tick	; $860C
	call fn_01_4C3C	; $860F
	jr z,loc_02_860C	; $8612
	jp c,loc_02_8081	; $8614
	ld a,(ix+$1A)	; $8617
	call fn_01_40B9	; $861A
	ld a,(party_x)	; $861D
	add a,l	; $8620
	and $0F	; $8621
	ld l,a	; $8623
	ld a,(party_y)	; $8624
	add a,h	; $8627
	and $0F	; $8628
	ld h,a	; $862A
	call fn_01_6C6A	; $862B
	jr c,loc_02_85D2	; $862E
	call fn_01_48F7	; $8630
	ld a,(iy+$03)	; $8633
	call fn_01_430A	; $8636
	call nc,fn_02_85EE	; $8639
	jp loc_00_3B52	; $863C
	ld hl,$95D9	; $863F
	call fn_00_2E7F	; $8642
	ld a,$2D	; $8645
	call print_char	; $8647
	ld (ix+$1A),$00	; $864A

loc_02_864E:
	call frame_tick	; $864E
	ld c,$02	; $8651
	ld de,$0102	; $8653
	call fn_01_49F9	; $8656
	jr z,loc_02_864E	; $8659
	jp c,loc_02_8081	; $865B
	ld hl,$95D5	; $865E
	call fn_02_806B	; $8661
	call print_newline	; $8664
	bit 0,(ix+$1A)	; $8667
	jp nz,loc_02_8674	; $866B
	call cast_spell	; $866E
	jp loc_02_8084	; $8671

loc_02_8674:
	ld hl,reagents	; $8674
	xor a	; $8677
	ld b,$08	; $8678

loc_02_867A:
	or (hl)	; $867A
	inc hl	; $867B
	djnz loc_02_867A	; $867C
	jr nz,loc_02_869D	; $867E
	call print_inline	; $8680
	.INCBIN "bank02.bin" SKIP $0683 READ 23	; text, 23 bytes
	jp loc_02_8084	; $869A

loc_02_869D:
	call print_inline	; $869D
	.INCBIN "bank02.bin" SKIP $06A0 READ 17	; text, 17 bytes
	ld (ix+$1C),$00	; $86B1
	ld (ix+$1D),$00	; $86B5
	ld a,$0D	; $86B9
	call fn_01_7552	; $86BB
	jr loc_02_8701	; $86BE

loc_02_86C0:
	call frame_tick	; $86C0
	ld a,(joy_pressed)	; $86C3
	and $7F	; $86C6
	ld hl,$86D0	; $86C8
	call switch_on_a	; $86CB
	jr loc_02_86C0	; $86CE

tbl_02_86D0:
	.db $01
	.dw loc_02_86E3
	.db $02
	.dw loc_02_86E7
	.db $04
	.dw loc_02_870B
	.db $08
	.dw loc_02_870B
	.db $10
	.dw loc_02_8081
	.db $20
	.dw loc_02_871A
	.db $FF

loc_02_86E3:
	ld b,$0C	; $86E3
	jr loc_02_86E9	; $86E5

loc_02_86E7:
	ld b,$0E	; $86E7

loc_02_86E9:
	ld a,(ix+$1C)	; $86E9
	add a,$02	; $86EC
	call fn_01_49E0	; $86EE
	ld a,(ix+$1C)	; $86F1
	add a,b	; $86F4
	ld l,a	; $86F5
	ld h,$0D	; $86F6
	call divide_8	; $86F8
	ld (ix+$1C),h	; $86FB

loc_02_86FE:
	call fn_01_40D9	; $86FE

loc_02_8701:
	ld a,(ix+$1C)	; $8701
	add a,$02	; $8704
	call fn_01_49D5	; $8706
	jr loc_02_86C0	; $8709

loc_02_870B:
	ld a,(ix+$1D)	; $870B
	xor $01	; $870E
	ld (ix+$1D),a	; $8710
	add a,$0D	; $8713
	call fn_01_7552	; $8715
	jr loc_02_86FE	; $8718

loc_02_871A:
	call fn_01_40C9	; $871A
	xor a	; $871D
	bit 0,(ix+$1D)	; $871E
	jr z,loc_02_8726	; $8722
	ld a,$0D	; $8724

loc_02_8726:
	add a,(ix+$1C)	; $8726
	ld (ix+$1C),a	; $8729
	add a,$65	; $872C
	call fn_01_5516	; $872E
	ld (ix+$1E),$00	; $8731
	ld (ix+$1F),$00	; $8735
	ld a,$11	; $8739
	call fn_01_7552	; $873B
	jr loc_02_8781	; $873E

loc_02_8740:
	call frame_tick	; $8740
	ld a,(joy_pressed)	; $8743
	and $7F	; $8746
	ld hl,$8750	; $8748
	call switch_on_a	; $874B
	jr loc_02_8740	; $874E

tbl_02_8750:
	.db $01
	.dw loc_02_875D
	.db $02
	.dw loc_02_8761
	.db $10
	.dw loc_02_8081
	.db $20
	.dw loc_02_87BD
	.db $FF

loc_02_875D:
	ld b,$08	; $875D
	jr loc_02_8763	; $875F

loc_02_8761:
	ld b,$0A	; $8761

loc_02_8763:
	call fn_02_87B1	; $8763
	call fn_01_49E0	; $8766
	ld a,(ix+$1E)	; $8769
	add a,b	; $876C
	ld l,a	; $876D
	ld h,$09	; $876E
	call divide_8	; $8770
	ld (ix+$1E),h	; $8773
	call fn_02_87B1	; $8776
	call fn_01_49D5	; $8779
	call fn_01_40D9	; $877C
	jr loc_02_8740	; $877F

loc_02_8781:
	ld hl,(text_col)	; $8781
	push hl	; $8784
	ld hl,$021C	; $8785
	ld (text_col),hl	; $8788
	ld hl,text_row	; $878B
	ld bc,$0880	; $878E

loc_02_8791:
	ld a,(ix+$1F)	; $8791
	and c	; $8794
	ld a,$3A	; $8795
	jr z,loc_02_879B	; $8797
	ld a,$40	; $8799

loc_02_879B:
	call put_glyph	; $879B
	inc (hl)	; $879E
	inc (hl)	; $879F
	rrc c	; $87A0
	djnz loc_02_8791	; $87A2
	pop hl	; $87A4
	ld (text_col),hl	; $87A5
	call fn_02_87B1	; $87A8
	call fn_01_49D5	; $87AB
	jp loc_02_8740	; $87AE

fn_02_87B1:
	ld a,(ix+$1E)	; $87B1
	cp $08	; $87B4
	jr c,loc_02_87BA	; $87B6
	xor a	; $87B8
	ret	; $87B9

loc_02_87BA:
	add a,a	; $87BA
	inc a	; $87BB
	ret	; $87BC

loc_02_87BD:
	ld a,(ix+$1E)	; $87BD
	cp $08	; $87C0
	jr z,loc_02_87E8	; $87C2
	ld hl,reagents	; $87C4
	rst $28	; $87C7
	or a	; $87C8
	jp z,loc_02_8740	; $87C9
	ld b,(ix+$1E)	; $87CC
	inc b	; $87CF
	ld a,$01	; $87D0

loc_02_87D2:
	rrca	; $87D2
	djnz loc_02_87D2	; $87D3
	ld b,a	; $87D5
	xor (ix+$1F)	; $87D6
	ld (ix+$1F),a	; $87D9
	and b	; $87DC
	push af	; $87DD
	call nz,fn_01_40C9	; $87DE
	pop af	; $87E1
	call z,fn_01_40D1	; $87E2
	jp loc_02_8781	; $87E5

loc_02_87E8:
	call fn_01_40C9	; $87E8
	call print_newline	; $87EB
	ld a,(ix+$1F)	; $87EE
	or a	; $87F1
	jr nz,loc_02_8801	; $87F2
	call print_inline	; $87F4
	.INCBIN "bank02.bin" SKIP $07F7 READ 7	; text, 7 bytes
	jp loc_02_8084	; $87FE

loc_02_8801:
	call print_inline	; $8801
	.INCBIN "bank02.bin" SKIP $0804 READ 20	; text, 20 bytes
	ld a,$02	; $8818
	call fn_01_4A66	; $881A

loc_02_881D:
	call frame_tick	; $881D
	call fn_01_4A9F	; $8820
	jr c,loc_02_881D	; $8823
	ld (ix+$20),l	; $8825
	call print_newline	; $8828
	inc l	; $882B
	dec l	; $882C
	jp z,loc_02_8084	; $882D
	call print_inline	; $8830
	.INCBIN "bank02.bin" SKIP $0833 READ 7	; text, 7 bytes
	ld (ix+$1A),$03	; $883A

loc_02_883E:
	ld a,$0A	; $883E
	call fn_00_3046	; $8840
	ld a,$2E	; $8843
	call print_char	; $8845
	dec (ix+$1A)	; $8848
	jr nz,loc_02_883E	; $884B
	call print_newline	; $884D
	ld a,(ix+$20)	; $8850
	ld (ix+$16),a	; $8853
	ld (ix+$21),$00	; $8856

loc_02_885A:
	ld hl,reagents	; $885A
	ld d,(ix+$1F)	; $885D
	ld bc,$0801	; $8860

loc_02_8863:
	rlc d	; $8863
	jr nc,loc_02_886E	; $8865
	inc c	; $8867
	ld a,(hl)	; $8868
	or a	; $8869
	jr z,loc_02_886E	; $886A
	dec c	; $886C
	dec (hl)	; $886D

loc_02_886E:
	inc hl	; $886E
	djnz loc_02_8863	; $886F
	dec c	; $8871
	jr nz,loc_02_8890	; $8872
	ld a,(ix+$1C)	; $8874
	ld hl,$88F3	; $8877
	rst $28	; $887A
	cp (ix+$1F)	; $887B
	jr nz,loc_02_8890	; $887E
	inc (ix+$21)	; $8880
	ld a,(ix+$1C)	; $8883
	ld hl,mixtures	; $8886
	rst $30	; $8889
	ld a,(hl)	; $888A
	cp $63	; $888B
	jr nc,loc_02_8890	; $888D
	inc (hl)	; $888F

loc_02_8890:
	dec (ix+$16)	; $8890
	jr nz,loc_02_885A	; $8893
	ld a,(ix+$21)	; $8895
	or a	; $8898
	jr nz,loc_02_88AC	; $8899
	call print_inline	; $889B
	.INCBIN "bank02.bin" SKIP $089E READ 12	; text, 12 bytes
	jr loc_02_88ED	; $88AA

loc_02_88AC:
	cp (ix+$20)	; $88AC
	jr nz,loc_02_88BF	; $88AF
	call print_inline	; $88B1
	.INCBIN "bank02.bin" SKIP $08B4 READ 9	; text, 9 bytes
	jr loc_02_88ED	; $88BD

loc_02_88BF:
	call print_inline	; $88BF
	.INCBIN "bank02.bin" SKIP $08C2 READ 35	; text, 35 bytes
	ld l,(ix+$21)	; $88E5
	ld h,$00	; $88E8
	call fn_01_5627	; $88EA

loc_02_88ED:
	call print_newline	; $88ED
	jp loc_02_8084	; $88F0

spell_recipes:
	.INCBIN "bank02.bin" SKIP $08F3 READ 26	; data, 26 bytes
	call print_newline	; $890D
	call fn_02_957C	; $8910
	jp z,loc_02_8077	; $8913
	call print_inline	; $8916
	.INCBIN "bank02.bin" SKIP $0919 READ 12	; text, 12 bytes
	call fn_01_4BCB	; $8925

loc_02_8928:
	call frame_tick	; $8928
	call fn_01_4BD2	; $892B
	jr z,loc_02_8928	; $892E
	jp c,loc_02_8081	; $8930
	ld c,(ix+$08)	; $8933
	call fn_01_4466	; $8936
	jp c,loc_02_807C	; $8939
	call fn_01_5B13	; $893C
	jp loc_02_8084	; $893F
	ld hl,$95FA	; $8942
	call fn_00_2E7F	; $8945
	ld a,$2D	; $8948
	call print_char	; $894A
	ld (ix+$1A),$00	; $894D

loc_02_8951:
	call frame_tick	; $8951
	ld c,$02	; $8954
	ld de,$0102	; $8956
	call fn_01_49F9	; $8959
	jr z,loc_02_8951	; $895C
	jp c,loc_02_8081	; $895E
	ld hl,$95F6	; $8961
	call fn_02_806B	; $8964
	call print_newline	; $8967
	bit 0,(ix+$1A)	; $896A
	jp z,loc_02_8974	; $896E
	jp loc_02_8B22	; $8971

loc_02_8974:
	ld a,$0A	; $8974
	call fn_01_7552	; $8976
	call print_inline	; $8979
	.INCBIN "bank02.bin" SKIP $097C READ 8	; text, 8 bytes
	ld (ix+$1A),$00	; $8984

loc_02_8988:
	call frame_tick	; $8988
	ld c,$03	; $898B
	ld a,($C916)	; $898D
	or a	; $8990
	jr z,loc_02_8994	; $8991
	inc c	; $8993

loc_02_8994:
	ld de,$0202	; $8994
	call fn_01_49F9	; $8997
	jr z,loc_02_8988	; $899A
	jp c,loc_02_8081	; $899C
	call fn_01_4324	; $899F
	ld a,(ix+$1A)	; $89A2
	push af	; $89A5
	ld hl,$8AF8	; $89A6
	rst $20	; $89A9
	call print_text_hl	; $89AA
	call print_newline	; $89AD
	pop af	; $89B0
	ld hl,$C913	; $89B1
	rst $30	; $89B4
	ld a,(hl)	; $89B5
	or a	; $89B6
	jr nz,loc_02_89D6	; $89B7
	call fn_01_40D1	; $89B9
	call print_inline	; $89BC
	.INCBIN "bank02.bin" SKIP $09BF READ 20	; text, 20 bytes
	jp loc_02_8084	; $89D3

loc_02_89D6:
	ld a,(ix+$1A)	; $89D6
	ld hl,$89DF	; $89D9
	jp fn_00_0018	; $89DC

tbl_02_89DF:
	.dw fn_02_89E7
	.dw fn_02_8A14
	.dw fn_02_8A31
	.dw fn_02_8A88

fn_02_89E7:
	ld a,(map_mode)	; $89E7
	cp $02	; $89EA
	jp nz,loc_02_8072	; $89EC
	ld hl,$C913	; $89EF
	dec (hl)	; $89F2
	ld a,($C95B)	; $89F3
	add a,$C8	; $89F6
	jr nc,loc_02_89FC	; $89F8
	ld a,$FF	; $89FA

loc_02_89FC:
	ld ($C95B),a	; $89FC
	call print_inline	; $89FF
	.INCBIN "bank02.bin" SKIP $0A02 READ 15	; text, 15 bytes
	jp loc_02_8084	; $8A11

fn_02_8A14:
	ld hl,$C914	; $8A14
	dec (hl)	; $8A17
	call print_inline	; $8A18
	.INCBIN "bank02.bin" SKIP $0A1B READ 16	; text, 16 bytes
	call fn_01_61FC	; $8A2B
	jp loc_02_8084	; $8A2E

fn_02_8A31:
	ld a,(map_mode)	; $8A31
	cp $01	; $8A34
	jp nz,loc_02_8072	; $8A36
	ld c,$01	; $8A39
	ld hl,(party_x)	; $8A3B
	inc l	; $8A3E
	call fn_02_8A72	; $8A3F
	ld hl,(party_x)	; $8A42
	dec l	; $8A45
	call fn_02_8A72	; $8A46
	ld hl,(party_x)	; $8A49
	inc h	; $8A4C
	call fn_02_8A72	; $8A4D
	ld hl,(party_x)	; $8A50
	dec h	; $8A53
	call fn_02_8A72	; $8A54
	dec c	; $8A57
	jp z,loc_02_8072	; $8A58
	ld hl,$C915	; $8A5B
	dec (hl)	; $8A5E
	call print_inline	; $8A5F
	.INCBIN "bank02.bin" SKIP $0A62 READ 13	; text, 13 bytes
	jp loc_02_8084	; $8A6F

fn_02_8A72:
	ld a,l	; $8A72
	cp $20	; $8A73
	ret nc	; $8A75
	ld a,h	; $8A76
	cp $20	; $8A77
	ret nc	; $8A79
	call fn_02_B5A4	; $8A7A
	ld a,(hl)	; $8A7D
	cp $3A	; $8A7E
	ret nz	; $8A80
	ld (hl),$3B	; $8A81
	inc l	; $8A83
	ld (hl),$00	; $8A84
	inc c	; $8A86
	ret	; $8A87

fn_02_8A88:
	ld a,(map_mode)	; $8A88
	cp $02	; $8A8B
	jp nc,loc_02_8072	; $8A8D
	call print_inline	; $8A90
	.INCBIN "bank02.bin" SKIP $0A93 READ 39	; text, 39 bytes
	ld a,(party_y)	; $8ABA
	call fn_02_8ADB	; $8ABD
	call print_inline	; $8AC0
	.INCBIN "bank02.bin" SKIP $0AC3 READ 12	; text, 12 bytes
	ld a,(party_x)	; $8ACF
	call fn_02_8ADB	; $8AD2
	call print_newline	; $8AD5
	jp loc_02_8084	; $8AD8

fn_02_8ADB:
	push af	; $8ADB
	rrca	; $8ADC
	rrca	; $8ADD
	rrca	; $8ADE
	rrca	; $8ADF
	call fn_02_8AF1	; $8AE0
	ld a,$27	; $8AE3
	call print_char	; $8AE5
	pop af	; $8AE8
	call fn_02_8AF1	; $8AE9
	ld a,$22	; $8AEC
	jp print_char	; $8AEE

fn_02_8AF1:
	and $0F	; $8AF1
	add a,$41	; $8AF3
	jp print_char	; $8AF5

tbl_02_8AF8:
	.dw $8B00
	.dw $8B06
	.dw $8B10
	.dw fn_02_8B1A

fn_02_8B00:
	.INCBIN "bank02.bin" SKIP $0B00 READ 6	; data, 6 bytes

fn_02_8B06:
	.INCBIN "bank02.bin" SKIP $0B06 READ 10	; data, 10 bytes

fn_02_8B10:
	.INCBIN "bank02.bin" SKIP $0B10 READ 10	; data, 10 bytes

fn_02_8B1A:
	ld d,e	; $8B1A
	ld h,l	; $8B1B
	ld a,b	; $8B1C
	ld (hl),h	; $8B1D
	ld h,c	; $8B1E
	ld l,(hl)	; $8B1F
	ld (hl),h	; $8B20
	nop	; $8B21

loc_02_8B22:
	call fn_01_651E	; $8B22
	jp loc_02_8084	; $8B25
	ld hl,$9613	; $8B28
	call fn_00_2E7F	; $8B2B
	ld a,$2D	; $8B2E
	call print_char	; $8B30
	ld (ix+$1A),$00	; $8B33

loc_02_8B37:
	call frame_tick	; $8B37
	ld c,$02	; $8B3A
	ld de,$0102	; $8B3C
	call fn_01_49F9	; $8B3F
	jr z,loc_02_8B37	; $8B42
	jp c,loc_02_8081	; $8B44
	ld hl,$960F	; $8B47
	call fn_02_806B	; $8B4A
	call print_newline	; $8B4D
	bit 0,(ix+$1A)	; $8B50
	jr nz,loc_02_8B5B	; $8B54
	call fn_01_57D7	; $8B56
	jr loc_02_8B5E	; $8B59

loc_02_8B5B:
	call fn_01_5855	; $8B5B

loc_02_8B5E:
	jp loc_02_8084	; $8B5E
	call fn_01_4324	; $8B61
	call print_newline	; $8B64
	call fn_02_957C	; $8B67
	ld iy,$8C76	; $8B6A
	jr z,loc_02_8B74	; $8B6E
	ld iy,$8C07	; $8B70

loc_02_8B74:
	ld de,$0005	; $8B74
	ld hl,(party_x)	; $8B77
	ld bc,(location)	; $8B7A

loc_02_8B7E:
	ld a,(iy+$00)	; $8B7E
	cp $FF	; $8B81
	jr z,loc_02_8B9F	; $8B83
	cp c	; $8B85
	jr nz,loc_02_8B9B	; $8B86
	ld a,(iy+$01)	; $8B88
	cp l	; $8B8B
	jr nz,loc_02_8B9B	; $8B8C
	ld a,(iy+$02)	; $8B8E
	cp h	; $8B91
	jr nz,loc_02_8B9B	; $8B92
	ld l,(iy+$03)	; $8B94
	ld h,(iy+$04)	; $8B97
	jp (hl)	; $8B9A

loc_02_8B9B:
	add iy,de	; $8B9B
	jr loc_02_8B7E	; $8B9D

loc_02_8B9F:
	call fn_01_40D1	; $8B9F
	ld hl,$8E61	; $8BA2
	call print_text_hl	; $8BA5
	jp loc_02_8084	; $8BA8

fn_02_8BAB:
	ld hl,($C967)	; $8BAB
	call fn_00_33F6	; $8BAE
	ld ($C967),hl	; $8BB1
	ret nz	; $8BB4
	pop af	; $8BB5
	jr loc_02_8B9F	; $8BB6

fn_02_8BB8:
	ld a,($C95D)	; $8BB8
	ld hl,$C95F	; $8BBB
	or (hl)	; $8BBE
	ret z	; $8BBF
	pop af	; $8BC0
	jr loc_02_8B9F	; $8BC1

fn_02_8BC3:
	ld hl,karma	; $8BC3
	ld b,$08	; $8BC6
	xor a	; $8BC8

loc_02_8BC9:
	or (hl)	; $8BC9
	inc hl	; $8BCA
	djnz loc_02_8BC9	; $8BCB
	ret z	; $8BCD
	pop af	; $8BCE
	jr loc_02_8B9F	; $8BCF

fn_02_8BD1:
	push hl	; $8BD1
	call fn_01_40E1	; $8BD2
	ld hl,$8E70	; $8BD5
	call print_text_hl	; $8BD8
	pop hl	; $8BDB
	call print_text_hl	; $8BDC
	ld a,$05	; $8BDF
	ld c,$05	; $8BE1
	jp karma_gain	; $8BE3

loc_02_8BE6:
	ld hl,reagents	; $8BE6
	rst $30	; $8BE9
	call random	; $8BEA
	and $07	; $8BED
	add a,$02	; $8BEF
	add a,(hl)	; $8BF1
	ld (hl),a	; $8BF2
	cp $64	; $8BF3
	jr c,loc_02_8C04	; $8BF5
	ld (hl),$63	; $8BF7
	ld a,$10	; $8BF9
	call fn_00_05F1	; $8BFB
	ld hl,$8E7F	; $8BFE
	call print_text_hl	; $8C01

loc_02_8C04:
	jp loc_02_8084	; $8C04
	.INCBIN "bank02.bin" SKIP $0C07 READ 117	; data, 117 bytes

sub_02_8C7C:
	call fn_02_8BB8	; $8C7C
	call fn_02_8BAB	; $8C7F
	ld hl,$8E8E	; $8C82
	call fn_02_8BD1	; $8C85
	ld a,$07	; $8C88
	jp loc_02_8BE6	; $8C8A
	call fn_02_8BB8	; $8C8D
	call fn_02_8BAB	; $8C90
	ld hl,$8E9E	; $8C93
	call fn_02_8BD1	; $8C96
	ld a,$06	; $8C99
	jp loc_02_8BE6	; $8C9B

sub_02_8C9E:
	ld hl,$C951	; $8C9E
	bit 2,(hl)	; $8CA1
	jp nz,loc_02_8B9F	; $8CA3
	set 2,(hl)	; $8CA6
	ld hl,$8EAB	; $8CA8

loc_02_8CAB:
	call fn_02_8BD1	; $8CAB
	ld c,$00	; $8CAE
	ld hl,$0190	; $8CB0
	call member_gain_xp	; $8CB3
	jp loc_02_8084	; $8CB6
	.INCBIN "bank02.bin" SKIP $0CB9 READ 29	; data, 29 bytes

sub_02_8CD6:
	call nc,$B8CD	; $8CD6
	adc a,e	; $8CD9
	ld hl,$C951	; $8CDA
	bit 0,(hl)	; $8CDD
	jp nz,loc_02_8B9F	; $8CDF
	bit 1,(hl)	; $8CE2
	jp nz,loc_02_8B9F	; $8CE4
	set 0,(hl)	; $8CE7
	ld hl,$8EF2	; $8CE9
	jr loc_02_8CAB	; $8CEC

sub_02_8CEE:
	call fn_02_8BB8	; $8CEE
	ld hl,$C955	; $8CF1
	bit 7,(hl)	; $8CF4
	jp nz,loc_02_8B9F	; $8CF6
	set 7,(hl)	; $8CF9
	ld hl,$8F14	; $8CFB
	call fn_02_8BD1	; $8CFE
	ld c,$00	; $8D01
	ld hl,$00C8	; $8D03
	call member_gain_xp	; $8D06
	jp loc_02_8084	; $8D09
	.INCBIN "bank02.bin" SKIP $0D0C READ 14	; data, 14 bytes

sub_02_8D1A:
	ex (sp),hl	; $8D1A
	ld hl,$C951	; $8D1B
	bit 3,(hl)	; $8D1E
	jp nz,loc_02_8B9F	; $8D20
	set 3,(hl)	; $8D23
	ld hl,$8F38	; $8D25
	jp loc_02_8CAB	; $8D28

sub_02_8D2B:
	ld hl,$C951	; $8D2B
	bit 4,(hl)	; $8D2E
	jp nz,loc_02_8B9F	; $8D30
	set 4,(hl)	; $8D33
	ld hl,$8F4C	; $8D35
	jp loc_02_8CAB	; $8D38

sub_02_8D3B:
	ld hl,$8F61	; $8D3B
	call print_text_hl	; $8D3E
	ld hl,$D000	; $8D41
	ld de,$8800	; $8D44
	ld bc,$0800	; $8D47
	call fn_00_0C49	; $8D4A
	ld hl,$8E20	; $8D4D
	call fn_00_2E7F	; $8D50
	ld (ix+$1A),$00	; $8D53
	ld (ix+$1B),$00	; $8D57

loc_02_8D5B:
	ld hl,$8F8D	; $8D5B
	call print_text_hl	; $8D5E

loc_02_8D61:
	call fn_00_335C	; $8D61
	bit 0,(ix+$1B)	; $8D64
	call z,fn_00_129D	; $8D68
	call yield	; $8D6B
	ld c,$10	; $8D6E
	ld de,$0001	; $8D70
	call fn_01_49F9	; $8D73
	jr z,loc_02_8D61	; $8D76
	jr c,loc_02_8D94	; $8D78
	set 0,(ix+$1B)	; $8D7A
	ld a,(ix+$1A)	; $8D7E
	add a,$41	; $8D81
	call print_char	; $8D83
	call print_newline	; $8D86
	ld a,(ix+$1A)	; $8D89
	call fn_00_3762	; $8D8C
	call fn_01_4F8A	; $8D8F
	jr loc_02_8D5B	; $8D92

loc_02_8D94:
	ld hl,$8800	; $8D94
	ld de,$D000	; $8D97
	ld bc,$0800	; $8D9A
	call fn_00_0C49	; $8D9D
	call fn_00_276D	; $8DA0
	call fn_00_1A7C	; $8DA3
	call print_newline	; $8DA6
	jp loc_02_8084	; $8DA9

sub_02_8DAC:
	call fn_02_8BC3	; $8DAC
	ld a,($C91E)	; $8DAF
	or a	; $8DB2
	jp nz,loc_02_8B9F	; $8DB3
	ld a,$08	; $8DB6
	ld ($C91E),a	; $8DB8
	ld hl,$8F9A	; $8DBB
	jp loc_02_8CAB	; $8DBE

sub_02_8DC1:
	call fn_02_8BC3	; $8DC1
	ld a,($C92E)	; $8DC4
	or a	; $8DC7
	jp nz,loc_02_8B9F	; $8DC8
	ld a,$08	; $8DCB
	ld ($C92E),a	; $8DCD
	ld hl,$8FAA	; $8DD0
	jp loc_02_8CAB	; $8DD3
	.INCBIN "bank02.bin" SKIP $0DD6 READ 15	; data, 15 bytes

sub_02_8DE5:
	ret m	; $8DE5
	ld a,(hl)	; $8DE6
	ld hl,$C956	; $8DE7
	ld c,a	; $8DEA
	and (hl)	; $8DEB
	jp nz,loc_02_8B9F	; $8DEC
	ld a,c	; $8DEF
	or (hl)	; $8DF0
	ld (hl),a	; $8DF1
	push bc	; $8DF2
	ld hl,$8FBB	; $8DF3
	call fn_02_8BD1	; $8DF6
	pop af	; $8DF9
	add a,$97	; $8DFA
	call fn_01_5516	; $8DFC
	ld hl,$8FC8	; $8DFF
	call print_text_hl	; $8E02
	ld c,$00	; $8E05
	ld hl,$0064	; $8E07
	call member_gain_xp	; $8E0A
	jp loc_02_8084	; $8E0D
	.INCBIN "bank02.bin" SKIP $0E10 READ 443	; data, 443 bytes
	call fn_01_4324	; $8FCB
	call print_newline	; $8FCE
	ld a,($C990)	; $8FD1
	and $F0	; $8FD4
	ld hl,$8FF8	; $8FD6
	call switch_on_a	; $8FD9

loc_02_8FDC:
	call fn_01_40D1	; $8FDC
	call print_inline	; $8FDF
	.INCBIN "bank02.bin" SKIP $0FE2 READ 19	; text, 19 bytes
	jp loc_02_8084	; $8FF5

tbl_02_8FF8:
	.db $70
	.dw loc_02_9002
	.db $90
	.dw loc_02_90E3
	.db $B0
	.dw loc_02_91D6
	.db $FF

loc_02_9002:
	call print_inline	; $9002
	.INCBIN "bank02.bin" SKIP $1005 READ 41	; text, 41 bytes
	call fn_01_4BCB	; $902E

loc_02_9031:
	call frame_tick	; $9031
	call fn_01_4BD2	; $9034
	jr z,loc_02_9031	; $9037
	jp c,loc_02_8081	; $9039
	ld c,(ix+$08)	; $903C
	call fn_01_4466	; $903F
	jp c,loc_02_807C	; $9042
	call member_record	; $9045
	ld hl,(party_x)	; $9048
	call fn_01_6DC0	; $904B
	ld (hl),$00	; $904E
	ld a,(location)	; $9050
	sub $12	; $9053
	add a,a	; $9055
	add a,a	; $9056
	ld hl,$909D	; $9057
	rst $30	; $905A
	ex de,hl	; $905B
	ld a,(de)	; $905C
	inc de	; $905D
	call fn_02_922D	; $905E
	ld hl,$90BD	; $9061
	ld b,(iy+$06)	; $9064
	call fn_02_9088	; $9067
	ld (iy+$06),b	; $906A
	ld hl,$90C6	; $906D
	ld b,(iy+$07)	; $9070
	call fn_02_9088	; $9073
	ld (iy+$07),b	; $9076
	ld hl,$90D0	; $9079
	ld b,(iy+$08)	; $907C
	call fn_02_9088	; $907F
	ld (iy+$08),b	; $9082
	jp loc_02_8084	; $9085

fn_02_9088:
	ld a,(de)	; $9088
	inc de	; $9089
	or a	; $908A
	ret z	; $908B
	add a,b	; $908C
	cp $32	; $908D
	jr c,loc_02_9093	; $908F
	ld a,$32	; $9091

loc_02_9093:
	ld b,a	; $9093
	call print_text_hl	; $9094
	ld hl,$90DD	; $9097
	jp print_text_hl	; $909A
	.INCBIN "bank02.bin" SKIP $109D READ 70	; data, 70 bytes

loc_02_90E3:
	call print_inline	; $90E3
	.INCBIN "bank02.bin" SKIP $10E6 READ 34	; text, 34 bytes
	call fn_01_4BCB	; $9108

loc_02_910B:
	call frame_tick	; $910B
	call fn_01_4BD2	; $910E
	jr z,loc_02_910B	; $9111
	jp c,loc_02_8081	; $9113
	ld c,(ix+$08)	; $9116
	call fn_01_4466	; $9119
	jp c,loc_02_807C	; $911C
	call member_record	; $911F
	ld a,($C990)	; $9122
	and $0F	; $9125
	ld hl,$912E	; $9127
	rst $18	; $912A
	jp loc_02_8084	; $912B

tbl_02_912E:
	.dw $9138
	.dw fn_02_914E
	.dw fn_02_917A
	.dw fn_02_9191
	.dw fn_02_91B1
	.dw fn_00_10CD
	.INCBIN "bank02.bin" SKIP $113A READ 20	; data, 20 bytes

fn_02_914E:
	ld l,(iy+$00)	; $914E
	ld h,(iy+$01)	; $9151
	ld e,(iy+$02)	; $9154
	ld d,(iy+$03)	; $9157
	or a	; $915A
	sbc hl,de	; $915B
	jr z,$D9	; $915D
	ld (iy+$00),e	; $915F
	ld (iy+$01),d	; $9162
	call print_inline	; $9165
	.INCBIN "bank02.bin" SKIP $1168 READ 17	; text, 17 bytes
	ret	; $9179

fn_02_917A:
	call print_inline	; $917A
	.INCBIN "bank02.bin" SKIP $117D READ 15	; text, 15 bytes
	ld a,$01	; $918C
	jp fn_02_922D	; $918E

fn_02_9191:
	ld a,(iy+$1C)	; $9191
	cp $50	; $9194
	jr nz,$A0	; $9196
	ld (iy+$1C),$47	; $9198
	call print_inline	; $919C
	.INCBIN "bank02.bin" SKIP $119F READ 17	; text, 17 bytes
	ret	; $91B0

fn_02_91B1:
	ld a,(iy+$1C)	; $91B1
	cp $50	; $91B4
	jr z,$80	; $91B6
	ld (iy+$1C),$50	; $91B8
	call print_inline	; $91BC
	.INCBIN "bank02.bin" SKIP $11BF READ 18	; text, 18 bytes
	ld a,$01	; $91D1
	jp fn_02_922D	; $91D3

loc_02_91D6:
	ld a,(location)	; $91D6
	sub $12	; $91D9
	cp $06	; $91DB
	jp nc,loc_02_8FDC	; $91DD
	inc a	; $91E0
	ld b,a	; $91E1
	ld c,$80	; $91E2

loc_02_91E4:
	rlc c	; $91E4
	djnz loc_02_91E4	; $91E6
	ld hl,$C955	; $91E8
	ld a,(hl)	; $91EB
	and c	; $91EC
	jp nz,loc_02_8FDC	; $91ED
	ld a,(hl)	; $91F0
	or c	; $91F1
	ld (hl),a	; $91F2
	call fn_01_40E1	; $91F3
	call print_inline	; $91F6
	.INCBIN "bank02.bin" SKIP $11F9 READ 14	; text, 14 bytes
	ld a,(location)	; $9207
	add a,$8E	; $920A
	call fn_01_5516	; $920C
	call print_inline	; $920F
	.INCBIN "bank02.bin" SKIP $1212 READ 9	; text, 9 bytes
	ld a,$05	; $921B
	ld c,$05	; $921D
	call karma_gain	; $921F
	ld hl,$00C8	; $9222
	ld c,$00	; $9225
	call member_gain_xp	; $9227
	jp loc_02_8084	; $922A

fn_02_922D:
	push bc	; $922D
	push de	; $922E
	push hl	; $922F
	ld l,a	; $9230
	ld h,$64	; $9231
	call multiply_8x8	; $9233
	ld c,(ix+$08)	; $9236
	call member_hurt	; $9239
	ld a,$13	; $923C
	call fn_00_05F1	; $923E
	ld c,(ix+$08)	; $9241
	call fn_01_49A6	; $9244
	call wait_vblank	; $9247
	call fn_01_4999	; $924A
	call wait_vblank	; $924D
	pop hl	; $9250
	pop de	; $9251
	pop bc	; $9252
	ret	; $9253
	ld hl,$962E	; $9254
	call fn_00_2E7F	; $9257
	ld a,$2D	; $925A
	call print_char	; $925C
	ld (ix+$1A),$00	; $925F

loc_02_9263:
	call frame_tick	; $9263
	ld c,$04	; $9266
	ld de,$0102	; $9268
	call fn_01_49F9	; $926B
	jr z,loc_02_9263	; $926E
	jp c,loc_02_8081	; $9270
	ld hl,$9626	; $9273
	call fn_02_806B	; $9276
	ld a,(ix+$1A)	; $9279
	ld hl,$9282	; $927C
	jp fn_00_0018	; $927F

tbl_02_9282:
	.dw fn_02_928A
	.dw fn_02_92B7
	.dw fn_02_9367
	.dw fn_02_9370

fn_02_928A:
	call fn_01_4324	; $928A
	call print_newline	; $928D
	ld a,(map_mode)	; $9290
	cp $00	; $9293
	jr z,loc_02_929C	; $9295
	cp $02	; $9297
	jp nz,loc_02_8072	; $9299

loc_02_929C:
	ld a,($C959)	; $929C
	cp $1F	; $929F
	jr z,loc_02_92A9	; $92A1
	call fn_01_4703	; $92A3
	jp loc_02_8084	; $92A6

loc_02_92A9:
	ld a,(game_mode)	; $92A9
	ld ($C9B7),a	; $92AC
	ld a,$15	; $92AF
	ld (game_mode),a	; $92B1
	jp main_loop	; $92B4

fn_02_92B7:
	call print_newline	; $92B7
	ld a,(party_size)	; $92BA
	cp $02	; $92BD
	jr nc,loc_02_92E2	; $92BF
	call fn_01_40D1	; $92C1
	call print_inline	; $92C4
	.INCBIN "bank02.bin" SKIP $12C7 READ 24	; text, 24 bytes
	jp loc_02_8084	; $92DF

loc_02_92E2:
	call fn_01_4BCB	; $92E2
	call print_inline	; $92E5
	.INCBIN "bank02.bin" SKIP $12E8 READ 11	; text, 11 bytes

loc_02_92F3:
	call frame_tick	; $92F3
	call fn_01_4BD2	; $92F6
	jr z,loc_02_92F3	; $92F9
	jp c,loc_02_8081	; $92FB
	ld a,(ix+$08)	; $92FE
	or a	; $9301
	jr z,loc_02_9349	; $9302
	ld (ix+$1B),a	; $9304
	call print_inline	; $9307
	.INCBIN "bank02.bin" SKIP $130A READ 11	; text, 11 bytes

loc_02_9315:
	call frame_tick	; $9315
	call fn_01_4BD2	; $9318
	jr z,loc_02_9315	; $931B
	jp c,loc_02_8081	; $931D
	ld a,(ix+$08)	; $9320
	or a	; $9323
	jr z,loc_02_9349	; $9324
	cp (ix+$1B)	; $9326
	jr nz,loc_02_9331	; $9329
	call fn_01_46C2	; $932B
	jp loc_02_8084	; $932E

loc_02_9331:
	ld c,a	; $9331
	call member_record	; $9332
	push iy	; $9335
	pop hl	; $9337
	ld c,(ix+$1B)	; $9338
	call member_record	; $933B
	push iy	; $933E
	pop de	; $9340
	ld b,$1D	; $9341
	call fn_00_3340	; $9343
	jp loc_02_8084	; $9346

loc_02_9349:
	call fn_01_40D1	; $9349
	call print_avatar_name	; $934C
	call print_inline	; $934F
	.INCBIN "bank02.bin" SKIP $1352 READ 18	; text, 18 bytes
	jp loc_02_8084	; $9364

fn_02_9367:
	call fn_01_7588	; $9367
	call print_newline	; $936A
	jp loc_02_8084	; $936D

fn_02_9370:
	call print_newline	; $9370
	ld a,(map_mode)	; $9373
	or a	; $9376
	jr z,loc_02_937F	; $9377
	call fn_01_46DF	; $9379
	jp loc_02_8084	; $937C

loc_02_937F:
	call print_inline	; $937F
	.INCBIN "bank02.bin" SKIP $1382 READ 38	; text, 38 bytes
	ld hl,$8100	; $93A8
	call fn_00_0C3D	; $93AB
	ld (ix+$1A),a	; $93AE
	call fn_02_9498	; $93B1
	call fn_02_950F	; $93B4

loc_02_93B7:
	call frame_tick	; $93B7
	ld a,(joy_pressed)	; $93BA
	ld hl,$93C5	; $93BD
	call switch_on_a	; $93C0
	jr loc_02_93B7	; $93C3

tbl_02_93C5:
	.db $01
	.dw loc_02_93D2
	.db $02
	.dw loc_02_93DC
	.db $10
	.dw loc_02_8081
	.db $20
	.dw loc_02_93F5
	.db $FF

loc_02_93D2:
	call fn_01_40D9	; $93D2
	call fn_02_9515	; $93D5
	ld a,$02	; $93D8
	jr loc_02_93E4	; $93DA

loc_02_93DC:
	call fn_01_40D9	; $93DC
	call fn_02_9515	; $93DF
	ld a,$04	; $93E2

loc_02_93E4:
	add a,(ix+$1A)	; $93E4
	ld l,a	; $93E7
	ld h,$03	; $93E8
	call divide_8	; $93EA
	ld (ix+$1A),h	; $93ED
	call fn_02_950F	; $93F0
	jr loc_02_93B7	; $93F3

loc_02_93F5:
	call fn_01_40C9	; $93F5
	ld a,(ix+$1A)	; $93F8
	ld ($C0CD),a	; $93FB
	ld hl,$8100	; $93FE
	call fn_00_0C43	; $9401
	add a,$31	; $9404
	call print_char	; $9406
	call print_newline	; $9409
	ld a,$FF	; $940C
	ld ($C1C0),a	; $940E
	ld hl,party_name	; $9411
	ld de,$C1C1	; $9414
	ld bc,$000D	; $9417
	ldir	; $941A
	ld a,($C76B)	; $941C
	ld ($C1CE),a	; $941F
	ld a,($C75B)	; $9422
	ld ($C1CF),a	; $9425
	ld a,($C76C)	; $9428
	ld ($C1D0),a	; $942B
	ld hl,move_count	; $942E
	ld de,$C1D1	; $9431
	ld bc,$0005	; $9434
	ldir	; $9437
	ld a,($C0CE)	; $9439
	ld ($C1D6),a	; $943C
	ld a,(ix+$1A)	; $943F
	add a,a	; $9442
	add a,a	; $9443
	ld e,a	; $9444
	ld d,$00	; $9445
	ld iy,$948C	; $9447
	add iy,de	; $944B
	ld hl,$C500	; $944D
	ld e,(iy+$00)	; $9450
	ld d,(iy+$01)	; $9453
	ld bc,$0100	; $9456
	call fn_00_0C49	; $9459
	ld hl,location_people	; $945C
	ld bc,$036C	; $945F
	call fn_00_0C49	; $9462
	ld hl,$C1C0	; $9465
	ld e,(iy+$02)	; $9468
	ld d,(iy+$03)	; $946B
	ld bc,$0040	; $946E
	call fn_00_0C49	; $9471
	call fn_02_9498	; $9474
	call fn_02_950F	; $9477
	call print_inline	; $947A
	.INCBIN "bank02.bin" SKIP $147D READ 6	; text, 6 bytes
	call wait_button	; $9483
	call print_newline	; $9486
	jp loc_02_8084	; $9489
	.INCBIN "bank02.bin" SKIP $148C READ 12	; data, 12 bytes

fn_02_9498:
	ld hl,$9537	; $9498
	call fn_00_2E79	; $949B
	ld hl,$8040	; $949E
	ld de,$CBA6	; $94A1
	call fn_02_94BC	; $94A4
	ld hl,$8080	; $94A7
	ld de,$CCA6	; $94AA
	call fn_02_94BC	; $94AD
	ld hl,$80C0	; $94B0
	ld de,$CDA6	; $94B3
	call fn_02_94BC	; $94B6
	jp fn_00_2E54	; $94B9

fn_02_94BC:
	push de	; $94BC
	ld de,$C1C0	; $94BD
	ld bc,$0040	; $94C0
	call fn_00_0C49	; $94C3
	pop de	; $94C6
	ld a,($C1C0)	; $94C7
	or a	; $94CA
	ret z	; $94CB
	push de	; $94CC
	ld hl,$C1C1	; $94CD
	call fn_00_2D6C	; $94D0
	pop de	; $94D3
	ld hl,$0040	; $94D4
	add hl,de	; $94D7
	ex de,hl	; $94D8
	ld a,($C1D6)	; $94D9
	call fn_00_2D9A	; $94DC
	ld hl,$0004	; $94DF
	add hl,de	; $94E2
	ex de,hl	; $94E3
	ld a,($C1CE)	; $94E4
	call fn_00_2D9A	; $94E7
	ld hl,$9529	; $94EA
	call fn_00_2D6C	; $94ED
	ld a,($C1CF)	; $94F0
	add a,$30	; $94F3
	call fn_00_2D9A	; $94F5
	ld hl,$0028	; $94F8
	add hl,de	; $94FB
	ex de,hl	; $94FC
	ld hl,$C1D1	; $94FD
	call fn_00_36FF	; $9500
	ld hl,$C068	; $9503
	call fn_00_2D6C	; $9506
	ld hl,$9531	; $9509
	jp fn_00_2D6C	; $950C

fn_02_950F:
	call fn_02_951B	; $950F
	jp fn_00_2DE3	; $9512

fn_02_9515:
	call fn_02_951B	; $9515
	jp fn_00_2DC4	; $9518

fn_02_951B:
	ld a,(ix+$1A)	; $951B
	add a,a	; $951E
	add a,a	; $951F
	add a,$05	; $9520
	ld d,a	; $9522
	ld e,$12	; $9523
	ld bc,$040E	; $9525
	ret	; $9528
	.INCBIN "bank02.bin" SKIP $1529 READ 83	; data, 83 bytes

fn_02_957C:
	ld a,($C959)	; $957C
	cp $18	; $957F
	ret	; $9581
	.INCBIN "bank02.bin" SKIP $1582 READ 208	; data, 208 bytes

loc_02_9652:
	ld c,a	; $9652
	call place_index	; $9653
	add a,a	; $9656
	add a,a	; $9657
	add a,a	; $9658
	ld hl,$96C1	; $9659
	rst $30	; $965C
	ld a,c	; $965D
	ld bc,$0800	; $965E

loc_02_9661:
	cp (hl)	; $9661
	jr z,loc_02_9685	; $9662
	inc hl	; $9664
	inc c	; $9665
	djnz loc_02_9661	; $9666
	ld c,$08	; $9668
	cp $18	; $966A
	jr nz,loc_02_9675	; $966C
	ld a,(location)	; $966E
	cp $0E	; $9671
	jr z,loc_02_9685	; $9673

loc_02_9675:
	ld c,$09	; $9675
	cp $19	; $9677
	jr nz,loc_02_9682	; $9679
	ld a,(location)	; $967B
	cp $01	; $967E
	jr z,loc_02_9685	; $9680

loc_02_9682:
	jp loc_02_810D	; $9682

loc_02_9685:
	ld a,c	; $9685
	ld hl,$9741	; $9686
	rst $20	; $9689
	push hl	; $968A
	ld a,c	; $968B
	add a,a	; $968C
	add a,a	; $968D
	add a,a	; $968E
	add a,a	; $968F
	ld hl,$9755	; $9690
	rst $30	; $9693
	call place_index	; $9694
	rst $28	; $9697
	dec a	; $9698
	ld (shop_number),a	; $9699
	ld a,$16	; $969C
	ld (message_bank),a	; $969E
	ld hl,$8000	; $96A1
	call fn_00_0807	; $96A4
	ld (message_table),hl	; $96A7
	ld a,$04	; $96AA
	call play_music_later	; $96AC
	jp print_newline	; $96AF

loc_02_96B2:
	call print_message	; $96B2

loc_02_96B5:
	call print_newline	; $96B5
	call fn_00_33E5	; $96B8
	call play_music_later	; $96BB
	jp loc_02_8084	; $96BE
	.INCBIN "bank02.bin" SKIP $16C1 READ 128	; data, 128 bytes

shop_handlers:
	.dw fn_02_97D5
	.dw fn_02_9AE3
	.dw fn_02_9DE9
	.dw fn_02_9EE2
	.dw fn_02_A1EB
	.dw fn_02_A3CF
	.dw fn_02_A647
	.dw fn_02_A869
	.dw fn_02_AA14
	.dw fn_02_AA75

shop_numbers:
	.INCBIN "bank02.bin" SKIP $1755 READ 128	; data, 128 bytes

fn_02_97D5:
	ld a,$31	; $97D5
	call print_message	; $97D7
	ld a,(shop_number)	; $97DA
	call print_message	; $97DD
	call fn_01_548D	; $97E0
	ld a,$08	; $97E3
	call fn_01_7552	; $97E5
	call wait_button	; $97E8
	call fn_01_54CB	; $97EB
	ld a,(shop_number)	; $97EE
	add a,$06	; $97F1
	call print_message	; $97F3
	ld a,$32	; $97F6
	call print_message	; $97F8
	call fn_02_AB4B	; $97FB

loc_02_97FE:
	call frame_tick	; $97FE
	call fn_02_AB55	; $9801
	jr z,loc_02_97FE	; $9804
	push af	; $9806
	ld a,$08	; $9807
	call fn_01_7552	; $9809
	pop af	; $980C
	jr c,loc_02_9816	; $980D
	or a	; $980F
	jp z,loc_02_9861	; $9810
	jp loc_02_99A6	; $9813

loc_02_9816:
	call print_newline	; $9816

loc_02_9819:
	call print_newline	; $9819
	ld a,(shop_number)	; $981C
	add a,$06	; $981F
	call print_message	; $9821
	ld a,$33	; $9824
	jp loc_02_96B2	; $9826

weapon_shop_stock:
	.INCBIN "bank02.bin" SKIP $1829 READ 24	; data, 24 bytes

weapon_prices:
	.INCBIN "bank02.bin" SKIP $1841 READ 32	; data, 32 bytes

loc_02_9861:
	call print_newline	; $9861
	ld a,$1C	; $9864
	call print_message	; $9866
	call wait_button	; $9869
	call fn_01_54CB	; $986C
	ld a,$1D	; $986F
	call print_message	; $9871
	ld (ix+$1A),$00	; $9874
	ld (ix+$16),$04	; $9878

loc_02_987C:
	call print_newline	; $987C
	ld a,$20	; $987F
	call print_char	; $9881
	ld a,$20	; $9884
	call print_char	; $9886
	call fn_02_9999	; $9889
	call fn_02_AB85	; $988C
	ld a,$73	; $988F
	call print_char	; $9891
	inc (ix+$1A)	; $9894
	dec (ix+$16)	; $9897
	jr nz,loc_02_987C	; $989A
	call fn_01_548D	; $989C
	call wait_button	; $989F
	call print_newline	; $98A2

loc_02_98A5:
	call print_newline	; $98A5
	ld a,$1E	; $98A8
	call print_message	; $98AA
	call fn_00_2E1B	; $98AD
	ld (ix+$1A),$00	; $98B0
	ld b,$04	; $98B4
	ld de,$0213	; $98B6

loc_02_98B9:
	push bc	; $98B9
	push de	; $98BA
	call fn_02_9999	; $98BB
	add a,$25	; $98BE
	call fn_00_340F	; $98C0
	call fn_00_2D69	; $98C3
	pop de	; $98C6
	pop bc	; $98C7
	inc d	; $98C8
	inc d	; $98C9
	inc (ix+$1A)	; $98CA
	djnz loc_02_98B9	; $98CD
	call fn_00_2E54	; $98CF
	ld (ix+$1A),$00	; $98D2

loc_02_98D6:
	call frame_tick	; $98D6
	ld c,$04	; $98D9
	ld de,$0102	; $98DB
	call fn_01_49F4	; $98DE
	jr z,loc_02_98D6	; $98E1
	ld a,$08	; $98E3
	call fn_01_7552	; $98E5
	jp c,loc_02_9816	; $98E8
	call fn_02_9999	; $98EB
	ld (ix+$1B),a	; $98EE
	call fn_02_AB85	; $98F1
	call fn_01_54CB	; $98F4
	ld a,(ix+$1B)	; $98F7
	ld hl,$9841	; $98FA
	rst $20	; $98FD
	call fn_01_4439	; $98FE
	ld a,$1F	; $9901
	jr c,loc_02_9971	; $9903
	ld a,(ix+$1B)	; $9905
	add a,$0B	; $9908
	call print_message	; $990A
	call wait_button	; $990D
	call fn_01_54CB	; $9910
	ld a,$20	; $9913
	call print_message	; $9915
	ld a,$02	; $9918
	call fn_01_4A66	; $991A

loc_02_991D:
	call frame_tick	; $991D
	call fn_01_4A9F	; $9920
	jr c,loc_02_991D	; $9923
	ld (ix+$21),l	; $9925
	call fn_01_54CB	; $9928
	ld a,(ix+$21)	; $992B
	or a	; $992E
	ld a,$21	; $992F
	jr z,loc_02_9971	; $9931
	ld a,(ix+$1B)	; $9933
	ld hl,$9841	; $9936
	rst $20	; $9939
	ex de,hl	; $993A
	ld hl,(gold)	; $993B
	ld b,(ix+$21)	; $993E
	or a	; $9941
	ld a,$1A	; $9942

loc_02_9944:
	sbc hl,de	; $9944
	jr c,loc_02_9971	; $9946
	djnz loc_02_9944	; $9948
	ld (gold),hl	; $994A
	call fn_01_43DA	; $994D
	ld a,(ix+$1B)	; $9950
	ld hl,$C91F	; $9953
	rst $30	; $9956
	ld a,(hl)	; $9957
	add a,(ix+$21)	; $9958
	cp $64	; $995B
	jr c,loc_02_9961	; $995D
	ld a,$63	; $995F

loc_02_9961:
	ld (hl),a	; $9961
	ld a,$08	; $9962
	call fn_01_7552	; $9964
	ld a,(shop_number)	; $9967
	add a,$06	; $996A
	call print_message	; $996C
	ld a,$1B	; $996F

loc_02_9971:
	call print_message	; $9971
	call wait_button	; $9974
	call fn_01_54CB	; $9977
	ld a,$22	; $997A
	call print_message	; $997C
	call fn_01_4B5E	; $997F

loc_02_9982:
	call frame_tick	; $9982
	call fn_01_4B69	; $9985
	jr c,loc_02_9982	; $9988
	ld a,$08	; $998A
	call fn_01_7552	; $998C
	bit 0,(ix+$07)	; $998F
	jp z,loc_02_98A5	; $9993
	jp loc_02_9819	; $9996

fn_02_9999:
	ld a,(shop_number)	; $9999
	add a,a	; $999C
	add a,a	; $999D
	add a,(ix+$1A)	; $999E
	ld hl,$9829	; $99A1
	rst $28	; $99A4
	ret	; $99A5

loc_02_99A6:
	call print_newline	; $99A6
	ld (ix+$1A),$23	; $99A9
	call fn_02_ABB7	; $99AD
	ld a,$25	; $99B0
	jp z,loc_02_9ADA	; $99B2

loc_02_99B5:
	ld a,$08	; $99B5
	call fn_01_7552	; $99B7
	ld a,(ix+$1A)	; $99BA
	call print_message	; $99BD
	ld a,$24	; $99C0
	call print_message	; $99C2
	ld (ix+$1A),$00	; $99C5

loc_02_99C9:
	call frame_tick	; $99C9
	ld c,(ix+$1B)	; $99CC
	ld de,$0101	; $99CF
	call fn_01_49F4	; $99D2
	jr z,loc_02_99C9	; $99D5
	ld a,$08	; $99D7
	call fn_01_7552	; $99D9
	jp c,loc_02_9816	; $99DC
	ld b,(ix+$1A)	; $99DF
	inc b	; $99E2
	xor a	; $99E3
	ld c,a	; $99E4
	ld hl,$C91F	; $99E5

loc_02_99E8:
	inc hl	; $99E8
	inc c	; $99E9
	cp (hl)	; $99EA
	jr z,loc_02_99E8	; $99EB
	djnz loc_02_99E8	; $99ED
	ld (ix+$1B),c	; $99EF
	ld (ix+$1C),l	; $99F2
	ld (ix+$1D),h	; $99F5
	ld a,c	; $99F8
	call fn_02_AB85	; $99F9
	call fn_01_54CB	; $99FC
	ld (ix+$06),$00	; $99FF
	ld (ix+$21),$01	; $9A03
	ld l,(ix+$1C)	; $9A07
	ld h,(ix+$1D)	; $9A0A
	ld a,(hl)	; $9A0D
	dec a	; $9A0E
	jr z,loc_02_9A50	; $9A0F
	ld a,$26	; $9A11
	call print_message	; $9A13
	ld a,(ix+$1B)	; $9A16
	call fn_02_AB85	; $9A19
	ld a,$27	; $9A1C
	call print_message	; $9A1E
	ld a,$02	; $9A21
	call fn_01_4A66	; $9A23

loc_02_9A26:
	call frame_tick	; $9A26
	call fn_01_4A9F	; $9A29
	jr c,loc_02_9A26	; $9A2C
	ld (ix+$21),l	; $9A2E
	call fn_01_54CB	; $9A31
	set 0,(ix+$06)	; $9A34
	ld a,(ix+$21)	; $9A38
	or a	; $9A3B
	ld a,$21	; $9A3C
	jp z,loc_02_9ADA	; $9A3E
	ld l,(ix+$1C)	; $9A41
	ld h,(ix+$1D)	; $9A44
	ld a,(hl)	; $9A47
	cp (ix+$21)	; $9A48
	ld a,$28	; $9A4B
	jp c,loc_02_9ADA	; $9A4D

loc_02_9A50:
	ld a,$29	; $9A50
	call print_message	; $9A52
	ld hl,$9841	; $9A55
	call fn_02_AB92	; $9A58
	bit 0,(ix+$06)	; $9A5B
	jr z,loc_02_9A7F	; $9A5F
	ld a,(ix+$21)	; $9A61
	dec a	; $9A64
	jr z,loc_02_9A73	; $9A65
	ld a,$2A	; $9A67
	call print_message	; $9A69
	ld a,$2B	; $9A6C
	call print_message	; $9A6E
	jr loc_02_9A8D	; $9A71

loc_02_9A73:
	ld a,$2A	; $9A73
	call print_message	; $9A75
	ld a,$2C	; $9A78
	call print_message	; $9A7A
	jr loc_02_9A8D	; $9A7D

loc_02_9A7F:
	ld a,$2D	; $9A7F
	call print_message	; $9A81
	ld a,(ix+$1B)	; $9A84
	call fn_02_AB85	; $9A87
	call fn_01_548D	; $9A8A

loc_02_9A8D:
	call wait_button	; $9A8D
	call fn_01_54CB	; $9A90
	ld a,$2E	; $9A93
	call print_message	; $9A95
	call fn_01_4B5E	; $9A98

loc_02_9A9B:
	call frame_tick	; $9A9B
	call fn_01_4B69	; $9A9E
	jr c,loc_02_9A9B	; $9AA1
	ld a,$08	; $9AA3
	call fn_01_7552	; $9AA5
	bit 0,(ix+$07)	; $9AA8
	ld a,$2F	; $9AAC
	jr nz,loc_02_9ACB	; $9AAE
	ld l,(ix+$1C)	; $9AB0
	ld h,(ix+$1D)	; $9AB3
	ld a,(hl)	; $9AB6
	sub (ix+$21)	; $9AB7
	ld (hl),a	; $9ABA
	ld l,(ix+$1F)	; $9ABB
	ld h,(ix+$20)	; $9ABE
	call fn_01_4412	; $9AC1
	ld a,$08	; $9AC4
	call fn_01_7552	; $9AC6
	ld a,$30	; $9AC9

loc_02_9ACB:
	ld (ix+$1A),a	; $9ACB
	call fn_02_ABB7	; $9ACE
	jp z,loc_02_9819	; $9AD1
	call print_newline	; $9AD4
	jp loc_02_99B5	; $9AD7

loc_02_9ADA:
	call print_message	; $9ADA
	call wait_button	; $9ADD
	jp loc_02_9816	; $9AE0

fn_02_9AE3:
	ld a,$29	; $9AE3
	call print_message	; $9AE5
	ld a,(shop_number)	; $9AE8
	call print_message	; $9AEB
	call fn_01_548D	; $9AEE
	ld a,$09	; $9AF1
	call fn_01_7552	; $9AF3
	call wait_button	; $9AF6
	call fn_01_54CB	; $9AF9
	ld a,(shop_number)	; $9AFC
	add a,$05	; $9AFF
	call print_message	; $9B01
	ld a,$2A	; $9B04
	call print_message	; $9B06
	call fn_02_AB4B	; $9B09

loc_02_9B0C:
	call frame_tick	; $9B0C
	call fn_02_AB55	; $9B0F
	jr z,loc_02_9B0C	; $9B12
	push af	; $9B14
	ld a,$09	; $9B15
	call fn_01_7552	; $9B17
	pop af	; $9B1A
	jr c,loc_02_9B24	; $9B1B
	or a	; $9B1D
	jp z,loc_02_9B5B	; $9B1E
	jp loc_02_9CAC	; $9B21

loc_02_9B24:
	call print_newline	; $9B24

loc_02_9B27:
	call print_newline	; $9B27
	ld a,(shop_number)	; $9B2A
	add a,$05	; $9B2D
	call print_message	; $9B2F
	ld a,$2B	; $9B32
	jp loc_02_96B2	; $9B34

armour_shop_stock:
	.INCBIN "bank02.bin" SKIP $1B37 READ 20	; data, 20 bytes

armour_prices:
	.INCBIN "bank02.bin" SKIP $1B4B READ 16	; data, 16 bytes

loc_02_9B5B:
	call print_newline	; $9B5B
	ld a,$13	; $9B5E
	call print_message	; $9B60
	call wait_button	; $9B63
	call fn_01_54CB	; $9B66
	ld a,$14	; $9B69
	call print_message	; $9B6B
	ld (ix+$1A),$00	; $9B6E
	ld (ix+$16),$04	; $9B72

loc_02_9B76:
	call fn_02_9C9F	; $9B76
	or a	; $9B79
	jr z,loc_02_9B8F	; $9B7A
	call print_newline	; $9B7C
	ld a,$20	; $9B7F
	call print_char	; $9B81
	ld a,$20	; $9B84
	call print_char	; $9B86
	call fn_02_9C9F	; $9B89
	call fn_02_AB89	; $9B8C

loc_02_9B8F:
	inc (ix+$1A)	; $9B8F
	dec (ix+$16)	; $9B92
	jr nz,loc_02_9B76	; $9B95
	call fn_01_548D	; $9B97
	call wait_button	; $9B9A
	call print_newline	; $9B9D

loc_02_9BA0:
	call print_newline	; $9BA0
	ld a,$15	; $9BA3
	call print_message	; $9BA5
	call fn_00_2E1B	; $9BA8
	ld (ix+$1A),$00	; $9BAB
	ld (ix+$1B),$00	; $9BAF
	ld b,$04	; $9BB3
	ld de,$0213	; $9BB5

loc_02_9BB8:
	push bc	; $9BB8
	push de	; $9BB9
	call fn_02_9C9F	; $9BBA
	or a	; $9BBD
	jr z,loc_02_9BCB	; $9BBE
	inc (ix+$1B)	; $9BC0
	add a,$35	; $9BC3
	call fn_00_340F	; $9BC5
	call fn_00_2D69	; $9BC8

loc_02_9BCB:
	pop de	; $9BCB
	pop bc	; $9BCC
	inc d	; $9BCD
	inc d	; $9BCE
	inc (ix+$1A)	; $9BCF
	djnz loc_02_9BB8	; $9BD2
	call fn_00_2E54	; $9BD4
	ld (ix+$1A),$00	; $9BD7

loc_02_9BDB:
	call frame_tick	; $9BDB
	ld c,(ix+$1B)	; $9BDE
	ld de,$0102	; $9BE1
	call fn_01_49F4	; $9BE4
	jr z,loc_02_9BDB	; $9BE7
	ld a,$09	; $9BE9
	call fn_01_7552	; $9BEB
	jp c,loc_02_9B24	; $9BEE
	call fn_02_9C9F	; $9BF1
	ld (ix+$1B),a	; $9BF4
	call fn_02_AB89	; $9BF7
	call fn_01_54CB	; $9BFA
	ld a,(ix+$1B)	; $9BFD
	ld hl,$9B4B	; $9C00
	rst $20	; $9C03
	call fn_01_4439	; $9C04
	ld a,$16	; $9C07
	jr c,loc_02_9C77	; $9C09
	ld a,(ix+$1B)	; $9C0B
	add a,$0A	; $9C0E
	call print_message	; $9C10
	call wait_button	; $9C13
	call fn_01_54CB	; $9C16
	ld a,$17	; $9C19
	call print_message	; $9C1B
	ld a,$02	; $9C1E
	call fn_01_4A66	; $9C20

loc_02_9C23:
	call frame_tick	; $9C23
	call fn_01_4A9F	; $9C26
	jr c,loc_02_9C23	; $9C29
	ld (ix+$21),l	; $9C2B
	call fn_01_54CB	; $9C2E
	ld a,(ix+$21)	; $9C31
	or a	; $9C34
	ld a,$18	; $9C35
	jr z,loc_02_9C77	; $9C37
	ld a,(ix+$1B)	; $9C39
	ld hl,$9B4B	; $9C3C
	rst $20	; $9C3F
	ex de,hl	; $9C40
	ld hl,(gold)	; $9C41
	ld b,(ix+$21)	; $9C44
	or a	; $9C47
	ld a,$11	; $9C48

loc_02_9C4A:
	sbc hl,de	; $9C4A
	jr c,loc_02_9C77	; $9C4C
	djnz loc_02_9C4A	; $9C4E
	ld (gold),hl	; $9C50
	call fn_01_43DA	; $9C53
	ld a,(ix+$1B)	; $9C56
	ld hl,$C917	; $9C59
	rst $30	; $9C5C
	ld a,(hl)	; $9C5D
	add a,(ix+$21)	; $9C5E
	cp $64	; $9C61
	jr c,loc_02_9C67	; $9C63
	ld a,$63	; $9C65

loc_02_9C67:
	ld (hl),a	; $9C67
	ld a,$09	; $9C68
	call fn_01_7552	; $9C6A
	ld a,(shop_number)	; $9C6D
	add a,$05	; $9C70
	call print_message	; $9C72
	ld a,$12	; $9C75

loc_02_9C77:
	call print_message	; $9C77
	call wait_button	; $9C7A
	call fn_01_54CB	; $9C7D
	ld a,$19	; $9C80
	call print_message	; $9C82
	call fn_01_4B5E	; $9C85

loc_02_9C88:
	call frame_tick	; $9C88
	call fn_01_4B69	; $9C8B
	jr c,loc_02_9C88	; $9C8E
	ld a,$09	; $9C90
	call fn_01_7552	; $9C92
	bit 0,(ix+$07)	; $9C95
	jp z,loc_02_9BA0	; $9C99
	jp loc_02_9B27	; $9C9C

fn_02_9C9F:
	ld a,(shop_number)	; $9C9F
	add a,a	; $9CA2
	add a,a	; $9CA3
	add a,(ix+$1A)	; $9CA4
	ld hl,$9B37	; $9CA7
	rst $28	; $9CAA
	ret	; $9CAB

loc_02_9CAC:
	call print_newline	; $9CAC
	ld (ix+$1A),$1A	; $9CAF
	call fn_02_ABBE	; $9CB3
	ld a,$1C	; $9CB6
	jp z,loc_02_9DE0	; $9CB8

loc_02_9CBB:
	ld a,$09	; $9CBB
	call fn_01_7552	; $9CBD
	ld a,(ix+$1A)	; $9CC0
	call print_message	; $9CC3
	ld a,$1B	; $9CC6
	call print_message	; $9CC8
	ld (ix+$1A),$00	; $9CCB

loc_02_9CCF:
	call frame_tick	; $9CCF
	ld c,(ix+$1B)	; $9CD2
	ld de,$0202	; $9CD5
	call fn_01_49F4	; $9CD8
	jr z,loc_02_9CCF	; $9CDB
	ld a,$09	; $9CDD
	call fn_01_7552	; $9CDF
	jp c,loc_02_9B24	; $9CE2
	ld b,(ix+$1A)	; $9CE5
	inc b	; $9CE8
	xor a	; $9CE9
	ld c,a	; $9CEA
	ld hl,$C917	; $9CEB

loc_02_9CEE:
	inc hl	; $9CEE
	inc c	; $9CEF
	cp (hl)	; $9CF0
	jr z,loc_02_9CEE	; $9CF1
	djnz loc_02_9CEE	; $9CF3
	ld (ix+$1B),c	; $9CF5
	ld (ix+$1C),l	; $9CF8
	ld (ix+$1D),h	; $9CFB
	ld a,c	; $9CFE
	call fn_02_AB89	; $9CFF
	call fn_01_54CB	; $9D02
	ld (ix+$06),$00	; $9D05
	ld (ix+$21),$01	; $9D09
	ld l,(ix+$1C)	; $9D0D
	ld h,(ix+$1D)	; $9D10
	ld a,(hl)	; $9D13
	dec a	; $9D14
	jr z,loc_02_9D56	; $9D15
	ld a,$1D	; $9D17
	call print_message	; $9D19
	ld a,(ix+$1B)	; $9D1C
	call fn_02_AB89	; $9D1F
	ld a,$1E	; $9D22
	call print_message	; $9D24
	ld a,$02	; $9D27
	call fn_01_4A66	; $9D29

loc_02_9D2C:
	call frame_tick	; $9D2C
	call fn_01_4A9F	; $9D2F
	jr c,loc_02_9D2C	; $9D32
	ld (ix+$21),l	; $9D34
	call fn_01_54CB	; $9D37
	set 0,(ix+$06)	; $9D3A
	ld a,(ix+$21)	; $9D3E
	or a	; $9D41
	ld a,$18	; $9D42
	jp z,loc_02_9DE0	; $9D44
	ld l,(ix+$1C)	; $9D47
	ld h,(ix+$1D)	; $9D4A
	ld a,(hl)	; $9D4D
	cp (ix+$21)	; $9D4E
	ld a,$1F	; $9D51
	jp c,loc_02_9DE0	; $9D53

loc_02_9D56:
	ld a,$20	; $9D56
	call print_message	; $9D58
	ld hl,$9B4B	; $9D5B
	call fn_02_AB92	; $9D5E
	bit 0,(ix+$06)	; $9D61
	jr z,loc_02_9D85	; $9D65
	ld a,(ix+$21)	; $9D67
	dec a	; $9D6A
	jr z,loc_02_9D79	; $9D6B
	ld a,$21	; $9D6D
	call print_message	; $9D6F
	ld a,$22	; $9D72
	call print_message	; $9D74
	jr loc_02_9D93	; $9D77

loc_02_9D79:
	ld a,$21	; $9D79
	call print_message	; $9D7B
	ld a,$23	; $9D7E
	call print_message	; $9D80
	jr loc_02_9D93	; $9D83

loc_02_9D85:
	ld a,$25	; $9D85
	call print_message	; $9D87
	ld a,(ix+$1B)	; $9D8A
	call fn_02_AB89	; $9D8D
	call fn_01_548D	; $9D90

loc_02_9D93:
	call wait_button	; $9D93
	call fn_01_54CB	; $9D96
	ld a,$26	; $9D99
	call print_message	; $9D9B
	call fn_01_4B5E	; $9D9E

loc_02_9DA1:
	call frame_tick	; $9DA1
	call fn_01_4B69	; $9DA4
	jr c,loc_02_9DA1	; $9DA7
	ld a,$09	; $9DA9
	call fn_01_7552	; $9DAB
	bit 0,(ix+$07)	; $9DAE
	ld a,$27	; $9DB2
	jr nz,loc_02_9DD1	; $9DB4
	ld l,(ix+$1C)	; $9DB6
	ld h,(ix+$1D)	; $9DB9
	ld a,(hl)	; $9DBC
	sub (ix+$21)	; $9DBD
	ld (hl),a	; $9DC0
	ld l,(ix+$1F)	; $9DC1
	ld h,(ix+$20)	; $9DC4
	call fn_01_4412	; $9DC7
	ld a,$09	; $9DCA
	call fn_01_7552	; $9DCC
	ld a,$28	; $9DCF

loc_02_9DD1:
	ld (ix+$1A),a	; $9DD1
	call fn_02_ABBE	; $9DD4
	jp z,loc_02_9B27	; $9DD7
	call print_newline	; $9DDA
	jp loc_02_9CBB	; $9DDD

loc_02_9DE0:
	call print_message	; $9DE0
	call wait_button	; $9DE3
	jp loc_02_9B24	; $9DE6

fn_02_9DE9:
	ld a,$0A	; $9DE9
	call print_message	; $9DEB
	ld a,(shop_number)	; $9DEE
	call print_message	; $9DF1
	call fn_01_548D	; $9DF4
	call wait_button	; $9DF7
	call fn_01_54CB	; $9DFA
	ld a,(shop_number)	; $9DFD
	add a,$05	; $9E00
	call print_message	; $9E02
	ld a,$0B	; $9E05
	call print_message	; $9E07
	call fn_01_4B5E	; $9E0A

loc_02_9E0D:
	call frame_tick	; $9E0D
	call fn_01_4B69	; $9E10
	jr c,loc_02_9E0D	; $9E13
	call fn_01_4324	; $9E15
	bit 0,(ix+$07)	; $9E18
	jp nz,loc_02_9ED0	; $9E1C
	call print_newline	; $9E1F
	ld a,$0D	; $9E22
	call print_message	; $9E24
	ld a,(shop_number)	; $9E27
	ld hl,$9ED8	; $9E2A
	rst $20	; $9E2D
	ld (ix+$1C),l	; $9E2E
	ld (ix+$1D),h	; $9E31
	call fn_01_5627	; $9E34
	ld a,$0E	; $9E37
	call print_message	; $9E39
	call wait_button	; $9E3C

loc_02_9E3F:
	call fn_01_54CB	; $9E3F

loc_02_9E42:
	ld a,$0F	; $9E42
	call print_message	; $9E44
	ld a,$03	; $9E47
	call fn_01_4A66	; $9E49

loc_02_9E4C:
	call frame_tick	; $9E4C
	call fn_01_4A9F	; $9E4F
	jr c,loc_02_9E4C	; $9E52
	call fn_01_54CB	; $9E54
	ld (ix+$20),l	; $9E57
	ld (ix+$21),h	; $9E5A
	ld a,h	; $9E5D
	or l	; $9E5E
	ld a,$10	; $9E5F
	jr z,loc_02_9ECD	; $9E61
	ld e,(ix+$1C)	; $9E63
	ld d,(ix+$1D)	; $9E66
	call fn_00_2AC8	; $9E69
	call fn_01_4423	; $9E6C
	jr c,loc_02_9E9E	; $9E6F
	ld l,(ix+$20)	; $9E71
	ld h,(ix+$21)	; $9E74
	ld de,$0019	; $9E77
	call fn_00_2AC8	; $9E7A
	call fn_02_ABD2	; $9E7D
	ld a,$14	; $9E80
	call print_message	; $9E82
	call fn_01_4B5E	; $9E85

loc_02_9E88:
	call frame_tick	; $9E88
	call fn_01_4B69	; $9E8B
	jr c,loc_02_9E88	; $9E8E
	call fn_01_4324	; $9E90
	bit 0,(ix+$07)	; $9E93
	jr nz,loc_02_9ED0	; $9E97
	call print_newline	; $9E99
	jr loc_02_9E42	; $9E9C

loc_02_9E9E:
	ld hl,(gold)	; $9E9E
	ld e,(ix+$1C)	; $9EA1
	ld d,(ix+$1D)	; $9EA4
	or a	; $9EA7
	sbc hl,de	; $9EA8
	ld a,$11	; $9EAA
	jr c,loc_02_9ECD	; $9EAC
	ld a,$12	; $9EAE
	call print_message	; $9EB0
	ld hl,(gold)	; $9EB3
	ld e,(ix+$1C)	; $9EB6
	ld d,(ix+$1D)	; $9EB9
	call fn_00_2B3F	; $9EBC
	call fn_01_5627	; $9EBF
	ld a,$13	; $9EC2
	call print_message	; $9EC4
	call wait_button	; $9EC7
	jp loc_02_9E3F	; $9ECA

loc_02_9ECD:
	call print_message	; $9ECD

loc_02_9ED0:
	call print_newline	; $9ED0
	ld a,$0C	; $9ED3
	jp loc_02_96B2	; $9ED5

food_prices:
	.INCBIN "bank02.bin" SKIP $1ED8 READ 10	; data, 10 bytes

fn_02_9EE2:
	ld (ix+$17),$00	; $9EE2
	ld a,(shop_number)	; $9EE6
	add a,$06	; $9EE9
	call print_message	; $9EEB
	ld a,$30	; $9EEE
	call print_message	; $9EF0
	ld a,(shop_number)	; $9EF3
	call print_message	; $9EF6
	call fn_01_548D	; $9EF9
	call wait_button	; $9EFC
	call print_newline	; $9EFF

loc_02_9F02:
	call print_newline	; $9F02
	ld a,(shop_number)	; $9F05
	add a,$06	; $9F08
	call print_message	; $9F0A
	ld a,$31	; $9F0D
	call print_message	; $9F0F
	ld hl,$9F7A	; $9F12
	call fn_02_AB4E	; $9F15

loc_02_9F18:
	call frame_tick	; $9F18
	ld c,$02	; $9F1B
	ld de,$0102	; $9F1D
	call fn_01_49F4	; $9F20
	jr z,loc_02_9F18	; $9F23
	call fn_01_4324	; $9F25
	jr c,loc_02_9F63	; $9F28
	bit 0,(ix+$1A)	; $9F2A
	jp z,loc_02_9F89	; $9F2E
	jp loc_02_A021	; $9F31

loc_02_9F34:
	ld a,$32	; $9F34
	call print_message	; $9F36

loc_02_9F39:
	call random	; $9F39
	and $01	; $9F3C
	ld a,$33	; $9F3E
	jr z,loc_02_9F44	; $9F40
	ld a,$2F	; $9F42

loc_02_9F44:
	call print_message	; $9F44
	call fn_01_4B5E	; $9F47

loc_02_9F4A:
	call frame_tick	; $9F4A
	call fn_01_4B69	; $9F4D
	jr c,loc_02_9F4A	; $9F50
	call fn_01_4324	; $9F52
	bit 0,(ix+$07)	; $9F55
	jr z,loc_02_9F02	; $9F59
	jr loc_02_9F66	; $9F5B

loc_02_9F5D:
	call print_message	; $9F5D
	call wait_button	; $9F60

loc_02_9F63:
	call print_newline	; $9F63

loc_02_9F66:
	call print_newline	; $9F66
	ld a,$34	; $9F69
	jp loc_02_96B2	; $9F6B
	.INCBIN "bank02.bin" SKIP $1F6E READ 27	; data, 27 bytes

loc_02_9F89:
	ld hl,$9F7C	; $9F89
	call print_text_hl	; $9F8C
	call fn_01_54CB	; $9F8F
	ld a,$1F	; $9F92
	call print_message	; $9F94
	ld a,(shop_number)	; $9F97
	add a,$0C	; $9F9A
	call print_message	; $9F9C
	ld a,$20	; $9F9F
	call print_message	; $9FA1
	ld a,(shop_number)	; $9FA4
	ld hl,$9F74	; $9FA7
	rst $28	; $9FAA
	ld (ix+$1C),a	; $9FAB
	ld l,a	; $9FAE
	ld h,$00	; $9FAF
	call fn_01_5627	; $9FB1
	ld a,$21	; $9FB4
	call print_message	; $9FB6
	ld a,$02	; $9FB9
	call fn_01_4A66	; $9FBB

loc_02_9FBE:
	call frame_tick	; $9FBE
	call fn_01_4A9F	; $9FC1
	jr c,loc_02_9FBE	; $9FC4
	ld (ix+$1D),l	; $9FC6
	ld a,l	; $9FC9
	or a	; $9FCA
	jp z,loc_02_9F63	; $9FCB
	call fn_01_54CB	; $9FCE
	ld h,(ix+$1C)	; $9FD1
	call multiply_8x8	; $9FD4
	call fn_01_4423	; $9FD7
	jr c,loc_02_9FE7	; $9FDA
	ld l,(ix+$1D)	; $9FDC
	ld h,$00	; $9FDF
	call fn_02_ABD2	; $9FE1
	jp loc_02_9F34	; $9FE4

loc_02_9FE7:
	ld hl,(gold)	; $9FE7
	ld e,(ix+$1C)	; $9FEA
	ld d,$00	; $9FED
	or a	; $9FEF
	sbc hl,de	; $9FF0
	ld a,$22	; $9FF2
	jp c,loc_02_9F5D	; $9FF4
	ld a,$23	; $9FF7
	call print_message	; $9FF9
	ld hl,(gold)	; $9FFC
	ld e,(ix+$1C)	; $9FFF
	ld d,$00	; $A002
	call fn_00_2B3F	; $A004
	push hl	; $A007
	call fn_01_5627	; $A008
	pop hl	; $A00B
	dec hl	; $A00C
	ld a,h	; $A00D
	or l	; $A00E
	ld a,$24	; $A00F
	jr z,loc_02_A015	; $A011
	ld a,$35	; $A013

loc_02_A015:
	call print_message	; $A015
	call wait_button	; $A018
	call fn_01_54CB	; $A01B
	jp loc_02_9F39	; $A01E

loc_02_A021:
	ld hl,$9F83	; $A021
	call print_text_hl	; $A024
	call fn_01_54CB	; $A027
	inc (ix+$17)	; $A02A
	ld a,(ix+$17)	; $A02D
	cp $04	; $A030
	jr c,loc_02_A041	; $A032
	ld a,(shop_number)	; $A034
	add a,$06	; $A037
	call print_message	; $A039
	ld a,$25	; $A03C
	jp loc_02_9F5D	; $A03E

loc_02_A041:
	ld a,$26	; $A041
	call print_message	; $A043
	ld a,$02	; $A046
	call fn_01_4A66	; $A048

loc_02_A04B:
	call frame_tick	; $A04B
	call fn_01_4A9F	; $A04E
	jr c,loc_02_A04B	; $A051
	ld (ix+$21),l	; $A053
	inc l	; $A056
	dec l	; $A057
	jp z,loc_02_9F63	; $A058
	call fn_01_54CB	; $A05B
	ld a,(ix+$21)	; $A05E
	cp $02	; $A061
	ld a,$27	; $A063
	jp c,loc_02_9F5D	; $A065
	ld l,(ix+$21)	; $A068
	ld h,$00	; $A06B
	call fn_01_4423	; $A06D
	ld a,$28	; $A070
	jp c,loc_02_9F5D	; $A072
	ld a,(ix+$21)	; $A075
	cp $03	; $A078
	jp c,loc_02_9F34	; $A07A
	call fn_02_A173	; $A07D
	jp z,loc_02_9F34	; $A080
	ld a,$29	; $A083
	call print_message	; $A085
	call fn_00_2E1B	; $A088
	ld (ix+$1B),$00	; $A08B
	ld hl,$C9B1	; $A08F
	ld de,$0213	; $A092
	ld bc,$0600	; $A095

loc_02_A098:
	push bc	; $A098
	push hl	; $A099
	ld a,(hl)	; $A09A
	or a	; $A09B
	jr z,loc_02_A0AD	; $A09C
	inc (ix+$1B)	; $A09E
	ld hl,$A19F	; $A0A1
	ld a,c	; $A0A4
	rst $20	; $A0A5
	push de	; $A0A6
	call fn_00_2D69	; $A0A7
	pop de	; $A0AA
	inc d	; $A0AB
	inc d	; $A0AC

loc_02_A0AD:
	pop hl	; $A0AD
	pop bc	; $A0AE
	inc hl	; $A0AF
	inc c	; $A0B0
	djnz loc_02_A098	; $A0B1
	call fn_00_2E54	; $A0B3
	ld (ix+$1A),$00	; $A0B6

loc_02_A0BA:
	call frame_tick	; $A0BA
	ld c,(ix+$1B)	; $A0BD
	ld de,$0102	; $A0C0
	call fn_01_49F4	; $A0C3
	jr z,loc_02_A0BA	; $A0C6
	ld (ix+$1B),$FF	; $A0C8
	jr c,loc_02_A0EA	; $A0CC
	ld b,(ix+$1A)	; $A0CE
	inc b	; $A0D1
	xor a	; $A0D2
	ld c,$FF	; $A0D3
	ld hl,$C9B0	; $A0D5

loc_02_A0D8:
	inc hl	; $A0D8
	inc c	; $A0D9
	cp (hl)	; $A0DA
	jr z,loc_02_A0D8	; $A0DB
	djnz loc_02_A0D8	; $A0DD
	ld (ix+$1B),c	; $A0DF
	ld a,c	; $A0E2
	ld hl,$A19F	; $A0E3
	rst $20	; $A0E6
	call print_text_hl	; $A0E7

loc_02_A0EA:
	call fn_01_54CB	; $A0EA
	ld a,(ix+$1B)	; $A0ED
	ld hl,shop_number	; $A0F0
	cp (hl)	; $A0F3
	jr z,loc_02_A104	; $A0F4
	ld a,$2A	; $A0F6
	call print_message	; $A0F8
	call wait_button	; $A0FB
	call fn_01_54CB	; $A0FE
	jp loc_02_9F39	; $A101

loc_02_A104:
	ld hl,$9F6E	; $A104
	rst $28	; $A107
	ld (ix+$1C),a	; $A108

loc_02_A10B:
	ld a,(ix+$21)	; $A10B
	cp (ix+$1C)	; $A10E
	jr nc,loc_02_A148	; $A111
	ld a,$2B	; $A113
	call print_message	; $A115
	ld a,$02	; $A118
	call fn_01_4A66	; $A11A

loc_02_A11D:
	call frame_tick	; $A11D
	call fn_01_4A9F	; $A120
	jr c,loc_02_A11D	; $A123
	call fn_01_54CB	; $A125
	ld a,l	; $A128
	or a	; $A129
	ld a,$2C	; $A12A
	jp z,loc_02_9F5D	; $A12C
	ld a,l	; $A12F
	add a,(ix+$21)	; $A130
	ld (ix+$21),a	; $A133
	call fn_01_4423	; $A136
	jr nc,loc_02_A10B	; $A139
	ld a,$2D	; $A13B
	call print_message	; $A13D
	call print_newline	; $A140
	ld a,$2C	; $A143
	jp loc_02_9F5D	; $A145

loc_02_A148:
	ld a,(shop_number)	; $A148
	add a,$06	; $A14B
	call print_message	; $A14D
	ld a,$2E	; $A150
	call print_message	; $A152
	ld a,(shop_number)	; $A155
	add a,$19	; $A158
	call print_message	; $A15A
	call wait_button	; $A15D
	call fn_01_54CB	; $A160
	ld a,(shop_number)	; $A163
	ld hl,$A1E5	; $A166
	rst $28	; $A169
	ld hl,$C500	; $A16A
	rst $30	; $A16D
	ld (hl),$01	; $A16E
	jp loc_02_9F39	; $A170

fn_02_A173:
	ld hl,$C9B1	; $A173
	ld a,($C535)	; $A176
	ld (hl),a	; $A179
	inc hl	; $A17A
	ld a,($C51F)	; $A17B
	ld (hl),a	; $A17E
	inc hl	; $A17F
	ld a,($C52B)	; $A180
	ld (hl),a	; $A183
	inc hl	; $A184
	ld a,($C526)	; $A185
	ld (hl),a	; $A188
	inc hl	; $A189
	ld a,($C51A)	; $A18A
	ld (hl),a	; $A18D
	inc hl	; $A18E
	ld a,($C529)	; $A18F
	ld (hl),a	; $A192
	inc hl	; $A193
	ld hl,$C9B1	; $A194
	ld b,$06	; $A197
	xor a	; $A199

loc_02_A19A:
	or (hl)	; $A19A
	inc hl	; $A19B
	djnz loc_02_A19A	; $A19C
	ret	; $A19E

tbl_02_A19F:
	.dw $A1AB
	.dw fn_02_A1B7
	.dw fn_02_A1BF
	.dw fn_02_A1CB
	.dw fn_02_A1D4
	.dw fn_02_A1DA
	.dw fn_01_4C42
	.INCBIN "bank02.bin" SKIP $21AD READ 10	; data, 10 bytes

fn_02_A1B7:
	ld d,e	; $A1B7
	ld b,l	; $A1B8
	ld e,b	; $A1B9
	ld d,h	; $A1BA
	ld b,c	; $A1BB
	ld c,(hl)	; $A1BC
	ld d,h	; $A1BD
	nop	; $A1BE

fn_02_A1BF:
	ld d,a	; $A1BF
	ld c,b	; $A1C0
	ld c,c	; $A1C1
	ld d,h	; $A1C2
	ld b,l	; $A1C3
	jr nz,loc_02_A219	; $A1C4
	ld d,h	; $A1C6
	ld c,a	; $A1C7
	ld c,(hl)	; $A1C8
	ld b,l	; $A1C9
	nop	; $A1CA

fn_02_A1CB:
	ld c,l	; $A1CB
	ld b,c	; $A1CC
	ld c,(hl)	; $A1CD
	ld b,h	; $A1CE
	ld d,d	; $A1CF
	ld b,c	; $A1D0
	ld c,e	; $A1D1
	ld b,l	; $A1D2
	nop	; $A1D3

fn_02_A1D4:
	ld d,e	; $A1D4
	ld c,e	; $A1D5
	ld d,l	; $A1D6
	ld c,h	; $A1D7
	ld c,h	; $A1D8
	nop	; $A1D9

fn_02_A1DA:
	ld c,(hl)	; $A1DA
	ld c,c	; $A1DB
	ld b,a	; $A1DC
	ld c,b	; $A1DD
	ld d,h	; $A1DE
	ld d,e	; $A1DF
	ld c,b	; $A1E0
	ld b,c	; $A1E1
	ld b,h	; $A1E2
	ld b,l	; $A1E3
	nop	; $A1E4
	ld b,h	; $A1E5
	ld b,l	; $A1E6
	ld b,e	; $A1E7
	ccf	; $A1E8
	ld b,b	; $A1E9
	ld b,c	; $A1EA

fn_02_A1EB:
	ld a,$08	; $A1EB
	call print_message	; $A1ED
	ld a,(shop_number)	; $A1F0
	call print_message	; $A1F3
	call fn_01_548D	; $A1F6
	ld a,$0C	; $A1F9
	call fn_01_7552	; $A1FB
	call wait_button	; $A1FE
	call fn_01_54CB	; $A201
	ld a,$09	; $A204
	call print_message	; $A206
	ld a,(shop_number)	; $A209
	add a,$04	; $A20C
	call print_message	; $A20E
	ld a,$0A	; $A211
	call print_message	; $A213
	call fn_01_4B5E	; $A216

loc_02_A219:
	call frame_tick	; $A219
	call fn_01_4B69	; $A21C
	jr c,loc_02_A219	; $A21F
	ld a,$0C	; $A221
	call fn_01_7552	; $A223
	bit 0,(ix+$07)	; $A226
	jp nz,loc_02_A3A4	; $A22A
	call print_newline	; $A22D
	ld a,$0B	; $A230
	call print_message	; $A232
	call wait_button	; $A235
	call fn_01_54CB	; $A238
	ld a,$0C	; $A23B
	call print_message	; $A23D
	call wait_button	; $A240
	call print_newline	; $A243

loc_02_A246:
	call print_newline	; $A246
	ld a,$0D	; $A249
	call print_message	; $A24B
	call fn_00_2E1B	; $A24E
	ld de,$0213	; $A251
	ld bc,$065D	; $A254

loc_02_A257:
	push bc	; $A257
	push de	; $A258
	ld a,c	; $A259
	call fn_00_340F	; $A25A
	call fn_00_2D69	; $A25D
	pop de	; $A260
	pop bc	; $A261
	inc d	; $A262
	inc d	; $A263
	inc c	; $A264
	djnz loc_02_A257	; $A265
	call fn_00_2E54	; $A267
	ld (ix+$1A),$00	; $A26A

loc_02_A26E:
	call frame_tick	; $A26E
	ld c,$06	; $A271
	ld de,$0102	; $A273
	call fn_01_49F4	; $A276
	jr z,loc_02_A26E	; $A279
	ld a,$0C	; $A27B
	call fn_01_7552	; $A27D
	jp c,loc_02_A3A1	; $A280
	ld a,(ix+$1A)	; $A283
	call fn_02_AB8D	; $A286
	call fn_01_54CB	; $A289
	ld a,$0E	; $A28C
	call print_message	; $A28E
	ld a,(ix+$1A)	; $A291
	call fn_02_AB8D	; $A294
	ld a,$0F	; $A297
	call print_message	; $A299
	ld a,(shop_number)	; $A29C
	add a,a	; $A29F
	ld b,a	; $A2A0
	add a,a	; $A2A1
	add a,b	; $A2A2
	add a,(ix+$1A)	; $A2A3
	ld hl,$A3B7	; $A2A6
	rst $28	; $A2A9
	ld (ix+$1C),a	; $A2AA
	ld l,a	; $A2AD
	ld h,$00	; $A2AE
	call fn_01_5627	; $A2B0
	ld a,$10	; $A2B3
	call print_message	; $A2B5
	ld a,$02	; $A2B8
	call fn_01_4A66	; $A2BA

loc_02_A2BD:
	call frame_tick	; $A2BD
	call fn_01_4A9F	; $A2C0
	jr c,loc_02_A2BD	; $A2C3
	ld (ix+$1D),l	; $A2C5
	call fn_01_54CB	; $A2C8
	inc l	; $A2CB
	dec l	; $A2CC
	jr nz,loc_02_A2D7	; $A2CD

loc_02_A2CF:
	ld a,$15	; $A2CF
	call print_message	; $A2D1
	jp loc_02_A388	; $A2D4

loc_02_A2D7:
	ld a,$11	; $A2D7
	call print_message	; $A2D9
	ld l,(ix+$1C)	; $A2DC
	ld h,(ix+$1D)	; $A2DF
	call multiply_8x8	; $A2E2
	ld (ix+$1E),l	; $A2E5
	ld (ix+$1F),h	; $A2E8
	call fn_01_5627	; $A2EB
	ld a,$12	; $A2EE
	call print_message	; $A2F0
	ld a,$03	; $A2F3
	call fn_01_4A66	; $A2F5

loc_02_A2F8:
	call frame_tick	; $A2F8
	call fn_01_4A9F	; $A2FB
	jr c,loc_02_A2F8	; $A2FE
	ld (ix+$20),l	; $A300
	ld (ix+$21),h	; $A303
	call fn_01_54CB	; $A306
	ld a,h	; $A309
	or l	; $A30A
	jr z,loc_02_A2CF	; $A30B
	ld e,(ix+$1E)	; $A30D
	ld d,(ix+$1F)	; $A310
	or a	; $A313
	sbc hl,de	; $A314
	jr nc,loc_02_A341	; $A316
	ex de,hl	; $A318
	ld hl,$0000	; $A319
	or a	; $A31C
	sbc hl,de	; $A31D
	ld de,$0003	; $A31F
	call fn_00_2B3F	; $A322
	ld de,$0004	; $A325
	or a	; $A328
	sbc hl,de	; $A329
	jr nc,loc_02_A330	; $A32B
	ex de,hl	; $A32D
	jr loc_02_A331	; $A32E

loc_02_A330:
	add hl,de	; $A330

loc_02_A331:
	ld a,l	; $A331
	ld c,$00	; $A332
	call karma_lose	; $A334
	ld c,$03	; $A337
	call karma_lose	; $A339
	ld c,$05	; $A33C
	call karma_lose	; $A33E

loc_02_A341:
	ld l,(ix+$20)	; $A341
	ld h,(ix+$21)	; $A344
	call fn_01_4423	; $A347
	jr nc,loc_02_A350	; $A34A
	ld a,$13	; $A34C
	jr loc_02_A37A	; $A34E

loc_02_A350:
	ld a,$02	; $A350
	ld c,$00	; $A352
	call karma_gain	; $A354
	ld c,$03	; $A357
	call karma_gain	; $A359
	ld c,$05	; $A35C
	call karma_gain	; $A35E
	ld a,(ix+$1A)	; $A361
	ld hl,reagents	; $A364
	rst $30	; $A367
	ld a,(hl)	; $A368
	add a,(ix+$1D)	; $A369
	cp $63	; $A36C
	jr c,loc_02_A372	; $A36E
	ld a,$63	; $A370

loc_02_A372:
	ld (hl),a	; $A372
	ld a,$0C	; $A373
	call fn_01_7552	; $A375
	ld a,$14	; $A378

loc_02_A37A:
	call print_message	; $A37A
	call wait_button	; $A37D
	call fn_01_54CB	; $A380
	ld a,$16	; $A383
	call print_message	; $A385

loc_02_A388:
	call fn_01_4B5E	; $A388

loc_02_A38B:
	call frame_tick	; $A38B
	call fn_01_4B69	; $A38E
	jr c,loc_02_A38B	; $A391
	ld a,$0C	; $A393
	call fn_01_7552	; $A395
	bit 0,(ix+$07)	; $A398
	jp z,loc_02_A246	; $A39C
	jr loc_02_A3A4	; $A39F

loc_02_A3A1:
	call print_newline	; $A3A1

loc_02_A3A4:
	call print_newline	; $A3A4
	call fn_01_4324	; $A3A7
	ld a,(shop_number)	; $A3AA
	add a,$04	; $A3AD
	call print_message	; $A3AF
	ld a,$17	; $A3B2
	jp loc_02_96B2	; $A3B4

reagent_prices:
	.INCBIN "bank02.bin" SKIP $23B7 READ 24	; data, 24 bytes

fn_02_A3CF:
	ld a,$24	; $A3CF
	call print_message	; $A3D1
	ld a,(shop_number)	; $A3D4
	call print_message	; $A3D7
	call fn_01_548D	; $A3DA
	call wait_button	; $A3DD
	call fn_01_54CB	; $A3E0
	ld (ix+$17),$00	; $A3E3
	ld a,(shop_number)	; $A3E7
	add a,$0A	; $A3EA
	call print_message	; $A3EC
	ld a,$25	; $A3EF

loc_02_A3F1:
	call print_message	; $A3F1
	call fn_01_4B5E	; $A3F4

loc_02_A3F7:
	call frame_tick	; $A3F7
	call fn_01_4B69	; $A3FA
	jr c,loc_02_A3F7	; $A3FD
	call fn_01_4324	; $A3FF
	call print_newline	; $A402
	bit 0,(ix+$07)	; $A405
	jr z,loc_02_A465	; $A409

loc_02_A40B:
	ld hl,(party)	; $A40B
	ld de,$FE70	; $A40E
	add hl,de	; $A411
	jr nc,loc_02_A458	; $A412
	ld a,$22	; $A414
	call print_message	; $A416
	call fn_01_4B5E	; $A419

loc_02_A41C:
	call frame_tick	; $A41C
	call fn_01_4B69	; $A41F
	jr c,loc_02_A41C	; $A422
	call fn_01_4324	; $A424
	call print_newline	; $A427
	bit 0,(ix+$07)	; $A42A
	jr nz,loc_02_A451	; $A42E
	ld a,$05	; $A430
	ld c,$04	; $A432
	call karma_gain	; $A434
	ld hl,(party)	; $A437
	ld de,$FF9C	; $A43A
	add hl,de	; $A43D
	ld (party),hl	; $A43E
	call fn_01_4324	; $A441
	ld a,$23	; $A444
	call print_message	; $A446
	call wait_button	; $A449
	call fn_01_54CB	; $A44C
	jr loc_02_A458	; $A44F

loc_02_A451:
	ld a,$05	; $A451
	ld c,$04	; $A453
	call karma_lose	; $A455

loc_02_A458:
	ld a,(shop_number)	; $A458
	add a,$0A	; $A45B
	call print_message	; $A45D
	ld a,$29	; $A460
	jp loc_02_96B2	; $A462

loc_02_A465:
	bit 0,(ix+$17)	; $A465
	jr nz,loc_02_A482	; $A469
	ld a,(shop_number)	; $A46B
	add a,$0A	; $A46E
	call print_message	; $A470
	ld a,$26	; $A473
	call print_message	; $A475
	call wait_button	; $A478
	call fn_01_54CB	; $A47B
	set 0,(ix+$17)	; $A47E

loc_02_A482:
	ld a,$27	; $A482
	call print_message	; $A484
	ld hl,$A511	; $A487
	call fn_02_AB4E	; $A48A

loc_02_A48D:
	call frame_tick	; $A48D
	ld c,$03	; $A490
	ld de,$0102	; $A492
	call fn_01_49F4	; $A495
	jr z,loc_02_A48D	; $A498
	call fn_01_4324	; $A49A
	jr nc,loc_02_A4A5	; $A49D
	call fn_01_54CB	; $A49F
	jp loc_02_A40B	; $A4A2

loc_02_A4A5:
	ld a,(ix+$1A)	; $A4A5
	ld hl,$A50B	; $A4A8
	rst $20	; $A4AB
	call print_text_hl	; $A4AC
	call fn_01_54CB	; $A4AF
	ld a,(shop_number)	; $A4B2
	add a,$0A	; $A4B5
	call print_message	; $A4B7
	ld a,$14	; $A4BA
	call print_message	; $A4BC
	call print_newline	; $A4BF
	ld a,$15	; $A4C2
	call print_message	; $A4C4
	call fn_01_4BCB	; $A4C7

loc_02_A4CA:
	call frame_tick	; $A4CA
	call fn_01_4BD2	; $A4CD
	jr z,loc_02_A4CA	; $A4D0
	push af	; $A4D2
	jr nc,loc_02_A4DD	; $A4D3
	ld a,$16	; $A4D5
	call print_message	; $A4D7
	call print_newline	; $A4DA

loc_02_A4DD:
	call fn_01_4324	; $A4DD
	call print_newline	; $A4E0
	pop af	; $A4E3
	jr c,loc_02_A4FE	; $A4E4
	ld a,(ix+$1A)	; $A4E6
	ld hl,$A4EF	; $A4E9
	jp fn_00_0018	; $A4EC

tbl_02_A4EF:
	.dw fn_02_A534
	.dw fn_02_A57F
	.dw fn_02_A5CF
	.dw $82CD
	.INCBIN "bank02.bin" SKIP $24F7 READ 7	; data, 7 bytes

loc_02_A4FE:
	ld a,(shop_number)	; $A4FE
	add a,$0A	; $A501
	call print_message	; $A503
	ld a,$28	; $A506
	jp loc_02_A3F1	; $A508

tbl_02_A50B:
	.dw $A513
	.dw fn_02_A51C
	.dw fn_02_A526
	.dw fn_00_0213
	.dw fn_01_5543
	.dw fn_01_4952
	.dw fn_01_474E
	.INCBIN "bank02.bin" SKIP $2519 READ 3	; data, 3 bytes

fn_02_A51C:
	ld c,b	; $A51C
	ld b,l	; $A51D
	ld b,c	; $A51E
	ld c,h	; $A51F
	ld c,c	; $A520
	ld c,(hl)	; $A521
	ld b,a	; $A522
	nop	; $A523
	inc de	; $A524
	.INCBIN "bank02.bin" SKIP $2525 READ 1	; data, 1 bytes

fn_02_A526:
	ld d,d	; $A526
	ld b,l	; $A527
	ld d,e	; $A528
	ld d,l	; $A529
	ld d,d	; $A52A
	ld d,d	; $A52B
	ld b,l	; $A52C
	ld b,e	; $A52D
	ld d,h	; $A52E
	ld c,c	; $A52F
	ld c,a	; $A530
	ld c,(hl)	; $A531
	nop	; $A532
	rst $38	; $A533

fn_02_A534:
	ld c,(ix+$08)	; $A534
	call member_record	; $A537
	ld a,(iy+$1C)	; $A53A
	cp $50	; $A53D
	ld a,$19	; $A53F
	jp nz,$A4F5	; $A541
	ld a,$1A	; $A544
	call print_message	; $A546
	call wait_button	; $A549
	call fn_01_54CB	; $A54C
	ld hl,$0064	; $A54F
	call fn_01_4439	; $A552
	jr nc,loc_02_A567	; $A555
	ld a,$1B	; $A557
	call print_message	; $A559
	call wait_button	; $A55C
	call fn_01_54CB	; $A55F
	ld a,$1C	; $A562
	jp $A4F5	; $A564

loc_02_A567:
	call fn_02_A5F8	; $A567

loc_02_A56A:
	call frame_tick	; $A56A
	call fn_02_A606	; $A56D
	jr c,loc_02_A56A	; $A570
	call fn_02_A627	; $A572
	ld (iy+$1C),$47	; $A575
	call fn_01_4324	; $A579
	jp loc_02_A4FE	; $A57C

fn_02_A57F:
	ld c,(ix+$08)	; $A57F
	call member_record	; $A582
	ld l,(iy+$00)	; $A585
	ld h,(iy+$01)	; $A588
	ld e,(iy+$02)	; $A58B
	ld d,(iy+$03)	; $A58E
	or a	; $A591
	sbc hl,de	; $A592
	ld a,$1D	; $A594
	jp z,$A4F5	; $A596
	ld a,$1E	; $A599
	call print_message	; $A59B
	call wait_button	; $A59E
	call fn_01_54CB	; $A5A1
	ld hl,$00C8	; $A5A4
	call fn_01_4439	; $A5A7
	ld a,$1F	; $A5AA
	jp c,$A4F5	; $A5AC
	call fn_02_A5F8	; $A5AF

loc_02_A5B2:
	call frame_tick	; $A5B2
	call fn_02_A606	; $A5B5
	jr c,loc_02_A5B2	; $A5B8
	call fn_02_A627	; $A5BA
	ld l,(iy+$02)	; $A5BD
	ld h,(iy+$03)	; $A5C0
	ld (iy+$00),l	; $A5C3
	ld (iy+$01),h	; $A5C6
	call fn_01_4324	; $A5C9
	jp loc_02_A4FE	; $A5CC

fn_02_A5CF:
	ld c,(ix+$08)	; $A5CF
	call member_record	; $A5D2
	ld a,(iy+$1C)	; $A5D5
	cp $44	; $A5D8
	ld a,$20	; $A5DA
	jp nz,$A4F5	; $A5DC
	ld a,$21	; $A5DF
	call print_message	; $A5E1
	call wait_button	; $A5E4
	call fn_01_54CB	; $A5E7
	ld hl,$012C	; $A5EA
	call fn_01_4439	; $A5ED
	jp nc,loc_02_A567	; $A5F0
	ld a,$1F	; $A5F3
	jp $A4F5	; $A5F5

fn_02_A5F8:
	ld (ix+$20),l	; $A5F8
	ld (ix+$21),h	; $A5FB
	ld a,$17	; $A5FE
	call print_message	; $A600
	jp fn_01_4B5E	; $A603

fn_02_A606:
	call fn_01_4B69	; $A606
	ret c	; $A609
	call fn_01_4324	; $A60A
	call print_newline	; $A60D
	bit 0,(ix+$07)	; $A610
	jr z,loc_02_A61C	; $A614
	pop af	; $A616
	ld a,$18	; $A617
	jp $A4F5	; $A619

loc_02_A61C:
	ld l,(ix+$20)	; $A61C
	ld h,(ix+$21)	; $A61F
	call fn_01_4423	; $A622
	or a	; $A625
	ret	; $A626

fn_02_A627:
	ld a,$15	; $A627
	call fn_00_05F1	; $A629
	ld c,(ix+$08)	; $A62C
	call fn_01_49A6	; $A62F
	call wait_vblank	; $A632
	call fn_01_41A9	; $A635
	call wait_vblank	; $A638
	call fn_01_41A9	; $A63B
	call wait_vblank	; $A63E
	ld c,(ix+$08)	; $A641
	jp member_record	; $A644

fn_02_A647:
	ld a,($C959)	; $A647
	cp $1F	; $A64A
	ld a,$17	; $A64C
	jp nz,loc_02_96B2	; $A64E
	ld a,$18	; $A651
	call print_message	; $A653
	ld a,(shop_number)	; $A656
	call print_message	; $A659
	ld a,$19	; $A65C
	call print_message	; $A65E
	ld a,(shop_number)	; $A661
	add a,$07	; $A664
	call print_message	; $A666
	call print_inline	; $A669
	.INCBIN "bank02.bin" SKIP $266C READ 3	; text, 3 bytes
	call wait_button	; $A66F
	call fn_01_54CB	; $A672
	ld a,$1A	; $A675
	call print_message	; $A677
	call fn_01_4B5E	; $A67A

loc_02_A67D:
	call frame_tick	; $A67D
	call fn_01_4B69	; $A680
	jr c,loc_02_A67D	; $A683
	call fn_01_4324	; $A685
	call print_newline	; $A688
	bit 0,(ix+$07)	; $A68B
	jr z,loc_02_A69E	; $A68F
	ld a,(shop_number)	; $A691
	add a,$07	; $A694
	call print_message	; $A696
	ld a,$21	; $A699
	jp loc_02_96B2	; $A69B

loc_02_A69E:
	ld a,(shop_number)	; $A69E
	add a,$0E	; $A6A1
	call print_message	; $A6A3
	call wait_button	; $A6A6
	call fn_01_54CB	; $A6A9
	ld a,(shop_number)	; $A6AC
	cp $03	; $A6AF
	jr nz,loc_02_A6E4	; $A6B1
	ld a,$1B	; $A6B3
	call print_message	; $A6B5
	ld hl,$A85C	; $A6B8
	call fn_02_AB4E	; $A6BB

loc_02_A6BE:
	call frame_tick	; $A6BE
	ld c,$03	; $A6C1
	ld de,$0102	; $A6C3
	call fn_01_49F4	; $A6C6
	jr z,loc_02_A6BE	; $A6C9
	call fn_01_4324	; $A6CB
	jp c,loc_02_96B5	; $A6CE
	ld a,(ix+$1A)	; $A6D1
	add a,$31	; $A6D4
	call print_char	; $A6D6
	call fn_01_54CB	; $A6D9
	ld a,(ix+$1A)	; $A6DC
	ld hl,$A853	; $A6DF
	jr loc_02_A709	; $A6E2

loc_02_A6E4:
	ld a,$1C	; $A6E4
	call print_message	; $A6E6
	call fn_01_4B5E	; $A6E9

loc_02_A6EC:
	call frame_tick	; $A6EC
	call fn_01_4B69	; $A6EF
	jr c,loc_02_A6EC	; $A6F2
	call fn_01_4324	; $A6F4
	call print_newline	; $A6F7
	bit 0,(ix+$07)	; $A6FA
	ld a,$1D	; $A6FE
	jp nz,loc_02_96B2	; $A700
	ld a,(shop_number)	; $A703
	ld hl,$A83E	; $A706

loc_02_A709:
	ld b,a	; $A709
	add a,a	; $A70A
	add a,b	; $A70B
	rst $30	; $A70C
	ld a,(hl)	; $A70D
	ld (ix+$0A),a	; $A70E
	inc hl	; $A711
	ld a,(hl)	; $A712
	ld (ix+$0C),a	; $A713
	inc hl	; $A716
	ld l,(hl)	; $A717
	ld h,$00	; $A718
	call fn_01_4423	; $A71A
	ld a,$1E	; $A71D
	jp c,loc_02_96B2	; $A71F
	ld a,$1F	; $A722
	call print_message	; $A724
	call wait_button	; $A727
	call print_newline	; $A72A
	call random	; $A72D
	and $03	; $A730
	jr nz,loc_02_A742	; $A732
	call print_newline	; $A734
	ld a,$20	; $A737
	call print_message	; $A739
	call wait_button	; $A73C
	call print_newline	; $A73F

loc_02_A742:
	ld a,$09	; $A742
	call play_music_later	; $A744
	ld a,$05	; $A747
	call fn_00_3046	; $A749
	ld l,(ix+$0A)	; $A74C
	ld h,(ix+$0C)	; $A74F
	ld (party_x),hl	; $A752
	ld (view_x),hl	; $A755
	ld a,$05	; $A758
	call fn_00_3046	; $A75A
	ld hl,$0038	; $A75D
	ld ($C959),hl	; $A760
	ld a,$03	; $A763
	call fn_02_A81A	; $A765
	ld a,$01	; $A768
	call fn_00_3046	; $A76A
	ld a,$0C	; $A76D
	call fn_02_A81A	; $A76F
	ld a,$01	; $A772
	call fn_00_3046	; $A774
	ld a,$30	; $A777
	call fn_02_A81A	; $A779
	ld a,$C8	; $A77C
	call fn_00_3046	; $A77E
	call fn_02_A836	; $A781
	ld a,$01	; $A784
	call fn_00_3046	; $A786
	call fn_02_A836	; $A789
	ld a,$01	; $A78C
	call fn_00_3046	; $A78E
	call fn_02_A836	; $A791
	ld a,$05	; $A794
	call fn_00_3046	; $A796
	ld hl,$8A1F	; $A799
	ld ($C959),hl	; $A79C
	ld a,(party_size)	; $A79F
	ld b,a	; $A7A2
	ld c,$00	; $A7A3

loc_02_A7A5:
	call fn_01_4445	; $A7A5
	jr c,loc_02_A7B7	; $A7A8
	ld a,$32	; $A7AA
	call random_below	; $A7AC
	add a,a	; $A7AF
	ld hl,$0064	; $A7B0
	rst $30	; $A7B3
	call member_heal	; $A7B4

loc_02_A7B7:
	inc c	; $A7B7
	djnz loc_02_A7A5	; $A7B8
	call fn_01_4776	; $A7BA
	call fn_01_4324	; $A7BD
	ld c,$00	; $A7C0
	call fn_01_4445	; $A7C2
	jr c,loc_02_A7E7	; $A7C5
	call random	; $A7C7
	and $07	; $A7CA
	jr nz,loc_02_A7E7	; $A7CC
	call print_newline	; $A7CE
	ld a,$15	; $A7D1
	call print_message	; $A7D3
	call wait_button	; $A7D6
	call print_newline	; $A7D9
	ld a,$C8	; $A7DC
	ld ($C97D),a	; $A7DE
	ld ($C97E),a	; $A7E1
	jp loc_00_3B94	; $A7E4

loc_02_A7E7:
	ld a,(shop_number)	; $A7E7
	cp $05	; $A7EA
	jr nz,loc_02_A812	; $A7EC
	call random	; $A7EE
	and $03	; $A7F1
	jr nz,loc_02_A812	; $A7F3
	call fn_01_4933	; $A7F5
	ld (iy+$03),$9C	; $A7F8
	ld (iy+$00),$9C	; $A7FC
	ld hl,(party_x)	; $A800
	dec l	; $A803
	ld (iy+$01),l	; $A804
	ld (iy+$02),h	; $A807
	ld (iy+$06),$01	; $A80A
	ld (iy+$07),$10	; $A80E

loc_02_A812:
	call print_newline	; $A812
	ld a,$16	; $A815
	jp loc_02_96B2	; $A817

fn_02_A81A:
	ld d,a	; $A81A
	cpl	; $A81B
	ld e,a	; $A81C
	ld hl,$C023	; $A81D
	ld b,$20	; $A820

loc_02_A822:
	ld a,(hl)	; $A822
	and d	; $A823
	jr z,loc_02_A82D	; $A824
	dec a	; $A826
	and d	; $A827
	ld c,a	; $A828
	ld a,(hl)	; $A829
	and e	; $A82A
	or c	; $A82B
	ld (hl),a	; $A82C

loc_02_A82D:
	inc hl	; $A82D
	djnz loc_02_A822	; $A82E
	ld a,$FF	; $A830
	ld ($C009),a	; $A832
	ret	; $A835

fn_02_A836:
	ld hl,$7C83	; $A836
	ld c,$01	; $A839
	jp loc_00_0855	; $A83B
	.INCBIN "bank02.bin" SKIP $283E READ 43	; data, 43 bytes

fn_02_A869:
	ld a,$0A	; $A869
	call fn_01_7552	; $A86B
	ld a,$08	; $A86E
	call print_message	; $A870
	ld a,(shop_number)	; $A873
	add a,$02	; $A876
	call print_message	; $A878
	ld a,$09	; $A87B
	call print_message	; $A87D
	call wait_button	; $A880
	call fn_01_54CB	; $A883
	ld a,(shop_number)	; $A886
	add a,$02	; $A889
	call print_message	; $A88B
	ld a,$0A	; $A88E
	call print_message	; $A890
	ld a,(shop_number)	; $A893
	call print_message	; $A896
	call fn_01_548D	; $A899
	call wait_button	; $A89C
	call fn_01_54CB	; $A89F
	ld (ix+$1A),$00	; $A8A2
	ld (ix+$06),$00	; $A8A6
	ld a,$0B	; $A8AA

loc_02_A8AC:
	call print_message	; $A8AC
	call fn_01_4B5E	; $A8AF

loc_02_A8B2:
	call frame_tick	; $A8B2
	call fn_01_4B69	; $A8B5
	jr c,loc_02_A8B2	; $A8B8
	ld a,$0A	; $A8BA
	call fn_01_7552	; $A8BC
	call print_newline	; $A8BF
	bit 0,(ix+$07)	; $A8C2
	jr z,loc_02_A8D5	; $A8C6

loc_02_A8C8:
	ld a,(shop_number)	; $A8C8
	add a,$02	; $A8CB
	call print_message	; $A8CD
	ld a,$15	; $A8D0
	jp loc_02_96B2	; $A8D2

loc_02_A8D5:
	bit 0,(ix+$06)	; $A8D5
	jr nz,loc_02_A912	; $A8D9
	set 0,(ix+$06)	; $A8DB
	ld a,(shop_number)	; $A8DF
	add a,$02	; $A8E2
	call print_message	; $A8E4
	ld a,$0C	; $A8E7
	call print_message	; $A8E9
	call wait_button	; $A8EC
	call fn_01_54CB	; $A8EF
	ld a,$0D	; $A8F2
	call print_message	; $A8F4
	ld (ix+$1B),$03	; $A8F7
	ld a,($C545)	; $A8FB
	or a	; $A8FE
	jr z,loc_02_A909	; $A8FF
	inc (ix+$1B)	; $A901
	ld a,$0E	; $A904
	call print_message	; $A906

loc_02_A909:
	call fn_01_548D	; $A909
	call wait_button	; $A90C
	call fn_01_54CB	; $A90F

loc_02_A912:
	ld a,$0F	; $A912
	call print_message	; $A914
	call fn_00_2E1B	; $A917
	ld hl,$A9E2	; $A91A
	ld b,(ix+$1B)	; $A91D

loc_02_A920:
	push bc	; $A920
	call fn_00_2D65	; $A921
	pop bc	; $A924
	djnz loc_02_A920	; $A925
	call fn_00_2E54	; $A927

loc_02_A92A:
	call frame_tick	; $A92A
	ld c,$04	; $A92D
	ld de,$0102	; $A92F
	call fn_01_49F4	; $A932
	jr z,loc_02_A92A	; $A935
	ld a,$0A	; $A937
	call fn_01_7552	; $A939
	jr nc,loc_02_A943	; $A93C
	call print_newline	; $A93E
	jr loc_02_A8C8	; $A941

loc_02_A943:
	ld a,(ix+$1A)	; $A943
	ld hl,$A9DA	; $A946
	rst $20	; $A949
	call print_text_hl	; $A94A
	call fn_01_54CB	; $A94D
	ld a,(ix+$1A)	; $A950
	add a,$04	; $A953
	call print_message	; $A955
	call wait_button	; $A958
	call fn_01_54CB	; $A95B
	ld a,$10	; $A95E
	call print_message	; $A960
	call fn_01_4B5E	; $A963

loc_02_A966:
	call frame_tick	; $A966
	call fn_01_4B69	; $A969
	jr c,loc_02_A966	; $A96C
	ld a,$0A	; $A96E
	call fn_01_7552	; $A970
	call print_newline	; $A973
	bit 0,(ix+$07)	; $A976
	jr z,loc_02_A98A	; $A97A
	ld a,$11	; $A97C
	call print_message	; $A97E
	call wait_button	; $A981
	call fn_01_54CB	; $A984
	jp loc_02_A8C8	; $A987

loc_02_A98A:
	ld a,(ix+$1A)	; $A98A
	ld hl,$A9CE	; $A98D
	rst $20	; $A990
	call fn_01_4423	; $A991
	ld a,$12	; $A994
	jp c,loc_02_96B2	; $A996
	ld a,(ix+$1A)	; $A999
	ld hl,$AA0C	; $A99C
	rst $20	; $A99F
	ex de,hl	; $A9A0
	ld a,(ix+$1A)	; $A9A1
	ld hl,$A9D6	; $A9A4
	rst $28	; $A9A7
	ex de,hl	; $A9A8
	add a,(hl)	; $A9A9
	cp $64	; $A9AA
	jr c,loc_02_A9B0	; $A9AC
	ld a,$63	; $A9AE

loc_02_A9B0:
	ld (hl),a	; $A9B0
	ld a,$0A	; $A9B1
	call fn_01_7552	; $A9B3
	ld a,$13	; $A9B6
	call print_message	; $A9B8
	call wait_button	; $A9BB
	call fn_01_54CB	; $A9BE
	ld a,(shop_number)	; $A9C1
	add a,$02	; $A9C4
	call print_message	; $A9C6
	ld a,$14	; $A9C9
	jp loc_02_A8AC	; $A9CB
	.INCBIN "bank02.bin" SKIP $29CE READ 12	; data, 12 bytes

tbl_02_A9DA:
	.dw fn_02_A9E4
	.dw fn_02_A9EC
	.dw fn_02_A9F8
	.dw fn_02_AA04
	.dw fn_00_0213

fn_02_A9E4:
	ld d,h	; $A9E4
	ld c,a	; $A9E5
	ld d,d	; $A9E6
	ld b,e	; $A9E7
	ld c,b	; $A9E8
	nop	; $A9E9
	inc de	; $A9EA
	inc b	; $A9EB

fn_02_A9EC:
	ld c,l	; $A9EC
	ld b,c	; $A9ED
	ld b,a	; $A9EE
	ld c,c	; $A9EF
	ld b,e	; $A9F0
	jr nz,loc_02_AA3A	; $A9F1
	ld b,l	; $A9F3
	ld c,l	; $A9F4
	nop	; $A9F5
	inc de	; $A9F6
	.INCBIN "bank02.bin" SKIP $29F7 READ 1	; data, 1 bytes

fn_02_A9F8:
	ld c,l	; $A9F8
	ld b,c	; $A9F9
	ld b,a	; $A9FA
	ld c,c	; $A9FB
	ld b,e	; $A9FC
	jr nz,$4B	; $A9FD
	ld b,l	; $A9FF
	ld e,c	; $AA00
	nop	; $AA01
	inc de	; $AA02
	ex af,af'	; $AA03

fn_02_AA04:
	ld d,e	; $AA04
	ld b,l	; $AA05
	ld e,b	; $AA06
	ld d,h	; $AA07
	ld b,c	; $AA08
	ld c,(hl)	; $AA09
	ld d,h	; $AA0A
	nop	; $AA0B
	inc de	; $AA0C
	ret	; $AA0D
	.INCBIN "bank02.bin" SKIP $2A0E READ 6	; data, 6 bytes

fn_02_AA14:
	ld a,$00	; $AA14
	call print_message	; $AA16
	call fn_01_4B5E	; $AA19

loc_02_AA1C:
	call frame_tick	; $AA1C
	call fn_01_4B69	; $AA1F
	jr c,loc_02_AA1C	; $AA22
	call print_newline	; $AA24
	bit 0,(ix+$07)	; $AA27
	ld a,$01	; $AA2B
	jp nz,loc_02_96B2	; $AA2D
	ld a,$02	; $AA30
	call print_message	; $AA32
	ld a,(party_size)	; $AA35
	add a,$30	; $AA38

loc_02_AA3A:
	call print_char	; $AA3A
	ld a,$03	; $AA3D
	call print_message	; $AA3F
	call fn_01_4B5E	; $AA42

loc_02_AA45:
	call frame_tick	; $AA45
	call fn_01_4B69	; $AA48
	jr c,loc_02_AA45	; $AA4B
	call print_newline	; $AA4D
	bit 0,(ix+$07)	; $AA50
	ld a,$01	; $AA54
	jp nz,loc_02_96B2	; $AA56
	ld a,(party_size)	; $AA59
	ld l,a	; $AA5C
	ld h,$64	; $AA5D
	call multiply_8x8	; $AA5F
	call fn_01_4423	; $AA62
	ld a,$04	; $AA65
	jp c,loc_02_96B2	; $AA67
	ld hl,$8614	; $AA6A
	ld ($C959),hl	; $AA6D
	ld a,$05	; $AA70
	jp loc_02_96B2	; $AA72

fn_02_AA75:
	ld c,$00	; $AA75
	call fn_01_4466	; $AA77
	jr nc,loc_02_AA91	; $AA7A
	ld a,$28	; $AA7C
	call print_message	; $AA7E
	call print_avatar_name	; $AA81
	ld a,$29	; $AA84
	call print_message	; $AA86
	call print_avatar_name	; $AA89
	ld a,$2A	; $AA8C
	jp loc_02_96B2	; $AA8E

loc_02_AA91:
	ld a,$2B	; $AA91
	call print_message	; $AA93
	call print_avatar_name	; $AA96
	call print_newline	; $AA99
	ld a,$2C	; $AA9C
	call print_message	; $AA9E
	call wait_button	; $AAA1
	call fn_01_54CB	; $AAA4
	call fn_02_ABE7	; $AAA7
	ld a,$2D	; $AAAA

loc_02_AAAC:
	call print_message	; $AAAC

loc_02_AAAF:
	call frame_tick	; $AAAF
	ld c,$08	; $AAB2
	ld de,$0102	; $AAB4
	call fn_01_49F4	; $AAB7
	jr z,loc_02_AAAF	; $AABA
	jr nc,loc_02_AAE9	; $AABC
	call fn_01_4324	; $AABE
	ld a,$2F	; $AAC1
	call print_message	; $AAC3
	call fn_01_54CB	; $AAC6
	ld hl,($C8FD)	; $AAC9
	ld de,$0064	; $AACC
	call fn_00_2B3F	; $AACF
	ex de,hl	; $AAD2
	ld hl,($C969)	; $AAD3
	or a	; $AAD6
	sbc hl,de	; $AAD7
	ex de,hl	; $AAD9
	ld ($C969),hl	; $AADA
	ld a,$03	; $AADD
	ld c,$06	; $AADF
	call nz,karma_gain	; $AAE1
	ld a,$32	; $AAE4
	jp loc_02_96B2	; $AAE6

loc_02_AAE9:
	ld a,(ix+$1A)	; $AAE9
	add a,$97	; $AAEC
	call fn_01_5516	; $AAEE
	call fn_01_54CB	; $AAF1
	ld a,(ix+$1A)	; $AAF4
	ld hl,karma	; $AAF7
	rst $28	; $AAFA
	or a	; $AAFB
	jr nz,loc_02_AB0B	; $AAFC
	ld a,$30	; $AAFE
	call print_message	; $AB00
	call wait_button	; $AB03
	call fn_01_54CB	; $AB06
	jr loc_02_AB46	; $AB09

loc_02_AB0B:
	ld b,$00	; $AB0B
	cp $14	; $AB0D
	jr c,loc_02_AB21	; $AB0F
	inc b	; $AB11
	cp $28	; $AB12
	jr c,loc_02_AB21	; $AB14
	inc b	; $AB16
	cp $3C	; $AB17
	jr c,loc_02_AB21	; $AB19
	inc b	; $AB1B
	cp $63	; $AB1C
	jr c,loc_02_AB21	; $AB1E
	inc b	; $AB20

loc_02_AB21:
	ld (ix+$1B),b	; $AB21
	ld a,b	; $AB24
	add a,a	; $AB25
	add a,a	; $AB26
	add a,a	; $AB27
	add a,(ix+$1A)	; $AB28
	call print_message	; $AB2B
	call wait_button	; $AB2E
	call fn_01_54CB	; $AB31
	ld a,(ix+$1B)	; $AB34
	cp $04	; $AB37
	jr nz,loc_02_AB46	; $AB39
	ld a,$31	; $AB3B
	call print_message	; $AB3D
	call wait_button	; $AB40
	call fn_01_54CB	; $AB43

loc_02_AB46:
	ld a,$2E	; $AB46
	jp loc_02_AAAC	; $AB48

fn_02_AB4B:
	ld hl,$AB77	; $AB4B

fn_02_AB4E:
	ld (ix+$1A),$00	; $AB4E
	jp fn_00_2E7F	; $AB52

fn_02_AB55:
	ld c,$02	; $AB55
	ld de,$0102	; $AB57
	call fn_01_49F9	; $AB5A
	ret z	; $AB5D
	ret c	; $AB5E
	bit 0,(ix+$1A)	; $AB5F
	ld hl,$AB79	; $AB63
	jr z,loc_02_AB6B	; $AB66
	ld hl,$AB7F	; $AB68

loc_02_AB6B:
	call print_text_hl	; $AB6B
	call print_newline	; $AB6E
	xor a	; $AB71
	dec a	; $AB72
	ld a,(ix+$1A)	; $AB73
	ret	; $AB76
	.INCBIN "bank02.bin" SKIP $2B77 READ 14	; data, 14 bytes

fn_02_AB85:
	add a,$25	; $AB85
	jr loc_02_AB8F	; $AB87

fn_02_AB89:
	add a,$35	; $AB89
	jr loc_02_AB8F	; $AB8B

fn_02_AB8D:
	add a,$5D	; $AB8D

loc_02_AB8F:
	jp fn_01_5516	; $AB8F

fn_02_AB92:
	ld a,(ix+$1B)	; $AB92
	rst $20	; $AB95
	ex de,hl	; $AB96
	ld b,(ix+$21)	; $AB97
	ld hl,$0000	; $AB9A

loc_02_AB9D:
	add hl,de	; $AB9D
	jr c,loc_02_ABA4	; $AB9E
	djnz loc_02_AB9D	; $ABA0
	jr loc_02_ABA7	; $ABA2

loc_02_ABA4:
	ld hl,$FFFF	; $ABA4

loc_02_ABA7:
	srl h	; $ABA7
	rr l	; $ABA9
	call cap_9999	; $ABAB
	ld (ix+$1F),l	; $ABAE
	ld (ix+$20),h	; $ABB1
	jp fn_01_5627	; $ABB4

fn_02_ABB7:
	ld hl,$C91F	; $ABB7
	ld b,$0F	; $ABBA
	jr loc_02_ABC3	; $ABBC

fn_02_ABBE:
	ld hl,$C917	; $ABBE
	ld b,$07	; $ABC1

loc_02_ABC3:
	xor a	; $ABC3
	ld c,a	; $ABC4

loc_02_ABC5:
	inc hl	; $ABC5
	cp (hl)	; $ABC6
	jr z,loc_02_ABCA	; $ABC7
	inc c	; $ABC9

loc_02_ABCA:
	djnz loc_02_ABC5	; $ABCA
	ld (ix+$1B),c	; $ABCC
	ld a,c	; $ABCF
	or a	; $ABD0
	ret	; $ABD1

fn_02_ABD2:
	ld de,(food)	; $ABD2
	add hl,de	; $ABD6
	call cap_9999	; $ABD7
	ld (food),hl	; $ABDA
	jr nc,loc_02_ABE4	; $ABDD
	ld a,$63	; $ABDF
	ld (food_fraction),a	; $ABE1

loc_02_ABE4:
	jp loc_01_43F6	; $ABE4

fn_02_ABE7:
	ld (ix+$1A),$00	; $ABE7
	call fn_00_2E1B	; $ABEB
	ld de,$0213	; $ABEE
	ld bc,$0897	; $ABF1

loc_02_ABF4:
	push bc	; $ABF4
	push de	; $ABF5
	ld a,c	; $ABF6
	call fn_00_340F	; $ABF7
	call fn_00_2D69	; $ABFA
	pop de	; $ABFD
	pop bc	; $ABFE
	inc d	; $ABFF
	inc d	; $AC00
	inc c	; $AC01
	djnz loc_02_ABF4	; $AC02
	jp fn_00_2E54	; $AC04

fn_02_AC07:
	call fn_00_0DC5	; $AC07
	call fn_00_0277	; $AC0A
	call fn_00_0281	; $AC0D
	call fn_00_2FA7	; $AC10
	ld a,$13	; $AC13
	ld hl,$A25A	; $AC15
	ld de,$26E0	; $AC18
	call fn_00_0749	; $AC1B
	ld a,$13	; $AC1E
	ld hl,$B14F	; $AC20
	ld de,$37E0	; $AC23
	ld bc,$0020	; $AC26
	call fn_00_07F4	; $AC29
	ld a,$14	; $AC2C
	ld hl,$AE88	; $AC2E
	ld de,$2440	; $AC31
	ld bc,$0160	; $AC34
	call fn_00_07F4	; $AC37
	ld a,$14	; $AC3A
	ld hl,$B148	; $AC3C
	ld de,$CA00	; $AC3F
	ld bc,$0600	; $AC42
	call fn_00_07EC	; $AC45
	call fn_00_0458	; $AC48
	ld a,$FF	; $AC4B
	ld (map_mode),a	; $AC4D
	ld ($C98F),a	; $AC50
	ld hl,$1701	; $AC53
	ld (text_col),hl	; $AC56
	ld de,$AC62	; $AC59
	call spawn_task	; $AC5C
	jp loc_00_0DE1	; $AC5F
	ld hl,$7C83	; $AC62
	ld bc,$0301	; $AC65
	call fn_00_0A22	; $AC68
	call wait_flag	; $AC6B
	ld a,$11	; $AC6E
	ld (game_mode),a	; $AC70
	ret	; $AC73

fn_02_AC74:
	call fn_00_0DD7	; $AC74
	ld de,$AC8C	; $AC77
	call spawn_task	; $AC7A
	jp loc_00_0DE1	; $AC7D

fn_02_AC80:
	call fn_00_0DD7	; $AC80
	ld de,$AF69	; $AC83
	call spawn_task	; $AC86
	jp loc_00_0DE1	; $AC89
	xor a	; $AC8C
	ld (location),a	; $AC8D
	call fn_00_323D	; $AC90
	call recenter_map_view	; $AC93
	call fn_00_0E17	; $AC96
	call fn_00_129D	; $AC99
	call fn_01_4324	; $AC9C
	call fn_00_30F6	; $AC9F
	ld a,$05	; $ACA2
	call play_music	; $ACA4
	xor a	; $ACA7
	ld ($C9A1),a	; $ACA8
	ld hl,$0000	; $ACAB
	ld ($C99C),hl	; $ACAE

loc_02_ACB1:
	ld hl,(party_x)	; $ACB1
	ld (world_x_saved),hl	; $ACB4
	call fn_00_129D	; $ACB7
	call yield	; $ACBA
	call fn_02_ACDC	; $ACBD
	call fn_02_AF57	; $ACC0

fn_02_ACC3:
	jr c,loc_02_ACD4	; $ACC3
	ld a,$7F	; $ACC5
	call print_char	; $ACC7
	jr loc_02_ACCF	; $ACCA

loc_02_ACCC:
	call frame_tick	; $ACCC

loc_02_ACCF:
	call fn_02_B08B	; $ACCF
	jr c,loc_02_ACCC	; $ACD2

loc_02_ACD4:
	call fn_00_3F14	; $ACD4
	call fn_02_B4CA	; $ACD7
	jr loc_02_ACB1	; $ACDA

fn_02_ACDC:
	ld a,($C9A1)	; $ACDC
	or a	; $ACDF
	ret z	; $ACE0
	call fn_02_957C	; $ACE1
	ret z	; $ACE4
	call fn_02_AEDA	; $ACE5
	ld a,(party_x)	; $ACE8
	ld hl,$C98D	; $ACEB
	cp (hl)	; $ACEE
	jr nz,loc_02_AD41	; $ACEF
	ld a,(party_y)	; $ACF1
	ld hl,$C98E	; $ACF4
	cp (hl)	; $ACF7
	jr nz,loc_02_AD41	; $ACF8
	ld a,($C98C)	; $ACFA
	cp $43	; $ACFD
	jr nz,loc_02_AD41	; $ACFF
	ld a,$15	; $AD01
	call fn_00_05F1	; $AD03
	call fn_01_41A9	; $AD06
	call fn_01_41A9	; $AD09
	ld a,($C95D)	; $AD0C
	cp $04	; $AD0F
	jr nz,loc_02_AD28	; $AD11
	ld a,($C95F)	; $AD13
	cp $04	; $AD16
	jr nz,loc_02_AD28	; $AD18
	ld a,$20	; $AD1A
	ld (location),a	; $AD1C
	ld hl,(party_x)	; $AD1F
	ld (world_return_x),hl	; $AD22
	jp loc_02_ADFF	; $AD25

loc_02_AD28:
	ld a,($C95F)	; $AD28
	ld hl,$7CA3	; $AD2B
	rst $20	; $AD2E
	ld (party_x),hl	; $AD2F
	call recenter_map_view	; $AD32
	call fn_00_276D	; $AD35
	call fn_00_129D	; $AD38
	call fn_01_41A9	; $AD3B
	jp fn_01_41A9	; $AD3E

loc_02_AD41:
	ld hl,$AE60	; $AD41
	ld de,(party_x)	; $AD44
	ld b,$20	; $AD48

loc_02_AD4A:
	ld a,e	; $AD4A
	cp (hl)	; $AD4B
	inc hl	; $AD4C
	jr nz,loc_02_AD53	; $AD4D
	ld a,d	; $AD4F
	cp (hl)	; $AD50
	jr z,loc_02_AD57	; $AD51

loc_02_AD53:
	inc hl	; $AD53
	djnz loc_02_AD4A	; $AD54
	ret	; $AD56

loc_02_AD57:
	ld a,$1F	; $AD57
	sub b	; $AD59
	sbc a,$FD	; $AD5A
	ld (location),a	; $AD5C
	ld (world_return_x),de	; $AD5F
	ld a,($C9BB)	; $AD63
	ld hl,$AD6D	; $AD66
	call switch_on_a	; $AD69
	ret	; $AD6C

tbl_02_AD6D:
	.db $09
	.dw loc_02_AD89
	.db $0A
	.dw loc_02_ADB2
	.db $0E
	.dw loc_02_ADDA
	.db $0B
	.dw loc_02_ADDA
	.db $0C
	.dw loc_02_ADDF
	.db $1D
	.dw loc_02_ADE4
	.db $46
	.dw loc_02_ADE9
	.db $4C
	.dw loc_02_ADE9
	.db $1E
	.dw loc_02_ADFF
	.db $FF

loc_02_AD89:
	ld hl,$BCD1	; $AD89
	call fn_02_AE20	; $AD8C
	ld a,(location)	; $AD8F
	add a,$7D	; $AD92
	call fn_01_5516	; $AD94
	call print_newline	; $AD97
	ld a,($C959)	; $AD9A
	cp $1F	; $AD9D
	jr z,loc_02_ADA7	; $AD9F
	call fn_01_4703	; $ADA1
	jp loc_02_AE5B	; $ADA4

loc_02_ADA7:
	ld hl,$0202	; $ADA7
	ld (party_x),hl	; $ADAA
	ld a,$13	; $ADAD
	jp loc_02_AE39	; $ADAF

loc_02_ADB2:
	ld hl,$BCD9	; $ADB2

loc_02_ADB5:
	call fn_02_AE20	; $ADB5
	call place_index	; $ADB8
	add a,$7F	; $ADBB
	call fn_01_5516	; $ADBD
	call print_newline	; $ADC0
	ld a,(location)	; $ADC3
	cp $06	; $ADC6
	ld hl,$1E0F	; $ADC8
	jr c,loc_02_ADD0	; $ADCB
	ld hl,$0F01	; $ADCD

loc_02_ADD0:
	ld (party_x),hl	; $ADD0
	ld (view_x),hl	; $ADD3
	ld a,$12	; $ADD6
	jr loc_02_AE39	; $ADD8

loc_02_ADDA:
	ld hl,$BCDF	; $ADDA
	jr loc_02_ADB5	; $ADDD

loc_02_ADDF:
	ld hl,$BCE6	; $ADDF
	jr loc_02_ADB5	; $ADE2

loc_02_ADE4:
	ld hl,$BCEE	; $ADE4
	jr loc_02_ADB5	; $ADE7

loc_02_ADE9:
	ld a,($C951)	; $ADE9
	and $1C	; $ADEC
	cp $1C	; $ADEE
	jr nz,loc_02_AE5B	; $ADF0
	ld a,($C952)	; $ADF2
	and $1C	; $ADF5
	cp $1C	; $ADF7
	jp nz,loc_02_AE5B	; $ADF9
	jp loc_02_AD89	; $ADFC

loc_02_ADFF:
	ld hl,$BCC6	; $ADFF
	call print_text_hl	; $AE02
	ld hl,$BCF3	; $AE05
	call print_text_hl	; $AE08
	ld hl,$BCCD	; $AE0B
	call print_text_hl	; $AE0E
	ld a,(location)	; $AE11
	add a,$7D	; $AE14
	call fn_01_5516	; $AE16
	call print_newline	; $AE19
	ld a,$17	; $AE1C
	jr loc_02_AE39	; $AE1E

fn_02_AE20:
	push hl	; $AE20
	ld hl,$BCC6	; $AE21
	call print_text_hl	; $AE24
	pop hl	; $AE27
	call print_text_hl	; $AE28
	ld a,$21	; $AE2B
	call print_char	; $AE2D
	call print_newline	; $AE30
	ld hl,$BCCD	; $AE33
	jp print_text_hl	; $AE36

loc_02_AE39:
	pop hl	; $AE39
	ld (ix+$08),a	; $AE3A
	call fn_00_0605	; $AE3D
	call fn_00_310B	; $AE40
	call fn_01_40E9	; $AE43
	xor a	; $AE46
	ld ($C9A4),a	; $AE47
	ld a,(ix+$08)	; $AE4A
	ld (game_mode),a	; $AE4D
	cp $12	; $AE50
	jp z,load_location	; $AE52
	cp $13	; $AE55
	jp z,fn_00_3D78	; $AE57
	ret	; $AE5A

loc_02_AE5B:
	xor a	; $AE5B
	ld (location),a	; $AE5C
	ret	; $AE5F
	.INCBIN "bank02.bin" SKIP $2E60 READ 64	; data, 64 bytes

loc_02_AEA0:
	ld a,(entry_direction)	; $AEA0
	cp $03	; $AEA3
	ret nz	; $AEA5
	ld a,(party_x)	; $AEA6
	cp $E5	; $AEA9
	ret c	; $AEAB
	cp $EA	; $AEAC
	ret nc	; $AEAE
	ld a,(party_y)	; $AEAF
	cp $D4	; $AEB2
	ret c	; $AEB4
	cp $D9	; $AEB5
	ret nc	; $AEB7
	ld a,($C976)	; $AEB8
	cp $01	; $AEBB
	ret z	; $AEBD
	call random	; $AEBE
	and $07	; $AEC1
	call fn_01_48F7	; $AEC3
	ld (iy+$03),$F0	; $AEC6
	ld (iy+$00),$F0	; $AECA
	ld (iy+$01),$E7	; $AECE
	ld a,(party_y)	; $AED2
	inc a	; $AED5
	ld (iy+$02),a	; $AED6
	ret	; $AED9

fn_02_AEDA:
	ld a,(entry_direction)	; $AEDA
	cp $02	; $AEDD
	ret nz	; $AEDF
	ld a,(party_x)	; $AEE0
	cp $DC	; $AEE3
	ret nz	; $AEE5
	ld a,(party_y)	; $AEE6
	cp $E0	; $AEE9
	ret nz	; $AEEB
	ld iy,location_people	; $AEEC
	ld de,$0008	; $AEF0
	ld hl,$AF3F	; $AEF3
	ld b,$08	; $AEF6

loc_02_AEF8:
	ld (iy+$03),$80	; $AEF8
	ld a,(hl)	; $AEFC
	inc hl	; $AEFD
	ld (iy+$01),a	; $AEFE
	ld a,(hl)	; $AF01
	inc hl	; $AF02
	ld (iy+$02),a	; $AF03
	ld a,(hl)	; $AF06
	inc hl	; $AF07
	ld (iy+$00),a	; $AF08
	add iy,de	; $AF0B
	djnz loc_02_AEF8	; $AF0D
	ld b,$18	; $AF0F

loc_02_AF11:
	ld a,(iy+$01)	; $AF11
	cp $DE	; $AF14
	jr c,loc_02_AF3A	; $AF16
	cp $E7	; $AF18
	jr nc,loc_02_AF3A	; $AF1A
	ld a,(iy+$02)	; $AF1C
	cp $DC	; $AF1F
	jr c,loc_02_AF3A	; $AF21
	cp $E5	; $AF23
	jr nc,loc_02_AF3A	; $AF25
	ld a,(iy+$03)	; $AF27
	cp $10	; $AF2A
	jr c,loc_02_AF3A	; $AF2C
	cp $14	; $AF2E
	jr nc,loc_02_AF3A	; $AF30
	ld (iy+$03),$00	; $AF32
	ld (iy+$00),$00	; $AF36

loc_02_AF3A:
	add iy,de	; $AF3A
	djnz loc_02_AF11	; $AF3C
	ret	; $AF3E
	.INCBIN "bank02.bin" SKIP $2F3F READ 24	; data, 24 bytes

fn_02_AF57:
	call fn_01_465D	; $AF57
	ret nc	; $AF5A
	ld hl,$BCBF	; $AF5B
	call print_text_hl	; $AF5E
	ld hl,$0100	; $AF61
	ld ($C99A),hl	; $AF64
	scf	; $AF67
	ret	; $AF68
	call fn_00_323D	; $AF69
	call fn_00_129D	; $AF6C
	call fn_01_4324	; $AF6F
	call fn_00_30F6	; $AF72
	call fn_00_33E5	; $AF75
	call play_music	; $AF78

loc_02_AF7B:
	xor a	; $AF7B
	ld ($C9A1),a	; $AF7C
	ld hl,$0000	; $AF7F
	ld ($C99C),hl	; $AF82

loc_02_AF85:
	call fn_00_129D	; $AF85
	call yield	; $AF88
	call fn_02_AFAA	; $AF8B
	call fn_02_AF57	; $AF8E
	jr c,loc_02_AFA2	; $AF91
	ld a,$7F	; $AF93
	call print_char	; $AF95
	jr loc_02_AF9D	; $AF98

loc_02_AF9A:
	call frame_tick	; $AF9A

loc_02_AF9D:
	call fn_02_B08B	; $AF9D
	jr c,loc_02_AF9A	; $AFA0

loc_02_AFA2:
	call fn_00_3F14	; $AFA2
	call fn_02_B4CA	; $AFA5
	jr loc_02_AF85	; $AFA8

fn_02_AFAA:
	ld a,($C9A1)	; $AFAA
	or a	; $AFAD
	ret z	; $AFAE
	ld a,(party_x)	; $AFAF
	cp $20	; $AFB2
	jr nc,loc_02_AFC9	; $AFB4
	ld a,(party_y)	; $AFB6
	cp $20	; $AFB9
	jr nc,loc_02_AFC9	; $AFBB
	ld a,($C990)	; $AFBD
	cp $1B	; $AFC0
	jr z,loc_02_AFE9	; $AFC2
	cp $1C	; $AFC4
	jr z,loc_02_B024	; $AFC6
	ret	; $AFC8

loc_02_AFC9:
	pop af	; $AFC9
	ld hl,$BD02	; $AFCA
	call print_text_hl	; $AFCD
	call fn_00_0605	; $AFD0
	call fn_00_310B	; $AFD3
	ld a,$11	; $AFD6
	ld (game_mode),a	; $AFD8
	ld hl,(world_return_x)	; $AFDB
	ld (party_x),hl	; $AFDE
	ld a,$FF	; $AFE1
	ld ($C98F),a	; $AFE3
	jp fn_01_4100	; $AFE6

loc_02_AFE9:
	ld a,($C959)	; $AFE9
	cp $1F	; $AFEC
	ret nz	; $AFEE
	pop af	; $AFEF
	call print_inline	; $AFF0
	.INCBIN "bank02.bin" SKIP $2FF3 READ 24	; text, 24 bytes
	ld a,$02	; $B00B

loc_02_B00D:
	ld (location),a	; $B00D
	call fn_00_31B1	; $B010
	call load_location	; $B013
	ld a,$FF	; $B016
	ld (map_mode),a	; $B018
	call fn_00_129D	; $B01B
	call fn_00_3164	; $B01E
	jp loc_02_AF7B	; $B021

loc_02_B024:
	ld a,($C959)	; $B024
	cp $1F	; $B027
	ret nz	; $B029
	call print_inline	; $B02A
	.INCBIN "bank02.bin" SKIP $302D READ 9	; text, 9 bytes
	ld a,(party_y)	; $B036
	cp $02	; $B039
	jr z,loc_02_B056	; $B03B
	pop af	; $B03D
	call print_inline	; $B03E
	.INCBIN "bank02.bin" SKIP $3041 READ 17	; text, 17 bytes
	ld a,$01	; $B052
	jr loc_02_B00D	; $B054

loc_02_B056:
	pop af	; $B056
	call print_inline	; $B057
	.INCBIN "bank02.bin" SKIP $305A READ 18	; text, 18 bytes
	call fn_00_31B1	; $B06C
	ld hl,$F0EF	; $B06F
	ld (world_return_x),hl	; $B072
	ld hl,$0A0A	; $B075
	ld (party_x),hl	; $B078
	ld a,$18	; $B07B
	ld (location),a	; $B07D
	ld a,$13	; $B080
	ld (game_mode),a	; $B082
	call fn_00_3D78	; $B085
	jp main_loop	; $B088

fn_02_B08B:
	xor a	; $B08B
	ld ($C9A1),a	; $B08C
	ld a,(joy_held)	; $B08F
	ld hl,$B246	; $B092
	call switch_on_a	; $B095
	ld a,(joy_pressed)	; $B098
	cp $10	; $B09B
	jr z,loc_02_B0A6	; $B09D
	cp $20	; $B09F
	jp z,loc_02_B161	; $B0A1
	scf	; $B0A4
	ret	; $B0A5

loc_02_B0A6:
	ld hl,$0100	; $B0A6
	ld ($C99A),hl	; $B0A9
	ld a,($C959)	; $B0AC
	cp $1F	; $B0AF
	jr nz,loc_02_B0B8	; $B0B1
	call fn_01_46B5	; $B0B3
	or a	; $B0B6
	ret	; $B0B7

loc_02_B0B8:
	ld hl,$B224	; $B0B8
	call print_text_hl	; $B0BB
	ld a,($C959)	; $B0BE
	cp $14	; $B0C1
	jr c,loc_02_B0DF	; $B0C3
	cp $16	; $B0C5
	jr c,loc_02_B0F0	; $B0C7
	ld hl,$B23C	; $B0C9
	call print_text_hl	; $B0CC
	call fn_02_B152	; $B0CF
	ld a,($C990)	; $B0D2
	cp $04	; $B0D5
	jp nz,loc_02_B15C	; $B0D7
	call fn_00_0DEB	; $B0DA
	jr loc_02_B142	; $B0DD

loc_02_B0DF:
	ld hl,$B232	; $B0DF
	call print_text_hl	; $B0E2
	ld hl,(party_x)	; $B0E5
	ld ($C973),hl	; $B0E8
	call fn_00_0E01	; $B0EB
	jr loc_02_B142	; $B0EE

loc_02_B0F0:
	ld hl,$B22A	; $B0F0
	call print_text_hl	; $B0F3
	call fn_02_B152	; $B0F6
	ld a,(map_mode)	; $B0F9
	cp $01	; $B0FC
	jr z,loc_02_B105	; $B0FE
	call fn_00_0DF6	; $B100
	jr loc_02_B142	; $B103

loc_02_B105:
	xor a	; $B105
	call fn_01_4906	; $B106
	jr nc,loc_02_B128	; $B109
	call random	; $B10B
	ld b,$20	; $B10E

loc_02_B110:
	inc a	; $B110
	and $1F	; $B111
	ld c,a	; $B113
	call fn_01_48F7	; $B114
	ld a,(iy+$03)	; $B117
	cp $14	; $B11A
	jr z,loc_02_B128	; $B11C
	cp $15	; $B11E
	jr z,loc_02_B128	; $B120
	ld a,c	; $B122
	djnz loc_02_B110	; $B123
	call fn_01_4933	; $B125

loc_02_B128:
	ld a,($C959)	; $B128
	ld (iy+$03),a	; $B12B
	ld (iy+$00),a	; $B12E
	ld hl,(party_x)	; $B131
	ld (iy+$01),l	; $B134
	ld (iy+$02),h	; $B137
	ld (iy+$06),$00	; $B13A
	ld (iy+$07),$00	; $B13E

loc_02_B142:
	ld hl,$8A1F	; $B142
	ld ($C959),hl	; $B145
	ld a,$FF	; $B148
	ld ($C98F),a	; $B14A
	call fn_01_40D1	; $B14D
	or a	; $B150
	ret	; $B151

fn_02_B152:
	ld hl,(party_x)	; $B152
	ld b,$20	; $B155
	call fn_01_42B7	; $B157
	ret c	; $B15A
	pop af	; $B15B

loc_02_B15C:
	call fn_01_46F1	; $B15C
	or a	; $B15F
	ret	; $B160

loc_02_B161:
	ld hl,$0100	; $B161
	ld ($C99A),hl	; $B164
	ld a,($C959)	; $B167
	cp $1F	; $B16A
	jp nz,loc_02_8000	; $B16C
	ld hl,(party_x)	; $B16F
	ld b,$20	; $B172
	call fn_01_42B7	; $B174
	jp c,loc_02_8000	; $B177
	call fn_01_48F7	; $B17A
	ld a,(iy+$00)	; $B17D
	ld hl,$B189	; $B180
	call switch_on_a	; $B183
	jp loc_02_8000	; $B186

tbl_02_B189:
	.db $10
	.dw loc_02_B19F
	.db $11
	.dw loc_02_B1A4
	.db $12
	.dw loc_02_B1A9
	.db $13
	.dw loc_02_B1AE
	.db $14
	.dw loc_02_B1E5
	.db $15
	.dw loc_02_B1EA
	.db $18
	.dw loc_02_B1FE
	.db $FF

loc_02_B19F:
	ld hl,$0010	; $B19F
	jr loc_02_B1B1	; $B1A2

loc_02_B1A4:
	ld hl,$0011	; $B1A4
	jr loc_02_B1B1	; $B1A7

loc_02_B1A9:
	ld hl,$0012	; $B1A9
	jr loc_02_B1B1	; $B1AC

loc_02_B1AE:
	ld hl,$0013	; $B1AE

loc_02_B1B1:
	ld ($C959),hl	; $B1B1
	ld hl,$B21D	; $B1B4
	call print_text_hl	; $B1B7
	ld hl,$B232	; $B1BA
	call print_text_hl	; $B1BD
	ld a,(party_x)	; $B1C0
	ld hl,$C973	; $B1C3
	cp (hl)	; $B1C6
	jr nz,loc_02_B1D2	; $B1C7
	ld a,(party_y)	; $B1C9
	ld hl,$C974	; $B1CC
	cp (hl)	; $B1CF
	jr z,loc_02_B1D8	; $B1D0

loc_02_B1D2:
	ld hl,$0032	; $B1D2
	ld ($C961),hl	; $B1D5

loc_02_B1D8:
	ld (iy+$03),$00	; $B1D8
	ld (iy+$00),$00	; $B1DC
	call fn_01_40C9	; $B1E0
	or a	; $B1E3
	ret	; $B1E4

loc_02_B1E5:
	ld hl,$8614	; $B1E5
	jr loc_02_B1ED	; $B1E8

loc_02_B1EA:
	ld hl,$8715	; $B1EA

loc_02_B1ED:
	ld ($C959),hl	; $B1ED
	ld hl,$B216	; $B1F0
	call print_text_hl	; $B1F3
	ld hl,$B22A	; $B1F6
	call print_text_hl	; $B1F9
	jr loc_02_B1D8	; $B1FC

loc_02_B1FE:
	ld hl,$0018	; $B1FE
	ld ($C959),hl	; $B201
	ld hl,$B21D	; $B204
	call print_text_hl	; $B207
	ld hl,$B23C	; $B20A
	call print_text_hl	; $B20D
	xor a	; $B210
	ld ($C98F),a	; $B211
	jr loc_02_B1D8	; $B214
	.INCBIN "bank02.bin" SKIP $3216 READ 48	; data, 48 bytes

tbl_02_B246:
	.db $01
	.dw loc_02_B253
	.db $02
	.dw $B256
	.db $04
	.dw $B259
	.db $08
	.dw $B25C
	.db $FF

loc_02_B253:
	ld a,$01	; $B253
	ld bc,$033E	; $B255
	ld bc,$003E	; $B258
	ld bc,$023E	; $B25B
	call fn_02_B263	; $B25E
	or a	; $B261
	ret	; $B262

fn_02_B263:
	ld (entry_direction),a	; $B263
	ld hl,$0100	; $B266
	ld ($C99A),hl	; $B269
	ld a,($C959)	; $B26C
	cp $18	; $B26F
	jp z,fn_01_473F	; $B271
	cp $14	; $B274
	jp c,loc_02_B326	; $B276
	ld a,(entry_direction)	; $B279
	call fn_01_40B9	; $B27C
	call fn_02_B464	; $B27F
	ret c	; $B282
	call fn_02_B403	; $B283
	jr nz,loc_02_B291	; $B286
	call print_inline	; $B288
	.INCBIN "bank02.bin" SKIP $328B READ 6	; text, 6 bytes

loc_02_B291:
	call fn_02_B3C6	; $B291
	ld a,(entry_direction)	; $B294
	cp $01	; $B297
	jr nz,loc_02_B2AA	; $B299
	ld a,($C9BB)	; $B29B
	cp $0E	; $B29E
	jp z,loc_01_40A6	; $B2A0
	ld a,($C992)	; $B2A3
	cp $0E	; $B2A6
	jr z,loc_02_B2BC	; $B2A8

loc_02_B2AA:
	call fn_02_B412	; $B2AA
	call fn_00_3EB7	; $B2AD
	jr nc,loc_02_B2BC	; $B2B0
	ld a,(map_mode)	; $B2B2
	or a	; $B2B5
	call z,fn_02_B40C	; $B2B6
	jp loc_01_40A6	; $B2B9

loc_02_B2BC:
	push af	; $B2BC
	call fn_02_B403	; $B2BD
	jr nz,loc_02_B2C8	; $B2C0
	ld hl,$0080	; $B2C2
	ld ($C99A),hl	; $B2C5

loc_02_B2C8:
	pop af	; $B2C8
	call fn_02_B309	; $B2C9
	call fn_02_B403	; $B2CC
	ld a,$0A	; $B2CF
	jr nz,loc_02_B2E0	; $B2D1
	ld a,(entry_direction)	; $B2D3
	cp $00	; $B2D6
	jr z,loc_02_B2FC	; $B2D8
	cp $02	; $B2DA
	jr z,loc_02_B301	; $B2DC

loc_02_B2DE:
	ld a,$0B	; $B2DE

loc_02_B2E0:
	call fn_00_05F1	; $B2E0
	call fn_02_B40C	; $B2E3
	ld a,(map_mode)	; $B2E6
	or a	; $B2E9
	ld a,(entry_direction)	; $B2EA
	jr nz,loc_02_B2F6	; $B2ED
	ld hl,$B3CF	; $B2EF
	rst $18	; $B2F2
	jp loc_02_AEA0	; $B2F3

loc_02_B2F6:
	ld hl,$B3D7	; $B2F6
	jp fn_00_0018	; $B2F9

loc_02_B2FC:
	ld hl,$8614	; $B2FC
	jr loc_02_B304	; $B2FF

loc_02_B301:
	ld hl,$8715	; $B301

loc_02_B304:
	ld ($C959),hl	; $B304
	jr loc_02_B2DE	; $B307

fn_02_B309:
	ld hl,$B310	; $B309
	call switch_on_a	; $B30C
	ret	; $B30F

tbl_02_B310:
	.db $03
	.dw loc_02_B382
	.db $05
	.dw loc_02_B382
	.db $06
	.dw loc_02_B39D
	.db $07
	.dw loc_02_B39D
	.db $09
	.dw loc_02_B39D
	.db $46
	.dw loc_02_B382
	.db $4C
	.dw loc_02_B39D
	.db $FF

loc_02_B326:
	ld a,(entry_direction)	; $B326
	or $10	; $B329
	ld hl,$C959	; $B32B
	cp (hl)	; $B32E
	jr z,loc_02_B343	; $B32F
	ld l,a	; $B331
	ld h,$00	; $B332
	ld ($C959),hl	; $B334
	call print_inline	; $B337
	.INCBIN "bank02.bin" SKIP $333A READ 6	; text, 6 bytes
	jp fn_02_B3C6	; $B340

loc_02_B343:
	call print_inline	; $B343
	.INCBIN "bank02.bin" SKIP $3346 READ 6	; text, 6 bytes
	call fn_02_B3C6	; $B34C
	call fn_02_B412	; $B34F
	call fn_02_B423	; $B352
	jp c,loc_01_40A6	; $B355
	call fn_02_B36C	; $B358
	ld a,$1F	; $B35B
	call fn_00_05F1	; $B35D
	call fn_02_B40C	; $B360
	ld a,(entry_direction)	; $B363
	ld hl,$B3CF	; $B366
	jp fn_00_0018	; $B369

fn_02_B36C:
	ld a,(entry_direction)	; $B36C
	ld b,a	; $B36F
	ld a,($C98A)	; $B370
	cp $04	; $B373
	jr z,loc_02_B3B5	; $B375
	cp b	; $B377
	jr z,loc_02_B39D	; $B378
	add a,$02	; $B37A
	and $03	; $B37C
	cp b	; $B37E
	jr z,loc_02_B382	; $B37F
	ret	; $B381

loc_02_B382:
	call print_inline	; $B382
	.INCBIN "bank02.bin" SKIP $3385 READ 16	; text, 16 bytes
	ld hl,($C99A)	; $B395
	add hl,hl	; $B398

loc_02_B399:
	ld ($C99A),hl	; $B399
	ret	; $B39C

loc_02_B39D:
	call print_inline	; $B39D
	.INCBIN "bank02.bin" SKIP $33A0 READ 12	; text, 12 bytes
	ld hl,($C99A)	; $B3AC
	ld e,l	; $B3AF
	ld d,h	; $B3B0
	add hl,hl	; $B3B1
	add hl,de	; $B3B2
	jr loc_02_B399	; $B3B3

loc_02_B3B5:
	call print_inline	; $B3B5
	.INCBIN "bank02.bin" SKIP $33B8 READ 7	; text, 7 bytes
	ld hl,($C99A)	; $B3BF
	add hl,hl	; $B3C2
	add hl,hl	; $B3C3
	jr loc_02_B399	; $B3C4

fn_02_B3C6:
	ld a,(entry_direction)	; $B3C6
	call fn_01_55A0	; $B3C9
	jp print_newline	; $B3CC

tbl_02_B3CF:
	.dw step_west
	.dw step_north
	.dw step_east
	.dw step_south
	.dw fn_02_B3DF
	.dw fn_02_B3E8
	.dw fn_02_B3F1
	.dw fn_02_B3FA

fn_02_B3DF:
	ld hl,view_x	; $B3DF
	dec (hl)	; $B3E2
	ld hl,party_x	; $B3E3
	dec (hl)	; $B3E6
	ret	; $B3E7

fn_02_B3E8:
	ld hl,view_y	; $B3E8
	dec (hl)	; $B3EB
	ld hl,party_y	; $B3EC
	dec (hl)	; $B3EF
	ret	; $B3F0

fn_02_B3F1:
	ld hl,view_x	; $B3F1
	inc (hl)	; $B3F4
	ld hl,party_x	; $B3F5
	inc (hl)	; $B3F8
	ret	; $B3F9

fn_02_B3FA:
	ld hl,view_y	; $B3FA
	inc (hl)	; $B3FD
	ld hl,party_y	; $B3FE
	inc (hl)	; $B401
	ret	; $B402

fn_02_B403:
	ld a,($C959)	; $B403
	cp $14	; $B406
	ret z	; $B408
	cp $15	; $B409
	ret	; $B40B

fn_02_B40C:
	ld a,$FF	; $B40C
	ld ($C9A1),a	; $B40E
	ret	; $B411

fn_02_B412:
	ld a,(entry_direction)	; $B412
	ld hl,$B41B	; $B415
	rst $20	; $B418
	ld a,(hl)	; $B419
	ret	; $B41A
	.INCBIN "bank02.bin" SKIP $341B READ 8	; data, 8 bytes

fn_02_B423:
	cp $02	; $B423
	ccf	; $B425
	ret nc	; $B426
	cp $8C	; $B427
	ret c	; $B429
	cp $90	; $B42A
	ccf	; $B42C
	ret	; $B42D

fn_02_B42E:
	push bc	; $B42E
	ld b,a	; $B42F
	ld a,($C98A)	; $B430
	cp $04	; $B433
	jr z,loc_02_B443	; $B435
	cp b	; $B437
	jr z,loc_02_B455	; $B438
	add a,$02	; $B43A
	and $03	; $B43C
	cp b	; $B43E
	jr z,loc_02_B44C	; $B43F
	jr loc_02_B45E	; $B441

loc_02_B443:
	ld a,($C8FD)	; $B443
	and $07	; $B446
	jr z,loc_02_B45E	; $B448
	jr loc_02_B461	; $B44A

loc_02_B44C:
	ld a,($C8FD)	; $B44C
	and $03	; $B44F
	jr z,loc_02_B461	; $B451
	jr loc_02_B45E	; $B453

loc_02_B455:
	ld a,($C8FD)	; $B455
	and $03	; $B458
	jr z,loc_02_B45E	; $B45A
	jr loc_02_B461	; $B45C

loc_02_B45E:
	or a	; $B45E
	jr loc_02_B462	; $B45F

loc_02_B461:
	scf	; $B461

loc_02_B462:
	pop bc	; $B462
	ret	; $B463

fn_02_B464:
	ld a,(map_mode)	; $B464
	or a	; $B467
	ret z	; $B468
	ld a,(party_x)	; $B469
	add a,l	; $B46C
	ld l,a	; $B46D
	ld e,a	; $B46E
	cp $20	; $B46F
	ret nc	; $B471
	ld a,(party_y)	; $B472
	add a,h	; $B475
	ld h,a	; $B476
	ld d,a	; $B477
	cp $20	; $B478
	ret nc	; $B47A
	call fn_02_B5A4	; $B47B
	ld a,(hl)	; $B47E
	cp $3A	; $B47F
	jr z,loc_02_B489	; $B481
	cp $3B	; $B483
	jr z,loc_02_B49C	; $B485
	or a	; $B487
	ret	; $B488

loc_02_B489:
	ld hl,$BD0E	; $B489
	call print_text_hl	; $B48C
	ld a,$10	; $B48F
	call fn_00_05F1	; $B491
	ld hl,$BD19	; $B494
	call print_text_hl	; $B497
	scf	; $B49A
	ret	; $B49B

loc_02_B49C:
	ld a,($C9A4)	; $B49C
	or a	; $B49F
	jr z,loc_02_B4AF	; $B4A0
	push hl	; $B4A2
	ld hl,($C9A2)	; $B4A3
	call fn_02_B5A4	; $B4A6
	ld (hl),$3B	; $B4A9
	inc l	; $B4AB
	ld (hl),$00	; $B4AC
	pop hl	; $B4AE

loc_02_B4AF:
	ld (hl),$3E	; $B4AF
	inc l	; $B4B1
	ld (hl),$00	; $B4B2
	ld a,$02	; $B4B4
	ld ($C9A4),a	; $B4B6
	ex de,hl	; $B4B9
	ld ($C9A2),hl	; $B4BA
	ld hl,$BD0E	; $B4BD
	call print_text_hl	; $B4C0
	ld a,$0E	; $B4C3
	call fn_00_05F1	; $B4C5
	scf	; $B4C8
	ret	; $B4C9

fn_02_B4CA:
	call fn_01_465D	; $B4CA
	call fn_02_B55E	; $B4CD
	call fn_02_B4EE	; $B4D0
	call fn_01_4776	; $B4D3
	call fn_02_957C	; $B4D6
	jr z,loc_02_B4E1	; $B4D9
	call fn_02_B5B3	; $B4DB
	call fn_02_BB17	; $B4DE

loc_02_B4E1:
	call fn_00_372E	; $B4E1
	call fn_00_3754	; $B4E4
	call fn_01_4324	; $B4E7
	call fn_02_B56D	; $B4EA
	ret	; $B4ED

fn_02_B4EE:
	ld iy,party	; $B4EE
	ld de,$001D	; $B4F2
	ld a,(party_size)	; $B4F5
	ld b,a	; $B4F8
	ld c,$00	; $B4F9
	ld (ix+$17),$01	; $B4FB

loc_02_B4FF:
	ld a,(iy+$1C)	; $B4FF
	cp $53	; $B502
	jr nz,loc_02_B513	; $B504
	call random	; $B506
	and $07	; $B509
	jr nz,loc_02_B523	; $B50B
	ld (iy+$1C),$47	; $B50D
	jr loc_02_B523	; $B511

loc_02_B513:
	cp $50	; $B513
	jr nz,loc_02_B523	; $B515
	ld hl,$0002	; $B517
	call member_take_damage	; $B51A
	call fn_01_49A6	; $B51D
	inc (ix+$17)	; $B520

loc_02_B523:
	add iy,de	; $B523
	inc c	; $B525
	djnz loc_02_B4FF	; $B526
	dec (ix+$17)	; $B528
	jr z,loc_02_B535	; $B52B
	ld a,$11	; $B52D
	call fn_00_05F1	; $B52F
	call wait_vblank	; $B532

loc_02_B535:
	ld a,(party_size)	; $B535
	call fn_01_4753	; $B538
	ret nc	; $B53B
	ld hl,$BD22	; $B53C
	call print_text_hl	; $B53F
	ld a,(party_size)	; $B542
	ld b,a	; $B545
	ld c,$00	; $B546

loc_02_B548:
	call fn_01_4445	; $B548
	jr c,loc_02_B553	; $B54B
	ld hl,$0002	; $B54D
	call member_take_damage	; $B550

loc_02_B553:
	inc c	; $B553
	djnz loc_02_B548	; $B554
	ld a,$11	; $B556
	call fn_00_05F1	; $B558
	jp fn_01_4896	; $B55B

fn_02_B55E:
	ld hl,$C961	; $B55E
	ld a,(hl)	; $B561
	cp $32	; $B562
	ret nc	; $B564
	call random	; $B565
	and $03	; $B568
	ret nz	; $B56A
	inc (hl)	; $B56B
	ret	; $B56C

fn_02_B56D:
	ld a,(map_mode)	; $B56D
	cp $01	; $B570
	ret nz	; $B572
	push hl	; $B573
	ld hl,$C9A4	; $B574
	ld a,(hl)	; $B577
	or a	; $B578
	jr z,loc_02_B5A2	; $B579
	dec (hl)	; $B57B
	jr nz,loc_02_B5A2	; $B57C
	ld a,($C9A2)	; $B57E
	ld hl,party_x	; $B581
	cp (hl)	; $B584
	jr nz,loc_02_B597	; $B585
	ld a,($C9A3)	; $B587
	ld hl,party_y	; $B58A
	cp (hl)	; $B58D
	jr nz,loc_02_B597	; $B58E
	ld a,$01	; $B590
	ld ($C9A4),a	; $B592
	jr loc_02_B5A2	; $B595

loc_02_B597:
	ld hl,($C9A2)	; $B597
	call fn_02_B5A4	; $B59A
	ld (hl),$3B	; $B59D
	inc l	; $B59F
	ld (hl),$00	; $B5A0

loc_02_B5A2:
	pop hl	; $B5A2
	ret	; $B5A3

fn_02_B5A4:
	xor a	; $B5A4
	srl h	; $B5A5
	rra	; $B5A7
	srl h	; $B5A8
	rra	; $B5AA
	add a,l	; $B5AB
	add a,l	; $B5AC
	ld l,a	; $B5AD
	ld a,h	; $B5AE
	or $D0	; $B5AF
	ld h,a	; $B5B1
	ret	; $B5B2

fn_02_B5B3:
	call fn_02_BBE7	; $B5B3
	ld hl,($C99A)	; $B5B6
	ld de,($C99C)	; $B5B9
	add hl,de	; $B5BD
	ld ($C99C),hl	; $B5BE
	inc h	; $B5C1
	dec h	; $B5C2
	ret z	; $B5C3

loc_02_B5C4:
	ld a,(map_mode)	; $B5C4
	or a	; $B5C7
	push af	; $B5C8
	call z,fn_02_B9E4	; $B5C9
	pop af	; $B5CC
	call nz,fn_02_B76F	; $B5CD
	ld hl,$C99D	; $B5D0
	dec (hl)	; $B5D3
	jp z,fn_02_BBE7	; $B5D4
	call wait_vblank	; $B5D7
	call fn_00_129D	; $B5DA
	call fn_02_BBE7	; $B5DD
	jr loc_02_B5C4	; $B5E0

fn_02_B5E2:
	push bc	; $B5E2
	push de	; $B5E3
	push ix	; $B5E4
	ld c,a	; $B5E6
	ld a,(map_mode)	; $B5E7
	or a	; $B5EA
	jr z,loc_02_B5F9	; $B5EB
	ld a,l	; $B5ED
	cp $20	; $B5EE
	jp nc,loc_02_B6B0	; $B5F0
	ld a,h	; $B5F3
	cp $20	; $B5F4
	jp nc,loc_02_B6B0	; $B5F6

loc_02_B5F9:
	ld a,c	; $B5F9
	call fn_00_3EEF	; $B5FA
	jp c,loc_02_B6B1	; $B5FD
	ld a,(iy+$03)	; $B600
	cp $8C	; $B603
	jr z,loc_02_B63B	; $B605
	cp $8E	; $B607
	jr z,loc_02_B63B	; $B609
	ld ix,location_people	; $B60B
	ld de,$0008	; $B60F
	ld b,$20	; $B612

loc_02_B614:
	ld a,(ix+$03)	; $B614
	or a	; $B617
	jr z,loc_02_B62B	; $B618
	cp $3C	; $B61A
	jr z,loc_02_B62B	; $B61C
	ld a,(ix+$01)	; $B61E
	cp l	; $B621
	jr nz,loc_02_B62B	; $B622
	ld a,(ix+$02)	; $B624
	cp h	; $B627
	jp z,loc_02_B6B0	; $B628

loc_02_B62B:
	add ix,de	; $B62B
	djnz loc_02_B614	; $B62D
	ld a,(party_x)	; $B62F
	cp l	; $B632
	jr nz,loc_02_B63B	; $B633
	ld a,(party_y)	; $B635
	cp h	; $B638
	jr z,loc_02_B6B0	; $B639

loc_02_B63B:
	ld b,(iy+$03)	; $B63B
	ld a,b	; $B63E
	cp $80	; $B63F
	jr z,loc_02_B656	; $B641
	ld a,(iy+$04)	; $B643
	cp l	; $B646
	jr nz,loc_02_B656	; $B647
	ld a,(iy+$05)	; $B649
	cp h	; $B64C
	jr nz,loc_02_B656	; $B64D
	call random	; $B64F
	and $03	; $B652
	jr z,loc_02_B6B0	; $B654

loc_02_B656:
	ld a,b	; $B656
	cp $80	; $B657
	jr nc,loc_02_B661	; $B659
	ld a,c	; $B65B
	call fn_00_3EB7	; $B65C
	jr loc_02_B6B1	; $B65F

loc_02_B661:
	cp $8E	; $B661
	jr nc,loc_02_B674	; $B663
	cp $80	; $B665
	ld a,c	; $B667
	jr nz,loc_02_B66F	; $B668
	cp $02	; $B66A
	ccf	; $B66C
	jr loc_02_B6B1	; $B66D

loc_02_B66F:
	cp $03	; $B66F
	ccf	; $B671
	jr loc_02_B6B1	; $B672

loc_02_B674:
	jr z,loc_02_B68A	; $B674
	cp $94	; $B676
	jr z,loc_02_B68A	; $B678
	cp $B4	; $B67A
	jr z,loc_02_B68A	; $B67C
	cp $F0	; $B67E
	jr z,loc_02_B68A	; $B680
	cp $F8	; $B682
	jr z,loc_02_B68A	; $B684
	cp $FC	; $B686
	jr nz,loc_02_B694	; $B688

loc_02_B68A:
	ld a,c	; $B68A
	cp $03	; $B68B
	jr c,loc_02_B6AD	; $B68D
	call fn_00_3EB7	; $B68F
	jr loc_02_B6B1	; $B692

loc_02_B694:
	cp $EC	; $B694
	jr z,loc_02_B69C	; $B696
	cp $9C	; $B698
	jr nz,loc_02_B6A7	; $B69A

loc_02_B69C:
	ld a,c	; $B69C
	cp $03	; $B69D
	jr c,loc_02_B6B1	; $B69F
	cp $45	; $B6A1
	jr z,loc_02_B6B0	; $B6A3
	jr loc_02_B6AD	; $B6A5

loc_02_B6A7:
	ld a,c	; $B6A7
	call fn_00_3EB7	; $B6A8
	jr loc_02_B6B1	; $B6AB

loc_02_B6AD:
	or a	; $B6AD
	jr loc_02_B6B1	; $B6AE

loc_02_B6B0:
	scf	; $B6B0

loc_02_B6B1:
	pop ix	; $B6B1
	pop de	; $B6B3
	pop bc	; $B6B4
	ret	; $B6B5

loc_02_B6B6:
	ld a,l	; $B6B6
	cp $20	; $B6B7
	ret nc	; $B6B9
	ld a,h	; $B6BA
	cp $20	; $B6BB
	ret nc	; $B6BD

fn_02_B6BE:
	ld a,(iy+$01)	; $B6BE
	ld (iy+$04),a	; $B6C1
	ld (iy+$01),l	; $B6C4
	ld a,(iy+$02)	; $B6C7
	ld (iy+$05),a	; $B6CA
	ld (iy+$02),h	; $B6CD
	ret	; $B6D0

fn_02_B6D1:
	push de	; $B6D1
	ld a,(iy+$01)	; $B6D2
	add a,l	; $B6D5
	ld e,a	; $B6D6
	ld a,(view_chunk_x)	; $B6D7
	add a,a	; $B6DA
	add a,a	; $B6DB
	add a,a	; $B6DC
	add a,a	; $B6DD
	neg	; $B6DE
	add a,e	; $B6E0
	cp $20	; $B6E1
	jp nc,loc_02_B730	; $B6E3
	ld e,a	; $B6E6
	ld a,(iy+$02)	; $B6E7
	add a,h	; $B6EA
	ld d,a	; $B6EB
	ld a,(view_chunk_y)	; $B6EC
	add a,a	; $B6EF
	add a,a	; $B6F0
	add a,a	; $B6F1
	add a,a	; $B6F2
	neg	; $B6F3
	add a,d	; $B6F5
	cp $20	; $B6F6
	jp nc,loc_02_B730	; $B6F8
	ld d,a	; $B6FB
	call random	; $B6FC
	and $01	; $B6FF
	jr z,loc_02_B715	; $B701
	inc l	; $B703
	dec l	; $B704
	jr z,loc_02_B719	; $B705

loc_02_B707:
	ld a,(iy+$01)	; $B707
	add a,l	; $B70A
	ld l,a	; $B70B
	ld a,d	; $B70C
	sub h	; $B70D
	ld d,a	; $B70E
	ld h,(iy+$02)	; $B70F
	jp loc_02_B724	; $B712

loc_02_B715:
	inc h	; $B715
	dec h	; $B716
	jr z,loc_02_B707	; $B717

loc_02_B719:
	ld a,(iy+$02)	; $B719
	add a,h	; $B71C
	ld h,a	; $B71D
	ld a,e	; $B71E
	sub l	; $B71F
	ld e,a	; $B720
	ld l,(iy+$01)	; $B721

loc_02_B724:
	call fn_02_BAF3	; $B724
	call fn_02_B5E2	; $B727
	jr c,loc_02_B730	; $B72A
	pop de	; $B72C
	jp fn_02_B6BE	; $B72D

loc_02_B730:
	pop de	; $B730
	dec c	; $B731
	ret z	; $B732
	call fn_02_BB03	; $B733
	jp fn_02_B6D1	; $B736

fn_02_B739:
	call random	; $B739
	and $01	; $B73C
	jr z,loc_02_B74F	; $B73E
	inc l	; $B740
	dec l	; $B741
	jr z,loc_02_B753	; $B742

loc_02_B744:
	ld a,(iy+$01)	; $B744
	add a,l	; $B747
	ld l,a	; $B748
	ld h,(iy+$02)	; $B749
	jp loc_02_B75B	; $B74C

loc_02_B74F:
	inc h	; $B74F
	dec h	; $B750
	jr z,loc_02_B744	; $B751

loc_02_B753:
	ld a,(iy+$02)	; $B753
	add a,h	; $B756
	ld h,a	; $B757
	ld l,(iy+$01)	; $B758

loc_02_B75B:
	push hl	; $B75B
	call fn_02_B5A4	; $B75C
	ld a,(hl)	; $B75F
	pop hl	; $B760
	call fn_02_B5E2	; $B761
	jp nc,loc_02_B6B6	; $B764
	dec c	; $B767
	ret z	; $B768
	call fn_02_BB03	; $B769
	jp fn_02_B739	; $B76C

fn_02_B76F:
	ld iy,location_people	; $B76F
	ld de,$0008	; $B773
	ld b,$20	; $B776

loc_02_B778:
	ld a,(iy+$03)	; $B778
	or a	; $B77B
	jr z,loc_02_B7E6	; $B77C
	ld a,(iy+$06)	; $B77E
	or a	; $B781
	jr z,loc_02_B7E6	; $B782
	jp m,loc_02_B798	; $B784
	call random	; $B787
	and $01	; $B78A
	jr z,loc_02_B7E6	; $B78C
	call fn_02_BB03	; $B78E
	ld c,$02	; $B791
	call fn_02_B739	; $B793
	jr loc_02_B7E6	; $B796

loc_02_B798:
	ld a,(party_x)	; $B798
	sub (iy+$01)	; $B79B
	ld l,a	; $B79E
	jp p,loc_02_B7A4	; $B79F
	neg	; $B7A2

loc_02_B7A4:
	ld c,a	; $B7A4
	ld a,(party_y)	; $B7A5
	sub (iy+$02)	; $B7A8
	ld h,a	; $B7AB
	jp p,loc_02_B7B1	; $B7AC
	neg	; $B7AF

loc_02_B7B1:
	add a,c	; $B7B1
	cp $02	; $B7B2
	jr c,loc_02_B7C7	; $B7B4
	ld a,l	; $B7B6
	call fn_02_BB0F	; $B7B7
	ld l,a	; $B7BA
	ld a,h	; $B7BB
	call fn_02_BB0F	; $B7BC
	ld h,a	; $B7BF
	ld c,$02	; $B7C0
	call fn_02_B739	; $B7C2
	jr loc_02_B7E6	; $B7C5

loc_02_B7C7:
	ld a,(iy+$06)	; $B7C7
	cp $FF	; $B7CA
	jr nz,loc_02_B7E6	; $B7CC
	call fn_00_129D	; $B7CE
	ld hl,$BD2F	; $B7D1
	call print_text_hl	; $B7D4
	ld a,(iy+$03)	; $B7D7
	call fn_01_47D3	; $B7DA
	call print_text_hl	; $B7DD
	call print_newline	; $B7E0
	jp loc_00_3BD9	; $B7E3

loc_02_B7E6:
	add iy,de	; $B7E6
	djnz loc_02_B778	; $B7E8
	ret	; $B7EA

fn_02_B7EB:
	push de	; $B7EB
	ld e,l	; $B7EC
	ld d,h	; $B7ED
	call fn_02_BAD5	; $B7EE
	call fn_02_B5E2	; $B7F1
	jr c,loc_02_B803	; $B7F4
	ld a,(iy+$00)	; $B7F6
	and $03	; $B7F9
	call fn_02_B42E	; $B7FB
	call nc,fn_02_B6BE	; $B7FE
	jr loc_02_B810	; $B801

loc_02_B803:
	call fn_02_BB0C	; $B803
	add a,(iy+$00)	; $B806
	and $03	; $B809
	or $80	; $B80B
	ld (iy+$00),a	; $B80D

loc_02_B810:
	pop de	; $B810
	ret	; $B811

loc_02_B812:
	push hl	; $B812
	ld a,e	; $B813
	call fn_00_000B	; $B814
	ld l,a	; $B817
	ld a,d	; $B818
	call fn_00_000B	; $B819
	ld h,a	; $B81C
	add a,l	; $B81D
	cp $06	; $B81E
	jr nc,loc_02_B829	; $B820
	call random	; $B822
	and $03	; $B825
	jr nz,loc_02_B844	; $B827

loc_02_B829:
	ld a,l	; $B829
	cp h	; $B82A
	jr nc,loc_02_B837	; $B82B
	bit 7,d	; $B82D
	ld a,$83	; $B82F
	jr z,loc_02_B83F	; $B831
	ld a,$81	; $B833
	jr loc_02_B83F	; $B835

loc_02_B837:
	bit 7,e	; $B837
	ld a,$82	; $B839
	jr z,loc_02_B83F	; $B83B
	ld a,$80	; $B83D

loc_02_B83F:
	ld (iy+$00),a	; $B83F
	jr loc_02_B865	; $B842

loc_02_B844:
	ld l,(iy+$01)	; $B844
	ld h,(iy+$02)	; $B847
	ld a,(iy+$00)	; $B84A
	and $03	; $B84D
	dec a	; $B84F
	jr z,loc_02_B85B	; $B850
	dec a	; $B852
	jr z,loc_02_B85E	; $B853
	dec a	; $B855
	jr z,loc_02_B861	; $B856
	dec l	; $B858
	jr loc_02_B862	; $B859

loc_02_B85B:
	dec h	; $B85B
	jr loc_02_B862	; $B85C

loc_02_B85E:
	inc l	; $B85E
	jr loc_02_B862	; $B85F

loc_02_B861:
	inc h	; $B861

loc_02_B862:
	call fn_02_B7EB	; $B862

loc_02_B865:
	pop hl	; $B865
	ret	; $B866

fn_02_B867:
	ld a,(iy+$00)	; $B867
	cp $81	; $B86A
	jr z,loc_02_B885	; $B86C
	cp $82	; $B86E
	jr z,loc_02_B894	; $B870
	cp $83	; $B872
	jr z,loc_02_B89E	; $B874
	ld a,l	; $B876
	or a	; $B877
	jp p,loc_02_B812	; $B878

loc_02_B87B:
	add a,(iy+$01)	; $B87B
	ld l,a	; $B87E
	ld h,(iy+$02)	; $B87F
	jp fn_02_B7EB	; $B882

loc_02_B885:
	ld a,h	; $B885
	or a	; $B886
	jp p,loc_02_B812	; $B887

loc_02_B88A:
	add a,(iy+$02)	; $B88A
	ld h,a	; $B88D
	ld l,(iy+$01)	; $B88E
	jp fn_02_B7EB	; $B891

loc_02_B894:
	ld a,l	; $B894
	or a	; $B895
	jp z,loc_02_B812	; $B896
	jp m,loc_02_B812	; $B899
	jr loc_02_B87B	; $B89C

loc_02_B89E:
	ld a,h	; $B89E
	or a	; $B89F
	jp z,loc_02_B812	; $B8A0
	jp m,loc_02_B812	; $B8A3
	jr loc_02_B88A	; $B8A6

fn_02_B8A8:
	ld a,(iy+$00)	; $B8A8
	cp $80	; $B8AB
	jr z,loc_02_B8B3	; $B8AD
	cp $82	; $B8AF
	jr nz,loc_02_B8BD	; $B8B1

loc_02_B8B3:
	inc l	; $B8B3
	dec l	; $B8B4
	jp nz,fn_02_B867	; $B8B5
	ld a,$07	; $B8B8
	jp fn_02_B8C7	; $B8BA

loc_02_B8BD:
	inc h	; $B8BD
	dec h	; $B8BE
	jp nz,fn_02_B867	; $B8BF
	ld a,$07	; $B8C2
	jp fn_02_B8C7	; $B8C4

fn_02_B8C7:
	push bc	; $B8C7
	push de	; $B8C8
	push hl	; $B8C9
	push iy	; $B8CA
	call fn_00_30DA	; $B8CC
	ld (ix+$09),$08	; $B8CF
	ex de,hl	; $B8D3
	inc e	; $B8D4
	inc d	; $B8D5
	ld hl,$0101	; $B8D6
	call fn_00_2BFE	; $B8D9
	add hl,hl	; $B8DC
	add hl,hl	; $B8DD
	ld (ix+$0E),l	; $B8DE
	ld (ix+$0F),h	; $B8E1
	ex de,hl	; $B8E4
	add hl,hl	; $B8E5
	add hl,hl	; $B8E6
	ld (ix+$10),l	; $B8E7
	ld (ix+$11),h	; $B8EA
	ld a,(party_x)	; $B8ED
	neg	; $B8F0
	add a,(iy+$01)	; $B8F2
	add a,$05	; $B8F5
	call fn_00_36F7	; $B8F7
	ld (ix+$0B),a	; $B8FA
	ld (ix+$0A),$00	; $B8FD
	ld a,(party_y)	; $B901
	neg	; $B904
	add a,(iy+$02)	; $B906
	add a,$05	; $B909
	call fn_00_36F7	; $B90B
	ld (ix+$0D),a	; $B90E
	ld (ix+$0C),$00	; $B911
	ld a,$1C	; $B915
	call fn_00_05F1	; $B917
	call fn_00_129D	; $B91A
	ld b,$03	; $B91D

loc_02_B91F:
	push bc	; $B91F
	call fn_02_B9C3	; $B920
	pop bc	; $B923
	djnz loc_02_B91F	; $B924
	ld b,$0C	; $B926

loc_02_B928:
	push bc	; $B928
	call fn_02_B9C3	; $B929
	ld l,(ix+$2C)	; $B92C
	ld h,(ix+$2D)	; $B92F
	ld a,$05	; $B932
	cp l	; $B934
	jr nz,loc_02_B93E	; $B935
	cp h	; $B937
	jr nz,loc_02_B93E	; $B938
	ld a,$FF	; $B93A
	jr loc_02_B958	; $B93C

loc_02_B93E:
	ld a,(party_x)	; $B93E
	add a,l	; $B941
	add a,$FB	; $B942
	ld l,a	; $B944
	ld a,(party_y)	; $B945
	add a,h	; $B948
	add a,$FB	; $B949
	ld h,a	; $B94B
	ld b,$20	; $B94C
	call fn_01_42B7	; $B94E
	jr nc,loc_02_B958	; $B951
	pop bc	; $B953
	djnz loc_02_B928	; $B954
	jr loc_02_B9BD	; $B956

loc_02_B958:
	pop bc	; $B958
	push af	; $B959
	ld l,(ix+$2C)	; $B95A
	ld h,(ix+$2D)	; $B95D
	call fn_00_3991	; $B960
	ld (ix+$0B),l	; $B963
	ld (ix+$0D),h	; $B966
	ld a,$12	; $B969
	call fn_00_05F1	; $B96B
	call fn_00_0950	; $B96E
	ld a,$04	; $B971
	call fn_00_307C	; $B973
	ld b,$03	; $B976
	call fn_00_0437	; $B978
	call fn_00_0950	; $B97B
	ld a,$05	; $B97E
	call fn_00_307C	; $B980
	ld b,$04	; $B983
	call fn_00_0437	; $B985
	call fn_00_0950	; $B988
	ld a,$06	; $B98B
	call fn_00_307C	; $B98D
	ld b,$05	; $B990
	call fn_00_0437	; $B992
	call fn_00_0950	; $B995
	call wait_vblank	; $B998
	pop af	; $B99B
	or a	; $B99C
	jp p,loc_02_B9A5	; $B99D
	call fn_01_4812	; $B9A0
	jr loc_02_B9BD	; $B9A3

loc_02_B9A5:
	push af	; $B9A5
	call fn_01_48F7	; $B9A6
	pop af	; $B9A9
	cp $08	; $B9AA
	jr nc,loc_02_B9B5	; $B9AC
	call random	; $B9AE
	and $03	; $B9B1
	jr nz,loc_02_B9BD	; $B9B3

loc_02_B9B5:
	ld (iy+$03),$00	; $B9B5
	ld (iy+$00),$00	; $B9B9

loc_02_B9BD:
	pop iy	; $B9BD
	pop hl	; $B9BF
	pop de	; $B9C0
	pop bc	; $B9C1
	ret	; $B9C2

fn_02_B9C3:
	call fn_00_0950	; $B9C3
	call fn_00_0BA4	; $B9C6
	call fn_00_399C	; $B9C9
	ld (ix+$2C),l	; $B9CC
	ld (ix+$2D),h	; $B9CF
	ld a,h	; $B9D2
	call fn_00_36ED	; $B9D3
	add a,l	; $B9D6
	add a,a	; $B9D7
	ld l,a	; $B9D8
	ld h,$D9	; $B9D9
	ld a,(hl)	; $B9DB
	cp $7E	; $B9DC
	call nz,fn_00_3074	; $B9DE
	jp wait_vblank	; $B9E1

fn_02_B9E4:
	ld iy,location_people	; $B9E4
	ld b,$08	; $B9E8

loc_02_B9EA:
	ld a,(iy+$03)	; $B9EA
	or a	; $B9ED
	jp z,loc_02_BAC5	; $B9EE
	ld a,(view_chunk_x)	; $B9F1
	add a,a	; $B9F4
	add a,a	; $B9F5
	add a,a	; $B9F6
	add a,a	; $B9F7
	neg	; $B9F8
	add a,(iy+$01)	; $B9FA
	cp $20	; $B9FD
	jr nc,loc_02_BA11	; $B9FF
	ld a,(view_chunk_y)	; $BA01
	add a,a	; $BA04
	add a,a	; $BA05
	add a,a	; $BA06
	add a,a	; $BA07
	neg	; $BA08
	add a,(iy+$02)	; $BA0A
	cp $20	; $BA0D
	jr c,loc_02_BA1C	; $BA0F

loc_02_BA11:
	ld (iy+$03),$00	; $BA11
	ld (iy+$00),$00	; $BA15
	jp loc_02_BAC5	; $BA19

loc_02_BA1C:
	ld a,(party_x)	; $BA1C
	sub (iy+$01)	; $BA1F
	ld e,a	; $BA22
	call fn_02_BB0F	; $BA23
	ld l,a	; $BA26
	ld a,(party_y)	; $BA27
	sub (iy+$02)	; $BA2A
	ld d,a	; $BA2D
	call fn_02_BB0F	; $BA2E
	ld h,a	; $BA31
	ld a,e	; $BA32
	call fn_00_000B	; $BA33
	ld c,a	; $BA36
	ld a,d	; $BA37
	call fn_00_000B	; $BA38
	add a,c	; $BA3B
	ld c,a	; $BA3C
	ld a,(iy+$03)	; $BA3D
	cp $8C	; $BA40
	jr z,loc_02_BA65	; $BA42
	cp $8E	; $BA44
	jr z,loc_02_BA65	; $BA46
	ld a,c	; $BA48
	cp $02	; $BA49
	jr nc,loc_02_BA65	; $BA4B
	call fn_00_129D	; $BA4D
	ld hl,$BD2F	; $BA50
	call print_text_hl	; $BA53
	ld a,(iy+$03)	; $BA56
	call fn_01_47D3	; $BA59
	call print_text_hl	; $BA5C
	call print_newline	; $BA5F
	jp loc_00_3BD9	; $BA62

loc_02_BA65:
	ld a,(iy+$03)	; $BA65
	cp $80	; $BA68
	jr nz,loc_02_BA7B	; $BA6A
	ld a,c	; $BA6C
	cp $04	; $BA6D
	jr nc,loc_02_BA76	; $BA6F
	call fn_02_B8A8	; $BA71
	jr loc_02_BAC5	; $BA74

loc_02_BA76:
	call fn_02_B867	; $BA76
	jr loc_02_BAC5	; $BA79

loc_02_BA7B:
	cp $88	; $BA7B
	jr z,loc_02_BA87	; $BA7D
	cp $E8	; $BA7F
	jr z,loc_02_BA87	; $BA81
	cp $F4	; $BA83
	jr c,loc_02_BA9F	; $BA85

loc_02_BA87:
	ld a,e	; $BA87
	cp $05	; $BA88
	jr nc,loc_02_BA9F	; $BA8A
	ld a,d	; $BA8C
	cp $05	; $BA8D
	jr nc,loc_02_BA9F	; $BA8F
	call random	; $BA91
	and $01	; $BA94
	jr z,loc_02_BA9F	; $BA96
	ld a,$09	; $BA98
	call fn_02_B8C7	; $BA9A
	jr loc_02_BAC5	; $BA9D

loc_02_BA9F:
	ld a,(iy+$03)	; $BA9F
	cp $8C	; $BAA2
	jr z,loc_02_BAB6	; $BAA4
	cp $8E	; $BAA6
	jr z,loc_02_BAB6	; $BAA8
	call fn_01_430A	; $BAAA
	jr c,loc_02_BAC0	; $BAAD
	call random	; $BAAF
	and $03	; $BAB2
	jr z,loc_02_BAC0	; $BAB4

loc_02_BAB6:
	call fn_02_BB03	; $BAB6
	ld c,$01	; $BAB9
	call fn_02_B6D1	; $BABB
	jr loc_02_BAC5	; $BABE

loc_02_BAC0:
	ld c,$02	; $BAC0
	call fn_02_B6D1	; $BAC2

loc_02_BAC5:
	ld de,$0008	; $BAC5
	add iy,de	; $BAC8
	dec b	; $BACA
	jp nz,loc_02_B9EA	; $BACB
	ret	; $BACE

fn_02_BACF:
	ld a,(location)	; $BACF
	or a	; $BAD2
	jr nz,fn_02_BAF3	; $BAD3

fn_02_BAD5:
	ld a,(view_chunk_x)	; $BAD5
	add a,a	; $BAD8
	add a,a	; $BAD9
	add a,a	; $BADA
	add a,a	; $BADB
	neg	; $BADC
	add a,e	; $BADE
	ld e,a	; $BADF
	cp $20	; $BAE0
	ccf	; $BAE2
	ret c	; $BAE3
	ld a,(view_chunk_y)	; $BAE4
	add a,a	; $BAE7
	add a,a	; $BAE8
	add a,a	; $BAE9
	add a,a	; $BAEA
	neg	; $BAEB
	add a,d	; $BAED
	ld d,a	; $BAEE
	cp $20	; $BAEF
	ccf	; $BAF1
	ret c	; $BAF2

fn_02_BAF3:
	xor a	; $BAF3
	srl d	; $BAF4
	rra	; $BAF6
	srl d	; $BAF7
	rra	; $BAF9
	add a,e	; $BAFA
	add a,e	; $BAFB
	ld e,a	; $BAFC
	ld a,d	; $BAFD
	or $D0	; $BAFE
	ld d,a	; $BB00
	ld a,(de)	; $BB01
	ret	; $BB02

fn_02_BB03:
	call fn_02_BB0C	; $BB03
	ld l,a	; $BB06
	call fn_02_BB0C	; $BB07
	ld h,a	; $BB0A
	ret	; $BB0B

fn_02_BB0C:
	call random	; $BB0C

fn_02_BB0F:
	or a	; $BB0F
	ret z	; $BB10
	ld a,$01	; $BB11
	ret p	; $BB13
	ld a,$FF	; $BB14
	ret	; $BB16

fn_02_BB17:
	ld a,(map_mode)	; $BB17
	or a	; $BB1A
	ret nz	; $BB1B
	call random	; $BB1C
	and $0F	; $BB1F
	ret nz	; $BB21
	ld iy,location_people	; $BB22
	ld b,$04	; $BB26

loc_02_BB28:
	push bc	; $BB28
	ld a,(iy+$03)	; $BB29
	or a	; $BB2C
	jp nz,loc_02_BBBB	; $BB2D
	call random	; $BB30
	and $1F	; $BB33
	ld e,a	; $BB35
	call random	; $BB36
	and $1F	; $BB39
	ld d,a	; $BB3B
	ld a,(view_x)	; $BB3C
	sub e	; $BB3F
	jr nc,loc_02_BB44	; $BB40
	neg	; $BB42

loc_02_BB44:
	cp $06	; $BB44
	jr c,loc_02_BBBB	; $BB46
	ld a,(view_y)	; $BB48
	sub d	; $BB4B
	jr nc,loc_02_BB50	; $BB4C
	neg	; $BB4E

loc_02_BB50:
	cp $06	; $BB50
	jr c,loc_02_BBBB	; $BB52
	push de	; $BB54
	call fn_02_BAF3	; $BB55
	pop de	; $BB58
	cp $02	; $BB59
	jr nc,loc_02_BB74	; $BB5B
	call random	; $BB5D
	and $07	; $BB60
	jr nz,loc_02_BBBB	; $BB62
	call random	; $BB64
	and $07	; $BB67
	add a,a	; $BB69
	add a,$80	; $BB6A
	cp $82	; $BB6C
	jr nz,loc_02_BB72	; $BB6E
	ld a,$80	; $BB70

loc_02_BB72:
	jr loc_02_BB9F	; $BB72

loc_02_BB74:
	cp $04	; $BB74
	jr c,loc_02_BBBB	; $BB76
	cp $08	; $BB78
	jr nc,loc_02_BBBB	; $BB7A
	ld hl,$BBD7	; $BB7C
	call fn_02_BBC6	; $BB7F
	ld b,$03	; $BB82
	jr c,loc_02_BB92	; $BB84
	ld hl,$BBDF	; $BB86
	call fn_02_BBC6	; $BB89
	ld b,$07	; $BB8C
	jr c,loc_02_BB92	; $BB8E
	ld b,$0F	; $BB90

loc_02_BB92:
	call random	; $BB92
	ld c,a	; $BB95
	call random	; $BB96
	and c	; $BB99
	and b	; $BB9A
	add a,a	; $BB9B
	add a,a	; $BB9C
	add a,$C0	; $BB9D

loc_02_BB9F:
	ld (iy+$03),a	; $BB9F
	ld (iy+$00),a	; $BBA2
	ld a,(view_chunk_x)	; $BBA5
	add a,a	; $BBA8
	add a,a	; $BBA9
	add a,a	; $BBAA
	add a,a	; $BBAB
	add a,e	; $BBAC
	ld (iy+$01),a	; $BBAD
	ld a,(view_chunk_y)	; $BBB0
	add a,a	; $BBB3
	add a,a	; $BBB4
	add a,a	; $BBB5
	add a,a	; $BBB6
	add a,d	; $BBB7
	ld (iy+$02),a	; $BBB8

loc_02_BBBB:
	ld bc,$0008	; $BBBB
	add iy,bc	; $BBBE
	pop bc	; $BBC0
	dec b	; $BBC1
	jp nz,loc_02_BB28	; $BBC2
	ret	; $BBC5

fn_02_BBC6:
	push bc	; $BBC6
	push de	; $BBC7
	ld de,$C8FD	; $BBC8
	xor a	; $BBCB
	ld b,$04	; $BBCC

loc_02_BBCE:
	ld a,(de)	; $BBCE
	sbc a,(hl)	; $BBCF
	inc de	; $BBD0
	inc hl	; $BBD1
	djnz loc_02_BBCE	; $BBD2
	pop de	; $BBD4
	pop bc	; $BBD5
	ret	; $BBD6
	.INCBIN "bank02.bin" SKIP $3BD7 READ 16	; data, 16 bytes

fn_02_BBE7:
	ld a,(map_mode)	; $BBE7
	or a	; $BBEA
	ret nz	; $BBEB
	ld iy,location_people	; $BBEC
	ld hl,(party_x)	; $BBF0
	ld de,$0008	; $BBF3
	ld b,$04	; $BBF6

loc_02_BBF8:
	ld a,(iy+$01)	; $BBF8
	cp l	; $BBFB
	jr nz,loc_02_BC0F	; $BBFC
	ld a,(iy+$02)	; $BBFE
	cp h	; $BC01
	jr nz,loc_02_BC0F	; $BC02
	ld a,(iy+$03)	; $BC04
	cp $8C	; $BC07
	jr z,loc_02_BC1F	; $BC09
	cp $8E	; $BC0B
	jr z,loc_02_BC54	; $BC0D

loc_02_BC0F:
	add iy,de	; $BC0F
	djnz loc_02_BBF8	; $BC11
	ret	; $BC13

fn_02_BC14:
	push hl	; $BC14
	call fn_00_129D	; $BC15
	pop hl	; $BC18
	ld ($D978),hl	; $BC19
	jp fn_00_1A7C	; $BC1C

loc_02_BC1F:
	ld hl,$008C	; $BC1F
	call fn_02_BC14	; $BC22
	call fn_01_4812	; $BC25
	ld iy,location_people	; $BC28
	ld hl,(party_x)	; $BC2C
	ld de,$0008	; $BC2F
	ld b,$20	; $BC32

loc_02_BC34:
	ld a,(iy+$01)	; $BC34
	cp l	; $BC37
	jr nz,loc_02_BC48	; $BC38
	ld a,(iy+$02)	; $BC3A
	cp h	; $BC3D
	jr nz,loc_02_BC48	; $BC3E
	ld (iy+$03),$00	; $BC40
	ld (iy+$00),$00	; $BC44

loc_02_BC48:
	add iy,de	; $BC48
	djnz loc_02_BC34	; $BC4A
	ld a,$0F	; $BC4C
	ld (game_mode),a	; $BC4E
	jp main_loop	; $BC51

loc_02_BC54:
	ld hl,$008E	; $BC54
	call fn_02_BC14	; $BC57
	call fn_01_4812	; $BC5A
	call fn_01_4812	; $BC5D
	call fn_01_4812	; $BC60
	call fn_01_4812	; $BC63
	ld iy,location_people	; $BC66
	ld hl,(party_x)	; $BC6A
	ld de,$0008	; $BC6D
	ld b,$20	; $BC70

loc_02_BC72:
	ld a,(iy+$01)	; $BC72
	cp l	; $BC75
	jr nz,loc_02_BC91	; $BC76
	ld a,(iy+$02)	; $BC78
	cp h	; $BC7B
	jr nz,loc_02_BC91	; $BC7C
	ld a,(iy+$03)	; $BC7E
	cp $10	; $BC81
	jr c,loc_02_BC89	; $BC83
	cp $14	; $BC85
	jr c,loc_02_BC91	; $BC87

loc_02_BC89:
	ld (iy+$03),$00	; $BC89
	ld (iy+$00),$00	; $BC8D

loc_02_BC91:
	add iy,de	; $BC91
	djnz loc_02_BC72	; $BC93
	ret	; $BC95

fn_02_BC96:
	call fn_00_0DD7	; $BC96
	ld de,$BCA2	; $BC99
	call spawn_task	; $BC9C
	jp loc_00_0DE1	; $BC9F

sub_02_BCA2:
	call fn_00_05E3	; $BCA2
	ld a,$1A	; $BCA5
	call fn_00_05F1	; $BCA7
	call fn_00_310B	; $BCAA
	ld hl,$4E7F	; $BCAD
	ld (party_x),hl	; $BCB0
	ld hl,$0010	; $BCB3
	ld ($C959),hl	; $BCB6
	ld a,$11	; $BCB9
	ld (game_mode),a	; $BCBB
	ret	; $BCBE
	.INCBIN "bank02.bin" SKIP $3CBF READ 143	; data, 143 bytes

sub_02_BD4E:
	ex af,af'	; $BD4E
	inc b	; $BD4F
	ex af,af'	; $BD50
	dec b	; $BD51
	inc b	; $BD52
	dec b	; $BD53
	ex af,af'	; $BD54
	inc b	; $BD55
	add hl,bc	; $BD56
	nop	; $BD57
	dec b	; $BD58
	dec b	; $BD59
	jp po,loc_00_040A	; $BD5A
	pop bc	; $BD5D
	rlca	; $BD5E
	and b	; $BD5F
	add a,a	; $BD60
	and $C2	; $BD61
	jp nz,$E6EA	; $BD63
	jp pe,fn_00_0000	; $BD66
	xor h	; $BD69
	add a,h	; $BD6A
	ld (bc),a	; $BD6B
	ld (bc),a	; $BD6C
	inc bc	; $BD6D
	add a,c	; $BD6E
	dec b	; $BD6F
	rlca	; $BD70
	inc de	; $BD71
	ex af,af'	; $BD72
	rlca	; $BD73
	ld (bc),a	; $BD74
	ld bc,$0102	; $BD75
	ld bc,$7E08	; $BD78
	ld a,(hl)	; $BD7B
	add a,c	; $BD7C
	ex af,af'	; $BD7D
	ex af,af'	; $BD7E
	inc bc	; $BD7F
	ld (bc),a	; $BD80
	inc bc	; $BD81
	inc bc	; $BD82
	inc b	; $BD83
	ex af,af'	; $BD84
	dec b	; $BD85
	inc b	; $BD86
	inc bc	; $BD87
	add a,h	; $BD88
	add a,c	; $BD89
	ex af,af'	; $BD8A
	ex af,af'	; $BD8B
	inc bc	; $BD8C
	inc bc	; $BD8D
	add a,c	; $BD8E
	ld (bc),a	; $BD8F
	ld b,$06	; $BD90
	ret nz	; $BD92
	nop	; $BD93
	inc bc	; $BD94
	pop bc	; $BD95
	rlca	; $BD96
	add a,c	; $BD97
	add a,c	; $BD98
	ex af,af'	; $BD99
	inc bc	; $BD9A
	add a,c	; $BD9B
	and c	; $BD9C
	add a,e	; $BD9D
	inc bc	; $BD9E
	inc b	; $BD9F
	nop	; $BDA0
	nop	; $BDA1
	jp nz,$00C0	; $BDA2
	rlca	; $BDA5
	nop	; $BDA6
	dec b	; $BDA7
	ret nz	; $BDA8
	ret nz	; $BDA9
	inc b	; $BDAA
	inc bc	; $BDAB
	ret nz	; $BDAC
	jp fn_00_0000	; $BDAD
	.INCBIN "bank02.bin" SKIP $3DB0 READ 592	; data, 592 bytes
