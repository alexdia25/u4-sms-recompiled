; Bank 1 (ROM $04000-$07FFF), mapped at $4000
.BANK 1 SLOT 1
.ORG $0000

	.INCBIN "bank01.bin" SKIP $0000 READ 4	; data, 4 bytes
	call fn_01_48CB	; $4004
	ld a,(iy+$1C)	; $4007
	cp $47	; $400A
	ret nz	; $400C
	ld (iy+$1C),$50	; $400D

loc_01_4011:
	call fn_01_4324	; $4011
	ld bc,($C97F)	; $4014
	jp fn_01_4247	; $4018

sub_01_401B:
	ld bc,($C97F)	; $401B
	call member_able	; $401F
	ret c	; $4022
	call fn_01_48CB	; $4023
	ld (iy+$1C),$53	; $4026
	call fn_01_48E1	; $402A
	ld (iy+$02),$38	; $402D
	ld (iy+$03),$38	; $4031
	jr loc_01_4011	; $4035

sub_01_4037:
	ld a,$20	; $4037
	call random_below	; $4039
	add a,$10	; $403C
	ld l,a	; $403E
	ld h,$00	; $403F
	ld bc,($C97F)	; $4041
	call member_hurt	; $4045
	jr loc_01_4011	; $4048

loc_01_404A:
	push bc	; $404A
	push de	; $404B
	push iy	; $404C
	call party_iter_setup	; $404E

loc_01_4051:
	call member_able	; $4051
	jr c,loc_01_4067	; $4054
	call random	; $4056
	and $03	; $4059
	jr nz,loc_01_4067	; $405B
	ld (iy+$1C),$53	; $405D
	call fn_01_4247	; $4061
	call fn_01_4324	; $4064

loc_01_4067:
	add iy,de	; $4067
	inc c	; $4069
	djnz loc_01_4051	; $406A
	pop iy	; $406C
	pop de	; $406E
	pop bc	; $406F
	ret	; $4070

loc_01_4071:
	push bc	; $4071
	push de	; $4072
	push iy	; $4073
	call party_iter_setup	; $4075

loc_01_4078:
	call member_able	; $4078
	jr c,loc_01_408E	; $407B
	call random	; $407D
	and $03	; $4080
	jr nz,loc_01_408E	; $4082
	ld (iy+$1C),$50	; $4084
	call fn_01_4247	; $4088
	call fn_01_4324	; $408B

loc_01_408E:
	add iy,de	; $408E
	inc c	; $4090
	djnz loc_01_4078	; $4091
	pop iy	; $4093
	pop de	; $4095
	pop bc	; $4096
	ret	; $4097

party_iter_setup:
	ld iy,party	; $4098
	ld de,$001D	; $409C
	ld a,(party_size)	; $409F
	ld b,a	; $40A2
	ld c,$00	; $40A3
	ret	; $40A5

loc_01_40A6:
	ld a,$10	; $40A6
	call fn_00_05F1	; $40A8
	call print_inline	; $40AB
	.INCBIN "bank01.bin" SKIP $00AE READ 10	; text, 10 bytes
	ret	; $40B8

fn_01_40B9:
	ld hl,$40BF	; $40B9
	jp fn_00_0020	; $40BC

tbl_01_40BF:
	.dw fn_00_00FF
	.INCBIN "bank01.bin" SKIP $00C1 READ 8	; data, 8 bytes

fn_01_40C9:
	push af	; $40C9
	ld a,$0C	; $40CA
	call fn_00_05F1	; $40CC
	pop af	; $40CF
	ret	; $40D0

fn_01_40D1:
	push af	; $40D1
	ld a,$0D	; $40D2
	call fn_00_05F1	; $40D4
	pop af	; $40D7
	ret	; $40D8

fn_01_40D9:
	push af	; $40D9
	ld a,$1E	; $40DA
	call fn_00_05F1	; $40DC
	pop af	; $40DF
	ret	; $40E0

fn_01_40E1:
	push af	; $40E1
	ld a,$20	; $40E2
	call fn_00_05F1	; $40E4
	pop af	; $40E7
	ret	; $40E8

fn_01_40E9:
	call fn_00_0C2D	; $40E9
	push bc	; $40EC
	push de	; $40ED
	push hl	; $40EE
	ld hl,location_people	; $40EF
	ld de,$8200	; $40F2
	ld bc,$0100	; $40F5
	ldir	; $40F8
	pop hl	; $40FA
	pop de	; $40FB
	pop bc	; $40FC
	jp loc_00_0C35	; $40FD

fn_01_4100:
	call fn_00_0C2D	; $4100
	push bc	; $4103
	push de	; $4104
	push hl	; $4105
	ld hl,$8200	; $4106
	ld de,location_people	; $4109
	ld bc,$0100	; $410C
	ldir	; $410F
	pop hl	; $4111
	pop de	; $4112
	pop bc	; $4113
	jp loc_00_0C35	; $4114

fn_01_4117:
	push bc	; $4117
	push de	; $4118
	push hl	; $4119
	ld hl,$0020	; $411A
	ld de,$DA00	; $411D
	ld bc,$11BE	; $4120

loc_01_4123:
	push bc	; $4123
	rst $08	; $4124
	ex de,hl	; $4125
	ld b,$08	; $4126

loc_01_4128:
	ini	; $4128
	jr nz,loc_01_4128	; $412A
	ex de,hl	; $412C
	ld bc,$0220	; $412D
	add hl,bc	; $4130
	pop bc	; $4131
	djnz loc_01_4123	; $4132
	ld b,$05	; $4134

loc_01_4136:
	push bc	; $4136
	ld hl,$0028	; $4137
	ld de,$0020	; $413A
	ld bc,$0068	; $413D
	call fn_01_4173	; $4140
	ld hl,$01C0	; $4143
	ld de,$01C8	; $4146
	ld bc,$FF98	; $4149
	call fn_01_4173	; $414C
	ld hl,$DA00	; $414F
	ld de,$0020	; $4152
	ld bc,$11BE	; $4155

loc_01_4158:
	push bc	; $4158
	rst $10	; $4159
	ld b,$08	; $415A

loc_01_415C:
	outi	; $415C
	jr nz,loc_01_415C	; $415E
	ex de,hl	; $4160
	ld bc,$0220	; $4161
	add hl,bc	; $4164
	ex de,hl	; $4165
	pop bc	; $4166
	djnz loc_01_4158	; $4167
	pop bc	; $4169
	djnz loc_01_4136	; $416A
	call fn_00_276D	; $416C
	pop hl	; $416F
	pop de	; $4170
	pop bc	; $4171
	ret	; $4172

fn_01_4173:
	ld a,$05	; $4173

loc_01_4175:
	ex af,af'	; $4175
	push bc	; $4176
	push de	; $4177
	push hl	; $4178
	ld bc,$11BE	; $4179

loc_01_417C:
	push bc	; $417C
	push hl	; $417D
	rst $08	; $417E
	ld hl,$DB00	; $417F
	ld b,$68	; $4182

loc_01_4184:
	ini	; $4184
	jr nz,loc_01_4184	; $4186
	rst $10	; $4188
	ld hl,$DB00	; $4189
	ld b,$68	; $418C

loc_01_418E:
	outi	; $418E
	jr nz,loc_01_418E	; $4190
	pop hl	; $4192
	ld bc,$0220	; $4193
	add hl,bc	; $4196
	ex de,hl	; $4197
	add hl,bc	; $4198
	ex de,hl	; $4199
	pop bc	; $419A
	djnz loc_01_417C	; $419B
	pop hl	; $419D
	pop de	; $419E
	pop bc	; $419F
	add hl,bc	; $41A0
	ex de,hl	; $41A1
	add hl,bc	; $41A2
	ex de,hl	; $41A3
	ex af,af'	; $41A4
	dec a	; $41A5
	jr nz,loc_01_4175	; $41A6
	ret	; $41A8

fn_01_41A9:
	push bc	; $41A9
	push de	; $41AA
	push hl	; $41AB
	ld hl,$0020	; $41AC
	ld b,$0B	; $41AF

loc_01_41B1:
	push bc	; $41B1
	push hl	; $41B2
	ld bc,$10FF	; $41B3

loc_01_41B6:
	push bc	; $41B6
	call fn_01_41D2	; $41B7
	ld hl,$0220	; $41BA
	add hl,de	; $41BD
	pop bc	; $41BE
	djnz loc_01_41B6	; $41BF
	ld c,$F0	; $41C1
	call fn_01_41D2	; $41C3
	pop hl	; $41C6
	pop bc	; $41C7
	ld de,$0030	; $41C8
	add hl,de	; $41CB
	djnz loc_01_41B1	; $41CC
	pop hl	; $41CE
	pop de	; $41CF
	pop bc	; $41D0
	ret	; $41D1

fn_01_41D2:
	rst $08	; $41D2
	ex de,hl	; $41D3
	ld hl,$C1C0	; $41D4
	ld b,$30	; $41D7

loc_01_41D9:
	in a,($BE)	; $41D9
	xor c	; $41DB
	ld (hl),a	; $41DC
	inc l	; $41DD
	djnz loc_01_41D9	; $41DE
	rst $10	; $41E0
	ld hl,$C1C0	; $41E1
	ld bc,$30BE	; $41E4

loc_01_41E7:
	outi	; $41E7
	jr nz,loc_01_41E7	; $41E9
	ret	; $41EB

fn_01_41EC:
	push bc	; $41EC
	push hl	; $41ED
	ld hl,$C027	; $41EE
	ld bc,$0C3F	; $41F1

loc_01_41F4:
	ld a,(hl)	; $41F4
	cpl	; $41F5
	and c	; $41F6
	ld (hl),a	; $41F7
	inc l	; $41F8
	djnz loc_01_41F4	; $41F9
	ld a,$FF	; $41FB
	ld ($C009),a	; $41FD
	pop hl	; $4200
	pop bc	; $4201
	jp wait_vblank	; $4202

member_gain_xp:
	push de	; $4205
	push hl	; $4206
	push iy	; $4207
	call member_record	; $4209
	ld e,(iy+$04)	; $420C
	ld d,(iy+$05)	; $420F
	add hl,de	; $4212
	call cap_9999	; $4213
	ld (iy+$04),l	; $4216
	ld (iy+$05),h	; $4219
	pop iy	; $421C
	pop hl	; $421E
	pop de	; $421F
	ret	; $4220

member_heal:
	push de	; $4221
	push hl	; $4222
	push iy	; $4223
	call member_record	; $4225
	ld e,(iy+$00)	; $4228
	ld d,(iy+$01)	; $422B
	add hl,de	; $422E
	ex de,hl	; $422F
	ld l,(iy+$02)	; $4230
	ld h,(iy+$03)	; $4233
	sbc hl,de	; $4236
	jr nc,loc_01_423C	; $4238
	add hl,de	; $423A
	ex de,hl	; $423B

loc_01_423C:
	ld (iy+$00),e	; $423C
	ld (iy+$01),d	; $423F
	pop iy	; $4242
	pop hl	; $4244
	pop de	; $4245
	ret	; $4246

fn_01_4247:
	call fn_01_49A6	; $4247
	ld a,$13	; $424A
	call fn_00_05F1	; $424C
	call wait_vblank	; $424F
	jp fn_01_4999	; $4252

karma_gain:
	push af	; $4255
	push bc	; $4256
	push hl	; $4257
	ld hl,karma	; $4258
	ld b,$00	; $425B
	add hl,bc	; $425D
	ld b,a	; $425E
	ld a,(hl)	; $425F
	or a	; $4260
	jr z,loc_01_426B	; $4261
	add a,b	; $4263
	cp $63	; $4264
	jr c,loc_01_426A	; $4266
	ld a,$63	; $4268

loc_01_426A:
	ld (hl),a	; $426A

loc_01_426B:
	pop hl	; $426B
	pop bc	; $426C
	pop af	; $426D
	ret	; $426E

karma_lose:
	push af	; $426F
	push bc	; $4270
	push hl	; $4271
	ld hl,karma	; $4272
	ld b,$00	; $4275
	add hl,bc	; $4277
	ld b,a	; $4278
	ld a,(hl)	; $4279
	or a	; $427A
	jr nz,loc_01_428F	; $427B
	ld (hl),$63	; $427D
	add a,$1B	; $427F
	call fn_00_05F1	; $4281
	call fn_00_342B	; $4284
	ld hl,$429B	; $4287
	call print_text_hl	; $428A
	jr loc_01_4297	; $428D

loc_01_428F:
	sub b	; $428F
	jr z,loc_01_4294	; $4290
	jr nc,loc_01_4296	; $4292

loc_01_4294:
	ld a,$01	; $4294

loc_01_4296:
	ld (hl),a	; $4296

loc_01_4297:
	pop hl	; $4297
	pop bc	; $4298
	pop af	; $4299
	ret	; $429A
	.INCBIN "bank01.bin" SKIP $029B READ 28	; data, 28 bytes

fn_01_42B7:
	push bc	; $42B7
	push de	; $42B8
	push iy	; $42B9
	ld iy,location_people	; $42BB
	ld de,$0008	; $42BF
	ld c,b	; $42C2

loc_01_42C3:
	ld a,(iy+$03)	; $42C3
	or a	; $42C6
	jr z,loc_01_42D5	; $42C7
	ld a,(iy+$01)	; $42C9
	cp l	; $42CC
	jr nz,loc_01_42D5	; $42CD
	ld a,(iy+$02)	; $42CF
	cp h	; $42D2
	jr z,loc_01_42DC	; $42D3

loc_01_42D5:
	add iy,de	; $42D5
	djnz loc_01_42C3	; $42D7
	scf	; $42D9
	jr loc_01_42DE	; $42DA

loc_01_42DC:
	ld a,c	; $42DC
	sub b	; $42DD

loc_01_42DE:
	pop iy	; $42DE
	pop de	; $42E0
	pop bc	; $42E1
	ret	; $42E2

fn_01_42E3:
	push bc	; $42E3
	ld a,(location)	; $42E4
	or a	; $42E7
	ld b,$08	; $42E8
	jr z,loc_01_42EE	; $42EA
	ld b,$20	; $42EC

loc_01_42EE:
	call fn_01_42B7	; $42EE
	pop bc	; $42F1
	ret	; $42F2

fn_01_42F3:
	push iy	; $42F3
	call member_record	; $42F5
	ld a,(iy+$1C)	; $42F8
	cp $53	; $42FB
	ld a,$38	; $42FD
	jr z,loc_01_4307	; $42FF
	ld a,(iy+$1B)	; $4301
	add a,a	; $4304
	add a,$20	; $4305

loc_01_4307:
	pop iy	; $4307
	ret	; $4309

fn_01_430A:
	push bc	; $430A
	push hl	; $430B
	cp $80	; $430C
	jr c,loc_01_431A	; $430E
	ld hl,$431E	; $4310
	ld bc,$0006	; $4313
	cpir	; $4316
	jr z,loc_01_431B	; $4318

loc_01_431A:
	ccf	; $431A

loc_01_431B:
	pop hl	; $431B
	pop bc	; $431C
	ret	; $431D
	.INCBIN "bank01.bin" SKIP $031E READ 6	; data, 6 bytes

fn_01_4324:
	push af	; $4324
	push bc	; $4325
	push de	; $4326
	push hl	; $4327
	push iy	; $4328
	ld hl,(text_col)	; $432A
	push hl	; $432D
	call fn_00_342B	; $432E
	call menu_window_clear	; $4331
	ld iy,party	; $4334
	ld de,$CA66	; $4338
	ld a,(party_size)	; $433B
	ld b,a	; $433E

loc_01_433F:
	push bc	; $433F
	push iy	; $4340
	pop hl	; $4342
	ld bc,$000D	; $4343
	add hl,bc	; $4346
	push de	; $4347
	call fn_00_2D6C	; $4348
	pop de	; $434B
	ld hl,$0048	; $434C
	add hl,de	; $434F
	ex de,hl	; $4350
	ld l,(iy+$00)	; $4351
	ld h,(iy+$01)	; $4354
	call fn_00_2D81	; $4357
	inc e	; $435A
	inc e	; $435B
	ld a,(iy+$1C)	; $435C
	add a,$40	; $435F
	ld (de),a	; $4361
	inc e	; $4362
	ld a,$01	; $4363
	ld (de),a	; $4365
	inc e	; $4366
	inc e	; $4367
	inc e	; $4368
	ld l,(iy+$09)	; $4369
	ld h,$00	; $436C
	call fn_00_2D87	; $436E
	ld hl,$0028	; $4371
	add hl,de	; $4374
	ex de,hl	; $4375
	ld bc,$001D	; $4376
	add iy,bc	; $4379
	pop bc	; $437B
	djnz loc_01_433F	; $437C
	call menu_window_show	; $437E
	ld hl,$0014	; $4381
	ld (text_col),hl	; $4384
	ld a,$46	; $4387
	call print_char	; $4389
	ld a,$3A	; $438C
	call print_char	; $438E
	ld hl,(food)	; $4391
	ld bc,$0430	; $4394
	call print_number	; $4397
	ld hl,$000A	; $439A
	ld (text_col),hl	; $439D
	ld a,($C959)	; $43A0
	cp $14	; $43A3
	jr nc,loc_01_43B8	; $43A5
	ld hl,$43D2	; $43A7
	call print_text_hl	; $43AA
	ld hl,($C961)	; $43AD
	ld bc,$0230	; $43B0
	call print_number	; $43B3
	jr loc_01_43C7	; $43B6

loc_01_43B8:
	ld hl,$43D7	; $43B8
	call print_text_hl	; $43BB
	ld hl,(gold)	; $43BE
	ld bc,$0430	; $43C1
	call print_number	; $43C4

loc_01_43C7:
	pop hl	; $43C7
	ld (text_col),hl	; $43C8
	pop iy	; $43CB
	pop hl	; $43CD
	pop de	; $43CE
	pop bc	; $43CF
	pop af	; $43D0
	ret	; $43D1
	.INCBIN "bank01.bin" SKIP $03D2 READ 8	; data, 8 bytes

show_gold:
	push bc	; $43DA
	push hl	; $43DB
	ld hl,(text_col)	; $43DC
	push hl	; $43DF
	ld hl,$000C	; $43E0
	ld (text_col),hl	; $43E3
	ld hl,(gold)	; $43E6
	ld bc,$0430	; $43E9
	call print_number	; $43EC
	pop hl	; $43EF
	ld (text_col),hl	; $43F0
	pop hl	; $43F3
	pop bc	; $43F4
	ret	; $43F5

show_food:
	push bc	; $43F6
	push hl	; $43F7
	ld hl,(text_col)	; $43F8
	push hl	; $43FB
	ld hl,$0016	; $43FC
	ld (text_col),hl	; $43FF
	ld hl,(food)	; $4402
	ld bc,$0430	; $4405
	call print_number	; $4408
	pop hl	; $440B
	ld (text_col),hl	; $440C
	pop hl	; $440F
	pop bc	; $4410
	ret	; $4411

receive_gold:
	push de	; $4412
	push hl	; $4413
	ld de,(gold)	; $4414
	add hl,de	; $4418
	call cap_9999	; $4419
	ld (gold),hl	; $441C
	pop hl	; $441F
	pop de	; $4420
	jr show_gold	; $4421

pay:
	push de	; $4423
	push hl	; $4424
	ex de,hl	; $4425
	ld hl,(gold)	; $4426
	or a	; $4429
	sbc hl,de	; $442A
	jr c,loc_01_4431	; $442C
	ld (gold),hl	; $442E

loc_01_4431:
	pop hl	; $4431
	pop de	; $4432
	ret c	; $4433
	call show_gold	; $4434
	or a	; $4437
	ret	; $4438

can_afford:
	push de	; $4439
	push hl	; $443A
	ex de,hl	; $443B
	ld hl,(gold)	; $443C
	or a	; $443F
	sbc hl,de	; $4440
	pop hl	; $4442
	pop de	; $4443
	ret	; $4444

member_can_act:
	push hl	; $4445
	push iy	; $4446
	ld a,(party_size)	; $4448
	dec a	; $444B
	cp c	; $444C
	jr c,loc_01_4462	; $444D
	call member_record	; $444F
	ld a,(iy+$1C)	; $4452
	cp $47	; $4455
	jr z,loc_01_4462	; $4457
	cp $53	; $4459
	jr z,loc_01_4462	; $445B
	cp $50	; $445D
	jr z,loc_01_4462	; $445F
	scf	; $4461

loc_01_4462:
	pop iy	; $4462
	pop hl	; $4464
	ret	; $4465

member_able:
	push hl	; $4466
	push iy	; $4467
	ld a,(party_size)	; $4469
	dec a	; $446C
	cp c	; $446D
	jr c,loc_01_447F	; $446E
	call member_record	; $4470
	ld a,(iy+$1C)	; $4473
	cp $47	; $4476
	jr z,loc_01_447F	; $4478
	cp $50	; $447A
	jr z,loc_01_447F	; $447C
	scf	; $447E

loc_01_447F:
	pop iy	; $447F
	pop hl	; $4481
	ret	; $4482

loc_01_4483:
	ld a,(location)	; $4483
	or a	; $4486
	call z,fn_01_40E9	; $4487
	ld iy,party	; $448A
	ld de,$001D	; $448E
	ld a,(party_size)	; $4491
	ld b,a	; $4494

loc_01_4495:
	ld (iy+$1C),$44	; $4495
	ld (iy+$00),$00	; $4499
	ld (iy+$01),$00	; $449D
	add iy,de	; $44A1
	djnz loc_01_4495	; $44A3
	call fn_01_4324	; $44A5
	call wait_vblank	; $44A8
	call fn_00_0605	; $44AB
	call fn_01_4EAC	; $44AE
	call fn_00_05E3	; $44B1
	call wait_vblank	; $44B4
	call fn_01_54CB	; $44B7
	call fn_01_54CB	; $44BA
	ld hl,$457C	; $44BD
	call fn_01_4574	; $44C0
	ld hl,$458C	; $44C3
	call fn_01_4574	; $44C6
	ld hl,$459A	; $44C9
	call fn_01_4574	; $44CC
	ld hl,$45AA	; $44CF
	call fn_01_4574	; $44D2
	ld hl,$45B9	; $44D5
	call fn_01_4574	; $44D8
	ld hl,$45C8	; $44DB
	call print_text_hl	; $44DE
	call print_avatar_name	; $44E1
	call fn_01_4577	; $44E4
	ld hl,$45D9	; $44E7
	call fn_01_4574	; $44EA
	ld hl,$8A1F	; $44ED
	ld ($C959),hl	; $44F0
	ld hl,$0813	; $44F3
	ld (party_x),hl	; $44F6
	ld (view_x),hl	; $44F9
	ld hl,$6C56	; $44FC
	ld (world_return_x),hl	; $44FF
	xor a	; $4502
	ld ($C976),a	; $4503
	ld ($C977),a	; $4506
	ld ($C9A4),a	; $4509
	ld ($C982),a	; $450C
	ld a,$FF	; $450F
	ld ($C98F),a	; $4511
	ld hl,$45EB	; $4514
	call print_text_hl	; $4517
	call print_newline	; $451A
	ld iy,party	; $451D
	ld de,$001D	; $4521
	ld a,(party_size)	; $4524
	ld b,a	; $4527

loc_01_4528:
	ld (iy+$1C),$47	; $4528
	ld a,(iy+$02)	; $452C
	ld (iy+$00),a	; $452F
	ld a,(iy+$03)	; $4532
	ld (iy+$01),a	; $4535
	add iy,de	; $4538
	djnz loc_01_4528	; $453A
	ld hl,weapons_owned	; $453C
	ld bc,$1000	; $453F
	call fn_00_333B	; $4542
	ld hl,$C917	; $4545

fn_01_4548:
	ld bc,$0800	; $4548
	call fn_00_333B	; $454B
	ld a,$63	; $454E
	ld (food_fraction),a	; $4550
	ld hl,$00C8	; $4553

fn_01_4556:
	ld (food),hl	; $4556
	ld (gold),hl	; $4559
	call fn_01_4324	; $455C
	ld a,$02	; $455F
	ld (location),a	; $4561
	ld a,$12	; $4564
	ld (game_mode),a	; $4566
	ld a,$FF	; $4569
	ld (map_mode),a	; $456B
	call load_location	; $456E
	jp main_loop	; $4571

fn_01_4574:
	call print_text_hl	; $4574

fn_01_4577:
	ld b,$14	; $4577
	jp fn_00_042D	; $4579
	.INCBIN "bank01.bin" SKIP $057C READ 225	; data, 225 bytes

fn_01_465D:
	push bc	; $465D
	ld a,(party_size)	; $465E
	ld b,a	; $4661
	ld c,$00	; $4662

loc_01_4664:
	call member_can_act	; $4664
	jr nc,loc_01_466F	; $4667
	inc c	; $4669
	djnz loc_01_4664	; $466A
	jp loc_01_4483	; $466C

loc_01_466F:
	call member_able	; $466F
	jr nc,loc_01_4677	; $4672
	inc c	; $4674
	djnz loc_01_466F	; $4675

loc_01_4677:
	pop bc	; $4677
	ret	; $4678

member_hurt:
	call member_take_damage	; $4679
	jp fn_01_4324	; $467C

member_take_damage:
	push de	; $467F
	push hl	; $4680
	push iy	; $4681
	call member_record	; $4683
	ex de,hl	; $4686
	ld l,(iy+$00)	; $4687
	ld h,(iy+$01)	; $468A
	or a	; $468D
	sbc hl,de	; $468E
	ld (iy+$00),l	; $4690
	ld (iy+$01),h	; $4693
	jr nc,loc_01_46B0	; $4696
	ld (iy+$00),$00	; $4698
	ld (iy+$01),$00	; $469C
	ld (iy+$1C),$44	; $46A0
	call member_object	; $46A4
	ld (iy+$02),$38	; $46A7
	ld (iy+$03),$38	; $46AB
	scf	; $46AF

loc_01_46B0:
	pop iy	; $46B0
	pop hl	; $46B2
	pop de	; $46B3
	ret	; $46B4

fn_01_46B5:
	call fn_01_40D1	; $46B5
	call print_inline	; $46B8
	.INCBIN "bank01.bin" SKIP $06BB READ 6	; text, 6 bytes
	ret	; $46C1

fn_01_46C2:
	call fn_01_40D1	; $46C2
	call print_inline	; $46C5
	.INCBIN "bank01.bin" SKIP $06C8 READ 7	; text, 7 bytes
	ret	; $46CF

fn_01_46D0:
	call fn_01_40D1	; $46D0
	call print_inline	; $46D3
	.INCBIN "bank01.bin" SKIP $06D6 READ 8	; text, 8 bytes
	ret	; $46DE

fn_01_46DF:
	call fn_01_40D1	; $46DF
	call print_inline	; $46E2
	.INCBIN "bank01.bin" SKIP $06E5 READ 11	; text, 11 bytes
	ret	; $46F0

fn_01_46F1:
	call fn_01_40D1	; $46F1
	call print_inline	; $46F4
	.INCBIN "bank01.bin" SKIP $06F7 READ 11	; text, 11 bytes
	ret	; $4702

fn_01_4703:
	call fn_01_40D1	; $4703
	call print_inline	; $4706
	.INCBIN "bank01.bin" SKIP $0709 READ 15	; text, 15 bytes
	ret	; $4718

fn_01_4719:
	call print_inline	; $4719
	.INCBIN "bank01.bin" SKIP $071C READ 16	; text, 16 bytes
	ret	; $472C

fn_01_472D:
	call fn_01_40D1	; $472D
	call print_inline	; $4730
	.INCBIN "bank01.bin" SKIP $0733 READ 11	; text, 11 bytes
	ret	; $473E

fn_01_473F:
	call fn_01_40D1	; $473F
	call print_inline	; $4742
	.INCBIN "bank01.bin" SKIP $0745 READ 9	; data, 9 bytes

fn_01_474E:
	ld a,c	; $474E
	ld hl,$0001	; $474F
	ret	; $4752

fn_01_4753:
	push bc	; $4753
	push hl	; $4754
	ld b,a	; $4755
	ld hl,food_fraction	; $4756
	ld a,(hl)	; $4759
	sub b	; $475A
	ld (hl),a	; $475B
	jr nc,loc_01_4773	; $475C
	add a,$64	; $475E
	ld (hl),a	; $4760
	scf	; $4761
	ld bc,$0200	; $4762

loc_01_4765:
	inc hl	; $4765
	ld a,(hl)	; $4766
	sbc a,c	; $4767
	ld (hl),a	; $4768
	djnz loc_01_4765	; $4769
	jr nc,loc_01_4773	; $476B
	ld b,$03	; $476D

loc_01_476F:
	ld (hl),c	; $476F
	dec hl	; $4770
	djnz loc_01_476F	; $4771

loc_01_4773:
	pop hl	; $4773
	pop bc	; $4774
	ret	; $4775

fn_01_4776:
	push bc	; $4776
	push de	; $4777
	push hl	; $4778
	push iy	; $4779
	ld iy,party	; $477B
	ld de,$001D	; $477F
	ld a,(party_size)	; $4782
	ld b,a	; $4785

loc_01_4786:
	ld a,(iy+$1C)	; $4786
	cp $44	; $4789
	jr z,loc_01_47A9	; $478B
	ld c,(iy+$08)	; $478D
	ld a,(iy+$1B)	; $4790
	ld hl,$47B3	; $4793
	rst $18	; $4796
	cp $64	; $4797
	jr c,loc_01_479D	; $4799
	ld a,$63	; $479B

loc_01_479D:
	ld c,a	; $479D
	ld a,(iy+$09)	; $479E
	inc a	; $47A1
	cp c	; $47A2
	jr c,loc_01_47A6	; $47A3
	ld a,c	; $47A5

loc_01_47A6:
	ld (iy+$09),a	; $47A6

loc_01_47A9:
	add iy,de	; $47A9
	djnz loc_01_4786	; $47AB
	pop iy	; $47AD
	pop hl	; $47AF
	pop de	; $47B0
	pop bc	; $47B1
	ret	; $47B2

tbl_01_47B3:
	.dw fn_01_47D0
	.dw fn_01_47C9
	.dw fn_01_47C3
	.dw fn_01_47CB
	.dw fn_01_47C5
	.dw fn_01_47C9
	.dw fn_01_47C9
	.dw fn_01_47C3

fn_01_47C3:
	xor a	; $47C3
	ret	; $47C4

fn_01_47C5:
	ld a,c	; $47C5
	srl a	; $47C6
	ret	; $47C8

fn_01_47C9:
	ld a,c	; $47C9
	ret	; $47CA

fn_01_47CB:
	ld a,c	; $47CB
	srl a	; $47CC
	add a,c	; $47CE
	ret	; $47CF

fn_01_47D0:
	ld a,c	; $47D0
	add a,a	; $47D1
	ret	; $47D2

fn_01_47D3:
	cp $80	; $47D3
	jr c,loc_01_47ED	; $47D5
	cp $90	; $47D7
	jr c,loc_01_47E5	; $47D9
	sub $80	; $47DB
	srl a	; $47DD
	srl a	; $47DF
	add a,$05	; $47E1
	jr loc_01_480F	; $47E3

loc_01_47E5:
	sub $80	; $47E5
	srl a	; $47E7
	add a,$01	; $47E9
	jr loc_01_480F	; $47EB

loc_01_47ED:
	cp $38	; $47ED
	jr nz,loc_01_47F5	; $47EF
	ld a,$9F	; $47F1
	jr loc_01_480F	; $47F3

