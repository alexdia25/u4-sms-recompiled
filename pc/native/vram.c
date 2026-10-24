// Copies into video memory, and the copies from another bank.
// - set_vram_address (rst $10, $0078): DE as the VDP's write address.
// - vram_write ($024C): BC bytes from HL to VRAM at DE; vram_fill ($022A):
//   the word HL, BC times; vram_fill_bytes ($0219): the byte A, BC times;
//   vram_byte_at ($0210) and vram_byte ($0213): A at DE, or where it was.
// - vram_rle ($0752) and vram_rle_slow ($0791): four run-length streams from
//   HL, one per bitplane, to VRAM at DE, DE+1, DE+2, DE+3; each byte written
//   is followed by three reads that step over the other planes (the slow
//   one waits between them). Codes: $00 ends; $01-$7F: the next byte that
//   many times; $80+n: n bytes as they are.
// - rle_interleaved ($0723): two such streams into RAM at DE and DE+1, every
//   other byte.
// - bank_push ($07CE) / bank_pop ($07DE): bank A in slot 2 for a call, the old
//   one kept on the stack; with them far_ldir ($07EC), far_vram_write
//   ($07F4), far_vram_rle ($0749), far_vram_rle_slow ($0788) and
//   far_rle_interleaved ($071A).
// - font_in_colour ($32BE) and title_picture ($32FA): the title's font in a
//   colour, and a picture of twelve tiles.
// - The title's proportional text: draw_char ($2EBD), draw_string ($2F6D),
//   draw_strings ($2F80) and draw_spaced ($2F88).
// Interrupts are on throughout (but for the two address writes), so the C
// goes instruction by instruction (I) and the machine catches up after each.
// Cycle counts are the original's.
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

static void I(int cycles) { cpu.cycles += cycles; native_boundary(); }

static uint8_t in_be(void) { return io_in((uint16_t)(cpu.a << 8 | 0xBE)); }
static void out_port(uint8_t port, uint8_t v) { io_out((uint16_t)(cpu.a << 8 | port), v); }

static void set_vram_address(void) {   // rst $10 (from the caller): $0078
	I(11); I(10);
	cpu.iff1 = cpu.iff2 = 0; I(4);
	cpu.a = cpu.e; I(4);
	out_port(0xBF, cpu.a); I(11);
	cpu.a = cpu.d; I(4);
	op_or(0x40); I(7);
	out_port(0xBF, cpu.a); I(11);
	cpu.iff1 = cpu.iff2 = 1;           // ei / ret: an interrupt pending comes after the ret
	cpu.cycles += 4 + 10;
	native_irq_point();
}

static void wait_vdp(void) { I(17); I(11); I(10); I(10); }   // call $0262: push af / pop af / ret

static void outi(void) { op_block(4, 3); I(16); }

// $0259: B, C = the count as blocks (A) of B bytes (0: 256), C = the data port
static void block_count(void) {
	I(17);
	cpu.a = cpu.b; I(4);
	cpu.b = cpu.c; I(4);
	cpu.c = 0xBE; I(7);
	cpu.b = op_inc8(cpu.b); I(4);
	cpu.b = op_dec8(cpu.b); I(4);
	if (cpu.f & FZ) { I(11); return; }
	I(5);
	cpu.a = op_inc8(cpu.a); I(4);
	I(10);
}

static void vram_write(void) {         // $024C
	set_vram_address();
	block_count();
	do {
		for (;;) {
			outi();
			if (cpu.f & FZ) { I(7); break; }
			I(12);
		}
		cpu.a = op_dec8(cpu.a); I(4);
		I(10);
	} while (!(cpu.f & FZ));
	I(10);
}

static void vram_byte(void) {          // $0213: A to VRAM at the address set
	wait_vdp();
	out_port(0xBE, cpu.a); I(11);
	I(10);
}

static void vram_byte_at(void) {       // $0210: A to VRAM at DE
	uint8_t a = cpu.a, f = cpu.f;
	push16((uint16_t)(a << 8 | f)); I(11);
	set_vram_address();
	pop16(); cpu.a = a; cpu.f = f; I(10);
	vram_byte();
}

