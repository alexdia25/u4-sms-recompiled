// Starting a fight (bank 0 $3AE2-$3CF8): "Combat", the battlefield chosen by
// where the party and the foe stand ($377E loads it), the foes rolled (how
// many, and now and then a stronger kind among them, each with its hit
// points), the party's figures set, then the combat game mode. Entered by
// jumps from the map ($3BD9), a dungeon ($3B52), a dungeon room ($3B94) and
// the ground's hazards ($3B9F); they end in the main loop.
// Cycle counts are the original's.
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

#define SLOT        cpu.ix
#define FOE         0xC97D          // the graphic of who was attacked
#define FOE_KIND    0xC97E          // ... the kind the foes are
#define FOE_TILE    0xC97C
#define UNDERFOOT   0xC990

static void call(uint16_t addr) { cpu.cycles += 17; call_original(addr); }
static uint8_t rnd(void) { call(0x2BBF); return cpu.a; }

static uint8_t strength_index(uint8_t a) { cpu.a = a; call(0x3ACE); return cpu.a; }
static uint8_t table_at(uint16_t t, uint8_t i) {   // ld hl,t / rst $28
	cpu.cycles += 10 + 48;
	SET_HL(t + i);
	cpu.a = mem_read(HL);
	return cpu.a;
}
static uint8_t random_below(uint8_t n) { cpu.a = n; call(0x2BEB); return cpu.a; }

// $3BC0: "Combat"
static void say_combat(void) {
	cpu.cycles += 17 + 17 + 19 + 17;
	text_print(0x3BC3);
	cpu.cycles += 19 + 10 + 10;
}

// $333B: n bytes at hl set to 0
static void clear(uint16_t at, uint8_t n) {
	cpu.cycles += 10 + 10 + 17;
	for (int i = 0; i < n; i++) { mem_write((uint16_t)(at + i), 0); cpu.cycles += 7 + 6 + (i < n - 1 ? 13 : 8); }
	cpu.cycles += 10;
	SET_HL(at + n);
	cpu.b = 0; cpu.c = 0;
}

// $3AE2: the battlefield, by the ground and the foe (a ship's own)
static void choose_arena(void) {
	cpu.cycles += 17 + 13 + 7 + 7;
	uint8_t arena;
	int at_sea = mem_read(0xC959) < 0x14;
	if (!at_sea) {
		cpu.cycles += 13 + 7 + 7 + 7;
		at_sea = (mem_read(UNDERFOOT) & 0xFC) == 0x10;
		if (!at_sea) cpu.cycles += 5;
	} else cpu.cycles += 5;
	cpu.cycles += 13 + 7 + 7;
	int pirates = mem_read(FOE) == 0x80;
	if (at_sea) {                      // $3AF2: on a ship
		if (pirates) { cpu.cycles += 7 + 13 + 7 + 12; mem_write(FOE_KIND, 0xC8); arena = 0x0B; }
		else {
			cpu.cycles += 5 + 13 + 7 + 7 + 7;
			if (mem_read(FOE_TILE) < 3) { cpu.cycles += 5; arena = 8; }
			else { cpu.cycles += 7 + 12; arena = 9; }
		}
	} else if (pirates) {              // $3B0F: pirates come ashore
		cpu.cycles += 7 + 13 + 7 + 12;
		mem_write(FOE_KIND, 0xC8);
		arena = 0x0C;
	} else {                           // $3B1F: by the ground
		cpu.cycles += 5 + 13 + 7 + 7 + 7;
		if (mem_read(FOE_TILE) < 3) { cpu.cycles += 5; arena = 0x0A; }
		else {
			cpu.cycles += 13 + 10 + 7;
			uint8_t g = mem_read(UNDERFOOT);
			arena = 0;
			int i;
			for (i = 0; i < 10; i++) {
				cpu.cycles += 7 + 6 + 7;
				if (mem_read(0x3B3E + 2 * i) == g) { cpu.cycles += 5 + 7; arena = mem_read(0x3B3F + 2 * i); break; }
				cpu.cycles += 6 + (i < 9 ? 13 : 8);
			}
			if (i == 10) { cpu.cycles += 4 + 12; arena = 0; }
		}
	}
	cpu.cycles += 10;                  // jp $377E: it returns for this one
	cpu.a = arena;
	call_original(0x377E);
}