loc_01_47F5:
	cp $20	; $47F5
	jr c,loc_01_480D	; $47F7
	cp $60	; $47F9
	jr nc,loc_01_480D	; $47FB
	cp $30	; $47FD
	jr c,loc_01_4805	; $47FF
	cp $50	; $4801
	jr c,loc_01_480D	; $4803

loc_01_4805:
	and $1F	; $4805
	srl a	; $4807
	add a,$4D	; $4809
	jr loc_01_480F	; $480B

loc_01_480D:
	ld a,$14	; $480D

loc_01_480F:
	jp item_name	; $480F

fn_01_4812:
	push bc	; $4812
	push de	; $4813
	push hl	; $4814
	push iy	; $4815
	call fn_01_4896	; $4817
	ld a,$13	; $481A
	call fn_00_05F1	; $481C
	call fn_01_4117	; $481F
	call fn_01_4887	; $4822
	ld a,($C959)	; $4825
	cp $14	; $4828
	jr c,loc_01_485B	; $482A
	ld iy,party	; $482C
	ld de,$001D	; $4830
	ld a,(party_size)	; $4833
	ld b,a	; $4836
	ld c,$00	; $4837

loc_01_4839:
	ld a,(iy+$1C)	; $4839
	cp $44	; $483C
	jr z,fn_01_4854	; $483E
	call random	; $4840
	and $01	; $4843
	jr z,fn_01_4854	; $4845
	ld a,$0F	; $4847
	call random_below	; $4849
	add a,$0A	; $484C
	ld l,a	; $484E
	ld h,$00	; $484F
	call member_hurt	; $4851

fn_01_4854:
	add iy,de	; $4854
	inc c	; $4856
	djnz loc_01_4839	; $4857
	jr loc_01_487E	; $4859

loc_01_485B:
	ld hl,($C961)	; $485B
	ld de,$000A	; $485E
	or a	; $4861
	sbc hl,de	; $4862
	ld ($C961),hl	; $4864
	jr nc,loc_01_487E	; $4867
	ld hl,$0000	; $4869
	ld ($C961),hl	; $486C
	call fn_01_4896	; $486F
	call fn_01_4324	; $4872
	ld hl,$48A5	; $4875
	call print_text_hl	; $4878
	jp loc_01_4483	; $487B

loc_01_487E:
	call fn_01_4324	; $487E
	pop iy	; $4881
	pop hl	; $4883
	pop de	; $4884
	pop bc	; $4885
	ret	; $4886

fn_01_4887:
	ld a,(party_size)	; $4887
	ld b,a	; $488A
	ld c,$00	; $488B

loc_01_488D:
	call fn_01_4999	; $488D
	inc c	; $4890
	djnz loc_01_488D	; $4891
	jp wait_vblank	; $4893

fn_01_4896:
	ld a,(party_size)	; $4896
	ld b,a	; $4899
	ld c,$00	; $489A

loc_01_489C:
	call fn_01_49A6	; $489C
	inc c	; $489F
	djnz loc_01_489C	; $48A0
	jp wait_vblank	; $48A2
	.INCBIN "bank01.bin" SKIP $08A5 READ 17	; data, 17 bytes

member_record:
	push bc	; $48B6
	push de	; $48B7
	push hl	; $48B8
	ld hl,$C734	; $48B9
	ld de,$001D	; $48BC

loc_01_48BF:
	ld b,c	; $48BF
	inc b	; $48C0

loc_01_48C1:
	add hl,de	; $48C1
	djnz loc_01_48C1	; $48C2
	push hl	; $48C4
	pop iy	; $48C5
	pop hl	; $48C7
	pop de	; $48C8
	pop bc	; $48C9
	ret	; $48CA

fn_01_48CB:
	push bc	; $48CB
	ld bc,($C97F)	; $48CC
	call member_record	; $48D0
	pop bc	; $48D3
	ret	; $48D4

member_object:
	push bc	; $48D5
	push de	; $48D6
	push hl	; $48D7
	ld hl,$C8B4	; $48D8
	ld de,$0005	; $48DB
	jp loc_01_48BF	; $48DE

fn_01_48E1:
	push bc	; $48E1
	ld bc,($C97F)	; $48E2
	call member_object	; $48E6
	pop bc	; $48E9
	ret	; $48EA

fn_01_48EB:
	push bc	; $48EB
	push de	; $48EC
	push hl	; $48ED
	ld hl,$C831	; $48EE
	ld de,$0008	; $48F1
	jp loc_01_48BF	; $48F4

fn_01_48F7:
	push bc	; $48F7
	add a,a	; $48F8
	add a,a	; $48F9
	add a,a	; $48FA
	ld c,a	; $48FB
	ld b,$00	; $48FC
	ld iy,location_people	; $48FE
	add iy,bc	; $4902
	pop bc	; $4904
	ret	; $4905

fn_01_4906:
	push bc	; $4906
	push de	; $4907
	ld c,a	; $4908
	ld a,(location)	; $4909
	or a	; $490C
	jr z,loc_01_4917	; $490D
	ld iy,location_people	; $490F
	ld b,$20	; $4913
	jr loc_01_491D	; $4915

loc_01_4917:
	ld iy,$C691	; $4917
	ld b,$18	; $491B

loc_01_491D:
	ld de,$0008	; $491D
	ld a,c	; $4920

loc_01_4921:
	cp (iy+$03)	; $4921
	jr z,loc_01_492D	; $4924
	add iy,de	; $4926
	djnz loc_01_4921	; $4928
	scf	; $492A
	jr loc_01_4930	; $492B

loc_01_492D:
	ld a,$20	; $492D
	sub b	; $492F

loc_01_4930:
	pop de	; $4930
	pop bc	; $4931
	ret	; $4932

fn_01_4933:
	push bc	; $4933
	push de	; $4934
	push hl	; $4935
	ld hl,(party_x)	; $4936
	ld a,(location)	; $4939
	or a	; $493C
	jr z,fn_01_4947	; $493D
	ld iy,location_people	; $493F
	ld c,$20	; $4943
	jr loc_01_494D	; $4945

fn_01_4947:
	ld iy,$C691	; $4947
	ld c,$18	; $494B

loc_01_494D:
	ld de,$0008	; $494D
	ld b,c	; $4950
	xor a	; $4951

fn_01_4952:
	cp (iy+$03)	; $4952
	jr z,loc_01_4995	; $4955
	add iy,de	; $4957
	djnz fn_01_4952	; $4959
	ld de,$FFF8	; $495B
	ld b,c	; $495E
	ld a,$3C	; $495F

loc_01_4961:
	add iy,de	; $4961
	cp (iy+$03)	; $4963
	jr z,loc_01_4995	; $4966
	djnz loc_01_4961	; $4968
	ld de,$0008	; $496A
	ld b,c	; $496D

loc_01_496E:
	ld a,(iy+$03)	; $496E
	cp $10	; $4971
	jr c,loc_01_4985	; $4973
	cp $16	; $4975
	jr nc,loc_01_4985	; $4977
	ld a,(iy+$01)	; $4979
	cp l	; $497C
	jr nz,loc_01_4995	; $497D
	ld a,(iy+$02)	; $497F
	cp h	; $4982
	jr nz,loc_01_4995	; $4983

loc_01_4985:
	add iy,de	; $4985
	djnz loc_01_496E	; $4987
	ld a,$20	; $4989
	sub c	; $498B
	ld b,a	; $498C
	ld a,c	; $498D
	call random_below	; $498E
	add a,b	; $4991
	call fn_01_48F7	; $4992

loc_01_4995:
	pop hl	; $4995
	pop de	; $4996
	pop bc	; $4997
	ret	; $4998

fn_01_4999:
	push bc	; $4999
	push de	; $499A
	push hl	; $499B
	call fn_01_49B3	; $499C
	call highlight_off	; $499F
	pop hl	; $49A2
	pop de	; $49A3
	pop bc	; $49A4
	ret	; $49A5

fn_01_49A6:
	push bc	; $49A6
	push de	; $49A7
	push hl	; $49A8
	call fn_01_49B3	; $49A9
	call highlight_on	; $49AC
	pop hl	; $49AF
	pop de	; $49B0
	pop bc	; $49B1
	ret	; $49B2

fn_01_49B3:
	ld a,c	; $49B3
	add a,a	; $49B4
	inc a	; $49B5
	ld d,a	; $49B6
	ld e,$12	; $49B7
	ld bc,$020E	; $49B9
	ret	; $49BC

member_name:
	push af	; $49BD
	push bc	; $49BE
	push hl	; $49BF
	push iy	; $49C0
	call member_record	; $49C2
	push iy	; $49C5
	pop hl	; $49C7
	ld bc,$000D	; $49C8
	add hl,bc	; $49CB
	call print_text_hl	; $49CC
	pop iy	; $49CF
	pop hl	; $49D1
	pop bc	; $49D2
	pop af	; $49D3
	ret	; $49D4

cursor_on:
	push bc	; $49D5
	push de	; $49D6
	call fn_01_49EB	; $49D7
	call highlight_on	; $49DA
	pop de	; $49DD
	pop bc	; $49DE
	ret	; $49DF

cursor_off:
	push bc	; $49E0
	push de	; $49E1
	call fn_01_49EB	; $49E2
	call highlight_off	; $49E5
	pop de	; $49E8
	pop bc	; $49E9
	ret	; $49EA

fn_01_49EB:
	add a,$01	; $49EB
	ld d,a	; $49ED
	ld e,$12	; $49EE
	ld bc,$010E	; $49F0
	ret	; $49F3

menu_step:
	ld hl,$4A0C	; $49F4
	jr loc_01_49FC	; $49F7

menu_step_wrap:
	ld hl,$4A12	; $49F9

loc_01_49FC:
	ld a,(joy_pressed)	; $49FC
	and $7F	; $49FF
	call switch_on_a	; $4A01

loc_01_4A04:
	call fn_01_4A5C	; $4A04
	call cursor_on	; $4A07
	xor a	; $4A0A
	ret	; $4A0B
	.INCBIN "bank01.bin" SKIP $0A0C READ 25	; data, 25 bytes
	call fn_01_4A5C	; $4A25
	call cursor_off	; $4A28
	ld a,$FF	; $4A2B

loc_01_4A2D:
	add a,(ix+$1A)	; $4A2D
	add a,c	; $4A30
	ld l,a	; $4A31
	ld h,c	; $4A32
	call divide_8	; $4A33
	ld (ix+$1A),h	; $4A36
	call fn_01_40D9	; $4A39
	jr loc_01_4A04	; $4A3C
	.INCBIN "bank01.bin" SKIP $0A3E READ 7	; data, 7 bytes
	call fn_01_4A5C	; $4A45
	call cursor_off	; $4A48
	ld a,$01	; $4A4B
	jr loc_01_4A2D	; $4A4D
	call fn_01_40D1	; $4A4F
	ld a,$FF	; $4A52
	add a,a	; $4A54
	ret	; $4A55
	call fn_01_40C9	; $4A56
	xor a	; $4A59
	dec a	; $4A5A
	ret	; $4A5B

fn_01_4A5C:
	ld h,(ix+$1A)	; $4A5C
	ld l,e	; $4A5F
	call multiply_8x8	; $4A60
	ld a,l	; $4A63
	add a,d	; $4A64
	ret	; $4A65

number_entry_start:
	ld (ix+$0E),a	; $4A66
	ld (ix+$0F),$00	; $4A69
	ld e,a	; $4A6D
	ld d,$00	; $4A6E
	ld hl,$4A9E	; $4A70
	or a	; $4A73
	sbc hl,de	; $4A74
	call print_text_hl	; $4A76
	ld b,(ix+$0E)	; $4A79

loc_01_4A7C:
	ld a,$02	; $4A7C
	call print_char	; $4A7E
	djnz loc_01_4A7C	; $4A81
	ld hl,(text_col)	; $4A83
	ld (ix+$10),l	; $4A86
	ld (ix+$11),h	; $4A89
	ld a,$30	; $4A8C
	call print_char	; $4A8E
	ld hl,$0030	; $4A91
	ld ($C065),hl	; $4A94
	ret	; $4A97
	.INCBIN "bank01.bin" SKIP $0A98 READ 7	; data, 7 bytes

number_entry_step:
	ld a,(joy_pressed)	; $4A9F
	and $7F	; $4AA2
	ld hl,$4ABB	; $4AA4
	call switch_on_a	; $4AA7

loc_01_4AAA:
	ld l,(ix+$10)	; $4AAA
	ld h,(ix+$11)	; $4AAD
	ld (text_col),hl	; $4AB0
	ld hl,$C065	; $4AB3
	call print_text_hl	; $4AB6
	scf	; $4AB9
	ret	; $4ABA

tbl_01_4ABB:
	.db $01
	.dw loc_01_4ACE
	.db $02
	.dw loc_01_4AD2
	.db $04
	.dw loc_01_4AEF
	.db $08
	.dw loc_01_4B09
	.db $10
	.dw loc_01_4B2A
	.db $20
	.dw loc_01_4B46
	.db $FF

loc_01_4ACE:
	ld b,$01	; $4ACE
	jr loc_01_4AD4	; $4AD0

loc_01_4AD2:
	ld b,$FF	; $4AD2

loc_01_4AD4:
	ld a,(ix+$0F)	; $4AD4
	ld hl,$C065	; $4AD7
	rst $30	; $4ADA
	ld a,(hl)	; $4ADB
	add a,b	; $4ADC
	cp $2F	; $4ADD
	jr nz,loc_01_4AE3	; $4ADF
	ld a,$39	; $4AE1

loc_01_4AE3:
	cp $3A	; $4AE3
	jr nz,loc_01_4AE9	; $4AE5
	ld a,$30	; $4AE7

loc_01_4AE9:
	ld (hl),a	; $4AE9
	call fn_01_40D9	; $4AEA
	jr loc_01_4AAA	; $4AED

loc_01_4AEF:
	ld a,(ix+$0F)	; $4AEF
	dec a	; $4AF2
	jp m,loc_01_4AAA	; $4AF3
	ld (ix+$0F),a	; $4AF6
	ld hl,$C066	; $4AF9
	rst $30	; $4AFC
	ld (hl),$00	; $4AFD
	ld a,$02	; $4AFF
	call print_char	; $4B01
	call fn_01_40D9	; $4B04
	jr loc_01_4AAA	; $4B07

loc_01_4B09:
	ld a,($C065)	; $4B09
	cp $30	; $4B0C
	jr z,loc_01_4AAA	; $4B0E
	ld a,(ix+$0F)	; $4B10
	inc a	; $4B13
	cp (ix+$0E)	; $4B14
	jr nc,loc_01_4AAA	; $4B17
	ld (ix+$0F),a	; $4B19
	ld hl,$C065	; $4B1C
	rst $30	; $4B1F
	ld (hl),$30	; $4B20
	inc hl	; $4B22
	ld (hl),$00	; $4B23
	call fn_01_40D9	; $4B25
	jr loc_01_4AAA	; $4B28

loc_01_4B2A:
	call fn_01_40D1	; $4B2A
	ld a,(ix+$0F)	; $4B2D
	or a	; $4B30
	jr z,loc_01_4B3D	; $4B31
	ld a,$02	; $4B33
	call print_char	; $4B35
	dec (ix+$0F)	; $4B38
	jr loc_01_4B2A	; $4B3B

loc_01_4B3D:
	ld hl,$0030	; $4B3D
	ld ($C065),hl	; $4B40
	jp loc_01_4AAA	; $4B43

loc_01_4B46:
	call fn_01_40C9	; $4B46
	ld hl,$0000	; $4B49
	ld de,$C065	; $4B4C

loc_01_4B4F:
	ld a,(de)	; $4B4F
	inc de	; $4B50
	sub $30	; $4B51
	ccf	; $4B53
	ret nc	; $4B54
	add hl,hl	; $4B55
	ld c,l	; $4B56
	ld b,h	; $4B57
	add hl,hl	; $4B58
	add hl,hl	; $4B59
	add hl,bc	; $4B5A
	rst $30	; $4B5B
	jr loc_01_4B4F	; $4B5C

yes_no_start:
	ld hl,$4BBF	; $4B5E
	call menu_draw	; $4B61
	ld (ix+$07),$00	; $4B64
	ret	; $4B68

yes_no_step:
	ld a,(joy_pressed)	; $4B69
	ld hl,$4B74	; $4B6C
	call switch_on_a	; $4B6F
	jr loc_01_4B8D	; $4B72

tbl_01_4B74:
	.db $01
	.dw loc_01_4B81
	.db $02
	.dw loc_01_4B81
	.db $10
	.dw loc_01_4B95
	.db $20
	.dw loc_01_4B9E
	.db $FF

loc_01_4B81:
	call fn_01_40D9	; $4B81
	call fn_01_4BB5	; $4B84
	call cursor_off	; $4B87
	inc (ix+$07)	; $4B8A

loc_01_4B8D:
	call fn_01_4BB5	; $4B8D
	call cursor_on	; $4B90
	scf	; $4B93
	ret	; $4B94

loc_01_4B95:
	call fn_01_40D1	; $4B95
	ld (ix+$07),$01	; $4B98
	jr loc_01_4BA1	; $4B9C

loc_01_4B9E:
	call fn_01_40C9	; $4B9E

loc_01_4BA1:
	ld hl,$4BC1	; $4BA1
	bit 0,(ix+$07)	; $4BA4
	jr z,loc_01_4BAD	; $4BA8
	ld hl,$4BC7	; $4BAA

loc_01_4BAD:
	call print_text_hl	; $4BAD
	call print_newline	; $4BB0
	or a	; $4BB3
	ret	; $4BB4

fn_01_4BB5:
	ld a,$01	; $4BB5
	bit 0,(ix+$07)	; $4BB7
	ret z	; $4BBB
	add a,$02	; $4BBC
	ret	; $4BBE
	.INCBIN "bank01.bin" SKIP $0BBF READ 12	; data, 12 bytes

fn_01_4BCB:
	ld (ix+$08),$00	; $4BCB
	jp fn_01_4324	; $4BCF

member_pick_step:
	ld a,(joy_pressed)	; $4BD2
	and $7F	; $4BD5
	ld hl,$4BE2	; $4BD7
	call switch_on_a	; $4BDA
	ld c,(ix+$08)	; $4BDD
	jr loc_01_4C0D	; $4BE0

tbl_01_4BE2:
	.db $01
	.dw loc_01_4BEF
	.db $02
	.dw loc_01_4BF3
	.db $10
	.dw loc_01_4C12
	.db $20
	.dw loc_01_4C19
	.db $FF

loc_01_4BEF:
	ld b,$FF	; $4BEF
	jr loc_01_4BF5	; $4BF1

loc_01_4BF3:
	ld b,$01	; $4BF3

loc_01_4BF5:
	call fn_01_40D9	; $4BF5
	ld c,(ix+$08)	; $4BF8
	call fn_01_4999	; $4BFB
	ld a,c	; $4BFE
	add a,b	; $4BFF

fn_01_4C00:
	ld hl,party_size	; $4C00
	add a,(hl)	; $4C03
	ld h,(hl)	; $4C04
	ld l,a	; $4C05
	call divide_8	; $4C06
	ld (ix+$08),h	; $4C09
	ld c,h	; $4C0C

loc_01_4C0D:
	call fn_01_49A6	; $4C0D
	xor a	; $4C10
	ret	; $4C11

loc_01_4C12:
	call fn_01_40D1	; $4C12
	ld a,$FF	; $4C15
	add a,a	; $4C17
	ret	; $4C18

loc_01_4C19:
	call fn_01_40C9	; $4C19
	ld c,(ix+$08)	; $4C1C
	call member_name	; $4C1F
	call print_newline	; $4C22
	xor a	; $4C25
	dec a	; $4C26
	ret	; $4C27

fn_01_4C28:
	call print_inline	; $4C28
	.INCBIN "bank01.bin" SKIP $0C2B READ 12	; text, 12 bytes
	ld (ix+$1A),$04	; $4C37
	ret	; $4C3B

fn_01_4C3C:
	ld a,(joy_pressed)	; $4C3C
	ld hl,$4C51	; $4C3F

fn_01_4C42:
	call switch_on_a	; $4C42

loc_01_4C45:
	ld a,(ix+$1A)	; $4C45
	ld hl,$4D00	; $4C48
	rst $20	; $4C4B
	call fn_00_3095	; $4C4C
	xor a	; $4C4F
	ret	; $4C50

tbl_01_4C51:
	.db $01
	.dw loc_01_4C64
	.db $02
	.dw $4C67
	.db $04
	.dw $4C6A
	.db $08
	.dw $4C6D
	.db $10
	.dw loc_01_4C77
	.db $20
	.dw loc_01_4C7D
	.db $FF

loc_01_4C64:
	ld a,$01	; $4C64
	ld bc,$033E	; $4C66
	ld bc,$003E	; $4C69
	ld bc,$023E	; $4C6C
	ld (ix+$1A),a	; $4C6F
	call fn_01_40D9	; $4C72
	jr loc_01_4C45	; $4C75

loc_01_4C77:
	ld a,$FF	; $4C77
	add a,a	; $4C79
	jp fn_01_40D1	; $4C7A

loc_01_4C7D:
	ld a,(ix+$1A)	; $4C7D
	cp $04	; $4C80
	jr z,loc_01_4C45	; $4C82
	call fn_01_55A0	; $4C84
	call print_newline	; $4C87
	xor a	; $4C8A
	dec a	; $4C8B
	jp fn_01_40C9	; $4C8C

fn_01_4C8F:
	ld a,(joy_pressed)	; $4C8F
	ld hl,$4CB4	; $4C92
	call switch_on_a	; $4C95

loc_01_4C98:
	ld a,(ix+$1A)	; $4C98
	call fn_01_4CA3	; $4C9B
	call fn_00_3095	; $4C9E
	xor a	; $4CA1
	ret	; $4CA2

fn_01_4CA3:
	call fn_01_40B9	; $4CA3
	call fn_01_48E1	; $4CA6
	ld a,(iy+$00)	; $4CA9
	add a,l	; $4CAC
	ld l,a	; $4CAD
	ld a,(iy+$01)	; $4CAE
	add a,h	; $4CB1
	ld h,a	; $4CB2
	ret	; $4CB3

tbl_01_4CB4:
	.db $01
	.dw loc_01_4CC7
	.db $02
	.dw loc_01_4CCA
	.db $04
	.dw loc_01_4CCD
	.db $08
	.dw loc_01_4CD0
	.db $10
	.dw loc_01_4CE8
	.db $20
	.dw loc_01_4CEE
	.db $FF

loc_01_4CC7:
	ld a,$01	; $4CC7
	.INCBIN "bank01.bin" SKIP $0CC9 READ 1	; data, 1 bytes

loc_01_4CCA:
	ld a,$03	; $4CCA
	.INCBIN "bank01.bin" SKIP $0CCC READ 1	; data, 1 bytes

loc_01_4CCD:
	ld a,$00	; $4CCD
	.INCBIN "bank01.bin" SKIP $0CCF READ 1	; data, 1 bytes

loc_01_4CD0:
	ld a,$02	; $4CD0
	ld d,a	; $4CD2
	call fn_01_4CA3	; $4CD3
	ld a,l	; $4CD6
	cp $0B	; $4CD7
	jr nc,loc_01_4C98	; $4CD9
	ld a,h	; $4CDB
	cp $0B	; $4CDC
	jr nc,loc_01_4C98	; $4CDE
	ld (ix+$1A),d	; $4CE0
	call fn_01_40D9	; $4CE3
	jr loc_01_4C98	; $4CE6

loc_01_4CE8:
	ld a,$FF	; $4CE8
	add a,a	; $4CEA
	jp fn_01_40D1	; $4CEB

loc_01_4CEE:
	ld a,(ix+$1A)	; $4CEE
	cp $04	; $4CF1
	jr z,loc_01_4C98	; $4CF3
	call fn_01_55A0	; $4CF5
	call print_newline	; $4CF8
	xor a	; $4CFB
	dec a	; $4CFC
	jp fn_01_40C9	; $4CFD

tbl_01_4D00:
	.dw fn_00_0504
	.dw fn_00_0405
	.dw fn_00_0506
	.dw fn_00_0605
	.INCBIN "bank01.bin" SKIP $0D08 READ 2	; data, 2 bytes

fn_01_4D0A:
	ld (ix+$0E),a	; $4D0A
	ld (ix+$0F),$00	; $4D0D
	ld (ix+$10),$FF	; $4D11
	ld (ix+$11),$00	; $4D15
	ld (ix+$0B),$00	; $4D19
	ld (ix+$0D),$00	; $4D1D
	xor a	; $4D21
	ld ($C065),a	; $4D22
	ld hl,$4E41	; $4D25
	jp menu_draw	; $4D28

fn_01_4D2B:
	ld a,(joy_pressed)	; $4D2B
	and $7F	; $4D2E
	ld hl,$4D83	; $4D30
	call switch_on_a	; $4D33

loc_01_4D36:
	ld (ix+$11),$00	; $4D36
	ld e,(ix+$0B)	; $4D3A
	ld d,(ix+$0D)	; $4D3D
	ld a,d	; $4D40
	cp $05	; $4D41
	jr c,loc_01_4D4F	; $4D43
	ld a,e	; $4D45
	cp $03	; $4D46
	jr c,loc_01_4D4F	; $4D48
	inc (ix+$11)	; $4D4A
	ld e,$03	; $4D4D

loc_01_4D4F:
	ld a,e	; $4D4F
	add a,a	; $4D50
	add a,a	; $4D51
	add a,a	; $4D52
	add a,a	; $4D53
	add a,$A0	; $4D54
	ld e,a	; $4D56
	ld a,d	; $4D57
	add a,a	; $4D58
	add a,a	; $4D59
	add a,a	; $4D5A
	add a,a	; $4D5B
	add a,$18	; $4D5C
	ld d,a	; $4D5E
	ld a,$02	; $4D5F
	bit 0,(ix+$11)	; $4D61
	jr z,loc_01_4D68	; $4D65
	inc a	; $4D67

loc_01_4D68:
	call fn_00_3082	; $4D68
	ld a,(ix+$10)	; $4D6B
	inc a	; $4D6E
	cp $06	; $4D6F
	jr c,loc_01_4D74	; $4D71
	xor a	; $4D73

loc_01_4D74:
	ld (ix+$10),a	; $4D74
	ld hl,$4EA6	; $4D77
	rst $28	; $4D7A
	ld h,a	; $4D7B
	ld l,$1F	; $4D7C
	call set_palette_entry	; $4D7E
	xor a	; $4D81
	ret	; $4D82

tbl_01_4D83:
	.db $01
	.dw loc_01_4D96
	.db $02
	.dw loc_01_4D9A
	.db $04
	.dw loc_01_4DB1
	.db $08
	.dw loc_01_4DD2
	.db $10
	.dw loc_01_4DF3
	.db $20
	.dw loc_01_4E0D
	.db $FF

loc_01_4D96:
	ld a,$FF	; $4D96
	jr loc_01_4D9C	; $4D98

loc_01_4D9A:
	ld a,$01	; $4D9A

loc_01_4D9C:
	call fn_01_40D9	; $4D9C
	add a,(ix+$0D)	; $4D9F
	jp p,loc_01_4DA7	; $4DA2
	ld a,$05	; $4DA5

loc_01_4DA7:
	cp $06	; $4DA7
	jr c,loc_01_4DAC	; $4DA9
	xor a	; $4DAB

loc_01_4DAC:
	ld (ix+$0D),a	; $4DAC
	jr loc_01_4D36	; $4DAF

loc_01_4DB1:
	call fn_01_40D9	; $4DB1
	ld a,(ix+$0B)	; $4DB4
	dec a	; $4DB7
	jp p,loc_01_4DBD	; $4DB8
	ld a,$04	; $4DBB

loc_01_4DBD:
	ld (ix+$0B),a	; $4DBD
	cp $03	; $4DC0
	jr nz,loc_01_4DCF	; $4DC2
	ld a,(ix+$0D)	; $4DC4
	cp $05	; $4DC7
	jr c,loc_01_4DF0	; $4DC9
	ld (ix+$0B),$02	; $4DCB

loc_01_4DCF:
	jp loc_01_4D36	; $4DCF

loc_01_4DD2:
	call fn_01_40D9	; $4DD2
	ld a,(ix+$0B)	; $4DD5
	inc a	; $4DD8
	cp $05	; $4DD9
	jr c,loc_01_4DDE	; $4DDB
	xor a	; $4DDD

loc_01_4DDE:
	ld (ix+$0B),a	; $4DDE
	cp $04	; $4DE1
	jr nz,loc_01_4DF0	; $4DE3
	ld a,(ix+$0D)	; $4DE5
	cp $05	; $4DE8
	jr c,loc_01_4DF0	; $4DEA
	ld (ix+$0B),$00	; $4DEC

loc_01_4DF0:
	jp loc_01_4D36	; $4DF0

loc_01_4DF3:
	ld a,(ix+$0F)	; $4DF3
	dec a	; $4DF6
	scf	; $4DF7
	ret m	; $4DF8
	call fn_01_40D1	; $4DF9
	ld (ix+$0F),a	; $4DFC
	.INCBIN "bank01.bin" SKIP $0DFF READ 1	; data, 1 bytes

fn_01_4E00:
	ld h,l	; $4E00
	ret nz	; $4E01
	rst $30	; $4E02
	ld (hl),$00	; $4E03
	ld a,$02	; $4E05
	call print_char	; $4E07
	jp loc_01_4D36	; $4E0A

loc_01_4E0D:
	ld a,(ix+$11)	; $4E0D
	or a	; $4E10
	jp nz,fn_01_40C9	; $4E11
	ld a,(ix+$0F)	; $4E14
	cp (ix+$0E)	; $4E17
	jp nc,loc_01_4D36	; $4E1A
	call fn_01_40C9	; $4E1D
	inc (ix+$0F)	; $4E20
	ld hl,$C065	; $4E23
	rst $30	; $4E26
	ex de,hl	; $4E27
	ld a,(ix+$0D)	; $4E28
	ld b,a	; $4E2B
	add a,a	; $4E2C
	add a,a	; $4E2D
	add a,b	; $4E2E
	add a,(ix+$0B)	; $4E2F
	ld hl,$4E8A	; $4E32
	rst $28	; $4E35
	ex de,hl	; $4E36
	ld (hl),a	; $4E37
	inc hl	; $4E38
	ld (hl),$00	; $4E39
	call print_char	; $4E3B
	jp loc_01_4D36	; $4E3E
	.INCBIN "bank01.bin" SKIP $0E41 READ 107	; data, 107 bytes

fn_01_4EAC:
	ld hl,$0000	; $4EAC
	ld de,$4EF1	; $4EAF

loc_01_4EB2:
	call fn_01_4ED5	; $4EB2
	ld a,(de)	; $4EB5
	cp $88	; $4EB6
	ret z	; $4EB8
	rrca	; $4EB9
	rrca	; $4EBA
	rrca	; $4EBB
	rrca	; $4EBC
	sra a	; $4EBD
	sra a	; $4EBF
	sra a	; $4EC1
	sra a	; $4EC3
	add a,l	; $4EC5
	ld l,a	; $4EC6
	ld a,(de)	; $4EC7
	sra a	; $4EC8
	sra a	; $4ECA
	sra a	; $4ECC
	sra a	; $4ECE
	add a,h	; $4ED0
	ld h,a	; $4ED1
	inc de	; $4ED2
	jr loc_01_4EB2	; $4ED3

fn_01_4ED5:
	push bc	; $4ED5
	push hl	; $4ED6
	ld bc,$1606	; $4ED7