static void vram_fill_bytes(void) {    // $0219: A, BC times, to VRAM at DE
	push16(HL); I(11);
	cpu.l = cpu.a; I(4);
	set_vram_address();
	block_count();
	do {
		do {
			io_out(BC, cpu.l); I(12);
			I(4);
			if (--cpu.b) I(13); else I(8);
		} while (cpu.b);
		cpu.a = op_dec8(cpu.a); I(4);
		I(10);
	} while (!(cpu.f & FZ));
	SET_HL(pop16()); I(10);
	I(10);
}

static void vram_fill(void) {          // $022A
	set_vram_address();
	wait_vdp();
	do {
		cpu.a = cpu.l; I(4);
		out_port(0xBE, cpu.a); I(11);
		wait_vdp();
		cpu.a = cpu.h; I(4);
		out_port(0xBE, cpu.a); I(11);
		SET_BC(BC - 1); I(6);
		cpu.a = cpu.b; I(4);
		op_or(cpu.c); I(4);
		I(10);
	} while (!(cpu.f & FZ));
	I(10);
}

static void skip_planes(int slow) {    // three reads, over the other planes
	if (!slow) { for (int i = 0; i < 3; i++) { cpu.a = in_be(); I(11); } return; }
	I(17);                             // call $07C1
	cpu.a = in_be(); I(11);
	wait_vdp();
	cpu.a = in_be(); I(11);
	wait_vdp();
	cpu.a = in_be(); I(11);
	I(10);
}

static void vram_plane(int slow) {     // $075F / $079E: one plane's stream
	set_vram_address();
	for (;;) {
		cpu.a = mem_read(HL); I(7);
		SET_HL(HL + 1); I(6);
		op_or(cpu.a); I(4);
		if (cpu.f & FZ) { I(11); return; }
		I(5);
		I(10);
		if (cpu.f & FS) {              // n bytes as they are
			op_and(0x7F); I(7);
			cpu.b = cpu.a; I(4);
			do {
				outi();
				uint8_t f = cpu.f;
				skip_planes(slow);
				cpu.f = f;
				I(10);
			} while (!(cpu.f & FZ));
			I(10);
		} else {                       // the next byte n times
			cpu.b = cpu.a; I(4);
			do {
				cpu.a = mem_read(HL); I(7);
				out_port(0xBE, cpu.a); I(11);
				skip_planes(slow);
				if (--cpu.b) I(13); else I(8);
			} while (cpu.b);
			SET_HL(HL + 1); I(6);
			I(10);
		}
	}
}

static void vram_rle_planes(int slow) {   // $0752 / $0791
	SET_BC(0x03BE); I(10);
	for (;;) {
		push16(BC); I(11);
		push16(DE); I(11);
		I(17);
		vram_plane(slow);
		SET_DE(pop16()); I(10);
		SET_BC(pop16()); I(10);
		SET_DE(DE + 1); I(6);
		if (--cpu.b) { I(13); continue; }
		I(8);
		break;
	}
	vram_plane(slow);                  // the fourth, which returns for the whole
}
static void vram_rle(void) { vram_rle_planes(0); }
static void vram_rle_slow(void) { vram_rle_planes(1); }

static void rle_stream(void) {         // $0729: one stream, every other byte
	for (;;) {
		cpu.a = mem_read(HL); I(7);
		SET_HL(HL + 1); I(6);
		op_or(cpu.a); I(4);
		if (cpu.f & FZ) { I(11); return; }
		I(5);
		I(10);
		if (cpu.f & FS) {
			op_and(0x7F); I(7);
			cpu.c = cpu.a; I(4);
			cpu.b = 0; I(7);
			do {
				op_block(4, 0); I(16);   // ldi
				SET_DE(DE + 1); I(6);
				I(10);
			} while (cpu.f & FP);
			I(10);
		} else {
			cpu.b = cpu.a; I(4);
			cpu.a = mem_read(HL); I(7);
			do {
				mem_write(DE, cpu.a); I(7);
				SET_DE(DE + 1); I(6);
				SET_DE(DE + 1); I(6);
				if (--cpu.b) { I(13); continue; }
				I(8);
				break;
			} while (1);
			SET_HL(HL + 1); I(6);
			I(10);
		}
	}
}