// $3BFC: the foes, and the party's figures
static void roll_foes(void) {
	cpu.cycles += 17 + 15 + 10 + 10 + 10;
	uint16_t iy = cpu.iy;
	for (int i = 0; i < 16; i++) { mem_write(0xC83E + 8 * i, 0); cpu.cycles += 7 + 11 + (i < 15 ? 13 : 8); }
	cpu.cycles += 13 + 7 + 7;
	uint8_t n;
	int set = 0;
	if (mem_read(0xC975) == 1) {       // a fight in a town
		cpu.cycles += 13 + 7 + 7;
		if (mem_read(0xC99F) != 4) {
			cpu.cycles += 13 + 7 + 7;
			if (mem_read(FOE) == 0x50) { cpu.cycles += 13 + 4 + 4 + 12; n = (uint8_t)(2 * mem_read(0xC957) - 1); }   // guards
			else { cpu.cycles += 5 + 4; n = 0; }
			cpu.cycles += 19 + 12;
			mem_write(SLOT + 0x17, n);
			set = 1;
		} else cpu.cycles += 5;
	} else cpu.cycles += 5;
	if (!set) {                        // $3C2D: how many, by the kind
		cpu.cycles += 13 + 7 + 7;
		uint8_t k = mem_read(FOE_KIND);
		int roll = 1;
		uint8_t a = k;
		if (k >= 0x80) {
			uint8_t r = rnd();
			cpu.cycles += 7 + 7;
			a = r & 7;                 // (A is this, not the kind, below: as the original)
			if (a) { cpu.cycles += 5; n = a; roll = 0; }
		} else cpu.cycles += 5;
		if (roll) {
			uint8_t m = table_at(0x3D53, strength_index(a));
			cpu.cycles += 4;
			cpu.h = m;
			n = random_below(m);
			cpu.cycles += 8 + 4;
			n = (uint8_t)(n + (m >> 1));
		}
		cpu.cycles += 19;
		mem_write(SLOT + 0x17, n);
	}
	cpu.cycles += 19 + 8 + 10 + 7 + 7;   // $3C4C: not more than twice the party
	n = mem_read(SLOT + 0x17);
	if ((uint8_t)(n >> 1) >= mem_read(0xC957)) {
		cpu.cycles += 7 + 4;
		n = random_below((uint8_t)(2 * mem_read(0xC957)));
		cpu.cycles += 19;
		mem_write(SLOT + 0x17, n);
	} else cpu.cycles += 5;
	// the first foe in a random slot, of the kind attacked
	rnd();
	cpu.cycles += 7 + 4;
	cpu.c = cpu.a & 0x0F;
	call(0x48EB);                      // IY = that slot
	cpu.cycles += 13 + 4 + 12;
	uint8_t d = mem_read(FOE_KIND);
	for (;;) {
		cpu.cycles += 19 + 19 + 4;     // $3CA6
		mem_write(cpu.iy + 5, d);
		mem_write(cpu.iy + 6, d);
		uint8_t hp = table_at(0x3CFB, strength_index(d));
		cpu.cycles += 4;
		uint8_t r = random_below(hp);
		cpu.cycles += 8 + 4 + 19 + 23 + 10;
		mem_write(cpu.iy + 4, (uint8_t)(r | (hp >> 1)));
		uint8_t left = (uint8_t)(mem_read(SLOT + 0x17) - 1);
		mem_write(SLOT + 0x17, left);
		if (left & 0x80) break;
		// $3C6E: the next one in a free slot
		uint8_t b = 0;
	next_slot:
		rnd();
		cpu.cycles += 7;
		uint8_t c = cpu.a & 0x0F;
		for (;;) {
			cpu.cycles += 4;
			cpu.c = c;
			call(0x48EB);
			cpu.cycles += 19 + 4 + 7;
			if (!mem_read(cpu.iy + 5)) { cpu.cycles += 5; break; }
			cpu.cycles += 4 + 4 + 12 + 7;
			c = (uint8_t)((c + 1) & 0x0F);
		}
		cpu.cycles += 13 + 4 + 7 + 7;  // $3C81: now and then a stronger kind
		d = mem_read(FOE_KIND);
		if (d < 0x81) { cpu.cycles += 5; continue; }
		rnd();
		cpu.cycles += 7 + 7 + 7;
		b = 2;
		int up = !(cpu.a & 0x1F);
		if (up) cpu.cycles += 5;
		else {
			rnd();
			cpu.cycles += 7 + 7 + 7;
			b = 1;
			if (cpu.a & 7) { cpu.cycles += 5; continue; }
		}
		cpu.cycles += 4;               // $3C9B
		d = table_at(0x3D2F, strength_index(d));
		cpu.cycles += 4;
		if (--b) { cpu.cycles += 13; goto next_slot; }
		cpu.cycles += 8;
	}
	cpu.cycles += 14 + 10 + 10;        // $3CC4: the party's figures
	cpu.iy = 0xC8B9;
	SET_DE(0x0005);
	cpu.b = 8; cpu.c = 0;
	do {
		call(0x4445);                  // member_can_act: carry if not
		cpu.cycles += 7;
		if (cpu.f & FC) { cpu.cycles += 19 + 12; mem_write(cpu.iy + 2, 0); }
		else {
			cpu.cycles += 5;
			call(0x42F3);              // their figure
			cpu.cycles += 19 + 19;
			mem_write(cpu.iy + 2, cpu.a);
			mem_write(cpu.iy + 3, cpu.a);
		}
		cpu.cycles += 15 + 4;
		cpu.iy += 5;
		cpu.c++;
		cpu.cycles += --cpu.b ? 13 : 8;
	} while (cpu.b);
	cpu.cycles += 14 + 10;
	cpu.iy = iy;
}