loc_01_4EDA:
	push bc	; $4EDA
	push hl	; $4EDB
	ld b,$16	; $4EDC

loc_01_4EDE:
	xor a	; $4EDE
	call fn_01_4F15	; $4EDF
	ld a,l	; $4EE2
	add a,c	; $4EE3
	ld l,a	; $4EE4
	djnz loc_01_4EDE	; $4EE5
	pop hl	; $4EE7
	pop bc	; $4EE8
	ld a,h	; $4EE9
	add a,c	; $4EEA
	ld h,a	; $4EEB
	djnz loc_01_4EDA	; $4EEC
	pop hl	; $4EEE
	pop bc	; $4EEF
	ret	; $4EF0
	.INCBIN "bank01.bin" SKIP $0EF1 READ 36	; data, 36 bytes

fn_01_4F15:
	push bc	; $4F15
	push hl	; $4F16
	push de	; $4F17
	ex af,af'	; $4F18
	ex de,hl	; $4F19
	ld a,e	; $4F1A
	and $F8	; $4F1B
	ld l,a	; $4F1D
	ld h,$00	; $4F1E
	rrca	; $4F20
	rrca	; $4F21
	ld b,a	; $4F22
	ld c,h	; $4F23
	add hl,hl	; $4F24
	add hl,hl	; $4F25
	add hl,bc	; $4F26
	ld c,l	; $4F27
	ld b,h	; $4F28
	ld l,d	; $4F29
	ld h,$00	; $4F2A
	add hl,hl	; $4F2C
	add hl,hl	; $4F2D
	add hl,bc	; $4F2E
	ld bc,$0020	; $4F2F
	add hl,bc	; $4F32
	ld c,$BF	; $4F33
	di	; $4F35
	out (c),l	; $4F36
	out (c),h	; $4F38
	ei	; $4F3A
	ld a,e	; $4F3B
	and $07	; $4F3C
	inc a	; $4F3E
	ld b,a	; $4F3F
	ld a,$01	; $4F40

loc_01_4F42:
	rrca	; $4F42
	djnz loc_01_4F42	; $4F43
	ld e,a	; $4F45
	ex af,af'	; $4F46
	ld d,a	; $4F47
	in a,($BE)	; $4F48

fn_01_4F4A:
	or e	; $4F4A
	rr d	; $4F4B
	jr c,loc_01_4F50	; $4F4D

fn_01_4F4F:
	xor e	; $4F4F

loc_01_4F50:
	ld ($C1C0),a	; $4F50
	in a,($BE)	; $4F53
	or e	; $4F55
	rr d	; $4F56
	jr c,loc_01_4F5B	; $4F58
	xor e	; $4F5A

loc_01_4F5B:
	ld ($C1C1),a	; $4F5B
	in a,($BE)	; $4F5E
	or e	; $4F60
	rr d	; $4F61
	jr c,loc_01_4F66	; $4F63
	xor e	; $4F65

loc_01_4F66:
	ld ($C1C2),a	; $4F66
	in a,($BE)	; $4F69
	or e	; $4F6B
	rr d	; $4F6C
	jr c,loc_01_4F71	; $4F6E
	xor e	; $4F70

loc_01_4F71:
	ld ($C1C3),a	; $4F71
	set 6,h	; $4F74
	di	; $4F76
	out (c),l	; $4F77
	out (c),h	; $4F79
	ei	; $4F7B
	pop de	; $4F7C
	ld hl,$C1C0	; $4F7D
	ld bc,$04BE	; $4F80

loc_01_4F83:
	outi	; $4F83
	jr nz,loc_01_4F83	; $4F85
	pop hl	; $4F87
	pop bc	; $4F88
	ret	; $4F89

fn_01_4F8A:
	ld a,(map_mode)	; $4F8A
	cp $02	; $4F8D
	jp z,loc_01_5041	; $4F8F
	ld a,$05	; $4F92
	call bank_push	; $4F94
	call fn_00_2740	; $4F97
	ld hl,$D000	; $4F9A
	exx	; $4F9D
	ld hl,$0028	; $4F9E
	ld b,$20	; $4FA1

loc_01_4FA3:
	push bc	; $4FA3
	push hl	; $4FA4
	ex de,hl	; $4FA5
	ld hl,$C1C0	; $4FA6
	ld bc,$1000	; $4FA9
	call fn_00_333B	; $4FAC
	ex de,hl	; $4FAF
	ld b,$10	; $4FB0

loc_01_4FB2:
	push bc	; $4FB2
	push hl	; $4FB3
	exx	; $4FB4
	ld a,(hl)	; $4FB5
	inc hl	; $4FB6
	inc hl	; $4FB7
	exx	; $4FB8
	add a,a	; $4FB9
	ld l,a	; $4FBA
	ld h,$00	; $4FBB
	add hl,hl	; $4FBD
	add hl,hl	; $4FBE
	add hl,hl	; $4FBF
	ld de,$AFE4	; $4FC0
	add hl,de	; $4FC3
	ex de,hl	; $4FC4
	ld hl,$C1C0	; $4FC5
	ld b,$10	; $4FC8

loc_01_4FCA:
	ld a,(de)	; $4FCA
	rrca	; $4FCB
	rrca	; $4FCC
	or (hl)	; $4FCD
	ld (hl),a	; $4FCE
	inc de	; $4FCF
	inc l	; $4FD0
	djnz loc_01_4FCA	; $4FD1
	exx	; $4FD3
	ld a,(hl)	; $4FD4
	inc hl	; $4FD5
	inc hl	; $4FD6
	exx	; $4FD7
	add a,a	; $4FD8
	ld l,a	; $4FD9
	ld h,$00	; $4FDA
	add hl,hl	; $4FDC
	add hl,hl	; $4FDD
	add hl,hl	; $4FDE
	ld de,$AFE4	; $4FDF
	add hl,de	; $4FE2
	ex de,hl	; $4FE3
	ld hl,$C1C0	; $4FE4
	ld bc,$C1D0	; $4FE7
	ld a,$10	; $4FEA

loc_01_4FEC:
	ex af,af'	; $4FEC
	ld a,(de)	; $4FED
	rlca	; $4FEE
	rlca	; $4FEF
	and $03	; $4FF0
	or (hl)	; $4FF2
	ld (hl),a	; $4FF3
	ld a,(de)	; $4FF4
	add a,a	; $4FF5
	add a,a	; $4FF6
	ld (bc),a	; $4FF7
	inc de	; $4FF8
	inc l	; $4FF9
	inc c	; $4FFA
	ex af,af'	; $4FFB
	dec a	; $4FFC
	jp nz,loc_01_4FEC	; $4FFD

fn_01_5000:
	pop de	; $5000
	rst $10	; $5001
	ld hl,$C1C0	; $5002
	ld bc,$10BE	; $5005

loc_01_5008:
	outi	; $5008
	jr nz,loc_01_5008	; $500A
	ld hl,$0220	; $500C
	add hl,de	; $500F
	push hl	; $5010
	ld hl,$C1D0	; $5011
	ld de,$C1C0	; $5014
	ld bc,$0010	; $5017
	ldir	; $501A
	pop hl	; $501C
	pop bc	; $501D
	dec b	; $501E
	jp nz,loc_01_4FB2	; $501F
	ex de,hl	; $5022
	rst $10	; $5023
	ld hl,$22CC	; $5024
	ld de,$C1C0	; $5027
	ld b,$10	; $502A

loc_01_502C:
	ld a,(de)	; $502C
	or (hl)	; $502D
	out ($BE),a	; $502E
	inc hl	; $5030
	inc e	; $5031
	djnz loc_01_502C	; $5032
	pop hl	; $5034
	pop bc	; $5035
	ld de,$0010	; $5036
	add hl,de	; $5039
	dec b	; $503A
	jp nz,loc_01_4FA3	; $503B
	jp bank_pop	; $503E

loc_01_5041:
	call fn_01_50F2	; $5041
	call fn_01_5186	; $5044
	ld a,$05	; $5047
	call bank_push	; $5049
	call fn_00_2740	; $504C
	ld hl,$D800	; $504F
	exx	; $5052
	ld hl,$0028	; $5053
	ld b,$20	; $5056

loc_01_5058:
	push bc	; $5058
	push hl	; $5059
	ex de,hl	; $505A
	ld hl,$C1C0	; $505B
	ld bc,$1000	; $505E
	call fn_00_333B	; $5061
	ex de,hl	; $5064
	ld b,$10	; $5065

loc_01_5067:
	push bc	; $5067
	push hl	; $5068
	exx	; $5069
	ld a,(hl)	; $506A
	inc hl	; $506B
	exx	; $506C
	add a,a	; $506D
	add a,a	; $506E
	add a,a	; $506F
	add a,a	; $5070
	add a,$E4	; $5071
	ld e,a	; $5073
	adc a,$B7	; $5074
	sub e	; $5076
	ld d,a	; $5077
	ld hl,$C1C0	; $5078
	ld b,$10	; $507B

loc_01_507D:
	ld a,(de)	; $507D
	rrca	; $507E
	rrca	; $507F
	or (hl)	; $5080
	ld (hl),a	; $5081
	inc de	; $5082
	inc l	; $5083
	djnz loc_01_507D	; $5084
	exx	; $5086
	ld a,(hl)	; $5087
	inc hl	; $5088
	exx	; $5089
	add a,a	; $508A
	add a,a	; $508B
	add a,a	; $508C
	add a,a	; $508D
	add a,$E4	; $508E
	ld e,a	; $5090
	adc a,$B7	; $5091
	sub e	; $5093
	ld d,a	; $5094
	ld hl,$C1C0	; $5095
	ld bc,$C1D0	; $5098
	ld a,$10	; $509B

loc_01_509D:
	ex af,af'	; $509D
	ld a,(de)	; $509E
	rlca	; $509F
	rlca	; $50A0
	and $03	; $50A1
	or (hl)	; $50A3
	ld (hl),a	; $50A4
	ld a,(de)	; $50A5
	add a,a	; $50A6
	add a,a	; $50A7
	ld (bc),a	; $50A8
	inc de	; $50A9
	inc l	; $50AA
	inc c	; $50AB
	ex af,af'	; $50AC
	dec a	; $50AD
	jp nz,loc_01_509D	; $50AE
	pop de	; $50B1
	rst $10	; $50B2
	ld hl,$C1C0	; $50B3
	ld bc,$10BE	; $50B6

loc_01_50B9:
	outi	; $50B9
	jr nz,loc_01_50B9	; $50BB
	ld hl,$0220	; $50BD
	add hl,de	; $50C0
	push hl	; $50C1
	ld hl,$C1D0	; $50C2
	ld de,$C1C0	; $50C5
	ld bc,$0010	; $50C8
	ldir	; $50CB
	pop hl	; $50CD
	pop bc	; $50CE
	dec b	; $50CF
	jp nz,loc_01_5067	; $50D0
	ex de,hl	; $50D3
	rst $10	; $50D4
	ld hl,$22CC	; $50D5
	ld de,$C1C0	; $50D8
	ld b,$10	; $50DB

loc_01_50DD:
	ld a,(de)	; $50DD
	or (hl)	; $50DE
	out ($BE),a	; $50DF
	inc hl	; $50E1
	inc e	; $50E2
	djnz loc_01_50DD	; $50E3
	pop hl	; $50E5
	pop bc	; $50E6
	ld de,$0010	; $50E7
	add hl,de	; $50EA
	dec b	; $50EB
	jp nz,loc_01_5058	; $50EC
	jp bank_pop	; $50EF

fn_01_50F2:
	ld a,$10	; $50F2
	call bank_push	; $50F4
	ld hl,$DC00	; $50F7
	ld d,$82	; $50FA
	exx	; $50FC
	ld a,($C970)	; $50FD
	add a,$D0	; $5100
	ld h,a	; $5102
	ld a,(party_y)	; $5103
	add a,a	; $5106
	add a,a	; $5107
	add a,a	; $5108
	add a,a	; $5109
	ld d,a	; $510A
	ld a,(party_x)	; $510B
	or d	; $510E
	ld l,a	; $510F
	ld e,$0F	; $5110
	ld c,$10	; $5112

loc_01_5114:
	ld b,$10	; $5114

loc_01_5116:
	ld a,(hl)	; $5116
	exx	; $5117
	ld e,a	; $5118
	ld a,(de)	; $5119
	cp $0F	; $511A
	jr nz,loc_01_5120	; $511C
	or $40	; $511E

loc_01_5120:
	ld (hl),a	; $5120
	inc l	; $5121
	exx	; $5122
	ld a,l	; $5123
	inc a	; $5124
	and e	; $5125
	or d	; $5126
	ld l,a	; $5127
	djnz loc_01_5116	; $5128
	add a,$10	; $512A
	ld l,a	; $512C
	and $F0	; $512D
	ld d,a	; $512F
	dec c	; $5130
	jr nz,loc_01_5114	; $5131
	ld de,$D800	; $5133
	call fn_01_516A	; $5136
	ld de,$D810	; $5139
	call fn_01_516A	; $513C
	ld de,$DA00	; $513F
	call fn_01_516A	; $5142
	ld de,$DA10	; $5145
	call fn_01_516A	; $5148
	ld hl,$D800	; $514B
	call fn_01_517E	; $514E
	ld hl,$DBE0	; $5151
	call fn_01_517E	; $5154
	ld hl,$D800	; $5157
	ld de,$001F	; $515A
	ld b,$20	; $515D

loc_01_515F:
	set 6,(hl)	; $515F
	add hl,de	; $5161
	set 6,(hl)	; $5162
	inc hl	; $5164
	djnz loc_01_515F	; $5165
	jp bank_pop	; $5167

fn_01_516A:
	ld hl,$DC00	; $516A
	ld a,$10	; $516D

loc_01_516F:
	ld bc,$0010	; $516F
	ldir	; $5172
	ex de,hl	; $5174
	ld bc,$0010	; $5175
	add hl,bc	; $5178
	ex de,hl	; $5179
	dec a	; $517A
	jr nz,loc_01_516F	; $517B
	ret	; $517D

fn_01_517E:
	ld b,$20	; $517E

loc_01_5180:
	set 6,(hl)	; $5180
	inc l	; $5182
	djnz loc_01_5180	; $5183
	ret	; $5185

fn_01_5186:
	push iy	; $5186
	ld iy,$DA10	; $5188
	ld de,$DC00	; $518C
	ld hl,$0020	; $518F

loc_01_5192:
	dec iy	; $5192
	bit 6,(iy+$00)	; $5194
	jr z,loc_01_5192	; $5198
	set 7,(iy+$00)	; $519A
	set 7,(iy+$20)	; $519E
	set 7,(iy-$20)	; $51A2
	inc iy	; $51A6
	ld c,h	; $51A8

loc_01_51A9:
	set 6,(iy+$00)	; $51A9
	set 7,(iy+$00)	; $51AD
	bit 6,(iy+$20)	; $51B1
	jr z,loc_01_51BF	; $51B5
	res 0,c	; $51B7
	set 7,(iy+$20)	; $51B9
	jr loc_01_51CF	; $51BD

loc_01_51BF:
	bit 0,c	; $51BF
	jr nz,loc_01_51CF	; $51C1
	set 0,c	; $51C3
	ld a,iyl	; $51C5
	add a,l	; $51C7
	ld (de),a	; $51C8
	inc e	; $51C9
	ld a,iyh	; $51CA
	adc a,h	; $51CC
	ld (de),a	; $51CD
	inc e	; $51CE

loc_01_51CF:
	bit 6,(iy-$20)	; $51CF
	jr z,loc_01_51DD	; $51D3
	res 1,c	; $51D5
	set 7,(iy-$20)	; $51D7
	jr loc_01_51ED	; $51DB

loc_01_51DD:
	bit 1,c	; $51DD
	jr nz,loc_01_51ED	; $51DF
	set 1,c	; $51E1
	ld a,iyl	; $51E3
	sub l	; $51E5
	ld (de),a	; $51E6
	inc e	; $51E7
	ld a,iyh	; $51E8
	sbc a,h	; $51EA
	ld (de),a	; $51EB
	inc e	; $51EC

loc_01_51ED:
	inc iy	; $51ED
	bit 6,(iy+$00)	; $51EF
	jp z,loc_01_51A9	; $51F3
	set 7,(iy+$00)	; $51F6
	set 7,(iy+$20)	; $51FA
	set 7,(iy-$20)	; $51FE
	inc e	; $5202
	dec e	; $5203
	jr z,loc_01_5211	; $5204
	dec e	; $5206
	ld a,(de)	; $5207
	ld iyh,a	; $5208
	dec e	; $520A
	ld a,(de)	; $520B
	ld iyl,a	; $520C
	jp loc_01_5192	; $520E

loc_01_5211:
	ld hl,$D800	; $5211
	ld bc,$0004	; $5214
	xor a	; $5217

loc_01_5218:
	bit 7,(hl)	; $5218
	jr nz,loc_01_521D	; $521A
	ld (hl),a	; $521C

loc_01_521D:
	inc hl	; $521D
	djnz loc_01_5218	; $521E
	dec c	; $5220
	jr nz,loc_01_5218	; $5221
	ld hl,$D800	; $5223
	ld bc,$2000	; $5226
	call fn_00_333B	; $5229
	ld hl,$DBE0	; $522C
	ld bc,$2000	; $522F
	call fn_00_333B	; $5232
	ld hl,$D800	; $5235
	ld de,$001F	; $5238
	ld bc,$2000	; $523B

loc_01_523E:
	ld (hl),c	; $523E
	add hl,de	; $523F
	ld (hl),c	; $5240
	inc hl	; $5241
	djnz loc_01_523E	; $5242
	pop iy	; $5244
	ret	; $5246

loc_01_5247:
	call bank_push	; $5247
	push bc	; $524A
	push de	; $524B
	push hl	; $524C
	call fn_01_5331	; $524D

loc_01_5250:
	push bc	; $5250
	push de	; $5251
	rst $10	; $5252
	ld de,($C610)	; $5253
	ld c,$BE	; $5257

loc_01_5259:
	push bc	; $5259
	push hl	; $525A
	ld l,(hl)	; $525B
	ld h,$00	; $525C
	add hl,hl	; $525E
	add hl,hl	; $525F
	add hl,hl	; $5260
	add hl,hl	; $5261
	add hl,hl	; $5262
	add hl,de	; $5263
	ld a,b	; $5264
	cp $08	; $5265
	jr c,loc_01_526D	; $5267
	ld b,$20	; $5269
	jr loc_01_5270	; $526B

loc_01_526D:
	add a,a	; $526D
	add a,a	; $526E
	ld b,a	; $526F

loc_01_5270:
	outi	; $5270
	jr nz,loc_01_5270	; $5272
	pop hl	; $5274
	pop bc	; $5275
	inc hl	; $5276
	ld a,b	; $5277
	sub $08	; $5278
	ld b,a	; $527A
	jr z,loc_01_527F	; $527B
	jr nc,loc_01_5259	; $527D

loc_01_527F:
	pop de	; $527F
	ex de,hl	; $5280
	ld bc,$0220	; $5281
	add hl,bc	; $5284
	ex de,hl	; $5285
	pop bc	; $5286
	dec c	; $5287
	jr nz,loc_01_5250	; $5288
	pop hl	; $528A
	pop de	; $528B
	pop bc	; $528C
	jp bank_pop	; $528D

loc_01_5290:
	call bank_push	; $5290
	push bc	; $5293
	push de	; $5294
	push hl	; $5295
	call fn_01_5311	; $5296
	ld bc,$533E	; $5299
	ld ($C612),bc	; $529C
	call fn_01_5331	; $52A0
	ld a,$20	; $52A3

loc_01_52A5:
	push af	; $52A5
	push bc	; $52A6
	push de	; $52A7
	push hl	; $52A8
	call wait_vblank	; $52A9

loc_01_52AC:
	push bc	; $52AC
	push de	; $52AD
	rst $10	; $52AE

loc_01_52AF:
	push bc	; $52AF
	push hl	; $52B0
	ld l,(hl)	; $52B1
	ld h,$00	; $52B2
	add hl,hl	; $52B4
	add hl,hl	; $52B5
	add hl,hl	; $52B6
	add hl,hl	; $52B7
	add hl,hl	; $52B8
	ld de,($C610)	; $52B9
	add hl,de	; $52BD
	ld a,b	; $52BE
	cp $08	; $52BF
	jr c,loc_01_52C5	; $52C1
	ld b,$08	; $52C3

loc_01_52C5:
	ld de,($C612)	; $52C5

loc_01_52C9:
	ld a,(de)	; $52C9
	ld c,a	; $52CA
	and (hl)	; $52CB
	out ($BE),a	; $52CC
	inc hl	; $52CE
	ld a,c	; $52CF
	and (hl)	; $52D0
	out ($BE),a	; $52D1
	inc hl	; $52D3
	ld a,c	; $52D4
	and (hl)	; $52D5
	out ($BE),a	; $52D6
	inc hl	; $52D8
	ld a,c	; $52D9
	and (hl)	; $52DA
	out ($BE),a	; $52DB
	inc hl	; $52DD
	inc de	; $52DE
	djnz loc_01_52C9	; $52DF
	pop hl	; $52E1
	pop bc	; $52E2
	inc hl	; $52E3
	ld a,b	; $52E4
	sub $08	; $52E5
	ld b,a	; $52E7
	jr z,loc_01_52ED	; $52E8
	jp nc,loc_01_52AF	; $52EA

loc_01_52ED:
	pop de	; $52ED
	ex de,hl	; $52EE
	ld bc,$0220	; $52EF
	add hl,bc	; $52F2
	ex de,hl	; $52F3
	pop bc	; $52F4
	dec c	; $52F5
	jp nz,loc_01_52AC	; $52F6
	ld hl,($C612)	; $52F9
	ld de,$0008	; $52FC
	add hl,de	; $52FF

fn_01_5300:
	ld ($C612),hl	; $5300
	pop hl	; $5303
	pop de	; $5304
	pop bc	; $5305
	pop af	; $5306
	dec a	; $5307
	jp nz,loc_01_52A5	; $5308
	pop hl	; $530B
	pop de	; $530C
	pop bc	; $530D
	jp bank_pop	; $530E

fn_01_5311:
	push bc	; $5311
	push de	; $5312
	push hl	; $5313
	ld a,$05	; $5314
	ld hl,$B8E4	; $5316
	ld de,$3E80	; $5319
	call fn_00_0788	; $531C
	call fn_00_0950	; $531F
	ld a,$05	; $5322
	ld hl,$B8F0	; $5324
	ld de,$0888	; $5327
	call fn_00_09BE	; $532A
	pop hl	; $532D
	pop de	; $532E
	pop bc	; $532F
	ret	; $5330

fn_01_5331:
	ld c,(hl)	; $5331
	inc hl	; $5332
	ld b,(hl)	; $5333
	inc hl	; $5334
	ld ($C610),bc	; $5335
	ld c,(hl)	; $5339
	inc hl	; $533A
	ld b,(hl)	; $533B
	inc hl	; $533C
	ret	; $533D
	.INCBIN "bank01.bin" SKIP $133E READ 290	; data, 290 bytes

put_glyph:
	push hl	; $5460
	push af	; $5461
	ld a,(text_col)	; $5462
	add a,a	; $5465
	add a,a	; $5466
	add a,a	; $5467
	ld l,a	; $5468
	ld a,(text_row)	; $5469
	rra	; $546C
	rr l	; $546D
	rra	; $546F
	rr l	; $5470
	add a,$CA	; $5472
	ld h,a	; $5474
	ld a,l	; $5475
	di	; $5476
	out ($BF),a	; $5477
	ld a,h	; $5479
	add a,$AE	; $547A
	out ($BF),a	; $547C
	ei	; $547E
	pop af	; $547F
	add a,$40	; $5480
	ld (hl),a	; $5482
	out ($BE),a	; $5483
	inc l	; $5485
	ld a,$01	; $5486
	ld (hl),a	; $5488
	pop hl	; $5489
	out ($BE),a	; $548A
	ret	; $548C

fn_01_548D:
	ld a,$22	; $548D

print_char:
	cp $01	; $548F
	jp z,print_newline	; $5491
	cp $02	; $5494
	jr z,loc_01_54B0	; $5496
	cp $20	; $5498
	jr z,loc_01_54BE	; $549A
	push af	; $549C
	ld a,(text_col)	; $549D
	cp $1F	; $54A0
	call nc,print_newline	; $54A2
	pop af	; $54A5
	call put_glyph	; $54A6

loc_01_54A9:
	push hl	; $54A9
	ld hl,text_col	; $54AA
	inc (hl)	; $54AD
	pop hl	; $54AE
	ret	; $54AF

loc_01_54B0:
	ld a,(text_col)	; $54B0
	or a	; $54B3
	ret z	; $54B4
	dec a	; $54B5
	ld (text_col),a	; $54B6
	ld a,$20	; $54B9
	jp put_glyph	; $54BB

loc_01_54BE:
	ld a,(text_col)	; $54BE
	cp $1F	; $54C1
	ret nc	; $54C3
	ld a,$20	; $54C4
	call put_glyph	; $54C6
	jr loc_01_54A9	; $54C9

fn_01_54CB:
	call print_newline	; $54CB

print_newline:
	push af	; $54CE
	push bc	; $54CF
	push de	; $54D0
	push hl	; $54D1
	ld hl,text_col	; $54D2
	ld (hl),$01	; $54D5
	inc hl	; $54D7
	inc (hl)	; $54D8
	ld a,(hl)	; $54D9
	cp $18	; $54DA
	jr c,loc_01_550B	; $54DC
	ld (hl),$17	; $54DE
	call wait_vblank	; $54E0
	ld hl,$CF00	; $54E3
	ld de,$CEC0	; $54E6
	ld bc,$0100	; $54E9
	ldir	; $54EC
	ld hl,$0160	; $54EE
	ld ($CFC2),hl	; $54F1
	ld hl,$CFC2	; $54F4
	ld de,$CFC4	; $54F7
	ld bc,$003A	; $54FA
	ldir	; $54FD
	ld hl,$CEC0	; $54FF
	ld de,$3CC0	; $5502
	ld bc,$0140	; $5505
	call fn_00_024C	; $5508

loc_01_550B:
	pop hl	; $550B
	pop de	; $550C
	pop bc	; $550D
	pop af	; $550E
	ret	; $550F

print_inline:
	ex (sp),hl	; $5510
	call print_text_hl	; $5511
	ex (sp),hl	; $5514
	ret	; $5515

print_item_name:
	call item_name	; $5516

print_text_hl:
	push bc	; $5519
	ld bc,$0100	; $551A

loc_01_551D:
	dec b	; $551D
	jr nz,loc_01_5541	; $551E
	push hl	; $5520

loc_01_5521:
	inc b	; $5521
	ld a,(hl)	; $5522
	inc hl	; $5523
	cp $20	; $5524
	jr z,loc_01_552E	; $5526
	dec a	; $5528
	jr z,loc_01_552E	; $5529
	inc a	; $552B
	jr nz,loc_01_5521	; $552C

loc_01_552E:
	pop hl	; $552E
	ld a,(text_col)	; $552F
	cp $01	; $5532
	jr z,loc_01_5541	; $5534
	add a,b	; $5536
	cp $21	; $5537
	jr c,loc_01_5541	; $5539
	call text_page_wait	; $553B
	call print_newline	; $553E

loc_01_5541:
	ld a,(hl)	; $5541
	dec a	; $5542

fn_01_5543:
	call z,text_page_wait	; $5543
	ld a,(hl)	; $5546
	inc hl	; $5547
	or a	; $5548
	jr z,loc_01_5550	; $5549
	call print_char	; $554B
	jr loc_01_551D	; $554E

loc_01_5550:
	pop bc	; $5550
	ret	; $5551

text_page_wait:
	inc c	; $5552
	ld a,c	; $5553
	cp $05	; $5554
	ret c	; $5556
	ld c,$00	; $5557
	push af	; $5559
	push bc	; $555A
	push de	; $555B
	push hl	; $555C
	push ix	; $555D
	push iy	; $555F

loc_01_5561:
	call wait_vblank	; $5561
	call wait_vblank	; $5564
	call fn_00_0950	; $5567
	call update_joypad	; $556A
	ld a,(joy_pressed)	; $556D
	and $30	; $5570
	jr nz,loc_01_5579	; $5572
	call fn_00_335C	; $5574
	jr loc_01_5561	; $5577

loc_01_5579:
	pop iy	; $5579
	pop ix	; $557B
	pop hl	; $557D
	pop de	; $557E
	pop bc	; $557F
	pop af	; $5580
	ret	; $5581

print_message:
	push bc	; $5582
	push hl	; $5583
	ld c,a	; $5584
	ld a,(message_bank)	; $5585
	ld hl,(message_table)	; $5588
	call fn_00_0807	; $558B
	ld a,(message_bank)	; $558E
	call fn_01_5597	; $5591
	pop hl	; $5594
	pop bc	; $5595
	ret	; $5596

fn_01_5597:
	call bank_push	; $5597
	call print_text_hl	; $559A
	jp bank_pop	; $559D

fn_01_55A0:
	push hl	; $55A0
	ld hl,$55AA	; $55A1
	rst $20	; $55A4
	call print_text_hl	; $55A5
	pop hl	; $55A8
	ret	; $55A9

tbl_01_55AA:
	.dw $55B4
	.dw $55B9
	.dw $55BF
	.dw $55C4
	.dw fn_01_55CA
	.dw $6557
	.dw fn_01_7473
	.dw fn_01_4E00
	.dw fn_01_726F
	.dw fn_01_6874
	.INCBIN "bank01.bin" SKIP $15BE READ 1	; data, 1 bytes

fn_01_55BF:
	.INCBIN "bank01.bin" SKIP $15BF READ 5	; data, 5 bytes

fn_01_55C4:
	.INCBIN "bank01.bin" SKIP $15C4 READ 6	; data, 6 bytes

fn_01_55CA:
	ld b,e	; $55CA
	ld h,c	; $55CB
	ld l,h	; $55CC
	ld l,l	; $55CD
	nop	; $55CE

print_number:
	push bc	; $55CF
	push hl	; $55D0
	ld a,c	; $55D1
	ld ($C0A7),a	; $55D2
	xor a	; $55D5
	ld ($C0A6),a	; $55D6
	dec b	; $55D9
	jr z,loc_01_55FD	; $55DA
	dec b	; $55DC
	jr z,loc_01_55F7	; $55DD
	dec b	; $55DF
	jr z,loc_01_55F1	; $55E0
	dec b	; $55E2
	jr z,loc_01_55EB	; $55E3
	ld bc,$2710	; $55E5
	call fn_01_5606	; $55E8

loc_01_55EB:
	ld bc,$03E8	; $55EB
	call fn_01_5606	; $55EE

loc_01_55F1:
	ld bc,$0064	; $55F1
	call fn_01_5606	; $55F4

loc_01_55F7:
	ld bc,$000A	; $55F7
	call fn_01_5606	; $55FA

loc_01_55FD:
	ld a,l	; $55FD
	add a,$30	; $55FE
	call print_char	; $5600
	pop hl	; $5603
	pop bc	; $5604
	ret	; $5605

fn_01_5606:
	xor a	; $5606

loc_01_5607:
	sbc hl,bc	; $5607
	inc a	; $5609
	jr nc,loc_01_5607	; $560A
	add hl,bc	; $560C
	dec a	; $560D
	jr z,loc_01_5618	; $560E
	ld ($C0A6),a	; $5610
	add a,$30	; $5613
	jp print_char	; $5615