static void rle_interleaved(void) {    // $0723
	push16(DE); I(11);
	I(17);
	rle_stream();
	SET_DE(pop16()); I(10);
	SET_DE(DE + 1); I(6);
	rle_stream();
}

// bank_push from a call at `site`: the old bank kept on the stack below
static void bank_push_body(void) {     // $07CE, the return on the stack
	wr16(0xC0C2, HL); I(16);
	SET_HL(0xDFFF); I(10);
	cpu.h = mem_read(HL); I(7);
	{ uint16_t t = rd16(cpu.sp); wr16(cpu.sp, HL); SET_HL(t); } I(19);
	push16(HL); I(11);
	SET_HL(rd16(0xC0C2)); I(16);
	mem_write(0xFFFF, cpu.a); I(13);
	I(10);                             // ret
}

static void bank_push(uint16_t site) {
	push16((uint16_t)(site + 3)); I(17);
	bank_push_body();
	pop16();
}

// as natives, for the original's calls: the return is popped for them
static void nat_bank_push(void) { bank_push_body(); }
static void bank_pop_body(void) {      // $07DE
	wr16(0xC0C2, HL); I(16);
	SET_HL(pop16()); I(10);
	cpu.l = cpu.a; I(4);
	cpu.a = cpu.h; I(4);
	mem_write(0xFFFF, cpu.a); I(13);
	cpu.a = cpu.l; I(4);
	SET_HL(rd16(0xC0C2)); I(16);
	I(10);                             // ret, to the caller's caller
}
static void nat_bank_pop(void) { bank_pop_body(); }   // reached by jp

static void bank_pop_body(void);
static void bank_pop(void) {           // jp $07DE: and the ret is the caller's
	I(10);
	bank_pop_body();
}

static void far_ldir(void) {           // $07EC
	bank_push(0x07EC);
	while (op_block(6, 0)) I(21);
	I(16);
	bank_pop();
}

static void far_call(uint16_t site, void (*body)(void)) {
	bank_push(site);
	push16((uint16_t)(site + 6)); I(17);
	body();
	pop16();                           // its ret, counted in the body
	bank_pop();
}
static void far_vram_write(void) { far_call(0x07F4, vram_write); }           // $07F4
static void far_vram_rle(void) { far_call(0x0749, vram_rle); }               // $0749
static void far_vram_rle_slow(void) { far_call(0x0788, vram_rle_slow); }     // $0788
static void far_rle_interleaved(void) { far_call(0x071A, rle_interleaved); } // $071A

// $32BE (by jp from the vector $7F3E): the font in colour A. Its 126
// glyphs (bank $14 at $8840, three planes a row) go to VRAM $2840, each
// plane masked by the colour's eight bytes at $9410 + A*8 (a byte a row);
// the fourth plane is stepped over with a read.
static void font_in_colour(void) {
	cpu.l = cpu.a; I(4);
	cpu.h = 0; I(7);
	for (int i = 0; i < 3; i++) { SET_HL(op_add16(HL, HL)); I(11); }
	SET_DE(0x9410); I(10);
	SET_HL(op_add16(HL, DE)); I(11);
	cpu.a = 0x14; I(7);
	bank_push(0x32CA);
	SET_DE(0x2840); I(10);
	set_vram_address();
	SET_DE(0x8840); I(10);
	cpu.a = 0x7E; I(7);
	do {
		op_ex_af(); I(4);
		push16(HL); I(11);
		cpu.b = 8; I(7);
		do {
			cpu.c = mem_read(HL); I(7);
			for (int p = 0; p < 3; p++) {
				cpu.a = mem_read(DE); I(7);
				op_and(cpu.c); I(4);
				out_port(0xBE, cpu.a); I(11);
				SET_DE(DE + 1); I(6);
			}
			SET_HL(HL + 1); I(6);
			I(4);
			cpu.a = in_be(); I(11);
			I(4);
			if (--cpu.b) I(13); else I(8);
		} while (cpu.b);
		SET_HL(pop16()); I(10);
		op_ex_af(); I(4);
		cpu.a = op_dec8(cpu.a); I(4);
		I(10);
	} while (!(cpu.f & FZ));
	bank_pop();
}