// $3CEA: into combat
static void begin(void) {
	cpu.cycles += 13 + 13;
	mem_write(0xC975, mem_read(0xC99F));
	roll_foes();
	cpu.cycles += 7 + 13 + 10;
	mem_write(0xC600, 0x14);
	native_continue_at(0x0CD2);
}

// $3B9F: "Combat", the tables cleared, the battlefield
static void fight(void) {
	say_combat();
	cpu.cycles += 13 + 13;
	mem_write(FOE_KIND, mem_read(FOE));
	clear(0xC839, 0x80);               // the foes
	clear(0xC8B9, 0x28);               // the party's figures
	choose_arena();
	cpu.cycles += 10;
	begin();
}

// $3BD9: from the map: the one attacked (IY) leaves the map for the fight
static void fight_on_map(void) {
	cpu.cycles += 19 + 13 + 19 + 19 + 20;
	mem_write(FOE, mem_read(cpu.iy + 3));
	SET_DE((uint16_t)(mem_read(cpu.iy + 2) << 8 | mem_read(cpu.iy + 1)));
	wr16(0xC97A, DE);
	call(0xBACF);                      // the ground where it stood
	cpu.cycles += 13 + 4 + 19 + 19 + 19 + 10;
	mem_write(FOE_TILE, cpu.a);
	mem_write(cpu.iy + 6, 0);
	mem_write(cpu.iy + 0, 0);
	mem_write(cpu.iy + 3, 0);
	cpu.a = 0;
	fight();
}

// $3B52: in a dungeon: the battlefield by the dungeon cell
static void fight_in_dungeon(void) {
	say_combat();
	cpu.cycles += 19 + 13 + 13 + 19 + 19 + 16 + 19 + 19 + 13 + 16 + 7;
	uint8_t g = mem_read(cpu.iy + 3);
	mem_write(FOE, g);
	mem_write(FOE_KIND, g);
	SET_HL((uint16_t)(mem_read(cpu.iy + 2) << 8 | mem_read(cpu.iy + 1)));
	wr16(0xC97A, HL);
	mem_write(cpu.iy + 3, 0);
	mem_write(cpu.iy + 0, 0);
	uint8_t a = table_at(0x3B84, (uint8_t)((mem_read(0xC990) >> 4) & 0x0F));
	cpu.cycles += 7;
	cpu.a = (uint8_t)(a + 0x0F);
	call(0x377E);
	cpu.cycles += 10;
	begin();
}

// $3B94: a dungeon room's fight: its own battlefield
static void fight_in_room(void) {
	say_combat();
	cpu.cycles += 7;
	cpu.a = 0x16;
	call(0x377E);
	cpu.cycles += 10;
	begin();
}

Native native_fight_on_map = { "fight_on_map", 0x3BD9, 0, fight_on_map, 0, 0, 0, .blocking = 1 };
Native native_fight = { "fight", 0x3B9F, 0, fight, 0, 0, 0, .blocking = 1 };
Native native_fight_in_dungeon = { "fight_in_dungeon", 0x3B52, 0, fight_in_dungeon, 0, 0, 0, .blocking = 1 };
Native native_fight_in_room = { "fight_in_room", 0x3B94, 0, fight_in_room, 0, 0, 0, .blocking = 1 };
Native native_begin_combat = { "begin_combat", 0x3CEA, 0, begin, 0, 0, 0, .blocking = 1 };