loc_01_5618:
	ld a,($C0A6)	; $5618
	or a	; $561B
	ld a,$30	; $561C
	jp nz,print_char	; $561E
	ld a,($C0A7)	; $5621
	jp print_char	; $5624

print_number_auto:
	push bc	; $5627
	push de	; $5628
	ex de,hl	; $5629
	ld bc,$0500	; $562A
	ld hl,$D8F0	; $562D
	add hl,de	; $5630
	jr c,loc_01_5649	; $5631
	dec b	; $5633
	ld hl,$FC18	; $5634
	add hl,de	; $5637
	jr c,loc_01_5649	; $5638
	dec b	; $563A
	ld hl,$FF9C	; $563B
	add hl,de	; $563E
	jr c,loc_01_5649	; $563F
	dec b	; $5641
	ld hl,$FFF6	; $5642
	add hl,de	; $5645
	jr c,loc_01_5649	; $5646
	dec b	; $5648

loc_01_5649:
	ex de,hl	; $5649
	call print_number	; $564A
	pop de	; $564D
	pop bc	; $564E
	ret	; $564F

print_avatar_name:
	push hl	; $5650
	ld hl,party_name	; $5651
	call print_text_hl	; $5654
	pop hl	; $5657
	ret	; $5658

fn_01_5659:
	ld e,(hl)	; $5659
	inc hl	; $565A
	inc e	; $565B
	ret z	; $565C
	dec e	; $565D
	ld d,(hl)	; $565E
	inc hl	; $565F
	ld ($C60E),de	; $5660

loc_01_5664:
	ld a,(hl)	; $5664
	inc hl	; $5665
	or a	; $5666
	jr z,fn_01_5659	; $5667
	push hl	; $5669
	call fn_01_5670	; $566A
	pop hl	; $566D
	jr loc_01_5664	; $566E

fn_01_5670:
	sub $20	; $5670
	add a,a	; $5672
	ld l,a	; $5673
	ld h,$00	; $5674
	ld e,l	; $5676
	ld d,h	; $5677
	add hl,hl	; $5678
	add hl,de	; $5679
	ld de,$BD92	; $567A
	add hl,de	; $567D
	ld a,(slot2_bank)	; $567E
	push af	; $5681
	ld a,$16	; $5682
	ld ($FFFF),a	; $5684
	ld a,($C60E)	; $5687
	add a,a	; $568A
	ld b,a	; $568B
	add a,a	; $568C
	add a,b	; $568D
	ld b,a	; $568E
	and $07	; $568F
	ld c,a	; $5691
	inc c	; $5692
	ld a,b	; $5693
	exx	; $5694
	and $F8	; $5695
	rrca	; $5697
	rrca	; $5698
	ld e,a	; $5699
	ld d,$00	; $569A
	ld hl,$543E	; $569C
	add hl,de	; $569F
	ld e,(hl)	; $56A0
	inc hl	; $56A1
	ld d,(hl)	; $56A2
	ld a,($C60F)	; $56A3
	add a,a	; $56A6
	ld b,a	; $56A7
	add a,a	; $56A8
	add a,b	; $56A9
	ld l,a	; $56AA
	ld h,$00	; $56AB
	add hl,hl	; $56AD
	add hl,hl	; $56AE
	add hl,de	; $56AF
	push hl	; $56B0
	rst $08	; $56B1
	ex de,hl	; $56B2
	ld hl,$C1C0	; $56B3
	ld bc,$18BE	; $56B6

loc_01_56B9:
	ini	; $56B9
	jr nz,loc_01_56B9	; $56BB
	ld hl,$0220	; $56BD
	add hl,de	; $56C0
	rst $08	; $56C1
	ld hl,$C1D8	; $56C2
	ld b,$18	; $56C5

loc_01_56C7:
	ini	; $56C7
	jr nz,loc_01_56C7	; $56C9
	pop de	; $56CB
	rst $10	; $56CC
	push de	; $56CD
	exx	; $56CE
	push iy	; $56CF
	ld iy,$C1C0	; $56D1
	ld d,$00	; $56D5
	ld a,$06	; $56D7

loc_01_56D9:
	ex af,af'	; $56D9
	ld a,(iy+$00)	; $56DA
	out ($BE),a	; $56DD
	ld a,(hl)	; $56DF
	inc hl	; $56E0
	ld e,d	; $56E1
	ld b,c	; $56E2
	dec b	; $56E3
	jr z,loc_01_56EC	; $56E4
	or a	; $56E6

loc_01_56E7:
	rra	; $56E7
	rr e	; $56E8
	djnz loc_01_56E7	; $56EA

loc_01_56EC:
	or (iy+$01)	; $56EC
	out ($BE),a	; $56EF
	ld a,e	; $56F1
	or (iy+$19)	; $56F2
	ld (iy+$19),a	; $56F5
	ld a,(iy+$02)	; $56F8
	out ($BE),a	; $56FB
	ld a,(iy+$03)	; $56FD
	out ($BE),a	; $5700
	ld e,$04	; $5702
	add iy,de	; $5704
	ex af,af'	; $5706
	dec a	; $5707
	jr nz,loc_01_56D9	; $5708
	pop iy	; $570A
	pop de	; $570C
	ld hl,$0220	; $570D
	add hl,de	; $5710
	ex de,hl	; $5711
	rst $10	; $5712
	ld hl,$C1D8	; $5713
	ld bc,$18BE	; $5716

loc_01_5719:
	outi	; $5719
	jr nz,loc_01_5719	; $571B
	ld hl,$C60E	; $571D
	inc (hl)	; $5720
	pop af	; $5721
	ld ($FFFF),a	; $5722
	ret	; $5725

fn_01_5726:
	call read_pads_both	; $5726
	cp $20	; $5729
	jr z,loc_01_5777	; $572B
	cp $10	; $572D
	ret nz	; $572F
	ld (ix+$1A),$00	; $5730
	ld (ix+$16),$04	; $5734

loc_01_5738:
	call print_newline	; $5738
	ld a,(ix+$1A)	; $573B
	call fn_01_575E	; $573E
	ld a,$10	; $5741
	ld (text_col),a	; $5743
	ld a,(ix+$1A)	; $5746
	add a,$04	; $5749
	call fn_01_575E	; $574B
	inc (ix+$1A)	; $574E
	dec (ix+$16)	; $5751
	jr nz,loc_01_5738	; $5754
	call print_newline	; $5756
	ld a,$7F	; $5759
	jp print_char	; $575B

fn_01_575E:
	push af	; $575E
	ld hl,karma	; $575F
	rst $28	; $5762
	ld l,a	; $5763
	ld h,$00	; $5764
	ld bc,$0230	; $5766
	call print_number	; $5769
	ld a,$3A	; $576C
	call print_char	; $576E
	pop af	; $5771
	add a,$97	; $5772
	jp print_item_name	; $5774

loc_01_5777:
	call task_enter_1	; $5777
	ld (ix+$1A),$00	; $577A

loc_01_577E:
	call fn_00_2740	; $577E
	call yield	; $5781
	call fn_01_57C5	; $5784

loc_01_5787:
	call fn_00_335C	; $5787
	call yield	; $578A
	ld a,(joy_pressed)	; $578D
	bit 5,a	; $5790
	jr nz,loc_01_57B9	; $5792
	bit 4,a	; $5794
	jr nz,loc_01_57B9	; $5796
	bit 0,a	; $5798
	jr nz,loc_01_57AB	; $579A
	bit 1,a	; $579C
	jr z,loc_01_5787	; $579E
	ld a,(ix+$1A)	; $57A0
	inc a	; $57A3
	cp $0A	; $57A4
	jr c,loc_01_57B4	; $57A6
	xor a	; $57A8
	jr loc_01_57B4	; $57A9

loc_01_57AB:
	ld a,(ix+$1A)	; $57AB
	dec a	; $57AE
	jp p,loc_01_57B4	; $57AF
	ld a,$09	; $57B2

loc_01_57B4:
	ld (ix+$1A),a	; $57B4
	jr loc_01_577E	; $57B7

loc_01_57B9:
	call fn_00_276D	; $57B9
	call fn_00_129D	; $57BC
	call yield	; $57BF
	jp task_return_1	; $57C2

fn_01_57C5:
	ld a,$16	; $57C5
	call bank_push	; $57C7
	ld a,(ix+$1A)	; $57CA
	ld hl,$B1CE	; $57CD
	rst $20	; $57D0
	call fn_01_5659	; $57D1
	jp bank_pop	; $57D4

fn_01_57D7:
	call task_enter_2	; $57D7
	ld a,(map_mode)	; $57DA
	cp $03	; $57DD
	jr c,loc_01_57E9	; $57DF
	ld a,($C97F)	; $57E1
	ld (ix+$08),a	; $57E4
	jr loc_01_57F7	; $57E7

loc_01_57E9:
	call fn_01_58C4	; $57E9

loc_01_57EC:
	call frame_tick	; $57EC
	call member_pick_step	; $57EF
	jr z,loc_01_57EC	; $57F2
	jp c,loc_01_58D0	; $57F4

loc_01_57F7:
	ld a,$0F	; $57F7
	call fn_01_7552	; $57F9
	ld hl,weapons_owned	; $57FC
	ld b,$0F	; $57FF
	call fn_01_58D7	; $5801
	call print_inline	; $5804
	.INCBIN "bank01.bin" SKIP $1807 READ 9	; text, 9 bytes
	ld (ix+$1A),$00	; $5810

loc_01_5814:
	call frame_tick	; $5814
	ld c,(ix+$1B)	; $5817
	ld de,$0001	; $581A
	call menu_step_wrap	; $581D
	jr z,loc_01_5814	; $5820
	jp c,loc_01_58D0	; $5822
	ld hl,weapons_owned	; $5825
	call fn_01_58E5	; $5828
	ld a,(ix+$1C)	; $582B
	add a,$25	; $582E
	call print_item_name	; $5830
	call print_newline	; $5833
	ld hl,$5960	; $5836
	ld de,$2501	; $5839
	call fn_01_58F9	; $583C
	ld c,(ix+$08)	; $583F
	call member_record	; $5842
	ld a,(iy+$0B)	; $5845
	ld hl,weapons_owned	; $5848
	call fn_01_5949	; $584B
	ld (iy+$0B),a	; $584E
	or a	; $5851
	jp task_return_2	; $5852

fn_01_5855:
	call task_enter_2	; $5855
	call fn_01_58C4	; $5858

loc_01_585B:
	call frame_tick	; $585B
	call member_pick_step	; $585E
	jr z,loc_01_585B	; $5861
	jp c,loc_01_58D0	; $5863
	ld a,$10	; $5866
	call fn_01_7552	; $5868
	ld hl,$C917	; $586B
	ld b,$07	; $586E
	call fn_01_58D7	; $5870
	call print_inline	; $5873
	.INCBIN "bank01.bin" SKIP $1876 READ 9	; text, 9 bytes
	ld (ix+$1A),$00	; $587F

loc_01_5883:
	call frame_tick	; $5883
	ld c,(ix+$1B)	; $5886
	ld de,$0002	; $5889
	call menu_step_wrap	; $588C
	jr z,loc_01_5883	; $588F
	jp c,loc_01_58D0	; $5891
	ld hl,$C917	; $5894
	call fn_01_58E5	; $5897
	ld a,(ix+$1C)	; $589A
	add a,$35	; $589D
	call print_item_name	; $589F
	call print_newline	; $58A2
	ld hl,$5970	; $58A5
	ld de,$3500	; $58A8
	call fn_01_58F9	; $58AB
	ld c,(ix+$08)	; $58AE
	call member_record	; $58B1
	ld a,(iy+$0C)	; $58B4
	ld hl,$C917	; $58B7
	call fn_01_5949	; $58BA
	ld (iy+$0C),a	; $58BD
	or a	; $58C0
	jp task_return_2	; $58C1

fn_01_58C4:
	call print_inline	; $58C4
	.INCBIN "bank01.bin" SKIP $18C7 READ 6	; text, 6 bytes
	jp fn_01_4BCB	; $58CD

loc_01_58D0:
	call fn_01_472D	; $58D0
	scf	; $58D3
	jp task_return_2	; $58D4

fn_01_58D7:
	ld c,$01	; $58D7
	xor a	; $58D9

loc_01_58DA:
	inc hl	; $58DA
	cp (hl)	; $58DB
	jr z,loc_01_58DF	; $58DC
	inc c	; $58DE

loc_01_58DF:
	djnz loc_01_58DA	; $58DF
	ld (ix+$1B),c	; $58E1
	ret	; $58E4

fn_01_58E5:
	xor a	; $58E5
	ld (ix+$1C),a	; $58E6
	ld b,(ix+$1A)	; $58E9
	inc b	; $58EC
	dec b	; $58ED
	ret z	; $58EE

loc_01_58EF:
	inc hl	; $58EF
	inc (ix+$1C)	; $58F0
	cp (hl)	; $58F3
	jr z,loc_01_58EF	; $58F4
	djnz loc_01_58EF	; $58F6
	ret	; $58F8

fn_01_58F9:
	ld c,(ix+$08)	; $58F9
	call member_record	; $58FC
	ld b,(iy+$1B)	; $58FF
	inc b	; $5902
	ld a,$01	; $5903

loc_01_5905:
	rrca	; $5905
	djnz loc_01_5905	; $5906
	ld b,a	; $5908
	ld a,(ix+$1C)	; $5909
	rst $28	; $590C
	and b	; $590D
	ret nz	; $590E
	pop af	; $590F
	call print_inline	; $5910
	.INCBIN "bank01.bin" SKIP $1913 READ 3	; text, 3 bytes
	ld a,(iy+$1B)	; $5916
	add a,$4D	; $5919
	call print_item_name	; $591B
	call print_inline	; $591E
	.INCBIN "bank01.bin" SKIP $1921 READ 14	; text, 14 bytes
	dec e	; $592F
	jr nz,loc_01_5938	; $5930
	call print_inline	; $5932
	.INCBIN "bank01.bin" SKIP $1935 READ 3	; text, 3 bytes

loc_01_5938:
	ld a,(ix+$1C)	; $5938
	add a,d	; $593B
	call print_item_name	; $593C
	call wait_button	; $593F

fn_01_5942:
	call print_newline	; $5942
	or a	; $5945
	jp task_return_2	; $5946

fn_01_5949:
	cp (ix+$1C)	; $5949
	ret z	; $594C
	push hl	; $594D
	rst $30	; $594E
	inc (hl)	; $594F
	ld a,(hl)	; $5950
	cp $64	; $5951
	jr c,loc_01_5957	; $5953
	ld (hl),$63	; $5955

loc_01_5957:
	pop hl	; $5957
	ld a,(ix+$1C)	; $5958
	push af	; $595B
	rst $30	; $595C
	dec (hl)	; $595D
	pop af	; $595E
	ret	; $595F
	.INCBIN "bank01.bin" SKIP $1960 READ 24	; data, 24 bytes

fn_01_5978:
	call fn_01_48E1	; $5978
	ld l,(iy+$00)	; $597B
	ld h,(iy+$01)	; $597E
	call fn_00_38F2	; $5981
	ld a,(hl)	; $5984
	cp $3C	; $5985
	jp nz,fn_01_46DF	; $5987
	ld (hl),$16	; $598A
	inc l	; $598C
	ld (hl),$00	; $598D
	ld a,(location)	; $598F
	cp $12	; $5992
	jr c,loc_01_59A3	; $5994
	ld hl,(party_x)	; $5996
	call fn_01_6DC0	; $5999
	ld a,(hl)	; $599C
	cp $40	; $599D
	jr nz,loc_01_59A3	; $599F
	ld (hl),$00	; $59A1

loc_01_59A3:
	ld a,($C97F)	; $59A3
	ld (ix+$08),a	; $59A6
	jp loc_01_5A04	; $59A9

sub_01_59AC:
	ld a,$13	; $59AC
	call fn_00_05F1	; $59AE
	call fn_01_4324	; $59B1
	call fn_01_49A6	; $59B4
	call wait_vblank	; $59B7
	ld a,$1E	; $59BA
	call random_below	; $59BC
	ld l,a	; $59BF
	ld h,$00	; $59C0
	call member_hurt	; $59C2
	call wait_vblank	; $59C5
	jp fn_01_4324	; $59C8

loc_01_59CB:
	call print_inline	; $59CB
	.INCBIN "bank01.bin" SKIP $19CE READ 18	; text, 18 bytes
	ld a,$50	; $59E0
	call random_below	; $59E2
	ld b,a	; $59E5
	ld a,$0A	; $59E6
	call random_below	; $59E8
	add a,b	; $59EB
	add a,$0A	; $59EC
	ld l,a	; $59EE
	ld h,$00	; $59EF
	push hl	; $59F1
	call print_number_auto	; $59F2
	call print_inline	; $59F5
	.INCBIN "bank01.bin" SKIP $19F8 READ 8	; text, 8 bytes
	pop hl	; $5A00
	jp receive_gold	; $5A01

loc_01_5A04:
	call random	; $5A04
	and $01	; $5A07
	jr z,loc_01_59CB	; $5A09
	call random	; $5A0B
	and $03	; $5A0E
	ld b,a	; $5A10
	call random	; $5A11
	and b	; $5A14
	ld (ix+$1A),a	; $5A15
	ld hl,$5AF4	; $5A18
	rst $20	; $5A1B
	call print_text_hl	; $5A1C
	call print_inline	; $5A1F
	.INCBIN "bank01.bin" SKIP $1A22 READ 8	; text, 8 bytes
	ld a,(ix+$08)	; $5A2A
	cp $FF	; $5A2D
	jr z,loc_01_5A43	; $5A2F
	ld c,a	; $5A31
	call member_record	; $5A32
	ld a,(iy+$07)	; $5A35
	add a,$19	; $5A38
	ld b,a	; $5A3A
	ld a,$64	; $5A3B
	call random_below	; $5A3D
	cp b	; $5A40
	jr nc,loc_01_5A56	; $5A41

loc_01_5A43:
	ld a,$19	; $5A43
	call fn_00_05F1	; $5A45
	call print_inline	; $5A48
	.INCBIN "bank01.bin" SKIP $1A4B READ 9	; text, 9 bytes
	jr loc_01_5A5D	; $5A54

loc_01_5A56:
	ld a,(ix+$1A)	; $5A56
	ld hl,$5A60	; $5A59
	rst $18	; $5A5C

loc_01_5A5D:
	jp loc_01_59CB	; $5A5D

tbl_01_5A60:
	.dw $5A68
	.dw fn_01_5A6B
	.dw fn_01_5A8D
	.dw fn_01_5A9D
	.dw fn_02_ACC3
	.INCBIN "bank01.bin" SKIP $1A6A READ 1	; data, 1 bytes

fn_01_5A6B:
	ld c,(ix+$08)	; $5A6B
	call member_record	; $5A6E
	ld (iy+$1C),$53	; $5A71
	call fn_01_4324	; $5A75
	ld a,(map_mode)	; $5A78
	cp $03	; $5A7B
	jr c,loc_01_5A8A	; $5A7D
	call member_object	; $5A7F
	ld (iy+$02),$38	; $5A82
	ld (iy+$03),$38	; $5A86

loc_01_5A8A:
	jp fn_01_4247	; $5A8A

fn_01_5A8D:
	ld c,(ix+$08)	; $5A8D
	call member_record	; $5A90
	ld (iy+$1C),$50	; $5A93
	call fn_01_4324	; $5A97
	jp fn_01_4247	; $5A9A

fn_01_5A9D:
	ld a,(map_mode)	; $5A9D
	cp $03	; $5AA0
	jp c,fn_01_4812	; $5AA2
	ld a,(party_size)	; $5AA5
	ld b,a	; $5AA8
	ld c,$00	; $5AA9

loc_01_5AAB:
	call member_object	; $5AAB
	ld a,(iy+$02)	; $5AAE
	or a	; $5AB1
	call nz,fn_01_49A6	; $5AB2
	inc c	; $5AB5
	djnz loc_01_5AAB	; $5AB6
	call wait_vblank	; $5AB8
	ld a,$13	; $5ABB
	call fn_00_05F1	; $5ABD
	call fn_01_4117	; $5AC0
	call fn_01_4887	; $5AC3
	ld a,(party_size)	; $5AC6
	ld b,a	; $5AC9
	ld c,$00	; $5ACA

loc_01_5ACC:
	call member_object	; $5ACC
	ld a,(iy+$02)	; $5ACF
	or a	; $5AD2
	jr z,loc_01_5AEE	; $5AD3
	call member_can_act	; $5AD5
	jr c,loc_01_5AEE	; $5AD8
	call random	; $5ADA
	and $01	; $5ADD
	jr z,loc_01_5AEE	; $5ADF
	ld a,$0F	; $5AE1
	call random_below	; $5AE3
	add a,$0A	; $5AE6
	ld l,a	; $5AE8
	ld h,$00	; $5AE9
	call member_hurt	; $5AEB

loc_01_5AEE:
	inc c	; $5AEE
	djnz loc_01_5ACC	; $5AEF
	jp fn_01_4324	; $5AF1

tbl_01_5AF4:
	.dw $5AFC
	.dw $5B01
	.dw $5B07
	.dw fn_01_5B0E
	.dw fn_01_6341
	.dw fn_01_6469
	.dw fn_01_5300
	.dw fn_01_656C
	.dw fn_01_7065
	.dw fn_01_5000
	.dw fn_01_696F
	.dw $6F73
	.INCBIN "bank01.bin" SKIP $1B0C READ 2	; data, 2 bytes

fn_01_5B0E:
	ld b,d	; $5B0E
	ld l,a	; $5B0F
	ld l,l	; $5B10
	ld h,d	; $5B11
	nop	; $5B12

fn_01_5B13:
	ld a,(map_mode)	; $5B13
	cp $02	; $5B16
	jr nz,loc_01_5B2D	; $5B18
	ld a,($C990)	; $5B1A
	cp $40	; $5B1D
	jp nz,fn_01_46DF	; $5B1F
	ld hl,(party_x)	; $5B22
	call fn_01_6DC0	; $5B25
	ld (hl),$00	; $5B28
	jp loc_01_5A04	; $5B2A

loc_01_5B2D:
	ld a,(location)	; $5B2D
	cp $00	; $5B30
	jr z,loc_01_5B5B	; $5B32
	ld hl,(party_x)	; $5B34
	call fn_02_B5A4	; $5B37
	ld a,(hl)	; $5B3A
	cp $3C	; $5B3B
	jr nz,loc_01_5B5B	; $5B3D
	ld (hl),$3E	; $5B3F
	inc l	; $5B41
	ld (hl),$00	; $5B42
	ld a,$01	; $5B44
	ld c,$00	; $5B46
	call karma_lose	; $5B48
	ld a,$01	; $5B4B
	ld c,$03	; $5B4D
	call karma_lose	; $5B4F
	ld a,$01	; $5B52
	ld c,$05	; $5B54
	call karma_lose	; $5B56
	jr loc_01_5B89	; $5B59

loc_01_5B5B:
	ld hl,(party_x)	; $5B5B
	ld iy,location_people	; $5B5E
	ld de,$0008	; $5B62
	ld b,$20	; $5B65

loc_01_5B67:
	ld a,(iy+$03)	; $5B67
	cp $3C	; $5B6A
	jr nz,loc_01_5B7A	; $5B6C
	ld a,(iy+$01)	; $5B6E
	cp l	; $5B71
	jr nz,loc_01_5B7A	; $5B72
	ld a,(iy+$02)	; $5B74
	cp h	; $5B77
	jr z,loc_01_5B81	; $5B78

loc_01_5B7A:
	add iy,de	; $5B7A
	djnz loc_01_5B67	; $5B7C
	jp fn_01_46DF	; $5B7E

loc_01_5B81:
	ld (iy+$03),$00	; $5B81
	ld (iy+$00),$00	; $5B85

loc_01_5B89:
	jp loc_01_5A04	; $5B89

cast_spell:
	call task_enter_2	; $5B8C
	ld bc,$01FF	; $5B8F
	call fn_01_5D54	; $5B92
	ld (ix+$1C),c	; $5B95
	ld (ix+$1D),$00	; $5B98
	ld (ix+$1E),c	; $5B9C
	jr nc,loc_01_5BBE	; $5B9F
	call print_inline	; $5BA1
	.INCBIN "bank01.bin" SKIP $1BA4 READ 23	; text, 23 bytes
	jp loc_01_5D50	; $5BBB

loc_01_5BBE:
	ld a,(map_mode)	; $5BBE
	cp $03	; $5BC1
	jr nc,loc_01_5BF1	; $5BC3
	call print_inline	; $5BC5
	.INCBIN "bank01.bin" SKIP $1BC8 READ 9	; text, 9 bytes
	call fn_01_4BCB	; $5BD1

loc_01_5BD4:
	call frame_tick	; $5BD4
	call member_pick_step	; $5BD7
	jr z,loc_01_5BD4	; $5BDA
	jp c,loc_01_5D4D	; $5BDC
	ld a,(ix+$08)	; $5BDF
	ld ($C97F),a	; $5BE2
	ld c,a	; $5BE5
	call member_able	; $5BE6
	jr nc,loc_01_5BF1	; $5BE9
	call fn_01_46D0	; $5BEB
	jp loc_01_5D50	; $5BEE

loc_01_5BF1:
	call print_inline	; $5BF1
	.INCBIN "bank01.bin" SKIP $1BF4 READ 13	; text, 13 bytes

loc_01_5C01:
	call menu_window_clear	; $5C01
	ld de,$CA66	; $5C04
	ld c,(ix+$1C)	; $5C07
	dec c	; $5C0A
	ld b,$08	; $5C0B

loc_01_5C0D:
	inc c	; $5C0D
	ld a,c	; $5C0E
	cp $1A	; $5C0F
	jr nc,loc_01_5C5F	; $5C11
	ld a,(map_mode)	; $5C13
	ld hl,$5D72	; $5C16
	rst $20	; $5C19
	ld a,c	; $5C1A
	rst $28	; $5C1B
	or a	; $5C1C
	jr z,loc_01_5C0D	; $5C1D
	ld a,c	; $5C1F
	ld hl,mixtures	; $5C20
	rst $28	; $5C23
	or a	; $5C24
	jr z,loc_01_5C0D	; $5C25
	push af	; $5C27
	ld a,c	; $5C28
	add a,$65	; $5C29
	call item_name	; $5C2B
	push de	; $5C2E
	call fn_00_2D6C	; $5C2F
	pop de	; $5C32
	ld hl,$004C	; $5C33
	add hl,de	; $5C36
	ex de,hl	; $5C37
	pop af	; $5C38
	ld l,a	; $5C39
	ld h,$00	; $5C3A
	push bc	; $5C3C
	call fn_00_2D87	; $5C3D
	pop bc	; $5C40
	ld a,$28	; $5C41
	call fn_00_2D9A	; $5C43
	ld a,c	; $5C46
	ld hl,$5DE1	; $5C47
	rst $28	; $5C4A
	ld l,a	; $5C4B
	ld h,$00	; $5C4C
	push bc	; $5C4E
	call fn_00_2D87	; $5C4F
	pop bc	; $5C52
	ld a,$29	; $5C53
	call fn_00_2D9A	; $5C55
	ld hl,$0028	; $5C58
	add hl,de	; $5C5B
	ex de,hl	; $5C5C
	djnz loc_01_5C0D	; $5C5D

loc_01_5C5F:
	ld c,(ix+$1D)	; $5C5F
	call fn_01_49A6	; $5C62
	call menu_window_show	; $5C65

loc_01_5C68:
	call frame_tick	; $5C68
	ld a,(joy_pressed)	; $5C6B
	and $7F	; $5C6E
	ld hl,$5C78	; $5C70
	call switch_on_a	; $5C73
	jr loc_01_5C68	; $5C76

tbl_01_5C78:
	.db $01
	.dw loc_01_5C85
	.db $02
	.dw loc_01_5CB6
	.db $10
	.dw loc_01_5D4D
	.db $20
	.dw loc_01_5CE8
	.db $FF

loc_01_5C85:
	ld b,$FF	; $5C85
	ld c,(ix+$1E)	; $5C87
	call fn_01_5D54	; $5C8A
	jr c,loc_01_5C68	; $5C8D
	ld (ix+$1E),c	; $5C8F
	call fn_01_40D9	; $5C92
	ld a,(ix+$1D)	; $5C95
	or a	; $5C98
	jr z,loc_01_5CA8	; $5C99
	dec (ix+$1D)	; $5C9B
	ld c,a	; $5C9E
	call fn_01_4999	; $5C9F
	dec c	; $5CA2
	call fn_01_49A6	; $5CA3
	jr loc_01_5C68	; $5CA6

loc_01_5CA8:
	ld b,$FF	; $5CA8
	ld c,(ix+$1C)	; $5CAA
	call fn_01_5D54	; $5CAD
	ld (ix+$1C),c	; $5CB0
	jp loc_01_5C01	; $5CB3

loc_01_5CB6:
	ld b,$01	; $5CB6
	ld c,(ix+$1E)	; $5CB8
	call fn_01_5D54	; $5CBB
	jr c,loc_01_5C68	; $5CBE
	ld (ix+$1E),c	; $5CC0
	call fn_01_40D9	; $5CC3
	ld a,(ix+$1D)	; $5CC6
	cp $07	; $5CC9
	jr z,loc_01_5CDA	; $5CCB
	inc (ix+$1D)	; $5CCD
	ld c,a	; $5CD0
	call fn_01_4999	; $5CD1
	inc c	; $5CD4
	call fn_01_49A6	; $5CD5
	jr loc_01_5C68	; $5CD8

loc_01_5CDA:
	ld b,$01	; $5CDA
	ld c,(ix+$1C)	; $5CDC
	call fn_01_5D54	; $5CDF
	ld (ix+$1C),c	; $5CE2
	jp loc_01_5C01	; $5CE5

loc_01_5CE8:
	call fn_01_40C9	; $5CE8
	ld a,(ix+$1E)	; $5CEB
	add a,$65	; $5CEE
	call print_item_name	; $5CF0
	ld a,$21	; $5CF3
	call print_char	; $5CF5
	call print_newline	; $5CF8
	call fn_01_5DCE	; $5CFB
	ld a,(ix+$1E)	; $5CFE
	ld hl,mixtures	; $5D01
	rst $30	; $5D04
	dec (hl)	; $5D05
	ld a,(ix+$1E)	; $5D06
	ld hl,$5DE1	; $5D09
	rst $28	; $5D0C
	ld (ix+$1F),a	; $5D0D
	ld bc,($C97F)	; $5D10
	call member_record	; $5D14
	ld a,(iy+$09)	; $5D17
	cp (ix+$1F)	; $5D1A
	jr c,loc_01_5D28	; $5D1D
	ld a,(ix+$1E)	; $5D1F
	ld hl,$5DFB	; $5D22
	jp fn_00_0018	; $5D25

loc_01_5D28:
	call print_inline	; $5D28
	.INCBIN "bank01.bin" SKIP $1D2B READ 15	; text, 15 bytes

loc_01_5D3A:
	call fn_01_40D1	; $5D3A
	call print_inline	; $5D3D
	.INCBIN "bank01.bin" SKIP $1D40 READ 9	; text, 9 bytes

loc_01_5D49:
	or a	; $5D49
	jp task_return_2	; $5D4A

loc_01_5D4D:
	call fn_01_472D	; $5D4D

loc_01_5D50:
	scf	; $5D50
	jp task_return_2	; $5D51

fn_01_5D54:
	ld a,(map_mode)	; $5D54
	ld hl,$5D72	; $5D57
	rst $20	; $5D5A
	ex de,hl	; $5D5B