// $32FA (by jp from the vector $7EE7): picture A of the title, twelve
// tiles listed at $ABB0 + A*12 (bank $14), each 32 bytes at $9610 + n*32,
// to VRAM at DE
static void title_picture(void) {
	cpu.l = cpu.a; I(4);
	cpu.h = 0; I(7);
	cpu.c = cpu.l; I(4);
	cpu.b = cpu.h; I(4);
	SET_HL(op_add16(HL, HL)); I(11);
	SET_HL(op_add16(HL, BC)); I(11);
	SET_HL(op_add16(HL, HL)); I(11);
	SET_HL(op_add16(HL, HL)); I(11);
	SET_BC(0xABB0); I(10);
	SET_HL(op_add16(HL, BC)); I(11);
	cpu.a = 0x14; I(7);
	bank_push(0x3309);
	set_vram_address();
	cpu.a = 0x0C; I(7);
	cpu.c = 0xBE; I(7);
	SET_DE(0x9610); I(10);
	do {
		push16(HL); I(11);
		cpu.l = mem_read(HL); I(7);
		cpu.h = 0; I(7);
		for (int i = 0; i < 5; i++) { SET_HL(op_add16(HL, HL)); I(11); }
		SET_HL(op_add16(HL, DE)); I(11);
		cpu.b = 0x20; I(7);
		for (;;) {
			outi();
			if (cpu.f & FZ) { I(7); break; }
			I(12);
		}
		SET_HL(pop16()); I(10);
		SET_HL(HL + 1); I(6);
		cpu.a = op_dec8(cpu.a); I(4);
		I(10);
	} while (!(cpu.f & FZ));
	bank_pop();
}

// ------------------------------------------------------- the title's text --
// A proportional font drawn into tiles already in VRAM: the text area's
// tiles start at $C608 (VRAM), $C60A apart a row of tiles; the pen is at
// pixel $C60C across, tile row $C60D. A glyph (8 rows, bank $0F at $BB58)
// is shifted to the pen's pixel and ORed into the two tiles it covers,
// which are read back first (to $C1C0 and $C1E0); the pen moves 8 on.
static void set_read_address(void) {   // rst $08: $006F, HL as the read address
	I(11); I(10);
	cpu.iff1 = cpu.iff2 = 0; I(4);
	cpu.a = cpu.l; I(4);
	out_port(0xBF, cpu.a); I(11);
	cpu.a = cpu.h; I(4);
	out_port(0xBF, cpu.a); I(11);
	cpu.iff1 = cpu.iff2 = 1;
	cpu.cycles += 4 + 10;
	native_irq_point();
}

static void ini_block(void) {          // ini / jr nz, B bytes from the VDP to HL
	for (;;) {
		op_block(4, 2); I(16);
		if (cpu.f & FZ) { I(7); break; }
		I(12);
	}
}

static void exx(void) { op_exx(); I(4); }

static void bank_pop(void);

static void draw_char(void) {          // $2EBD: character A
	op_sub8(0x20, 0, 1); I(7);
	cpu.l = cpu.a; I(4);
	cpu.h = 0; I(7);
	for (int i = 0; i < 3; i++) { SET_HL(op_add16(HL, HL)); I(11); }
	SET_DE(0xBB58); I(10);
	SET_HL(op_add16(HL, DE)); I(11);
	cpu.a = 0x0F; I(7);
	bank_push(0x2ECB);
	cpu.a = mem_read(0xC60C); I(13);
	cpu.b = cpu.a; I(4);
	op_and(0x07); I(7);
	cpu.a = op_inc8(cpu.a); I(4);
	cpu.c = cpu.a; I(4);               // the shift: pixel within the tile + 1
	cpu.a = cpu.b; I(4);
	exx();
	for (int i = 0; i < 3; i++) { op_rrca(); I(4); }
	op_and(0x1F); I(7);
	SET_HL(rd16(0xC608)); I(16);
	if (cpu.f & FZ) I(12);
	else {                             // the tile column: A rows of tiles on
		I(7);
		SET_DE(rd16(0xC60A)); I(20);
		cpu.b = cpu.a; I(4);
		do {
			SET_HL(op_add16(HL, DE)); I(11);
			if (--cpu.b) I(13); else I(8);
		} while (cpu.b);
	}
	{ uint16_t t = HL; SET_HL(DE); SET_DE(t); } I(4);
	cpu.a = mem_read(0xC60D); I(13);
	cpu.l = cpu.a; I(4);
	cpu.h = 0; I(7);
	SET_HL(op_add16(HL, HL)); I(11);
	SET_HL(op_add16(HL, HL)); I(11);
	SET_HL(op_add16(HL, DE)); I(11);
	push16(HL); I(11);
	set_read_address();
	{ uint16_t t = HL; SET_HL(DE); SET_DE(t); } I(4);
	SET_HL(0xC1C0); I(10);
	SET_BC(0x20BE); I(10);
	ini_block();
	SET_HL(rd16(0xC60A)); I(16);
	SET_HL(op_add16(HL, DE)); I(11);
	set_read_address();
	SET_HL(0xC1E0); I(10);
	cpu.b = 0x20; I(7);
	ini_block();
	SET_DE(pop16()); I(10);
	set_vram_address();
	push16(DE); I(11);
	exx();
	push16(cpu.iy); I(15);
	cpu.iy = 0xC1C0; I(14);
	cpu.d = 0; I(7);
	cpu.a = 0x08; I(7);
	do {                               // each row: the glyph's byte shifted across two tiles
		op_ex_af(); I(4);
		cpu.a = mem_read(cpu.iy); I(19);
		out_port(0xBE, cpu.a); I(11);
		cpu.a = mem_read(HL); I(7);
		SET_HL(HL + 1); I(6);
		cpu.e = cpu.d; I(4);
		cpu.b = cpu.c; I(4);
		cpu.b = op_dec8(cpu.b); I(4);
		if (cpu.f & FZ) I(12);
		else {
			I(7);
			op_or(cpu.a); I(4);
			do {
				op_rra(); I(4);
				cpu.e = op_rot(3, cpu.e); I(8);
				if (--cpu.b) I(13); else I(8);
			} while (cpu.b);
		}
		op_or(mem_read((uint16_t)(cpu.iy + 1))); I(19);
		out_port(0xBE, cpu.a); I(11);
		cpu.a = cpu.e; I(4);
		op_or(mem_read((uint16_t)(cpu.iy + 0x21))); I(19);
		mem_write((uint16_t)(cpu.iy + 0x21), cpu.a); I(19);
		cpu.a = mem_read((uint16_t)(cpu.iy + 2)); I(19);
		out_port(0xBE, cpu.a); I(11);
		cpu.a = mem_read((uint16_t)(cpu.iy + 3)); I(19);
		out_port(0xBE, cpu.a); I(11);
		cpu.e = 0x04; I(7);
		cpu.iy = op_add16(cpu.iy, DE); I(15);
		op_ex_af(); I(4);
		cpu.a = op_dec8(cpu.a); I(4);
		I(10);
	} while (!(cpu.f & FZ));
	cpu.iy = pop16(); I(14);
	SET_DE(pop16()); I(10);
	SET_HL(rd16(0xC60A)); I(16);
	SET_HL(op_add16(HL, DE)); I(11);
	{ uint16_t t = HL; SET_HL(DE); SET_DE(t); } I(4);
	set_vram_address();                // the next tile, with what the glyph left over
	SET_HL(0xC1E0); I(10);
	SET_BC(0x20BE); I(10);
	for (;;) {
		outi();
		if (cpu.f & FZ) { I(7); break; }
		I(12);
	}
	cpu.a = mem_read(0xC60C); I(13);
	op_add8(0x08, 0); I(7);
	mem_write(0xC60C, cpu.a); I(13);
	bank_pop();
}

static void pen_from_hl(void) {        // ld e,(hl) / inc hl / ld d,(hl) / inc hl / ld ($C60C),de
	cpu.e = mem_read(HL); I(7);
	SET_HL(HL + 1); I(6);
	cpu.d = mem_read(HL); I(7);
	SET_HL(HL + 1); I(6);
	wr16(0xC60C, DE); I(20);
}