loc_01_5D5C:
	ld a,c	; $5D5C
	add a,b	; $5D5D
	ld c,a	; $5D5E
	cp $1A	; $5D5F
	ccf	; $5D61
	ret c	; $5D62
	ld h,d	; $5D63
	ld l,e	; $5D64
	rst $28	; $5D65
	or a	; $5D66
	jr z,loc_01_5D5C	; $5D67
	ld a,c	; $5D69
	ld hl,mixtures	; $5D6A
	rst $28	; $5D6D
	or a	; $5D6E
	jr z,loc_01_5D5C	; $5D6F
	ret	; $5D71

spell_allowed_by_mode:
	.dw $5D80
	.dw $5D80
	.dw $5D9A
	.dw $5DB4
	.dw $5DB4
	.dw $5DB4
	.dw $5DB4
	.dw fn_00_0101
	.dw fn_00_0101
	.INCBIN "bank01.bin" SKIP $1D84 READ 22	; data, 22 bytes

fn_01_5D9A:
	.INCBIN "bank01.bin" SKIP $1D9A READ 26	; data, 26 bytes

fn_01_5DB4:
	.INCBIN "bank01.bin" SKIP $1DB4 READ 26	; data, 26 bytes

fn_01_5DCE:
	call fn_01_4324	; $5DCE
	ld a,(map_mode)	; $5DD1
	cp $03	; $5DD4
	ret c	; $5DD6
	ld bc,($C97F)	; $5DD7
	call fn_01_49A6	; $5DDB
	jp wait_vblank	; $5DDE

spell_mp_cost:
	.INCBIN "bank01.bin" SKIP $1DE1 READ 26	; data, 26 bytes

spell_routines:
	.dw $5E2F
	.dw fn_01_5E5F
	.dw fn_01_5ECC
	.dw fn_01_5EE7
	.dw fn_01_5F65
	.dw fn_01_5FF1
	.dw fn_01_5FF7
	.dw fn_01_604D
	.dw fn_01_606C
	.dw fn_01_6072
	.dw fn_01_6077
	.dw fn_01_607D
	.dw fn_01_6092
	.dw fn_01_6098
	.dw fn_01_609D
	.dw fn_01_60C5
	.dw fn_01_60CA
	.dw fn_01_60CF
	.dw fn_01_60F2
	.dw fn_01_6125
	.dw fn_01_61BB
	.dw fn_01_61EB
	.dw fn_01_6263
	.dw fn_01_6275
	.dw fn_01_6283
	.dw fn_01_62B1
	.dw fn_00_1FCD

sub_01_5E31:
	ld h,h	; $5E31
	call fn_01_6304	; $5E32
	ld c,(ix+$08)	; $5E35
	call member_record	; $5E38
	ld a,(iy+$1C)	; $5E3B
	cp $53	; $5E3E
	jp nz,loc_01_5D3A	; $5E40
	ld (iy+$1C),$47	; $5E43
	ld a,(map_mode)	; $5E47
	cp $03	; $5E4A
	jp c,loc_01_5D49	; $5E4C
	call fn_01_42F3	; $5E4F
	ld b,a	; $5E52
	call member_object	; $5E53
	ld (iy+$02),b	; $5E56
	ld (iy+$03),b	; $5E59
	jp loc_01_5D49	; $5E5C

fn_01_5E5F:
	ld a,($C959)	; $5E5F
	cp $14	; $5E62
	jp c,loc_01_5D3A	; $5E64
	cp $18	; $5E67
	jp z,loc_01_5D3A	; $5E69
	call fn_01_6335	; $5E6C
	call fn_01_643C	; $5E6F
	call fn_01_6304	; $5E72
	ld a,(party_x)	; $5E75
	ld hl,party_y	; $5E78
	and (hl)	; $5E7B
	cp $C0	; $5E7C
	jp nc,loc_01_5D3A	; $5E7E
	ld e,(ix+$0E)	; $5E81
	ld d,(ix+$10)	; $5E84
	ld hl,(view_x)	; $5E87
	ld c,l	; $5E8A
	ld b,h	; $5E8B

loc_01_5E8C:
	ld a,l	; $5E8C
	add a,e	; $5E8D
	ld l,a	; $5E8E
	ld a,h	; $5E8F
	add a,d	; $5E90
	ld h,a	; $5E91
	ld a,l	; $5E92
	cp $20	; $5E93
	jr nc,loc_01_5E9C	; $5E95
	ld a,h	; $5E97
	cp $20	; $5E98
	jr c,loc_01_5E8C	; $5E9A

loc_01_5E9C:
	ld a,l	; $5E9C
	sub e	; $5E9D
	ld l,a	; $5E9E
	ld a,h	; $5E9F
	sub d	; $5EA0
	ld h,a	; $5EA1
	ld a,l	; $5EA2
	cp c	; $5EA3
	jr nz,loc_01_5EAB	; $5EA4
	ld a,h	; $5EA6
	cp b	; $5EA7
	jp z,loc_01_5D3A	; $5EA8

loc_01_5EAB:
	push hl	; $5EAB
	call fn_02_B5A4	; $5EAC
	ld a,(hl)	; $5EAF
	pop hl	; $5EB0
	call fn_00_3EB7	; $5EB1
	jr c,loc_01_5E9C	; $5EB4
	ld a,(party_x)	; $5EB6
	add a,l	; $5EB9
	sub c	; $5EBA
	ld (party_x),a	; $5EBB
	ld a,(party_y)	; $5EBE
	add a,h	; $5EC1
	sub b	; $5EC2
	ld (party_y),a	; $5EC3
	call recenter_map_view	; $5EC6
	jp loc_01_5D49	; $5EC9

fn_01_5ECC:
	call fn_01_641F	; $5ECC
	call fn_01_6304	; $5ECF
	ld c,(ix+$08)	; $5ED2
	call member_record	; $5ED5
	ld a,(iy+$1C)	; $5ED8
	cp $50	; $5EDB
	jp nz,loc_01_5D3A	; $5EDD
	ld (iy+$1C),$47	; $5EE0
	jp loc_01_5D49	; $5EE4

fn_01_5EE7:
	ld a,(map_mode)	; $5EE7
	ld hl,$5F15	; $5EEA
	call switch_on_a	; $5EED
	call fn_01_645C	; $5EF0
	call fn_01_6304	; $5EF3
	call fn_01_48E1	; $5EF6
	ld l,(iy+$00)	; $5EF9
	ld h,(iy+$01)	; $5EFC
	call fn_00_38F2	; $5EFF
	ld e,(hl)	; $5F02
	inc l	; $5F03
	ld d,(hl)	; $5F04
	ld a,e	; $5F05
	cp $3C	; $5F06
	jr nz,loc_01_5F0D	; $5F08
	ld de,$0016	; $5F0A

loc_01_5F0D:
	call fn_01_6493	; $5F0D
	call fn_00_38F2	; $5F10
	jr loc_01_5F3B	; $5F13

tbl_01_5F15:
	.db $00
	.dw loc_01_5F1F
	.db $01
	.dw loc_01_5F28
	.db $02
	.dw loc_01_5F4C
	.db $FF

loc_01_5F1F:
	call fn_01_643C	; $5F1F
	call fn_01_6304	; $5F22
	jp loc_01_5D3A	; $5F25

loc_01_5F28:
	call fn_01_643C	; $5F28
	call fn_01_6304	; $5F2B
	call fn_01_646C	; $5F2E
	jp nc,loc_01_5D3A	; $5F31
	call fn_02_B5A4	; $5F34
	ld de,($C9BB)	; $5F37

loc_01_5F3B:
	ld a,(hl)	; $5F3B
	cp $44	; $5F3C
	jp c,loc_01_5D3A	; $5F3E
	cp $48	; $5F41
	jp nc,loc_01_5D3A	; $5F43
	ld (hl),e	; $5F46
	inc l	; $5F47
	ld (hl),d	; $5F48
	jp loc_01_5D49	; $5F49

loc_01_5F4C:
	call fn_01_643C	; $5F4C
	call fn_01_6304	; $5F4F
	call fn_01_6480	; $5F52
	call fn_01_6DC0	; $5F55
	ld a,(hl)	; $5F58
	and $F0	; $5F59
	cp $A0	; $5F5B
	jp nz,loc_01_5D3A	; $5F5D
	ld (hl),$00	; $5F60
	jp loc_01_5D49	; $5F62

fn_01_5F65:
	ld hl,$64CE	; $5F65
	call menu_draw	; $5F68
	call print_inline	; $5F6B
	.INCBIN "bank01.bin" SKIP $1F6E READ 14	; text, 14 bytes
	ld (ix+$1A),$00	; $5F7C

loc_01_5F80:
	call frame_tick	; $5F80
	ld c,$04	; $5F83
	ld de,$0102	; $5F85
	call menu_step_wrap	; $5F88
	jr z,loc_01_5F80	; $5F8B
	jp c,loc_01_5D4D	; $5F8D
	ld a,(ix+$1A)	; $5F90
	ld (ix+$07),a	; $5F93
	ld hl,$64C6	; $5F96
	rst $20	; $5F99
	call print_text_hl	; $5F9A
	call print_newline	; $5F9D
	ld a,(map_mode)	; $5FA0
	cp $03	; $5FA3
	jr nc,loc_01_5FC7	; $5FA5
	cp $02	; $5FA7
	jp nz,loc_01_5D3A	; $5FA9
	call fn_01_643C	; $5FAC
	call fn_01_6304	; $5FAF
	call fn_01_6480	; $5FB2
	call fn_01_6DC0	; $5FB5
	ld a,(hl)	; $5FB8
	cp $00	; $5FB9
	jp nz,loc_01_5D3A	; $5FBB
	ld a,(ix+$07)	; $5FBE
	or $A0	; $5FC1
	ld (hl),a	; $5FC3
	jp loc_01_5D49	; $5FC4

loc_01_5FC7:
	call fn_01_645C	; $5FC7
	call fn_01_6304	; $5FCA
	call fn_01_6493	; $5FCD
	call fn_00_38F2	; $5FD0
	ld a,(hl)	; $5FD3
	call fn_00_3EB7	; $5FD4
	jp c,loc_01_5D3A	; $5FD7
	ex de,hl	; $5FDA
	ld a,(ix+$07)	; $5FDB
	ld hl,$5FE9	; $5FDE
	rst $20	; $5FE1
	ex de,hl	; $5FE2
	ld (hl),e	; $5FE3
	inc l	; $5FE4
	ld (hl),d	; $5FE5
	jp loc_01_5D49	; $5FE6
	.INCBIN "bank01.bin" SKIP $1FE9 READ 8	; data, 8 bytes

fn_01_5FF1:
	ld bc,$4F08	; $5FF1
	jp loc_01_6383	; $5FF4

fn_01_5FF7:
	ld a,($C959)	; $5FF7
	cp $14	; $5FFA
	jp c,loc_01_5D3A	; $5FFC
	cp $18	; $5FFF
	jp z,loc_01_5D3A	; $6001
	call fn_01_6335	; $6004
	ld hl,$64A5	; $6007
	call menu_draw	; $600A
	call print_inline	; $600D
	.INCBIN "bank01.bin" SKIP $2010 READ 11	; text, 11 bytes
	ld (ix+$1A),$00	; $601B

loc_01_601F:
	call frame_tick	; $601F
	ld c,$08	; $6022
	ld de,$0102	; $6024
	call menu_step_wrap	; $6027
	jr z,loc_01_601F	; $602A
	jp c,loc_01_5D4D	; $602C
	ld a,(ix+$1A)	; $602F
	add a,$31	; $6032
	call print_char	; $6034
	call print_newline	; $6037
	call fn_01_6304	; $603A
	ld a,(ix+$1A)	; $603D
	ld hl,$7CA3	; $6040
	rst $20	; $6043
	ld (party_x),hl	; $6044
	call recenter_map_view	; $6047
	jp loc_01_5D49	; $604A

fn_01_604D:
	call fn_01_641F	; $604D
	call fn_01_6304	; $6050
	ld c,(ix+$08)	; $6053
	call member_can_act	; $6056
	jp c,loc_01_5D3A	; $6059
	ld a,$19	; $605C
	call random_below	; $605E
	add a,$4B	; $6061
	ld l,a	; $6063
	ld h,$00	; $6064
	call member_heal	; $6066
	jp loc_01_5D49	; $6069

fn_01_606C:
	ld bc,$4E09	; $606C
	jp loc_01_6383	; $606F

fn_01_6072:
	ld a,$4A	; $6072
	jp loc_01_640B	; $6074

fn_01_6077:
	ld bc,$8C0A	; $6077
	jp loc_01_6383	; $607A

fn_01_607D:
	call fn_01_6369	; $607D
	call fn_01_6304	; $6080
	ld a,($C95B)	; $6083
	add a,$C8	; $6086
	jr nc,loc_01_608C	; $6088
	ld a,$FF	; $608A

loc_01_608C:
	ld ($C95B),a	; $608C
	jp loc_01_5D49	; $608F

fn_01_6092:
	ld bc,$4D07	; $6092
	jp loc_01_6383	; $6095

fn_01_6098:
	ld a,$4E	; $6098
	jp loc_01_640B	; $609A

fn_01_609D:
	call fn_01_6304	; $609D
	ld a,(map_mode)	; $60A0
	cp $00	; $60A3
	jr nz,loc_01_60AF	; $60A5
	ld a,($C959)	; $60A7
	cp $18	; $60AA
	jp z,loc_01_5D3A	; $60AC

loc_01_60AF:
	ld a,(map_mode)	; $60AF
	cp $03	; $60B2
	jr c,loc_01_60BB	; $60B4
	call fn_01_5978	; $60B6
	jr loc_01_60C2	; $60B9

loc_01_60BB:
	ld (ix+$08),$FF	; $60BB
	call fn_01_5B13	; $60BF

loc_01_60C2:
	jp loc_01_5D49	; $60C2

fn_01_60C5:
	ld a,$50	; $60C5
	jp loc_01_640B	; $60C7

fn_01_60CA:
	ld a,$51	; $60CA
	jp loc_01_640B	; $60CC

fn_01_60CF:
	ld a,(map_mode)	; $60CF
	cp $03	; $60D2
	jp nc,loc_01_5D3A	; $60D4
	call fn_01_641F	; $60D7
	call fn_01_6304	; $60DA
	ld c,(ix+$08)	; $60DD
	call member_record	; $60E0
	ld a,(iy+$1C)	; $60E3
	cp $44	; $60E6
	jp nz,loc_01_5D3A	; $60E8
	ld (iy+$1C),$47	; $60EB
	jp loc_01_5D49	; $60EF

fn_01_60F2:
	call fn_01_6350	; $60F2
	call fn_01_6304	; $60F5
	ld iy,room_monsters	; $60F8
	ld de,$0008	; $60FC
	ld b,$10	; $60FF

loc_01_6101:
	ld a,(iy+$05)	; $6101
	or a	; $6104
	jr z,loc_01_611E	; $6105
	call fn_01_62F8	; $6107
	jr z,loc_01_611E	; $610A
	cp $FC	; $610C
	jr z,loc_01_611E	; $610E
	call random	; $6110
	cp (iy+$04)	; $6113
	jr c,loc_01_611E	; $6116
	jr z,loc_01_611E	; $6118
	ld (iy+$07),$01	; $611A

loc_01_611E:
	add iy,de	; $611E
	djnz loc_01_6101	; $6120
	jp loc_01_5D49	; $6122

fn_01_6125:
	call fn_01_6350	; $6125
	call fn_01_6304	; $6128
	ld a,$16	; $612B
	call fn_00_05F1	; $612D
	call fn_01_4117	; $6130
	ld (ix+$08),$00	; $6133
	ld (ix+$16),$10	; $6137

loc_01_613B:
	ld c,(ix+$08)	; $613B
	call fn_01_48EB	; $613E
	ld a,(iy+$05)	; $6141
	or a	; $6144
	jr z,loc_01_61A4	; $6145
	ld a,(iy+$04)	; $6147
	cp $C0	; $614A
	jr nc,loc_01_61A4	; $614C
	call random	; $614E
	and $01	; $6151
	jr z,loc_01_6169	; $6153
	call random	; $6155
	and $01	; $6158
	jr z,loc_01_61A4	; $615A
	ld a,(iy+$04)	; $615C
	cp $17	; $615F
	jr c,loc_01_61A4	; $6161
	ld (iy+$04),$17	; $6163
	jr loc_01_61A4	; $6167

loc_01_6169:
	ld a,(iy+$00)	; $6169
	call fn_00_36F7	; $616C
	ld (ix+$0B),a	; $616F
	ld a,(iy+$01)	; $6172
	call fn_00_36F7	; $6175
	ld (ix+$0D),a	; $6178
	ld a,$14	; $617B
	call fn_00_05F1	; $617D
	ld a,$04	; $6180
	call fn_01_61AF	; $6182
	ld a,$05	; $6185
	call fn_01_61AF	; $6187
	ld a,$06	; $618A
	call fn_01_61AF	; $618C
	ld a,($C97F)	; $618F
	ld b,a	; $6192
	ld c,(ix+$08)	; $6193
	ld a,$FF	; $6196
	call damage_monster	; $6198
	call print_newline	; $619B
	call fn_00_129D	; $619E
	call yield	; $61A1

loc_01_61A4:
	inc (ix+$08)	; $61A4
	dec (ix+$16)	; $61A7
	jr nz,loc_01_613B	; $61AA
	jp loc_01_5D49	; $61AC

fn_01_61AF:
	ld e,(ix+$0B)	; $61AF
	ld d,(ix+$0D)	; $61B2
	call fn_00_3082	; $61B5
	jp yield	; $61B8

fn_01_61BB:
	call fn_01_6350	; $61BB
	call fn_01_6304	; $61BE
	ld iy,room_monsters	; $61C1
	ld de,$0008	; $61C5
	ld b,$10	; $61C8

loc_01_61CA:
	ld a,(iy+$05)	; $61CA
	call fn_01_62F8	; $61CD
	jr nz,loc_01_61E4	; $61D0
	call random	; $61D2
	and $01	; $61D5
	jr z,loc_01_61E4	; $61D7
	ld a,(iy+$04)	; $61D9
	cp $17	; $61DC
	jr c,loc_01_61E4	; $61DE
	ld (iy+$04),$17	; $61E0

loc_01_61E4:
	add iy,de	; $61E4
	djnz loc_01_61CA	; $61E6
	jp loc_01_5D49	; $61E8

fn_01_61EB:
	call fn_01_6304	; $61EB
	ld a,(map_mode)	; $61EE
	cp $03	; $61F1
	jp nc,loc_01_5D3A	; $61F3
	call fn_01_61FC	; $61F6
	jp loc_01_5D49	; $61F9

fn_01_61FC:
	call task_enter_3	; $61FC
	ld a,$13	; $61FF
	ld hl,$B16F	; $6201
	ld de,$3E60	; $6204
	call fn_00_0788	; $6207
	ld hl,$031F	; $620A
	call set_palette_entry	; $620D
	call fn_01_4F8A	; $6210
	call print_inline	; $6213
	.INCBIN "bank01.bin" SKIP $2216 READ 12	; text, 12 bytes

loc_01_6222:
	call yield	; $6222
	ld a,(joy_pressed)	; $6225
	and $30	; $6228
	jr nz,loc_01_6257	; $622A
	call fn_00_335C	; $622C
	ld a,(map_mode)	; $622F
	cp $02	; $6232
	ld de,$4949	; $6234
	jr z,loc_01_6249	; $6237
	ld a,(view_x)	; $6239
	add a,a	; $623C
	add a,a	; $623D
	add a,$09	; $623E
	ld e,a	; $6240
	ld a,(view_y)	; $6241
	add a,a	; $6244
	add a,a	; $6245
	add a,$09	; $6246
	ld d,a	; $6248

loc_01_6249:
	ld c,$F3	; $6249
	inc (ix+$1F)	; $624B
	bit 1,(ix+$1F)	; $624E
	call nz,fn_00_09DA	; $6252
	jr loc_01_6222	; $6255

loc_01_6257:
	call fn_00_276D	; $6257
	call fn_00_129D	; $625A
	call print_newline	; $625D
	jp task_return_3	; $6260

fn_01_6263:
	call fn_01_6335	; $6263
	call fn_01_643C	; $6266
	call fn_01_6304	; $6269
	ld a,(ix+$1A)	; $626C
	ld ($C98A),a	; $626F
	jp loc_01_5D49	; $6272

fn_01_6275:
	call fn_01_6369	; $6275
	call fn_01_6304	; $6278
	ld a,$FF	; $627B
	ld ($C970),a	; $627D
	jp loc_01_5D49	; $6280

fn_01_6283:
	call fn_01_6369	; $6283
	call fn_01_6304	; $6286
	ld a,(location)	; $6289
	cp $19	; $628C
	jp z,loc_01_5D3A	; $628E
	ld hl,$C970	; $6291
	dec (hl)	; $6294
	jp m,loc_01_5D49	; $6295
	ld b,$20	; $6298

loc_01_629A:
	call fn_01_62E4	; $629A
	jr z,loc_01_62A8	; $629D
	djnz loc_01_629A	; $629F
	ld hl,$C970	; $62A1
	inc (hl)	; $62A4
	jp loc_01_5D3A	; $62A5

loc_01_62A8:
	ld (party_x),hl	; $62A8
	call fn_00_129D	; $62AB
	jp loc_01_5D49	; $62AE

fn_01_62B1:
	call fn_01_6369	; $62B1
	call fn_01_6304	; $62B4
	ld a,(location)	; $62B7
	cp $19	; $62BA
	jp z,loc_01_5D3A	; $62BC
	ld a,($C970)	; $62BF
	cp $07	; $62C2
	jp z,loc_01_5D3A	; $62C4
	inc a	; $62C7
	ld ($C970),a	; $62C8
	ld b,$20	; $62CB

loc_01_62CD:
	call fn_01_62E4	; $62CD
	jr z,loc_01_62DB	; $62D0
	djnz loc_01_62CD	; $62D2
	ld hl,$C970	; $62D4
	dec (hl)	; $62D7
	jp loc_01_5D3A	; $62D8

loc_01_62DB:
	ld (party_x),hl	; $62DB
	call fn_00_129D	; $62DE
	jp loc_01_5D49	; $62E1

fn_01_62E4:
	call random	; $62E4
	and $0F	; $62E7
	ld l,a	; $62E9
	call random	; $62EA
	and $0F	; $62ED
	ld h,a	; $62EF
	push hl	; $62F0
	call fn_01_6DC0	; $62F1
	ld a,(hl)	; $62F4
	pop hl	; $62F5
	or a	; $62F6
	ret	; $62F7

fn_01_62F8:
	cp $9C	; $62F8
	ret z	; $62FA
	cp $BC	; $62FB
	ret z	; $62FD
	cp $C4	; $62FE
	ret z	; $6300
	cp $E4	; $6301
	ret	; $6303

fn_01_6304:
	ld bc,($C97F)	; $6304
	call member_record	; $6308
	ld a,(iy+$09)	; $630B
	sub (ix+$1F)	; $630E
	ld (iy+$09),a	; $6311
	call fn_01_5DCE	; $6314
	ld a,$15	; $6317
	call fn_00_05F1	; $6319
	call wait_vblank	; $631C
	call fn_01_41A9	; $631F
	call wait_vblank	; $6322
	call fn_01_41A9	; $6325
	call wait_vblank	; $6328
	ld a,($C976)	; $632B
	cp $4E	; $632E
	ret nz	; $6330

loc_01_6331:
	pop af	; $6331
	jp loc_01_5D3A	; $6332

fn_01_6335:
	ld a,(map_mode)	; $6335
	cp $00	; $6338
	ret z	; $633A
	call print_inline	; $633B
	.INCBIN "bank01.bin" SKIP $233E READ 3	; data, 3 bytes

fn_01_6341:
	ld h,h	; $6341
	ld l,a	; $6342
	ld l,a	; $6343
	ld (hl),d	; $6344
	ld (hl),e	; $6345
	jr nz,loc_01_6397	; $6346
	ld l,(hl)	; $6348
	ld l,h	; $6349
	ld a,c	; $634A
	ld hl,$0001	; $634B
	jr loc_01_6331	; $634E

fn_01_6350:
	ld a,(map_mode)	; $6350
	cp $03	; $6353
	ret nc	; $6355
	call print_inline	; $6356
	.INCBIN "bank01.bin" SKIP $2359 READ 14	; text, 14 bytes
	jr loc_01_6331	; $6367

fn_01_6369:
	ld a,(map_mode)	; $6369
	cp $02	; $636C
	ret z	; $636E
	call print_inline	; $636F
	.INCBIN "bank01.bin" SKIP $2372 READ 15	; text, 15 bytes
	jr loc_01_6331	; $6381

loc_01_6383:
	ld (ix+$22),c	; $6383
	ld (ix+$23),b	; $6386
	call fn_01_6350	; $6389
	call fn_01_64F4	; $638C
	jp c,loc_01_5D50	; $638F
	call fn_00_37DD	; $6392
	.INCBIN "bank01.bin" SKIP $2395 READ 2	; data, 2 bytes

loc_01_6397:
	ld e,l	; $6397
	call fn_01_6304	; $6398
	call fn_01_48E1	; $639B
	ld l,(iy+$00)	; $639E
	ld h,(iy+$01)	; $63A1
	ld e,(ix+$20)	; $63A4
	ld d,(ix+$21)	; $63A7
	ld a,(ix+$22)	; $63AA
	call fn_00_38FC	; $63AD

loc_01_63B0:
	call fn_00_392F	; $63B0
	call fn_00_38BD	; $63B3
	jr nc,loc_01_63C3	; $63B6
	call fn_00_38EB	; $63B8
	call fn_00_3974	; $63BB
	jr nc,loc_01_63B0	; $63BE
	jp loc_01_5D3A	; $63C0

loc_01_63C3:
	ld (ix+$08),a	; $63C3
	call fn_00_3A91	; $63C6
	ld a,(ix+$23)	; $63C9
	ld hl,$63E6	; $63CC
	call switch_on_a	; $63CF
	ld a,$E8	; $63D2

loc_01_63D4:
	push af	; $63D4
	ld a,($C97F)	; $63D5
	ld b,a	; $63D8
	ld c,(ix+$08)	; $63D9
	pop af	; $63DC
	call damage_monster	; $63DD
	call print_newline	; $63E0
	jp loc_01_5D49	; $63E3

tbl_01_63E6:
	.db $4D
	.dw loc_01_63F0
	.db $4E
	.dw loc_01_63F9
	.db $4F
	.dw loc_01_6402
	.db $FF

loc_01_63F0:
	ld a,$40	; $63F0
	call random_below	; $63F2
	or $10	; $63F5
	jr loc_01_63D4	; $63F7

loc_01_63F9:
	ld a,$E0	; $63F9
	call random_below	; $63FB
	or $20	; $63FE
	jr loc_01_63D4	; $6400

loc_01_6402:
	ld a,$80	; $6402
	call random_below	; $6404
	or $18	; $6407
	jr loc_01_63D4	; $6409

loc_01_640B:
	ld (ix+$1B),a	; $640B
	call fn_01_6304	; $640E
	ld a,(ix+$1B)	; $6411
	ld ($C976),a	; $6414
	ld a,$0A	; $6417
	ld ($C977),a	; $6419
	jp loc_01_5D49	; $641C

fn_01_641F:
	call task_enter_3	; $641F
	call print_inline	; $6422
	.INCBIN "bank01.bin" SKIP $2425 READ 6	; text, 6 bytes
	call fn_01_4BCB	; $642B

loc_01_642E:
	call frame_tick	; $642E
	call member_pick_step	; $6431
	jr z,loc_01_642E	; $6434
	jp c,loc_01_5D4D	; $6436
	jp task_return_3	; $6439

fn_01_643C:
	call task_enter_3	; $643C
	call fn_01_4C28	; $643F

loc_01_6442:
	call frame_tick	; $6442
	call fn_01_4C3C	; $6445
	jr z,loc_01_6442	; $6448

loc_01_644A:
	jp c,loc_01_5D4D	; $644A
	ld a,(ix+$1A)	; $644D
	call fn_01_40B9	; $6450
	ld (ix+$0E),l	; $6453
	ld (ix+$10),h	; $6456
	jp task_return_3	; $6459

fn_01_645C:
	call task_enter_3	; $645C
	call fn_01_4C28	; $645F
	call frame_tick	; $6462
	call fn_01_4C8F	; $6465
	.INCBIN "bank01.bin" SKIP $2468 READ 1	; data, 1 bytes

fn_01_6469:
	ret m	; $6469
	jr loc_01_644A	; $646A

fn_01_646C:
	ld a,(party_x)	; $646C
	add a,(ix+$0E)	; $646F
	ld l,a	; $6472
	cp $20	; $6473
	ret nc	; $6475
	ld a,(party_y)	; $6476
	add a,(ix+$10)	; $6479
	ld h,a	; $647C
	cp $20	; $647D
	ret	; $647F

fn_01_6480:
	ld a,(party_x)	; $6480
	add a,(ix+$0E)	; $6483
	and $0F	; $6486
	ld l,a	; $6488
	ld a,(party_y)	; $6489
	add a,(ix+$10)	; $648C
	and $0F	; $648F
	ld h,a	; $6491
	ret	; $6492

fn_01_6493:
	call fn_01_48E1	; $6493
	ld a,(iy+$00)	; $6496
	add a,(ix+$0E)	; $6499
	ld l,a	; $649C
	ld a,(iy+$01)	; $649D
	add a,(ix+$10)	; $64A0
	ld h,a	; $64A3
	ret	; $64A4
	.INCBIN "bank01.bin" SKIP $24A5 READ 33	; data, 33 bytes

tbl_01_64C6:
	.dw $64D0
	.dw fn_01_64D9
	.dw fn_01_64E5
	.dw fn_01_64ED
	.dw fn_00_0213
	.dw $6F50
	.dw fn_01_7369
	.dw $6E6F
	.INCBIN "bank01.bin" SKIP $24D6 READ 3	; data, 3 bytes

fn_01_64D9:
	ld c,h	; $64D9
	ld l,c	; $64DA
	ld h,a	; $64DB
	ld l,b	; $64DC
	ld (hl),h	; $64DD
	ld l,(hl)	; $64DE
	ld l,c	; $64DF
	ld l,(hl)	; $64E0
	ld h,a	; $64E1
	nop	; $64E2
	inc de	; $64E3
	.INCBIN "bank01.bin" SKIP $24E4 READ 1	; data, 1 bytes

fn_01_64E5:
	ld b,(hl)	; $64E5
	ld l,h	; $64E6
	ld h,c	; $64E7
	ld l,l	; $64E8
	ld h,l	; $64E9
	nop	; $64EA
	inc de	; $64EB
	ex af,af'	; $64EC

fn_01_64ED:
	ld d,e	; $64ED
	ld l,h	; $64EE
	ld h,l	; $64EF
	ld h,l	; $64F0
	ld (hl),b	; $64F1
	nop	; $64F2
	rst $38	; $64F3

fn_01_64F4:
	ld hl,$C83E	; $64F4
	ld de,$0008	; $64F7
	ld b,$10	; $64FA
	xor a	; $64FC

loc_01_64FD:
	or (hl)	; $64FD
	add hl,de	; $64FE
	djnz loc_01_64FD	; $64FF
	ret nz	; $6501
	call fn_01_40D1	; $6502
	call print_inline	; $6505
	.INCBIN "bank01.bin" SKIP $2508 READ 20	; text, 20 bytes
	scf	; $651C
	ret	; $651D

fn_01_651E:
	call task_enter_2	; $651E
	ld a,$1A	; $6521
	ld (message_bank),a	; $6523
	ld hl,$8000	; $6526
	ld (message_table),hl	; $6529
	ld b,$01	; $652C
	ld hl,$C63D	; $652E
	ld a,($C955)	; $6531
	or a	; $6534
	jr z,loc_01_6542	; $6535
	call fn_01_683C	; $6537
	ld a,$00	; $653A
	jr z,loc_01_653F	; $653C
	inc a	; $653E

loc_01_653F:
	call fn_01_6838	; $653F

loc_01_6542:
	ld a,($C951)	; $6542
	and $04	; $6545
	ld a,$02	; $6547
	call nz,fn_01_6838	; $6549
	ld a,($C951)	; $654C
	and $08	; $654F
	ld a,$03	; $6551
	call nz,fn_01_6838	; $6553
	ld a,($C951)	; $6556
	and $10	; $6559
	ld a,$04	; $655B
	call nz,fn_01_6838	; $655D
	ld a,($C951)	; $6560
	and $E0	; $6563

fn_01_6565:
	jr z,loc_01_6572	; $6565
	call fn_01_683C	; $6567
	ld a,$05	; $656A

fn_01_656C:
	jr z,loc_01_656F	; $656C
	inc a	; $656E

loc_01_656F:
	call fn_01_6838	; $656F

loc_01_6572:
	ld a,($C952)	; $6572
	and $01	; $6575
	ld a,$07	; $6577
	call nz,fn_01_6838	; $6579
	ld a,($C952)	; $657C
	and $02	; $657F
	ld a,$08	; $6581
	call nz,fn_01_6838	; $6583
	ld a,($C951)	; $6586
	and $01	; $6589
	ld a,$09	; $658B
	call nz,fn_01_6838	; $658D
	djnz loc_01_6596	; $6590
	ld a,$01	; $6592
	jr loc_01_65C1	; $6594

loc_01_6596:
	ld (ix+$1B),b	; $6596
	ld a,$1A	; $6599
	call print_message	; $659B
	ld hl,$68F5	; $659E
	call fn_01_6851	; $65A1
	jr c,loc_01_65CA	; $65A4
	ld hl,$65AC	; $65A6
	jp fn_00_0018	; $65A9

tbl_01_65AC:
	.dw fn_01_65CD
	.dw fn_01_65CD
	.dw fn_01_66F4
	.dw fn_01_6704
	.dw fn_01_671C
	.dw fn_01_6747
	.dw fn_01_6747
	.dw fn_01_674C
	.dw fn_01_6765
	.dw fn_01_6789

loc_01_65C0:
	xor a	; $65C0

loc_01_65C1:
	call fn_01_40D1	; $65C1

loc_01_65C4:
	call print_message	; $65C4
	call print_newline	; $65C7

loc_01_65CA:
	jp task_return_2	; $65CA

fn_01_65CD:
	ld a,(map_mode)	; $65CD
	cp $02	; $65D0
	jr z,loc_01_65DE	; $65D2
	cp $05	; $65D4
	jp z,$6662	; $65D6
	ld a,$0D	; $65D9
	jp loc_01_65C1	; $65DB

loc_01_65DE:
	ld a,(location)	; $65DE
	cp $19	; $65E1
	jp nz,loc_01_65C0	; $65E3
	ld a,($C990)	; $65E6
	cp $B0	; $65E9
	jp nz,loc_01_65C0	; $65EB
	ld a,($C970)	; $65EE
	cp $07	; $65F1
	ld a,$0E	; $65F3
	call nz,print_message	; $65F5
	ld a,($C970)	; $65F8
	add a,$05	; $65FB
	call print_message	; $65FD
	ld a,$0F	; $6600
	call print_message	; $6602
	call print_newline	; $6605
	ld hl,$C63D	; $6608
	ld bc,$0800	; $660B

loc_01_660E:
	ld (hl),c	; $660E
	inc hl	; $660F
	inc c	; $6610
	djnz loc_01_660E	; $6611
	ld a,$08	; $6613
	call fn_01_68D7	; $6615
	ld (ix+$1B),$08	; $6618
	ld hl,$6941	; $661C
	call fn_01_6851	; $661F
	jp c,loc_01_65CA	; $6622
	ld hl,$C970	; $6625
	cp (hl)	; $6628
	jp nz,loc_01_65C0	; $6629
	ld a,$10	; $662C
	call print_message	; $662E
	call fn_01_68B8	; $6631
	call fn_01_6851	; $6634
	jp c,loc_01_65CA	; $6637
	ld hl,$C970	; $663A
	cp (hl)	; $663D
	jp nz,loc_01_65C0	; $663E
	ld a,($C970)	; $6641
	cp $07	; $6644
	jr z,loc_01_665A	; $6646
	ld hl,(party_x)	; $6648
	call fn_01_6DC0	; $664B
	ld (hl),$20	; $664E
	ld a,$FF	; $6650
	ld ($C9A1),a	; $6652
	ld a,$12	; $6655
	jp loc_01_65C4	; $6657

loc_01_665A:
	ld a,$18	; $665A
	ld (game_mode),a	; $665C
	.INCBIN "bank01.bin" SKIP $265F READ 1	; data, 1 bytes

fn_01_6660:
	jp nc,$3A0C	; $6660
	add a,h	; $6663
	ret	; $6664

sub_01_6665:
	cp $FF	; $6665
	jp z,loc_01_65C0	; $6667
	call fn_01_48E1	; $666A
	ld a,(iy+$00)	; $666D
	cp (iy+$01)	; $6670
	jp nz,loc_01_65C0	; $6673
	cp $05	; $6676
	jp nz,loc_01_65C0	; $6678
	ld a,$02	; $667B
	call print_message	; $667D
	call print_newline	; $6680
	call fn_01_68B8	; $6683
	ld (ix+$16),$04	; $6686
	ld (ix+$17),$00	; $668A
	ld (ix+$1A),$00	; $668E

loc_01_6692:
	ld a,$45	; $6692
	sub (ix+$16)	; $6694
	call print_char	; $6697
	call print_inline	; $669A
	.INCBIN "bank01.bin" SKIP $269D READ 4	; text, 4 bytes
	ld hl,$6998	; $66A1
	call fn_01_6855	; $66A4
	jp c,loc_01_65CA	; $66A7
	ld hl,$6849	; $66AA
	rst $28	; $66AD
	ld b,a	; $66AE
	and (ix+$17)	; $66AF
	ld a,$03	; $66B2
	jp nz,loc_01_65C1	; $66B4
	ld a,b	; $66B7
	or (ix+$17)	; $66B8
	ld (ix+$17),a	; $66BB
	dec (ix+$16)	; $66BE
	jr nz,loc_01_6692	; $66C1
	ld a,($C984)	; $66C3
	ld hl,$66F1	; $66C6
	rst $28	; $66C9
	cp (ix+$17)	; $66CA
	jp nz,loc_01_65C0	; $66CD
	ld a,($C984)	; $66D0
	ld hl,$684E	; $66D3
	rst $28	; $66D6
	ld b,a	; $66D7
	ld hl,$C951	; $66D8
	ld a,(hl)	; $66DB
	and b	; $66DC
	jp nz,loc_01_65C0	; $66DD
	ld a,(hl)	; $66E0
	or b	; $66E1
	ld (hl),a	; $66E2
	ld a,$04	; $66E3
	call print_message	; $66E5
	call wait_button	; $66E8
	call print_newline	; $66EB
	jp loc_01_65CA	; $66EE
	.INCBIN "bank01.bin" SKIP $26F1 READ 3	; data, 3 bytes

fn_01_66F4:
	call fn_01_6827	; $66F4
	jp nz,loc_01_65C0	; $66F7
	ld hl,$C952	; $66FA
	set 4,(hl)	; $66FD
	ld a,$13	; $66FF
	jp loc_01_65C4	; $6701

fn_01_6704:
	call fn_01_6827	; $6704
	jp nz,loc_01_65C0	; $6707
	ld a,($C952)	; $670A
	and $10	; $670D
	jp z,loc_01_65C0	; $670F
	ld hl,$C952	; $6712
	set 3,(hl)	; $6715
	ld a,$14	; $6717
	jp loc_01_65C4	; $6719

fn_01_671C:
	call fn_01_6827	; $671C
	jp nz,loc_01_65C0	; $671F
	ld a,($C952)	; $6722
	and $08	; $6725
	jp z,loc_01_65C0	; $6727
	ld hl,$C952	; $672A
	set 2,(hl)	; $672D
	ld a,$15	; $672F
	call print_message	; $6731
	ld a,$1C	; $6734
	call fn_00_05F1	; $6736
	call fn_01_4117	; $6739
	ld a,$FF	; $673C
	ld ($C9A1),a	; $673E
	call print_newline	; $6741
	jp loc_01_65CA	; $6744

fn_01_6747:
	ld a,$0D	; $6747
	jp loc_01_65C1	; $6749

fn_01_674C:
	ld a,(location)	; $674C
	or a	; $674F
	jp nz,loc_01_65C0	; $6750
	ld a,$01	; $6753
	ld ($C976),a	; $6755
	ld a,$0A	; $6758
	ld ($C977),a	; $675A
	call fn_01_4324	; $675D
	ld a,$16	; $6760
	jp loc_01_65C4	; $6762

fn_01_6765:
	ld a,(location)	; $6765
	or a	; $6768
	jp nz,loc_01_65C0	; $6769
	ld a,($C959)	; $676C
	cp $14	; $676F
	jp nc,loc_01_65C0	; $6771
	ld a,($C961)	; $6774
	cp $32	; $6777
	jp nz,loc_01_65C0	; $6779
	ld a,$63	; $677C
	ld ($C961),a	; $677E
	call fn_01_4324	; $6781
	ld a,$17	; $6784
	jp loc_01_65C4	; $6786

fn_01_6789:
	call fn_01_6827	; $6789
	jr z,loc_01_67F0	; $678C
	ld a,$18	; $678E
	call print_message	; $6790
	call fn_01_6813	; $6793
	ld iy,location_people	; $6796
	ld de,$0008	; $679A
	ld a,(location)	; $679D
	or a	; $67A0
	ld b,$20	; $67A1
	jr nz,loc_01_67A7	; $67A3
	ld b,$08	; $67A5

loc_01_67A7:
	ld a,(iy+$03)	; $67A7
	cp $5E	; $67AA
	jr nz,loc_01_67B8	; $67AC
	ld (iy+$06),$FF	; $67AE
	ld (iy+$07),$00	; $67B2
	jr loc_01_67C0	; $67B6

loc_01_67B8:
	ld (iy+$03),$00	; $67B8
	ld (iy+$00),$00	; $67BC

loc_01_67C0:
	add iy,de	; $67C0
	djnz loc_01_67A7	; $67C2
	ld iy,room_monsters	; $67C4
	ld de,$0008	; $67C8
	ld b,$10	; $67CB

loc_01_67CD:
	ld a,(iy+$05)	; $67CD
	cp $5E	; $67D0
	jr z,loc_01_67DC	; $67D2
	ld (iy+$05),$00	; $67D4
	ld (iy+$06),$00	; $67D8

loc_01_67DC:
	add iy,de	; $67DC
	djnz loc_01_67CD	; $67DE
	call fn_00_129D	; $67E0
	ld a,$05	; $67E3
	ld bc,$0800	; $67E5

loc_01_67E8:
	call karma_lose	; $67E8
	inc c	; $67EB
	djnz loc_01_67E8	; $67EC
	jr loc_01_680A	; $67EE

loc_01_67F0:
	ld a,$19	; $67F0
	call print_message	; $67F2
	call fn_01_6813	; $67F5
	ld hl,$C951	; $67F8
	res 0,(hl)	; $67FB
	set 1,(hl)	; $67FD
	ld a,$0A	; $67FF
	ld bc,$0800	; $6801

loc_01_6804:
	call karma_gain	; $6804
	inc c	; $6807
	djnz loc_01_6804	; $6808

loc_01_680A:
	call fn_01_4324	; $680A
	call print_newline	; $680D
	jp loc_01_65CA	; $6810

fn_01_6813:
	call fn_01_681F	; $6813
	call fn_01_41EC	; $6816
	call fn_01_681F	; $6819
	call fn_01_41EC	; $681C

fn_01_681F:
	ld a,$1C	; $681F
	call fn_00_05F1	; $6821
	jp fn_01_4117	; $6824

fn_01_6827:
	ld a,(location)	; $6827
	or a	; $682A
	ret nz	; $682B
	ld a,(party_x)	; $682C
	cp $E9	; $682F
	ret nz	; $6831
	ld a,(party_y)	; $6832
	cp $E9	; $6835
	ret	; $6837

fn_01_6838:
	ld (hl),a	; $6838
	inc hl	; $6839
	inc b	; $683A
	ret	; $683B

fn_01_683C:
	push bc	; $683C
	push hl	; $683D
	ld hl,$6849	; $683E
	ld bc,$0008	; $6841
	cpir	; $6844
	pop hl	; $6846
	pop bc	; $6847
	ret	; $6848
	.INCBIN "bank01.bin" SKIP $2849 READ 8	; data, 8 bytes

fn_01_6851:
	ld (ix+$1A),$00	; $6851

fn_01_6855:
	ld (ix+$1C),l	; $6855
	ld (ix+$1D),h	; $6858
	call task_enter_3	; $685B
	call menu_window_clear	; $685E

fn_01_6861:
	ld b,(ix+$1B)	; $6861
	ld c,$00	; $6864
	ld de,$CAA6	; $6866

loc_01_6869:
	push bc	; $6869
	push de	; $686A
	ld a,c	; $686B
	ld hl,$C63D	; $686C
	rst $28	; $686F
	ld l,(ix+$1C)	; $6870
	.INCBIN "bank01.bin" SKIP $2873 READ 1	; data, 1 bytes

fn_01_6874:
	ld h,(hl)	; $6874
	dec e	; $6875
	rst $20	; $6876
	call fn_00_2D6C	; $6877
	pop de	; $687A
	pop bc	; $687B
	ld hl,$0080	; $687C
	add hl,de	; $687F
	ex de,hl	; $6880
	inc c	; $6881
	djnz loc_01_6869	; $6882
	call menu_window_show	; $6884

loc_01_6887:
	call frame_tick	; $6887
	ld c,(ix+$1B)	; $688A
	ld de,$0102	; $688D
	call menu_step_wrap	; $6890
	jr z,loc_01_6887	; $6893
	jr nc,loc_01_689E	; $6895
	call fn_01_472D	; $6897
	scf	; $689A
	jp task_return_3	; $689B

loc_01_689E:
	ld a,(ix+$1A)	; $689E
	ld hl,$C63D	; $68A1
	rst $28	; $68A4
	push af	; $68A5
	ld l,(ix+$1C)	; $68A6
	ld h,(ix+$1D)	; $68A9
	rst $20	; $68AC
	call print_text_hl	; $68AD
	call print_newline	; $68B0
	pop af	; $68B3
	or a	; $68B4
	jp task_return_3	; $68B5

fn_01_68B8:
	ld hl,$C63D	; $68B8
	ld de,$0000	; $68BB
	ld b,$08	; $68BE
	ld a,($C955)	; $68C0

loc_01_68C3:
	rrca	; $68C3
	jr nc,loc_01_68C9	; $68C4
	ld (hl),e	; $68C6
	inc hl	; $68C7
	inc d	; $68C8

loc_01_68C9:
	inc e	; $68C9
	djnz loc_01_68C3	; $68CA
	ld (ix+$1B),d	; $68CC
	ld a,d	; $68CF
	call fn_01_68D7	; $68D0
	ld hl,$6998	; $68D3
	ret	; $68D6

fn_01_68D7:
	push bc	; $68D7
	push de	; $68D8
	push hl	; $68D9
	ld de,$C63D	; $68DA
	ld b,a	; $68DD
	ld c,a	; $68DE

loc_01_68DF:
	push bc	; $68DF
	ld a,c	; $68E0
	call random_below	; $68E1
	ld hl,$C63D	; $68E4
	rst $30	; $68E7
	ld b,(hl)	; $68E8
	ld a,(de)	; $68E9
	ld (hl),a	; $68EA
	ld a,b	; $68EB
	ld (de),a	; $68EC
	pop bc	; $68ED
	inc de	; $68EE
	djnz loc_01_68DF	; $68EF
	pop hl	; $68F1
	pop de	; $68F2
	pop bc	; $68F3
	ret	; $68F4
	.INCBIN "bank01.bin" SKIP $28F5 READ 94	; data, 94 bytes

fn_01_6953:
	ld l,(hl)	; $6953
	ld h,l	; $6954
	ld (hl),e	; $6955
	ld (hl),h	; $6956
	ld a,c	; $6957
	nop	; $6958
	ld b,e	; $6959
	ld l,a	; $695A
	ld l,l	; $695B
	ld (hl),b	; $695C
	ld h,c	; $695D
	ld (hl),e	; $695E
	ld (hl),e	; $695F
	ld l,c	; $6960
	ld l,a	; $6961
	ld l,(hl)	; $6962
	nop	; $6963
	ld d,(hl)	; $6964
	ld h,c	; $6965
	ld l,h	; $6966
	ld l,a	; $6967
	ld (hl),d	; $6968
	nop	; $6969
	ld c,d	; $696A
	ld (hl),l	; $696B
	ld (hl),e	; $696C
	ld (hl),h	; $696D
	ld l,c	; $696E

fn_01_696F:
	ld h,e	; $696F
	ld h,l	; $6970
	nop	; $6971
	ld d,e	; $6972
	ld h,c	; $6973
	ld h,e	; $6974
	ld (hl),d	; $6975
	ld l,c	; $6976
	ld h,(hl)	; $6977
	ld l,c	; $6978
	ld h,e	; $6979
	ld h,l	; $697A
	nop	; $697B
	ld c,b	; $697C
	ld l,a	; $697D
	ld l,(hl)	; $697E
	ld l,a	; $697F
	ld (hl),d	; $6980
	nop	; $6981
	ld d,e	; $6982
	ld (hl),b	; $6983
	ld l,c	; $6984
	ld (hl),d	; $6985
	ld l,c	; $6986
	ld (hl),h	; $6987
	ld (hl),l	; $6988
	ld h,c	; $6989
	ld l,h	; $698A
	ld l,c	; $698B
	ld (hl),h	; $698C
	ld a,c	; $698D
	nop	; $698E
	ld c,b	; $698F
	ld (hl),l	; $6990
	ld l,l	; $6991
	ld l,c	; $6992
	ld l,h	; $6993
	ld l,c	; $6994
	ld (hl),h	; $6995
	ld a,c	; $6996
	nop	; $6997
	xor b	; $6998
	ld l,c	; $6999
	xor l	; $699A
	ld l,c	; $699B
	or h	; $699C
	ld l,c	; $699D
	cp b	; $699E
	ld l,c	; $699F
	cp (hl)	; $69A0
	ld l,c	; $69A1
	push bc	; $69A2
	ld l,c	; $69A3
	call z,$D269	; $69A4
	ld l,c	; $69A7
	ld b,d	; $69A8
	ld l,h	; $69A9
	ld (hl),l	; $69AA
	ld h,l	; $69AB
	nop	; $69AC
	ld e,c	; $69AD
	ld h,l	; $69AE
	ld l,h	; $69AF
	ld l,h	; $69B0
	ld l,a	; $69B1
	ld (hl),a	; $69B2
	nop	; $69B3
	ld d,d	; $69B4
	ld h,l	; $69B5
	ld h,h	; $69B6
	nop	; $69B7
	ld b,a	; $69B8
	ld (hl),d	; $69B9
	ld h,l	; $69BA
	ld h,l	; $69BB
	ld l,(hl)	; $69BC
	nop	; $69BD
	ld c,a	; $69BE
	ld (hl),d	; $69BF
	ld h,c	; $69C0
	ld l,(hl)	; $69C1
	ld h,a	; $69C2
	ld h,l	; $69C3
	nop	; $69C4
	ld d,b	; $69C5
	ld (hl),l	; $69C6
	ld (hl),d	; $69C7
	ld (hl),b	; $69C8
	ld l,h	; $69C9
	ld h,l	; $69CA
	nop	; $69CB
	ld d,a	; $69CC
	ld l,b	; $69CD
	ld l,c	; $69CE
	ld (hl),h	; $69CF
	ld h,l	; $69D0
	nop	; $69D1
	ld b,d	; $69D2
	ld l,h	; $69D3
	ld h,c	; $69D4
	ld h,e	; $69D5
	ld l,e	; $69D6
	nop	; $69D7

fn_01_69D8:
	call fn_00_0DD7	; $69D8
	ld de,$69E4	; $69DB
	call spawn_task	; $69DE
	jp loc_00_0DE1	; $69E1

loc_01_69E4:
	ld hl,(party_x)	; $69E4
	call fn_01_6DB9	; $69E7
	ld (ix+$1A),a	; $69EA
	and $F0	; $69ED
	cp $D0	; $69EF
	jp z,loc_01_6C07	; $69F1
	call fn_00_323D	; $69F4
	call fn_00_129D	; $69F7
	call fn_01_4324	; $69FA
	call fn_00_315F	; $69FD
	ld a,$08	; $6A00
	call play_music	; $6A02
	xor a	; $6A05
	ld ($C9A1),a	; $6A06
	ld hl,$0000	; $6A09
	ld ($C99C),hl	; $6A0C

loc_01_6A0F:
	call fn_00_129D	; $6A0F
	call yield	; $6A12
	call fn_01_6A9B	; $6A15
	call fn_01_6A52	; $6A18
	call fn_02_AF57	; $6A1B
	jr c,loc_01_6A4A	; $6A1E
	ld a,($C95B)	; $6A20
	or a	; $6A23
	jr nz,loc_01_6A35	; $6A24
	call print_inline	; $6A26
	.INCBIN "bank01.bin" SKIP $2A29 READ 12	; text, 12 bytes

loc_01_6A35:
	ld a,$7F	; $6A35
	call print_char	; $6A37
	jr loc_01_6A3F	; $6A3A

loc_01_6A3C:
	call frame_tick	; $6A3C

loc_01_6A3F:
	call fn_01_5726	; $6A3F
	call fn_01_6B0A	; $6A42
	jr c,loc_01_6A3C	; $6A45
	call fn_01_6A52	; $6A47

loc_01_6A4A:
	call fn_00_3F14	; $6A4A
	call fn_01_6AEA	; $6A4D
	jr loc_01_6A0F	; $6A50

fn_01_6A52:
	ld a,($C970)	; $6A52
	or a	; $6A55
	ret p	; $6A56
	pop af	; $6A57
	call print_inline	; $6A58
	.INCBIN "bank01.bin" SKIP $2A5B READ 14	; text, 14 bytes
	call fn_00_0605	; $6A69
	call fn_00_31B1	; $6A6C
	xor a	; $6A6F
	ld (location),a	; $6A70
	ld a,$11	; $6A73
	ld (game_mode),a	; $6A75
	ld hl,(world_return_x)	; $6A78
	ld (party_x),hl	; $6A7B
	ld a,$FF	; $6A7E
	ld ($C98F),a	; $6A80
	call fn_01_4100	; $6A83
	ld a,$FF	; $6A86
	ld ($C970),a	; $6A88
	ld a,(party_x)	; $6A8B
	cp $EF	; $6A8E
	ret nz	; $6A90
	ld a,(party_y)	; $6A91
	cp $F0	; $6A94
	ret nz	; $6A96
	call fn_00_0E0C	; $6A97
	ret	; $6A9A

fn_01_6A9B:
	ld a,($C9A1)	; $6A9B
	or a	; $6A9E
	ret z	; $6A9F
	ld a,($C990)	; $6AA0
	cp $10	; $6AA3
	jr z,loc_01_6AAC	; $6AA5
	cp $20	; $6AA7
	jr z,loc_01_6ABE	; $6AA9
	ret	; $6AAB

loc_01_6AAC:
	call print_inline	; $6AAC
	.INCBIN "bank01.bin" SKIP $2AAF READ 6	; text, 6 bytes
	ld hl,$C970	; $6AB5
	dec (hl)	; $6AB8
	jp m,print_newline	; $6AB9
	jr loc_01_6ACA	; $6ABC

loc_01_6ABE:
	call print_inline	; $6ABE
	.INCBIN "bank01.bin" SKIP $2AC1 READ 5	; text, 5 bytes
	ld hl,$C970	; $6AC6
	inc (hl)	; $6AC9

loc_01_6ACA:
	call print_inline	; $6ACA
	.INCBIN "bank01.bin" SKIP $2ACD READ 11	; text, 11 bytes
	ld a,($C970)	; $6AD8
	add a,$31	; $6ADB
	call print_char	; $6ADD
	call print_newline	; $6AE0
	pop af	; $6AE3
	call fn_00_31B1	; $6AE4
	jp loc_01_69E4	; $6AE7

fn_01_6AEA:
	call fn_01_465D	; $6AEA
	call fn_02_B4EE	; $6AED
	call fn_01_4776	; $6AF0
	call fn_01_6D1A	; $6AF3
	call fn_01_6CB9	; $6AF6
	ld hl,$C95B	; $6AF9
	ld a,(hl)	; $6AFC
	or a	; $6AFD
	jr z,loc_01_6B01	; $6AFE
	dec (hl)	; $6B00

loc_01_6B01:
	call fn_00_372E	; $6B01
	call fn_00_3754	; $6B04
	jp fn_01_4324	; $6B07

fn_01_6B0A:
	xor a	; $6B0A
	ld ($C9A1),a	; $6B0B
	ld a,(joy_held)	; $6B0E
	ld hl,$6B38	; $6B11
	call switch_on_a	; $6B14
	ld a,(joy_pressed)	; $6B17
	cp $10	; $6B1A
	jr z,loc_01_6B24	; $6B1C
	cp $20	; $6B1E
	jr z,loc_01_6B2F	; $6B20
	scf	; $6B22
	ret	; $6B23

loc_01_6B24:
	ld hl,$0100	; $6B24
	ld ($C99A),hl	; $6B27
	call fn_01_46B5	; $6B2A
	or a	; $6B2D
	ret	; $6B2E

loc_01_6B2F:
	ld hl,$0100	; $6B2F
	ld ($C99A),hl	; $6B32
	jp $8000	; $6B35

tbl_01_6B38:
	.db $01
	.dw loc_01_6B45
	.db $02
	.dw loc_01_6B49
	.db $04
	.dw loc_01_6B4D
	.db $08
	.dw loc_01_6B51
	.db $FF

loc_01_6B45:
	ld a,$01	; $6B45
	jr loc_01_6B53	; $6B47

loc_01_6B49:
	ld a,$03	; $6B49
	jr loc_01_6B53	; $6B4B

loc_01_6B4D:
	ld a,$00	; $6B4D
	jr loc_01_6B53	; $6B4F

loc_01_6B51:
	ld a,$02	; $6B51

loc_01_6B53:
	call fn_01_6B58	; $6B53
	or a	; $6B56
	ret	; $6B57

fn_01_6B58:
	ld (entry_direction),a	; $6B58
	ld hl,$0100	; $6B5B
	ld ($C99A),hl	; $6B5E
	ld hl,(party_x)	; $6B61
	call fn_01_6C59	; $6B64
	ld (ix+$1C),l	; $6B67
	ld (ix+$1D),h	; $6B6A
	call fn_01_6DB9	; $6B6D
	ld (ix+$1B),a	; $6B70
	cp $C0	; $6B73
	ld a,$0E	; $6B75
	jr z,loc_01_6B7B	; $6B77
	ld a,$0A	; $6B79

loc_01_6B7B:
	call fn_00_05F1	; $6B7B
	ld a,(entry_direction)	; $6B7E
	call fn_01_55A0	; $6B81
	call print_newline	; $6B84
	ld a,(ix+$1B)	; $6B87
	cp $C0	; $6B8A
	jr z,loc_01_6BD7	; $6B8C
	and $F0	; $6B8E
	cp $D0	; $6B90
	jr nz,loc_01_6BA2	; $6B92
	ld a,(ix+$1B)	; $6B94
	ld (ix+$1A),a	; $6B97
	ld l,(ix+$1C)	; $6B9A
	ld h,(ix+$1D)	; $6B9D
	jr loc_01_6C07	; $6BA0

loc_01_6BA2:
	ld l,(ix+$1C)	; $6BA2
	ld h,(ix+$1D)	; $6BA5
	call fn_01_6C6A	; $6BA8
	jp nc,loc_01_40A6	; $6BAB
	ld a,(ix+$1B)	; $6BAE
	cp $F0	; $6BB1
	jp z,loc_01_40A6	; $6BB3
	cp $A1	; $6BB6
	jp z,loc_01_40A6	; $6BB8
	cp $A2	; $6BBB
	jr nz,loc_01_6BC8	; $6BBD
	ld hl,$0200	; $6BBF
	ld ($C99A),hl	; $6BC2
	call fn_01_4719	; $6BC5

loc_01_6BC8:
	ld l,(ix+$1C)	; $6BC8
	ld h,(ix+$1D)	; $6BCB
	ld (party_x),hl	; $6BCE
	ld a,$FF	; $6BD1
	ld ($C9A1),a	; $6BD3
	ret	; $6BD6

loc_01_6BD7:
	ld a,(entry_direction)	; $6BD7
	ld l,(ix+$1C)	; $6BDA
	ld h,(ix+$1D)	; $6BDD
	call fn_01_6C59	; $6BE0
	ld (ix+$1E),l	; $6BE3
	ld (ix+$1F),h	; $6BE6
	call fn_01_6DB9	; $6BE9
	ld (ix+$1A),a	; $6BEC
	and $F0	; $6BEF
	cp $D0	; $6BF1
	jr nz,loc_01_6BA2	; $6BF3
	ld l,(ix+$1C)	; $6BF5
	ld h,(ix+$1D)	; $6BF8
	ld (party_x),hl	; $6BFB
	call fn_00_129D	; $6BFE
	ld l,(ix+$1E)	; $6C01
	ld h,(ix+$1F)	; $6C04

loc_01_6C07:
	ld a,$00	; $6C07
	call fn_01_6C59	; $6C09
	call fn_01_6DB9	; $6C0C
	cp (ix+$1A)	; $6C0F
	jr z,loc_01_6C19	; $6C12
	ld a,$02	; $6C14
	call fn_01_6C59	; $6C16

loc_01_6C19:
	ld a,$01	; $6C19
	call fn_01_6C59	; $6C1B
	call fn_01_6DB9	; $6C1E
	cp (ix+$1A)	; $6C21
	jr z,loc_01_6C2B	; $6C24
	ld a,$03	; $6C26
	call fn_01_6C59	; $6C28

loc_01_6C2B:
	ld (party_x),hl	; $6C2B
	ld a,(location)	; $6C2E
	cp $19	; $6C31
	ld b,$00	; $6C33
	jr nz,loc_01_6C40	; $6C35
	ld a,($C970)	; $6C37
	add a,a	; $6C3A
	add a,a	; $6C3B
	add a,a	; $6C3C
	and $F0	; $6C3D
	ld b,a	; $6C3F

loc_01_6C40:
	ld a,(ix+$1A)	; $6C40
	and $0F	; $6C43
	or b	; $6C45

fn_01_6C46:
	ld (dungeon_room),a	; $6C46
	call load_dungeon_room	; $6C49
	ld a,$02	; $6C4C
	ld ($C975),a	; $6C4E
	ld a,$16	; $6C51
	ld (game_mode),a	; $6C53
	jp main_loop	; $6C56