static void draw_string(void) {        // $2F6D: the pen's place, then text up to a 0
	pen_from_hl();
	for (;;) {
		cpu.a = mem_read(HL); I(7);
		SET_HL(HL + 1); I(6);
		op_or(cpu.a); I(4);
		if (cpu.f & FZ) { I(11); return; }
		I(5);
		push16(HL); I(11);
		I(17); draw_char();
		SET_HL(pop16()); I(10);
		I(12);
	}
}

static void draw_strings(void) {       // $2F80: B of them
	do {
		push16(BC); I(11);
		I(17); draw_string();
		SET_BC(pop16()); I(10);
		if (--cpu.b) I(13); else I(8);
	} while (cpu.b);
	I(10);
}

static void draw_spaced(void) {        // $2F88: each character followed by its width (0 ends)
	pen_from_hl();
	for (;;) {
		cpu.a = mem_read(HL); I(7);
		SET_HL(HL + 1); I(6);
		push16(HL); I(11);
		I(17); draw_char();
		SET_HL(pop16()); I(10);
		cpu.b = mem_read(HL); I(7);
		SET_HL(HL + 1); I(6);
		cpu.b = op_inc8(cpu.b); I(4);
		cpu.b = op_dec8(cpu.b); I(4);
		if (cpu.f & FZ) { I(11); return; }
		I(5);
		cpu.a = mem_read(0xC60C); I(13);
		op_add8(0xF8, 0); I(7);
		op_add8(cpu.b, 0); I(4);
		mem_write(0xC60C, cpu.a); I(13);
		I(12);
	}
}

// $2FA7: the title's font for text drawn into tiles: $63 glyphs of bank
// $0F at $BB40 to VRAM $2BA0, as the inverse in plane 0, the glyph in
// plane 1 and planes 2-3 clear
static void title_font(void) {
	cpu.a = 0x0F; I(7);
	bank_push(0x2FA9);
	SET_DE(0x2BA0); I(10);
	set_vram_address();
	SET_HL(0xBB40); I(10);
	cpu.c = 0x63; I(7);
	do {
		cpu.b = 0x08; I(7);
		do {
			cpu.a = mem_read(HL); I(7);
			SET_HL(HL + 1); I(6);
			op_cpl(); I(4);
			out_port(0xBE, cpu.a); I(11);
			wait_vdp();
			op_cpl(); I(4);
			out_port(0xBE, cpu.a); I(11);
			wait_vdp();
			op_xor(cpu.a); I(4);
			out_port(0xBE, cpu.a); I(11);
			wait_vdp();
			out_port(0xBE, cpu.a); I(11);
			wait_vdp();
			if (--cpu.b) I(13); else I(8);
		} while (cpu.b);
		cpu.c = op_dec8(cpu.c); I(4);
		if (cpu.f & FZ) { I(7); break; }
		I(12);
	} while (1);
	bank_pop();
}

// for the other files (game.h)
void vram_bank_push(uint16_t site) { bank_push(site); }
void vram_bank_pop(void) { bank_pop(); }
void vram_set_write_address(void) { set_vram_address(); }

static const uint16_t bank_scratch[] = { 0xC0C2, 0xC0C3 };
#define VRAM(name, addr) Native native_##name = { #name, addr, 0, name, 0, bank_scratch, 2, .long_calls = 1 };
VRAM(vram_write, 0x024C)
VRAM(vram_fill, 0x022A)
VRAM(vram_rle, 0x0752)
VRAM(vram_rle_slow, 0x0791)
VRAM(rle_interleaved, 0x0723)
VRAM(far_ldir, 0x07EC)
VRAM(far_vram_write, 0x07F4)
VRAM(far_vram_rle, 0x0749)
VRAM(far_vram_rle_slow, 0x0788)
VRAM(far_rle_interleaved, 0x071A)
VRAM(font_in_colour, 0x32BE)
VRAM(title_picture, 0x32FA)
VRAM(vram_byte, 0x0213)
VRAM(vram_byte_at, 0x0210)
VRAM(vram_fill_bytes, 0x0219)
VRAM(nat_bank_push, 0x07CE)
VRAM(nat_bank_pop, 0x07DE)
VRAM(draw_char, 0x2EBD)
VRAM(draw_string, 0x2F6D)
VRAM(draw_strings, 0x2F80)
VRAM(draw_spaced, 0x2F88)
VRAM(title_font, 0x2FA7)