fn_01_6C59:
	push de	; $6C59
	ex de,hl	; $6C5A
	call fn_01_40B9	; $6C5B
	ld a,e	; $6C5E
	add a,l	; $6C5F
	and $0F	; $6C60
	ld l,a	; $6C62
	ld a,d	; $6C63
	add a,h	; $6C64
	and $0F	; $6C65
	ld h,a	; $6C67
	pop de	; $6C68
	ret	; $6C69

fn_01_6C6A:
	push bc	; $6C6A
	push de	; $6C6B
	push iy	; $6C6C
	call fn_01_6C97	; $6C6E

loc_01_6C71:
	ld a,(iy+$03)	; $6C71
	or a	; $6C74
	jr z,loc_01_6C83	; $6C75
	ld a,(iy+$01)	; $6C77
	cp l	; $6C7A
	jr nz,loc_01_6C83	; $6C7B
	ld a,(iy+$02)	; $6C7D
	cp h	; $6C80
	jr z,loc_01_6C8A	; $6C81

loc_01_6C83:
	add iy,de	; $6C83
	djnz loc_01_6C71	; $6C85
	scf	; $6C87
	jr loc_01_6C92	; $6C88

loc_01_6C8A:
	ld a,($C970)	; $6C8A
	add a,a	; $6C8D
	add a,a	; $6C8E
	add a,$04	; $6C8F
	sub b	; $6C91

loc_01_6C92:
	pop iy	; $6C92
	pop de	; $6C94
	pop bc	; $6C95
	ret	; $6C96

fn_01_6C97:
	ex de,hl	; $6C97
	ld a,($C970)	; $6C98
	ld hl,$6CA9	; $6C9B
	rst $20	; $6C9E
	push hl	; $6C9F
	pop iy	; $6CA0
	ex de,hl	; $6CA2
	ld de,$0008	; $6CA3
	ld b,$04	; $6CA6
	ret	; $6CA8
	.INCBIN "bank01.bin" SKIP $2CA9 READ 16	; data, 16 bytes

fn_01_6CB9:
	call fn_01_6C97	; $6CB9

loc_01_6CBC:
	ld a,(iy+$03)	; $6CBC
	or a	; $6CBF
	jr nz,loc_01_6D15	; $6CC0
	call random	; $6CC2
	and $0F	; $6CC5
	ld l,a	; $6CC7
	call random	; $6CC8
	and $0F	; $6CCB
	ld h,a	; $6CCD
	ld a,(party_x)	; $6CCE
	sub l	; $6CD1
	add a,$05	; $6CD2
	and $0F	; $6CD4
	cp $0B	; $6CD6
	jr nc,loc_01_6CE6	; $6CD8
	ld a,(party_y)	; $6CDA
	sub h	; $6CDD
	add a,$05	; $6CDE
	and $0F	; $6CE0
	cp $0B	; $6CE2
	jr c,loc_01_6D15	; $6CE4

loc_01_6CE6:
	call fn_01_6DB9	; $6CE6
	or a	; $6CE9
	jr nz,loc_01_6D15	; $6CEA
	call fn_01_6C6A	; $6CEC
	jr nc,loc_01_6D15	; $6CEF
	call random	; $6CF1
	and $03	; $6CF4
	ld c,a	; $6CF6
	ld a,($C970)	; $6CF7
	add a,c	; $6CFA
	add a,a	; $6CFB
	add a,a	; $6CFC
	add a,$90	; $6CFD
	cp $AC	; $6CFF
	jr z,loc_01_6D15	; $6D01
	ld (iy+$03),a	; $6D03
	ld (iy+$00),a	; $6D06
	ld (iy+$01),l	; $6D09
	ld (iy+$04),l	; $6D0C
	ld (iy+$02),h	; $6D0F
	ld (iy+$05),h	; $6D12

loc_01_6D15:
	add iy,de	; $6D15
	djnz loc_01_6CBC	; $6D17
	ret	; $6D19

fn_01_6D1A:
	call fn_01_6C97	; $6D1A

loc_01_6D1D:
	ld a,(iy+$03)	; $6D1D
	or a	; $6D20
	jr z,loc_01_6D91	; $6D21
	ld a,(party_x)	; $6D23
	sub (iy+$01)	; $6D26
	inc a	; $6D29
	and $0F	; $6D2A
	cp $03	; $6D2C
	jr nc,loc_01_6D3D	; $6D2E
	ld a,(party_y)	; $6D30
	sub (iy+$02)	; $6D33
	inc a	; $6D36
	and $0F	; $6D37
	cp $03	; $6D39
	jr c,loc_01_6D96	; $6D3B

loc_01_6D3D:
	ld a,(iy+$03)	; $6D3D
	cp $B0	; $6D40
	jr z,loc_01_6D91	; $6D42
	cp $AC	; $6D44
	jr z,loc_01_6D91	; $6D46
	ld c,$03	; $6D48

loc_01_6D4A:
	call random	; $6D4A
	and $03	; $6D4D
	call fn_01_40B9	; $6D4F
	ld a,(iy+$01)	; $6D52
	add a,l	; $6D55
	and $0F	; $6D56
	ld l,a	; $6D58
	ld a,(iy+$02)	; $6D59
	add a,h	; $6D5C
	and $0F	; $6D5D
	ld h,a	; $6D5F
	call fn_01_6DB9	; $6D60
	and $F0	; $6D63
	cp $A0	; $6D65
	jr z,loc_01_6D7A	; $6D67
	cp $C0	; $6D69
	jr z,loc_01_6D7A	; $6D6B
	cp $D0	; $6D6D
	jr z,loc_01_6D7A	; $6D6F
	cp $F0	; $6D71
	jr z,loc_01_6D7A	; $6D73
	call fn_01_6C6A	; $6D75
	jr c,loc_01_6D7F	; $6D78

loc_01_6D7A:
	dec c	; $6D7A
	jr nz,loc_01_6D4A	; $6D7B
	jr loc_01_6D91	; $6D7D

loc_01_6D7F:
	ld a,(iy+$01)	; $6D7F
	ld (iy+$04),a	; $6D82
	ld a,(iy+$02)	; $6D85
	ld (iy+$05),a	; $6D88
	ld (iy+$01),l	; $6D8B
	ld (iy+$02),h	; $6D8E

loc_01_6D91:
	add iy,de	; $6D91
	djnz loc_01_6D1D	; $6D93
	ret	; $6D95

loc_01_6D96:
	call fn_00_129D	; $6D96
	call print_inline	; $6D99
	.INCBIN "bank01.bin" SKIP $2D9C READ 14	; text, 14 bytes
	ld a,(iy+$03)	; $6DAA
	call fn_01_47D3	; $6DAD
	call print_text_hl	; $6DB0
	call print_newline	; $6DB3
	jp loc_00_3B52	; $6DB6

fn_01_6DB9:
	push hl	; $6DB9
	call fn_01_6DC0	; $6DBA
	ld a,(hl)	; $6DBD
	pop hl	; $6DBE
	ret	; $6DBF

fn_01_6DC0:
	ld a,h	; $6DC0
	add a,a	; $6DC1
	add a,a	; $6DC2
	add a,a	; $6DC3
	add a,a	; $6DC4
	or l	; $6DC5
	ld l,a	; $6DC6
	ld a,($C970)	; $6DC7
	add a,$D0	; $6DCA
	ld h,a	; $6DCC
	ret	; $6DCD

loc_01_6DCE:
	call talk_setup	; $6DCE
	ld hl,talk_flags	; $6DD1
	ld bc,$2000	; $6DD4
	call fn_00_333B	; $6DD7
	ld (ix+$1C),$00	; $6DDA
	ld (ix+$1D),$00	; $6DDE
	ld hl,$71D9	; $6DE2
	call print_text_hl	; $6DE5
	ld b,$02	; $6DE8
	call fn_00_354D	; $6DEA
	call random	; $6DED
	and $01	; $6DF0
	jp z,loc_01_6DFB	; $6DF2
	call print_newline	; $6DF5
	call fn_01_6F61	; $6DF8

loc_01_6DFB:
	call wait_button	; $6DFB

loc_01_6DFE:
	ld hl,$71F3	; $6DFE
	call print_text_hl	; $6E01

loc_01_6E04:
	call talk_topics	; $6E04

loc_01_6E07:
	call frame_tick	; $6E07
	ld a,(joy_pressed)	; $6E0A
	and $7F	; $6E0D
	ld hl,$6E17	; $6E0F
	call switch_on_a	; $6E12
	jr loc_01_6E07	; $6E15

tbl_01_6E17:
	.db $01
	.dw loc_01_6E2A
	.db $02
	.dw loc_01_6E51
	.db $10
	.dw loc_01_6F3C
	.db $20
	.dw loc_01_6E7D
	.db $FF

fn_01_6E24:
	ld a,(ix+$1D)	; $6E24
	add a,a	; $6E27
	inc a	; $6E28
	ret	; $6E29

loc_01_6E2A:
	ld a,(ix+$1C)	; $6E2A
	add a,(ix+$1D)	; $6E2D
	jr z,loc_01_6E07	; $6E30
	call fn_01_40D9	; $6E32
	ld a,(ix+$1D)	; $6E35
	or a	; $6E38
	jr z,loc_01_6E4C	; $6E39
	call fn_01_6E24	; $6E3B
	call cursor_off	; $6E3E
	dec (ix+$1D)	; $6E41
	call fn_01_6E24	; $6E44
	call cursor_on	; $6E47
	jr loc_01_6E07	; $6E4A

loc_01_6E4C:
	dec (ix+$1C)	; $6E4C
	jr loc_01_6E04	; $6E4F

loc_01_6E51:
	ld a,(ix+$1C)	; $6E51
	add a,(ix+$1D)	; $6E54
	inc a	; $6E57
	cp (ix+$1B)	; $6E58
	jr nc,loc_01_6E07	; $6E5B
	call fn_01_40D9	; $6E5D
	ld a,(ix+$1D)	; $6E60
	cp $07	; $6E63
	jr nc,loc_01_6E78	; $6E65
	call fn_01_6E24	; $6E67
	call cursor_off	; $6E6A
	inc (ix+$1D)	; $6E6D
	call fn_01_6E24	; $6E70
	call cursor_on	; $6E73
	jr loc_01_6E07	; $6E76

loc_01_6E78:
	inc (ix+$1C)	; $6E78
	jr loc_01_6E04	; $6E7B

loc_01_6E7D:
	call fn_01_40C9	; $6E7D
	ld a,(ix+$1C)	; $6E80
	add a,(ix+$1D)	; $6E83
	ld hl,$C63D	; $6E86
	rst $28	; $6E89
	cp $0A	; $6E8A
	jr c,loc_01_6E90	; $6E8C
	add a,$FB	; $6E8E

loc_01_6E90:
	ld ($C9A7),a	; $6E90
	call fn_00_3642	; $6E93
	call random	; $6E96
	ld hl,talk_turn_away	; $6E99
	cp (hl)	; $6E9C
	jr nc,loc_01_6ED8	; $6E9D
	neg	; $6E9F
	add a,(hl)	; $6EA1
	cp $40	; $6EA2
	jr c,loc_01_6ECC	; $6EA4
	call fn_00_3666	; $6EA6
	jr nz,loc_01_6EB8	; $6EA9
	ld b,$01	; $6EAB
	call fn_00_354D	; $6EAD
	ld hl,$71E4	; $6EB0
	call print_text_hl	; $6EB3
	jr loc_01_6EBB	; $6EB6

loc_01_6EB8:
	call fn_01_71B1	; $6EB8

loc_01_6EBB:
	ld hl,$7301	; $6EBB
	call print_text_hl	; $6EBE
	ld iy,($C9A5)	; $6EC1
	ld (iy+$06),$FF	; $6EC5
	jp loc_01_6F3C	; $6EC9

loc_01_6ECC:
	call fn_01_71A4	; $6ECC
	ld hl,$7311	; $6ECF
	call print_text_hl	; $6ED2
	jp loc_01_6F3C	; $6ED5

loc_01_6ED8:
	ld a,($C9A7)	; $6ED8
	ld hl,$6F45	; $6EDB
	rst $18	; $6EDE
	call wait_button	; $6EDF

loc_01_6EE2:
	ld a,($C9A7)	; $6EE2
	add a,$80	; $6EE5
	ld l,a	; $6EE7
	ld h,$C5	; $6EE8
	ld (hl),$01	; $6EEA
	ld a,($C9A7)	; $6EEC
	ld hl,talk_question_topic	; $6EEF
	cp (hl)	; $6EF2
	jp z,loc_01_7099	; $6EF3
	cp $05	; $6EF6
	jp nz,loc_01_6DFE	; $6EF8
	ld a,(ix+$09)	; $6EFB
	cp $AC	; $6EFE
	jp nz,loc_01_6DFE	; $6F00
	call fn_01_54CB	; $6F03
	ld hl,$732A	; $6F06
	call print_text_hl	; $6F09
	ld a,$04	; $6F0C
	call fn_01_4D0A	; $6F0E

loc_01_6F11:
	call frame_tick	; $6F11
	call fn_01_4D2B	; $6F14
	jr z,loc_01_6F11	; $6F17
	jr c,loc_01_6F11	; $6F19
	call fn_01_54CB	; $6F1B
	ld hl,$C065	; $6F1E
	ld de,$734B	; $6F21
	call fn_00_34BB	; $6F24
	jr c,loc_01_6F30	; $6F27
	ld b,$06	; $6F29
	call fn_01_6F8D	; $6F2B
	jr loc_01_6F36	; $6F2E

loc_01_6F30:
	ld hl,$733A	; $6F30
	call print_text_hl	; $6F33

loc_01_6F36:
	call wait_button	; $6F36
	jp loc_01_6DFE	; $6F39

loc_01_6F3C:
	call fn_01_40D1	; $6F3C
	call fn_01_54CB	; $6F3F

loc_01_6F42:
	jp loc_02_8084	; $6F42

tbl_01_6F45:
	.dw fn_01_6F57
	.dw fn_01_6F61
	.dw fn_01_6F72
	.dw fn_01_6F7D
	.dw fn_01_6F81
	.dw fn_01_6F85
	.dw fn_01_6F89
	.dw talk_join
	.dw fn_01_7039

fn_01_6F57:
	pop af	; $6F57
	ld hl,$720E	; $6F58
	call print_text_hl	; $6F5B
	jp loc_01_6F42	; $6F5E

fn_01_6F61:
	call fn_01_71B1	; $6F61
	ld hl,$71ED	; $6F64
	call print_text_hl	; $6F67
	ld b,$01	; $6F6A
	call fn_00_354D	; $6F6C
	jp fn_01_548D	; $6F6F

fn_01_6F72:
	ld hl,$7205	; $6F72
	call print_text_hl	; $6F75
	ld b,$02	; $6F78
	jp fn_00_354D	; $6F7A

fn_01_6F7D:
	ld b,$03	; $6F7D
	jr fn_01_6F8D	; $6F7F

fn_01_6F81:
	ld b,$04	; $6F81
	jr fn_01_6F8D	; $6F83

fn_01_6F85:
	ld b,$05	; $6F85
	jr fn_01_6F8D	; $6F87

fn_01_6F89:
	ld b,$06	; $6F89
	jr fn_01_6F8D	; $6F8B

fn_01_6F8D:
	call fn_01_548D	; $6F8D
	call fn_00_354D	; $6F90
	jp fn_01_548D	; $6F93

talk_join:
	ld iy,($C9A5)	; $6F96
	ld a,(iy+$07)	; $6F9A
	cp $01	; $6F9D
	jr nz,loc_01_6FB5	; $6F9F
	ld a,(location)	; $6FA1
	sub $06	; $6FA4
	cp $08	; $6FA6
	jr nc,loc_01_6FB5	; $6FA8
	ld (ix+$07),a	; $6FAA
	ld a,($C76C)	; $6FAD
	cp (ix+$07)	; $6FB0
	jr nz,loc_01_6FBE	; $6FB3

loc_01_6FB5:
	call fn_01_71B1	; $6FB5
	ld hl,$7216	; $6FB8
	jp print_text_hl	; $6FBB

loc_01_6FBE:
	ld a,(ix+$07)	; $6FBE
	ld hl,karma	; $6FC1
	rst $28	; $6FC4
	or a	; $6FC5
	jr z,loc_01_6FE2	; $6FC6
	cp $28	; $6FC8
	jr nc,loc_01_6FE2	; $6FCA
	ld hl,$722B	; $6FCC
	call print_text_hl	; $6FCF
	ld a,(ix+$07)	; $6FD2
	ld hl,$734E	; $6FD5
	rst $20	; $6FD8
	call print_text_hl	; $6FD9
	ld hl,$723A	; $6FDC
	jp print_text_hl	; $6FDF

loc_01_6FE2:
	ld a,(party_size)	; $6FE2
	ld hl,$C75B	; $6FE5
	sub (hl)	; $6FE8
	jr c,loc_01_6FFD	; $6FE9
	ld hl,$722B	; $6FEB
	call print_text_hl	; $6FEE
	ld hl,$7258	; $6FF1
	call print_text_hl	; $6FF4
	ld hl,$723A	; $6FF7
	jp print_text_hl	; $6FFA

loc_01_6FFD:
	ld hl,$7264	; $6FFD
	call print_text_hl	; $7000
	ld hl,$C749	; $7003
	ld bc,$0800	; $7006
	call fn_00_333B	; $7009
	ld iy,$C76E	; $700C
	ld de,$001D	; $7010
	ld a,(ix+$07)	; $7013

loc_01_7016:
	cp (iy+$1B)	; $7016
	jr z,loc_01_701F	; $7019
	add iy,de	; $701B
	jr loc_01_7016	; $701D

loc_01_701F:
	push iy	; $701F
	pop hl	; $7021
	ld a,(party_size)	; $7022
	ld c,a	; $7025
	call member_record	; $7026
	push iy	; $7029
	pop de	; $702B
	ld b,$1D	; $702C
	call fn_00_3340	; $702E
	ld hl,party_size	; $7031
	inc (hl)	; $7034
	pop af	; $7035
	jp loc_01_6F42	; $7036

fn_01_7039:
	ld iy,($C9A5)	; $7039
	ld a,(iy+$03)	; $703D
	cp $58	; $7040
	jr z,loc_01_704D	; $7042
	call fn_01_71B1	; $7044
	ld hl,$7282	; $7047
	jp print_text_hl	; $704A

loc_01_704D:
	pop af	; $704D
	ld hl,$72A4	; $704E
	call print_text_hl	; $7051
	ld a,$02	; $7054
	call number_entry_start	; $7056

loc_01_7059:
	call frame_tick	; $7059
	call number_entry_step	; $705C
	jr c,loc_01_7059	; $705F
	ld a,h	; $7061
	or l	; $7062
	.INCBIN "bank01.bin" SKIP $3063 READ 2	; data, 2 bytes

fn_01_7065:
	ld l,(hl)	; $7065
	call fn_01_54CB	; $7066
	call pay	; $7069
	jr nc,loc_01_707A	; $706C
	ld hl,$72AF	; $706E
	call print_text_hl	; $7071
	call wait_button	; $7074
	jp loc_01_6EE2	; $7077

loc_01_707A:
	call fn_01_71B1	; $707A
	ld hl,$72CD	; $707D
	call print_text_hl	; $7080
	call wait_button	; $7083
	ld hl,(last_karma_time)	; $7086
	call fn_00_33F6	; $7089
	ld (last_karma_time),hl	; $708C
	ld a,$02	; $708F
	ld c,$01	; $7091
	call nz,karma_gain	; $7093
	jp loc_01_6EE2	; $7096

loc_01_7099:
	call fn_01_54CB	; $7099
	ld b,$07	; $709C
	call fn_01_6F8D	; $709E
	call wait_button	; $70A1
	ld hl,$731E	; $70A4
	call print_text_hl	; $70A7
	call yes_no_start	; $70AA

loc_01_70AD:
	call frame_tick	; $70AD
	call yes_no_step	; $70B0
	jr c,loc_01_70AD	; $70B3
	call print_newline	; $70B5
	bit 0,(ix+$07)	; $70B8
	jr nz,loc_01_70CF	; $70BC
	ld a,($C9AA)	; $70BE
	or a	; $70C1
	ld a,$05	; $70C2
	ld c,$07	; $70C4
	call nz,karma_lose	; $70C6
	ld a,$09	; $70C9
	ld b,$08	; $70CB
	jr loc_01_70E6	; $70CD

loc_01_70CF:
	ld a,($C9AA)	; $70CF
	or a	; $70D2
	jr z,loc_01_70E2	; $70D3
	ld hl,(last_karma_time)	; $70D5
	call fn_00_33F6	; $70D8
	ld a,$0A	; $70DB
	ld c,$07	; $70DD
	call nz,karma_gain	; $70DF

loc_01_70E2:
	ld a,$0A	; $70E2
	ld b,$09	; $70E4

loc_01_70E6:
	ld ($C9A7),a	; $70E6
	call fn_01_6F8D	; $70E9
	ld hl,(last_karma_time)	; $70EC
	call fn_00_33F6	; $70EF
	ld (last_karma_time),hl	; $70F2
	call wait_button	; $70F5
	ld a,($C9A7)	; $70F8
	cp $09	; $70FB
	jp nz,loc_01_6EE2	; $70FD
	ld a,(ix+$09)	; $7100
	ld iy,$73A7	; $7103
	ld de,$0006	; $7107
	ld b,$06	; $710A

loc_01_710C:
	cp (iy+$00)	; $710C
	jr z,loc_01_7118	; $710F
	add iy,de	; $7111
	djnz loc_01_710C	; $7113
	jp loc_01_6EE2	; $7115

loc_01_7118:
	call menu_window_clear	; $7118
	ld a,(iy+$01)	; $711B
	ld (ix+$1F),a	; $711E
	ld l,(iy+$02)	; $7121
	ld h,(iy+$03)	; $7124
	ld (ix+$0E),l	; $7127
	ld (ix+$0F),h	; $712A
	ld l,(iy+$04)	; $712D
	ld h,(iy+$05)	; $7130
	ld (ix+$10),l	; $7133
	ld (ix+$11),h	; $7136
	ld de,$0213	; $7139
	ld b,$08	; $713C

loc_01_713E:
	push de	; $713E
	call menu_text	; $713F
	pop de	; $7142
	inc d	; $7143
	inc d	; $7144
	djnz loc_01_713E	; $7145
	call menu_window_show	; $7147
	call fn_01_54CB	; $714A
	ld hl,$732A	; $714D
	call print_text_hl	; $7150
	ld (ix+$1A),$00	; $7153

loc_01_7157:
	call frame_tick	; $7157
	ld c,$08	; $715A
	ld de,$0102	; $715C
	call menu_step_wrap	; $715F
	jr z,loc_01_7157	; $7162
	jr c,loc_01_7157	; $7164
	ld l,(ix+$10)	; $7166
	ld h,(ix+$11)	; $7169
	ld d,(ix+$1A)	; $716C
	xor a	; $716F
	inc d	; $7170

loc_01_7171:
	dec d	; $7171
	jr z,loc_01_717B	; $7172
	ld bc,$0000	; $7174
	cpir	; $7177
	jr loc_01_7171	; $7179

loc_01_717B:
	call print_text_hl	; $717B
	call fn_01_54CB	; $717E
	ld a,(ix+$1A)	; $7181
	cp (ix+$1F)	; $7184
	jr nz,loc_01_7195	; $7187
	ld a,$06	; $7189
	ld ($C9A7),a	; $718B
	ld b,$06	; $718E
	call fn_01_6F8D	; $7190
	jr loc_01_719E	; $7193

loc_01_7195:
	ld l,(ix+$0E)	; $7195
	ld h,(ix+$0F)	; $7198
	call print_text_hl	; $719B

loc_01_719E:
	call wait_button	; $719E
	jp loc_01_6EE2	; $71A1

fn_01_71A4:
	push hl	; $71A4
	ld a,($C9AC)	; $71A5
	ld hl,$71BD	; $71A8
	rst $20	; $71AB
	call print_text_hl	; $71AC
	pop hl	; $71AF
	ret	; $71B0

fn_01_71B1:
	push hl	; $71B1
	call fn_01_71A4	; $71B2
	ld hl,$71E4	; $71B5
	call print_text_hl	; $71B8
	pop hl	; $71BB
	ret	; $71BC

tbl_01_71BD:
	.dw $71C5
	.dw $71C8
	.dw fn_01_71CC
	.dw fn_01_71CF

fn_01_71C5:
	.INCBIN "bank01.bin" SKIP $31C5 READ 3	; data, 3 bytes

fn_01_71C8:
	.INCBIN "bank01.bin" SKIP $31C8 READ 4	; data, 4 bytes

fn_01_71CC:
	ld c,c	; $71CC
	ld (hl),h	; $71CD
	nop	; $71CE

fn_01_71CF:
	ld d,h	; $71CF
	ld l,b	; $71D0
	ld h,l	; $71D1
	jr nz,$43	; $71D2
	ld l,b	; $71D4
	ld l,c	; $71D5
	ld l,h	; $71D6
	ld h,h	; $71D7
	nop	; $71D8
	.INCBIN "bank01.bin" SKIP $31D9 READ 62	; data, 62 bytes

loc_01_7217:
	.INCBIN "bank01.bin" SKIP $3217 READ 88	; data, 88 bytes

fn_01_726F:
	ld h,l	; $726F
	ld h,h	; $7270
	jr nz,loc_01_72E7	; $7271
	ld l,a	; $7273
	jr nz,loc_01_72E0	; $7274
	ld l,a	; $7276
	ld l,c	; $7277
	ld l,(hl)	; $7278
	jr nz,loc_01_72EF	; $7279
	ld l,b	; $727B
	ld h,l	; $727C
	ld h,l	; $727D
	ld hl,$0122	; $727E
	nop	; $7281
	.INCBIN "bank01.bin" SKIP $3282 READ 94	; data, 94 bytes

loc_01_72E0:
	ld l,b	; $72E0
	ld h,c	; $72E1
	ld l,h	; $72E2
	ld l,h	; $72E3
	jr nz,loc_01_7354	; $72E4
	ld h,l	; $72E6

loc_01_72E7:
	halt	; $72E7
	ld h,l	; $72E8
	ld (hl),d	; $72E9
	jr nz,loc_01_7352	; $72EA
	ld l,a	; $72EC
	ld (hl),d	; $72ED
	ld h,a	; $72EE

loc_01_72EF:
	ld h,l	; $72EF
	ld (hl),h	; $72F0
	jr nz,loc_01_7367	; $72F1
	ld l,b	; $72F3
	ld a,c	; $72F4
	jr nz,loc_01_7362	; $72F5
	ld l,c	; $72F7
	ld l,(hl)	; $72F8
	ld h,h	; $72F9
	ld l,(hl)	; $72FA
	ld h,l	; $72FB
	ld (hl),e	; $72FC
	ld (hl),e	; $72FD
	ld hl,$0022	; $72FE
	ld c,a	; $7301
	ld l,(hl)	; $7302
	jr nz,loc_01_736C	; $7303
	ld (hl),l	; $7305
	ld h,c	; $7306
	ld (hl),d	; $7307
	ld h,h	; $7308
	ld hl,$4620	; $7309
	ld l,a	; $730C
	ld l,a	; $730D
	ld l,h	; $730E
	.INCBIN "bank01.bin" SKIP $330F READ 63	; data, 63 bytes

tbl_01_734E:
	.dw fn_01_735E
	.dw fn_01_7365

loc_01_7352:
	ld (hl),e	; $7352
	ld (hl),e	; $7353

loc_01_7354:
	ld a,e	; $7354
	ld (hl),e	; $7355
	add a,b	; $7356
	ld (hl),e	; $7357
	adc a,h	; $7358
	ld (hl),e	; $7359
	sub (hl)	; $735A
	ld (hl),e	; $735B
	and b	; $735C
	ld (hl),e	; $735D

fn_01_735E:
	ld l,b	; $735E
	ld l,a	; $735F
	ld l,(hl)	; $7360
	ld h,l	; $7361

loc_01_7362:
	ld (hl),e	; $7362
	ld (hl),h	; $7363
	nop	; $7364

fn_01_7365:
	ld h,e	; $7365
	ld l,a	; $7366

loc_01_7367:
	ld l,l	; $7367
	ld (hl),b	; $7368

fn_01_7369:
	ld h,c	; $7369
	ld (hl),e	; $736A
	ld (hl),e	; $736B

loc_01_736C:
	ld l,c	; $736C
	ld l,a	; $736D
	ld l,(hl)	; $736E
	ld h,c	; $736F
	ld (hl),h	; $7370
	ld h,l	; $7371
	nop	; $7372
	halt	; $7373
	ld h,c	; $7374
	ld l,h	; $7375
	ld l,c	; $7376
	ld h,c	; $7377
	ld l,(hl)	; $7378
	ld (hl),h	; $7379
	nop	; $737A
	ld l,d	; $737B
	ld (hl),l	; $737C
	ld (hl),e	; $737D
	ld (hl),h	; $737E
	nop	; $737F
	ld (hl),e	; $7380
	ld h,c	; $7381
	ld h,e	; $7382
	ld (hl),d	; $7383
	ld l,c	; $7384
	ld h,(hl)	; $7385
	ld l,c	; $7386
	ld h,e	; $7387
	ld l,c	; $7388
	ld h,c	; $7389
	ld l,h	; $738A
	nop	; $738B
	ld l,b	; $738C
	ld l,a	; $738D
	ld l,(hl)	; $738E
	ld l,a	; $738F
	ld (hl),d	; $7390
	ld h,c	; $7391
	ld h,d	; $7392
	ld l,h	; $7393
	ld h,l	; $7394
	nop	; $7395
	ld (hl),e	; $7396
	ld (hl),b	; $7397
	ld l,c	; $7398
	ld (hl),d	; $7399
	ld l,c	; $739A
	ld (hl),h	; $739B
	ld (hl),l	; $739C
	ld h,c	; $739D
	ld l,h	; $739E
	nop	; $739F
	ld l,b	; $73A0
	ld (hl),l	; $73A1
	ld l,l	; $73A2
	ld h,d	; $73A3
	ld l,h	; $73A4
	ld h,l	; $73A5
	nop	; $73A6
	.INCBIN "bank01.bin" SKIP $33A7 READ 154	; data, 154 bytes

fn_01_7441:
	ld d,d	; $7441
	jr nz,loc_01_7497	; $7442
	ld c,c	; $7444
	ld c,h	; $7445
	ld c,e	; $7446
	nop	; $7447
	ld b,d	; $7448
	ld c,h	; $7449
	ld c,a	; $744A
	ld c,a	; $744B
	ld b,h	; $744C
	jr nz,loc_01_749C	; $744D
	ld c,a	; $744F
	ld d,e	; $7450
	ld d,e	; $7451
	nop	; $7452
	ld b,d	; $7453
	ld c,h	; $7454
	ld b,c	; $7455
	ld b,e	; $7456
	ld c,e	; $7457
	jr nz,loc_01_74AA	; $7458
	ld b,l	; $745A
	ld b,c	; $745B
	ld d,d	; $745C
	ld c,h	; $745D
	nop	; $745E
	ld c,(hl)	; $745F
	ld c,c	; $7460
	ld b,a	; $7461
	ld c,b	; $7462
	ld d,h	; $7463
	ld d,e	; $7464
	ld c,b	; $7465
	ld b,c	; $7466
	ld b,h	; $7467
	ld b,l	; $7468
	nop	; $7469
	ld c,l	; $746A
	ld b,c	; $746B
	ld c,(hl)	; $746C
	ld b,h	; $746D
	ld d,d	; $746E
	ld b,c	; $746F
	ld c,e	; $7470
	ld b,l	; $7471
	nop	; $7472

fn_01_7473:
	ld c,l	; $7473
	ld b,c	; $7474
	ld d,d	; $7475
	ld d,e	; $7476
	nop	; $7477
	ld d,(hl)	; $7478
	ld b,l	; $7479
	ld c,(hl)	; $747A
	ld d,l	; $747B
	ld d,e	; $747C
	nop	; $747D
	ld b,l	; $747E
	ld b,c	; $747F
	ld d,d	; $7480
	ld d,h	; $7481
	ld c,b	; $7482
	nop	; $7483
	ld c,e	; $7484
	ld d,d	; $7485
	ld e,c	; $7486
	ld d,b	; $7487
	ld d,h	; $7488
	ld c,a	; $7489
	ld c,(hl)	; $748A
	nop	; $748B
	ld d,(hl)	; $748C
	ld d,l	; $748D
	ld c,h	; $748E
	ld b,e	; $748F
	ld b,c	; $7490
	ld c,(hl)	; $7491
	nop	; $7492
	ld c,e	; $7493
	ld b,c	; $7494
	ld d,d	; $7495
	ld b,c	; $7496

loc_01_7497:
	ld c,l	; $7497
	ld b,l	; $7498
	ld c,c	; $7499
	ld c,e	; $749A
	ld c,a	; $749B

loc_01_749C:
	ld d,e	; $749C
	nop	; $749D
	ld b,a	; $749E
	ld c,h	; $749F
	ld c,a	; $74A0
	ld d,d	; $74A1
	ld b,c	; $74A2
	ld c,(hl)	; $74A3
	ld d,h	; $74A4
	ld c,b	; $74A5
	ld b,c	; $74A6
	nop	; $74A7
	ld d,e	; $74A8
	ld c,a	; $74A9

loc_01_74AA:
	ld d,d	; $74AA
	ld b,e	; $74AB
	ld b,l	; $74AC
	ld d,d	; $74AD
	ld c,c	; $74AE
	ld b,c	; $74AF
	nop	; $74B0
	ld b,h	; $74B1
	ld b,l	; $74B2
	ld b,e	; $74B3
	ld b,l	; $74B4
	ld c,c	; $74B5
	ld d,h	; $74B6
	nop	; $74B7
	ld b,h	; $74B8
	ld b,l	; $74B9
	ld d,e	; $74BA
	ld d,b	; $74BB
	ld c,c	; $74BC
	ld d,e	; $74BD
	ld b,l	; $74BE
	nop	; $74BF
	ld b,h	; $74C0
	ld b,l	; $74C1
	ld d,e	; $74C2
	ld d,h	; $74C3
	ld b,c	; $74C4
	ld d,d	; $74C5
	ld b,h	; $74C6
	nop	; $74C7
	ld d,a	; $74C8
	ld d,d	; $74C9
	ld c,a	; $74CA
	ld c,(hl)	; $74CB
	ld b,a	; $74CC
	nop	; $74CD
	ld b,e	; $74CE
	ld c,a	; $74CF
	ld d,(hl)	; $74D0
	ld b,l	; $74D1
	ld d,h	; $74D2
	ld c,a	; $74D3
	ld d,l	; $74D4
	ld d,e	; $74D5
	nop	; $74D6
	ld d,e	; $74D7
	ld c,b	; $74D8
	ld b,c	; $74D9
	ld c,l	; $74DA
	ld b,l	; $74DB
	nop	; $74DC
	ld c,b	; $74DD
	ld e,c	; $74DE
	ld d,h	; $74DF
	ld c,b	; $74E0
	ld c,h	; $74E1
	ld c,a	; $74E2
	ld d,h	; $74E3
	ld c,b	; $74E4
	nop	; $74E5
	ld d,h	; $74E6
	ld c,b	; $74E7
	ld b,l	; $74E8
	jr nz,loc_01_7532	; $74E9
	ld l,$53	; $74EB
	ld l,$41	; $74ED
	ld l,$00	; $74EF
	ld ($7257),hl	; $74F1
	ld l,a	; $74F4
	ld l,(hl)	; $74F5
	ld h,a	; $74F6
	ld hl,$5420	; $74F7
	ld (hl),d	; $74FA
	ld h,c	; $74FB
	halt	; $74FC
	ld h,l	; $74FD
	ld l,h	; $74FE
	jr nz,loc_01_7562	; $74FF
	ld l,(hl)	; $7501
	ld h,h	; $7502
	jr nz,$6C	; $7503
	ld h,l	; $7505
	ld h,c	; $7506
	ld (hl),d	; $7507
	ld l,(hl)	; $7508
	ld l,$22	; $7509
	nop	; $750B
	ld ($6854),hl	; $750C
	ld h,c	; $750F
	ld (hl),h	; $7510
	jr nz,loc_01_755C	; $7511
	jr nz,$63	; $7513
	ld h,c	; $7515
	ld l,(hl)	; $7516
	ld l,(hl)	; $7517
	ld l,a	; $7518
	ld (hl),h	; $7519
	jr nz,$68	; $751A
	ld h,l	; $751C
	ld l,h	; $751D
	ld (hl),b	; $751E
	jr nz,$74	; $751F
	ld l,b	; $7521
	ld h,l	; $7522
	ld h,l	; $7523
	jr nz,$77	; $7524
	ld l,c	; $7526
	ld (hl),h	; $7527
	ld l,b	; $7528
	ld l,$22	; $7529
	nop	; $752B
	ld ($6F48),hl	; $752C
	ld l,$22	; $752F
	nop	; $7531

loc_01_7532:
	ld ($6854),hl	; $7532
	ld l,a	; $7535
	ld (hl),l	; $7536
	jr nz,loc_01_759A	; $7537
	ld (hl),d	; $7539
	ld (hl),h	; $753A
	jr nz,$77	; $753B
	ld (hl),d	; $753D
	ld l,a	; $753E
	ld l,(hl)	; $753F
	ld h,a	; $7540
	ld hl,$0022	; $7541
	ld ($7349),hl	; $7544
	jr nz,loc_01_75B2	; $7547
	ld (hl),h	; $7549
	jr nz,$74	; $754A
	ld (hl),d	; $754C
	ld (hl),l	; $754D
	ld h,l	; $754E
	ccf	; $754F
	.INCBIN "bank01.bin" SKIP $3550 READ 2	; data, 2 bytes

fn_01_7552:
	push af	; $7552
	push bc	; $7553
	push de	; $7554
	push hl	; $7555
	push iy	; $7556
	ld c,a	; $7558
	ld hl,$7564	; $7559

loc_01_755C:
	rst $18	; $755C
	pop iy	; $755D
	pop hl	; $755F
	pop de	; $7560
	pop bc	; $7561

loc_01_7562:
	pop af	; $7562
	ret	; $7563

tbl_01_7564:
	.dw fn_01_7633
	.dw fn_01_7633
	.dw fn_01_7633
	.dw fn_01_7633
	.dw fn_01_7633
	.dw fn_01_7633
	.dw fn_01_7633
	inc sp	; $7572
	halt	; $7573
	.INCBIN "bank01.bin" SKIP $3574 READ 4	; data, 4 bytes

loc_01_7578:
	.INCBIN "bank01.bin" SKIP $3578 READ 12	; data, 12 bytes

loc_01_7584:
	.INCBIN "bank01.bin" SKIP $3584 READ 4	; data, 4 bytes

fn_01_7588:
	call task_enter_2	; $7588

loc_01_758B:
	ld (ix+$1A),$00	; $758B
	ld (ix+$1B),$00	; $758F
	ld (ix+$1C),$00	; $7593
	ld a,(map_mode)	; $7597

loc_01_759A:
	cp $03	; $759A
	jr c,loc_01_75A4	; $759C
	ld a,($C97F)	; $759E
	ld (ix+$1B),a	; $75A1

loc_01_75A4:
	ld a,(ix+$1A)	; $75A4
	or a	; $75A7
	jr z,loc_01_75B2	; $75A8
	cp $06	; $75AA
	jr z,loc_01_75B7	; $75AC
	add a,$07	; $75AE
	jr loc_01_75BC	; $75B0

loc_01_75B2:
	ld a,(ix+$1B)	; $75B2
	jr loc_01_75BC	; $75B5

loc_01_75B7:
	ld a,(ix+$1C)	; $75B7
	add a,$0D	; $75BA

loc_01_75BC:
	call fn_01_7552	; $75BC

loc_01_75BF:
	call frame_tick	; $75BF
	ld a,(joy_pressed)	; $75C2
	ld hl,$75CD	; $75C5
	call switch_on_a	; $75C8
	jr loc_01_75BF	; $75CB

tbl_01_75CD:
	.db $01
	.dw loc_01_75E0
	.db $02
	.dw loc_01_75E4
	.db $04
	.dw loc_01_75F8
	.db $08
	.dw loc_01_75FC
	.db $10
	.dw loc_01_7627
	.db $20
	.dw loc_01_762D
	.db $FF

loc_01_75E0:
	ld b,$06	; $75E0
	jr loc_01_75E6	; $75E2

loc_01_75E4:
	ld b,$08	; $75E4

loc_01_75E6:
	ld a,(ix+$1A)	; $75E6
	add a,b	; $75E9
	ld l,a	; $75EA
	ld h,$07	; $75EB
	call divide_8	; $75ED
	ld (ix+$1A),h	; $75F0
	call fn_01_40D9	; $75F3
	jr loc_01_75A4	; $75F6

loc_01_75F8:
	ld b,$FF	; $75F8
	jr loc_01_75FE	; $75FA

loc_01_75FC:
	ld b,$01	; $75FC

loc_01_75FE:
	ld a,(ix+$1A)	; $75FE
	cp $06	; $7601
	jr z,loc_01_7619	; $7603
	or a	; $7605
	jr nz,loc_01_75BF	; $7606
	ld a,(party_size)	; $7608
	ld h,a	; $760B
	add a,b	; $760C
	add a,(ix+$1B)	; $760D
	ld l,a	; $7610
	call divide_8	; $7611
	ld (ix+$1B),h	; $7614
	jr loc_01_7621	; $7617

loc_01_7619:
	ld a,(ix+$1C)	; $7619
	xor $01	; $761C
	ld (ix+$1C),a	; $761E

loc_01_7621:
	call fn_01_40D9	; $7621
	jp loc_01_75A4	; $7624

loc_01_7627:
	call fn_01_40D1	; $7627
	jp task_return_2	; $762A

loc_01_762D:
	call fn_01_40C9	; $762D
	jp loc_01_758B	; $7630

fn_01_7633:
	call member_record	; $7633
	ld hl,$7919	; $7636
	call fn_00_2E79	; $7639
	push iy	; $763C
	pop hl	; $763E
	ld bc,$000D	; $763F
	add hl,bc	; $7642
	call fn_01_78CE	; $7643
	inc a	; $7646
	srl a	; $7647
	neg	; $7649
	add a,$19	; $764B
	ld e,a	; $764D
	ld d,$01	; $764E
	call menu_text	; $7650
	ld de,$CAA6	; $7653
	ld a,(iy+$1A)	; $7656
	call fn_00_2D9A	; $7659
	inc e	; $765C
	inc e	; $765D
	ld a,(iy+$1B)	; $765E
	add a,$4D	; $7661
	call item_name	; $7663
	call fn_00_2D6C	; $7666
	ld a,(iy+$1C)	; $7669
	cp $47	; $766C
	ld hl,$79D7	; $766E
	jr z,loc_01_7688	; $7671
	cp $50	; $7673
	ld hl,$79E3	; $7675
	jr z,loc_01_7688	; $7678
	cp $53	; $767A
	ld hl,$79EC	; $767C
	jr z,loc_01_7688	; $767F
	cp $44	; $7681
	ld hl,$79F4	; $7683
	jr nz,loc_01_768E	; $7686

loc_01_7688:
	ld de,$CAE6	; $7688
	call fn_00_2D6C	; $768B

loc_01_768E:
	ld a,(iy+$0A)	; $768E
	add a,$30	; $7691
	ld de,$CB34	; $7693
	call fn_00_2D9A	; $7696
	ld a,(iy+$06)	; $7699
	ld de,$CB72	; $769C
	call fn_01_78C8	; $769F
	ld a,(iy+$07)	; $76A2
	ld de,$CBB2	; $76A5
	call fn_01_78C8	; $76A8
	ld a,(iy+$08)	; $76AB
	ld de,$CBF2	; $76AE
	call fn_01_78C8	; $76B1
	ld a,(iy+$09)	; $76B4
	ld de,$CC32	; $76B7
	call fn_01_78C8	; $76BA
	ld c,(iy+$08)	; $76BD
	ld a,(iy+$1B)	; $76C0
	ld hl,$47B3	; $76C3
	rst $18	; $76C6
	cp $64	; $76C7
	jr c,loc_01_76CD	; $76C9
	ld a,$63	; $76CB

loc_01_76CD:
	ld de,$CC72	; $76CD
	call fn_01_78C8	; $76D0
	ld l,(iy+$00)	; $76D3
	ld h,(iy+$01)	; $76D6
	ld de,$CCB0	; $76D9
	call fn_00_2D81	; $76DC
	ld l,(iy+$02)	; $76DF
	ld h,(iy+$03)	; $76E2
	ld de,$CCF0	; $76E5
	call fn_00_2D81	; $76E8
	ld l,(iy+$04)	; $76EB
	ld h,(iy+$05)	; $76EE
	ld de,$CD2E	; $76F1
	call fn_00_2D7B	; $76F4
	ld a,(iy+$0B)	; $76F7
	add a,$25	; $76FA
	ld d,$0E	; $76FC
	call fn_01_78EE	; $76FE
	ld a,(iy+$0C)	; $7701
	add a,$35	; $7704
	ld d,$10	; $7706
	call fn_01_78EE	; $7708
	jp menu_window_show	; $770B
	ld hl,$796D	; $770E
	call fn_01_78C2	; $7711
	ld hl,weapons_owned	; $7714
	ld b,$0F	; $7717
	call fn_01_78DE	; $7719

loc_01_771C:
	ld de,$CAA6	; $771C
	ld iy,$C920	; $771F
	ld bc,$0F00	; $7723

loc_01_7726:
	push bc	; $7726
	ld hl,$7A33	; $7727
	ld a,(iy+$00)	; $772A
	or a	; $772D
	call nz,fn_01_78FC	; $772E
	pop bc	; $7731
	inc iy	; $7732
	inc c	; $7734
	djnz loc_01_7726	; $7735
	jp menu_window_show	; $7737
	ld hl,$79B8	; $773A
	call fn_01_78C2	; $773D
	jr loc_01_771C	; $7740
	ld hl,$7976	; $7742
	call fn_01_78C2	; $7745
	ld hl,$C917	; $7748
	ld b,$07	; $774B
	call fn_01_78DE	; $774D

loc_01_7750:
	ld de,$CAE6	; $7750
	ld iy,$C918	; $7753
	ld bc,$0700	; $7757

loc_01_775A:
	push bc	; $775A
	ld hl,$7AC4	; $775B
	ld a,(iy+$00)	; $775E
	or a	; $7761
	jr z,loc_01_776C	; $7762
	call fn_01_78FC	; $7764
	ld hl,$0040	; $7767
	add hl,de	; $776A
	ex de,hl	; $776B

loc_01_776C:
	pop bc	; $776C
	inc iy	; $776D
	inc c	; $776F
	djnz loc_01_775A	; $7770
	jp menu_window_show	; $7772
	ld hl,$79C0	; $7775
	call fn_01_78C2	; $7778
	jr loc_01_7750	; $777B
	ld hl,$797F	; $777D
	call fn_01_78C2	; $7780
	ld de,$CAE6	; $7783
	ld iy,$C913	; $7786
	ld bc,$0300	; $778A

loc_01_778D:
	push bc	; $778D
	ld hl,$7B0F	; $778E
	call fn_01_78FC	; $7791
	ld hl,$0040	; $7794
	add hl,de	; $7797
	ex de,hl	; $7798
	pop bc	; $7799
	inc iy	; $779A
	inc c	; $779C
	djnz loc_01_778D	; $779D
	ld hl,$7B0F	; $779F
	ld a,(iy+$00)	; $77A2
	or a	; $77A5
	call nz,fn_01_78FC	; $77A6
	jp menu_window_show	; $77A9
	ld hl,$798B	; $77AC
	call fn_01_78C2	; $77AF
	ld de,$CAE6	; $77B2
	ld a,($C955)	; $77B5
	ld hl,$79F9	; $77B8
	ld bc,$783E	; $77BB
	call fn_01_781A	; $77BE
	ld a,($C956)	; $77C1
	ld hl,$7A00	; $77C4
	ld bc,$7846	; $77C7
	call fn_01_781A	; $77CA
	ld b,$04	; $77CD
	ld hl,$7A06	; $77CF
	call fn_01_7851	; $77D2
	ld b,$08	; $77D5
	ld hl,$7A0B	; $77D7
	call fn_01_7851	; $77DA
	ld b,$10	; $77DD
	ld hl,$7A10	; $77DF
	call fn_01_7851	; $77E2
	ld a,($C951)	; $77E5
	rlca	; $77E8
	rlca	; $77E9
	rlca	; $77EA
	and $07	; $77EB
	ld hl,$7A17	; $77ED
	ld bc,$784E	; $77F0
	call fn_01_781A	; $77F3
	ld b,$01	; $77F6
	ld hl,$7A22	; $77F8
	call fn_01_7856	; $77FB
	ld b,$02	; $77FE
	ld hl,$7A27	; $7800
	call fn_01_7856	; $7803
	ld b,$01	; $7806
	ld hl,$7A2D	; $7808
	call fn_01_7851	; $780B
	ld hl,$CAE6	; $780E
	or a	; $7811
	sbc hl,de	; $7812
	jp z,loc_01_78E5	; $7814
	jp menu_window_show	; $7817

fn_01_781A:
	or a	; $781A
	ret z	; $781B
	push af	; $781C
	push bc	; $781D
	push de	; $781E
	call fn_00_2D6C	; $781F
	pop de	; $7822
	ld hl,$0042	; $7823
	add hl,de	; $7826
	ex de,hl	; $7827
	pop hl	; $7828
	pop bc	; $7829
	ld c,b	; $782A
	ld b,$08	; $782B
	push de	; $782D

loc_01_782E:
	ld a,(hl)	; $782E
	inc hl	; $782F
	rr c	; $7830
	call c,fn_00_2D9A	; $7832
	djnz loc_01_782E	; $7835
	pop de	; $7837
	ld hl,$003E	; $7838
	add hl,de	; $783B
	ex de,hl	; $783C
	ret	; $783D
	.INCBIN "bank01.bin" SKIP $383E READ 19	; data, 19 bytes

fn_01_7851:
	ld a,($C951)	; $7851
	jr loc_01_7859	; $7854

fn_01_7856:
	ld a,($C952)	; $7856

loc_01_7859:
	and b	; $7859
	ret z	; $785A
	push de	; $785B
	call fn_00_2D6C	; $785C
	pop de	; $785F
	ld hl,$0040	; $7860
	add hl,de	; $7863
	ex de,hl	; $7864
	ret	; $7865
	ld hl,$7993	; $7866

loc_01_7869:
	call fn_01_78C2	; $7869
	ld de,$CAA6	; $786C
	ld iy,reagents	; $786F
	ld bc,$0800	; $7873

loc_01_7876:
	push bc	; $7876
	ld hl,$7B31	; $7877
	call fn_01_78FC	; $787A
	ld hl,$0040	; $787D
	add hl,de	; $7880
	ex de,hl	; $7881
	pop bc	; $7882
	inc iy	; $7883
	inc c	; $7885
	djnz loc_01_7876	; $7886
	jp menu_window_show	; $7888
	ld hl,$79C7	; $788B
	jr loc_01_7869	; $788E
	bit 0,c	; $7890
	jr z,loc_01_789F	; $7892
	ld hl,$799E	; $7894
	ld iy,mixtures	; $7897
	ld c,$00	; $789B
	jr loc_01_78A8	; $789D

loc_01_789F:
	ld hl,$79AB	; $789F
	ld iy,$C944	; $78A2
	ld c,$0D	; $78A6

loc_01_78A8:
	push bc	; $78A8
	call fn_01_78C2	; $78A9
	pop bc	; $78AC
	ld de,$CAE6	; $78AD
	ld b,$0D	; $78B0

loc_01_78B2:
	push bc	; $78B2
	ld hl,$7B84	; $78B3
	call fn_01_78FC	; $78B6
	pop bc	; $78B9
	inc iy	; $78BA
	inc c	; $78BC
	djnz loc_01_78B2	; $78BD
	jp menu_window_show	; $78BF

fn_01_78C2:
	call menu_window_clear	; $78C2
	jp menu_text_at	; $78C5

fn_01_78C8:
	ld l,a	; $78C8
	ld h,$00	; $78C9
	jp fn_00_2D87	; $78CB

fn_01_78CE:
	push bc	; $78CE
	push hl	; $78CF
	xor a	; $78D0
	ld b,a	; $78D1

loc_01_78D2:
	cp (hl)	; $78D2
	jr z,loc_01_78DA	; $78D3
	inc hl	; $78D5
	inc b	; $78D6
	jp loc_01_78D2	; $78D7

loc_01_78DA:
	ld a,b	; $78DA
	pop hl	; $78DB
	pop bc	; $78DC
	ret	; $78DD

fn_01_78DE:
	xor a	; $78DE

loc_01_78DF:
	inc hl	; $78DF
	or (hl)	; $78E0
	djnz loc_01_78DF	; $78E1
	ret nz	; $78E3
	pop af	; $78E4

loc_01_78E5:
	ld hl,$79CD	; $78E5
	call menu_text_at	; $78E8
	jp menu_window_show	; $78EB

fn_01_78EE:
	call item_name	; $78EE
	call fn_01_78CE	; $78F1
	neg	; $78F4
	add a,$1F	; $78F6
	ld e,a	; $78F8
	jp menu_text	; $78F9

fn_01_78FC:
	ld a,c	; $78FC
	rst $20	; $78FD
	push de	; $78FE
	call fn_00_2D6C	; $78FF
	pop de	; $7902
	ld hl,$0012	; $7903
	add hl,de	; $7906
	ex de,hl	; $7907
	ld a,$3A	; $7908
	call fn_00_2D9A	; $790A
	ld a,(iy+$00)	; $790D
	call fn_01_78C8	; $7910
	ld hl,$0028	; $7913
	add hl,de	; $7916
	ex de,hl	; $7917
	ret	; $7918
	.INCBIN "bank01.bin" SKIP $3919 READ 906	; data, 906 bytes

tbl_01_7CA3:
	.dw fn_03_85E0
	.dw fn_01_6660
	.INCBIN "bank01.bin" SKIP $3CA7 READ 345	; data, 345 bytes

bank01_vectors:
	jp karma_gain	; $7E00

fn_01_7E03:
	jp member_heal	; $7E03

fn_01_7E06:
	jp fn_00_37DD	; $7E06

fn_01_7E09:
	jp fn_00_36FF	; $7E09

fn_01_7E0C:
	jp loc_00_2D47	; $7E0C

fn_01_7E0F:
	jp loc_00_2D4A	; $7E0F

fn_01_7E12:
	jp fn_00_276D	; $7E12

fn_01_7E15:
	jp fn_01_46D0	; $7E15

fn_01_7E18:
	jp switch_on_a	; $7E18

fn_01_7E1B:
	jp cast_spell	; $7E1B

fn_01_7E1E:
	jp loc_00_332D	; $7E1E

fn_01_7E21:
	jp menu_window_clear	; $7E21

fn_01_7E24:
	jp fn_00_38BD	; $7E24

fn_01_7E27:
	jp fn_01_46B5	; $7E27

fn_01_7E2A:
	jp fn_01_7588	; $7E2A

fn_01_7E2D:
	jp fn_01_651E	; $7E2D

fn_01_7E30:
	jp loc_00_3CEA	; $7E30

fn_01_7E33:
	jp print_char	; $7E33

fn_01_7E36:
	jp damage_monster	; $7E36

fn_01_7E39:
	jp fn_01_40B9	; $7E39

fn_01_7E3C:
	jp divide_8	; $7E3C

fn_01_7E3F:
	jp loc_00_1B3B	; $7E3F

fn_01_7E42:
	jp loc_00_1B0A	; $7E42

fn_01_7E45:
	jp loc_00_1AF8	; $7E45

fn_01_7E48:
	jp fn_00_3D78	; $7E48

fn_01_7E4B:
	jp fn_00_2EBD	; $7E4B

fn_01_7E4E:
	jp loc_00_2F88	; $7E4E

fn_01_7E51:
	jp loc_00_2F80	; $7E51

fn_01_7E54:
	jp fn_00_3F14	; $7E54

fn_01_7E57:
	jp fn_00_333B	; $7E57

fn_01_7E5A:
	jp loc_00_0DDD	; $7E5A

fn_01_7E5D:
	jp loc_00_0DE1	; $7E5D

fn_01_7E60:
	jp menu_draw	; $7E60

fn_01_7E63:
	jp fn_01_4999	; $7E63

fn_01_7E66:
	jp fn_01_49A6	; $7E66

fn_01_7E69:
	jp fn_01_41A9	; $7E69

fn_01_7E6C:
	jp fn_01_4933	; $7E6C

fn_01_7E6F:
	jp fn_01_4D2B	; $7E6F

fn_01_7E72:
	jp fn_01_4D0A	; $7E72

fn_01_7E75:
	jp fn_01_430A	; $7E75

fn_01_7E78:
	jp loc_01_5247	; $7E78

fn_01_7E7B:
	jp loc_01_5290	; $7E7B

fn_01_7E7E:
	jp loc_01_40A6	; $7E7E

fn_01_7E81:
	jp fn_00_372E	; $7E81

fn_01_7E84:
	jp fn_00_0DD7	; $7E84

fn_01_7E87:
	jp fn_00_0DC5	; $7E87

fn_01_7E8A:
	jp fn_01_48E1	; $7E8A

fn_01_7E8D:
	jp fn_01_48EB	; $7E8D

fn_01_7E90:
	jp member_object	; $7E90

fn_01_7E93:
	jp fn_01_48F7	; $7E93

fn_01_7E96:
	jp member_record	; $7E96

fn_01_7E99:
	jp fn_00_3EB7	; $7E99

fn_01_7E9C:
	jp load_arena	; $7E9C

fn_01_7E9F:
	jp fn_01_4100	; $7E9F

fn_01_7EA2:
	jp fn_00_310B	; $7EA2

fn_01_7EA5:
	jp fn_00_31B1	; $7EA5

fn_01_7EA8:
	jp fn_00_323D	; $7EA8

fn_01_7EAB:
	jp loc_00_30F1	; $7EAB

fn_01_7EAE:
	jp fn_00_3255	; $7EAE

fn_01_7EB1:
	jp menu_step_wrap	; $7EB1

fn_01_7EB4:
	jp loc_00_2DA7	; $7EB4

fn_01_7EB7:
	jp fn_00_3974	; $7EB7

fn_01_7EBA:
	jp fn_00_392F	; $7EBA

fn_01_7EBD:
	jp fn_00_38FC	; $7EBD

fn_01_7EC0:
	jp fn_01_47D3	; $7EC0

fn_01_7EC3:
	jp multiply_8x8	; $7EC3

fn_01_7EC6:
	jp fn_01_465D	; $7EC6

fn_01_7EC9:
	jp loc_01_4483	; $7EC9

fn_01_7ECC:
	jp member_name	; $7ECC

fn_01_7ECF:
	jp member_able	; $7ECF

fn_01_7ED2:
	jp member_can_act	; $7ED2

fn_01_7ED5:
	jp fn_01_42F3	; $7ED5

fn_01_7ED8:
	jp random	; $7ED8

fn_01_7EDB:
	jp random_below	; $7EDB

fn_01_7EDE:
	jp fn_01_57D7	; $7EDE

fn_01_7EE1:
	jp fn_01_4776	; $7EE1

fn_01_7EE4:
	jp loc_00_11B4	; $7EE4

fn_01_7EE7:
	jp loc_00_32FA	; $7EE7

fn_01_7EEA:
	jp fn_00_2D0C	; $7EEA

fn_01_7EED:
	jp loc_00_2CFE	; $7EED

fn_01_7EF0:
	jp fn_00_2FA7	; $7EF0

fn_01_7EF3:
	jp loc_00_2FD8	; $7EF3

fn_01_7EF6:
	jp fn_00_30DA	; $7EF6

fn_01_7EF9:
	jp loc_00_326D	; $7EF9

fn_01_7EFC:
	jp fn_00_2D9A	; $7EFC

fn_01_7EFF:
	jp loc_00_2E88	; $7EFF

fn_01_7F02:
	jp item_name	; $7F02

fn_01_7F05:
	jp loc_00_2E02	; $7F05

fn_01_7F08:
	jp fn_00_2D6C	; $7F08

fn_01_7F0B:
	jp fn_00_2D5D	; $7F0B

fn_01_7F0E:
	jp menu_text	; $7F0E

fn_01_7F11:
	jp fn_01_40D1	; $7F11

fn_01_7F14:
	jp fn_01_40D9	; $7F14

fn_01_7F17:
	jp fn_01_40C9	; $7F17

fn_01_7F1A:
	jp fn_00_3A91	; $7F1A

fn_01_7F1D:
	jp fn_01_4719	; $7F1D

fn_01_7F20:
	jp fn_01_5978	; $7F20

fn_01_7F23:
	jp fn_00_3754	; $7F23

fn_01_7F26:
	jp fn_00_3082	; $7F26

fn_01_7F29:
	jp fn_01_4324	; $7F29

fn_01_7F2C:
	jp fn_00_34BB	; $7F2C

fn_01_7F2F:
	jp karma_lose	; $7F2F

fn_01_7F32:
	jp fn_01_4753	; $7F32

fn_01_7F35:
	jp pay	; $7F35

fn_01_7F38:
	jp member_hurt	; $7F38

fn_01_7F3B:
	jp fn_00_3340	; $7F3B

fn_01_7F3E:
	jp loc_00_32BE	; $7F3E

fn_01_7F41:
	jp menu_window_show	; $7F41

fn_01_7F44:
	jp fn_01_4117	; $7F44

fn_01_7F47:
	jp fn_00_3EEF	; $7F47

fn_01_7F4A:
	jp fn_01_62F8	; $7F4A

fn_01_7F4D:
	jp fn_00_129D	; $7F4D

fn_01_7F50:
	jp wait_button	; $7F50

fn_01_7F53:
	jp fn_00_3046	; $7F53

fn_01_7F56:
	jp frame_tick	; $7F56

fn_01_7F59:
	jp fn_00_305A	; $7F59

fn_01_7F5C:
	jp fn_00_335C	; $7F5C

fn_01_7F5F:
	jp loc_00_336E	; $7F5F

fn_01_7F62:
	jp print_text_hl	; $7F62

fn_01_7F65:
	jp print_message	; $7F65

fn_01_7F68:
	jp fn_01_55A0	; $7F68

fn_01_7F6B:
	jp print_item_name	; $7F6B

fn_01_7F6E:
	jp print_newline	; $7F6E

fn_01_7F71:
	jp fn_01_54CB	; $7F71

vec_print_inline:
	jp print_inline	; $7F74

fn_01_7F77:
	jp loc_00_2D1C	; $7F77
	.INCBIN "bank01.bin" SKIP $3F7A READ 134	; data, 134 bytes
