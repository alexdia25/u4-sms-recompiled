// Combat rules, rewritten from banks 0, 1 and 3.
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"

#define PARTY          0xC751   // 29-byte member records: +0 hp (16-bit), +$1C status
#define PARTY_OBJECTS  0xC8B9   // 5 bytes per member on the combat/map screen: +2/+3 graphic
#define STATUS_DEAD    'D'

// Cycle counts below are the original's (T-states from entry to ret).
// member_record ($48B6) and member_object ($48D5) step through the table
// one entry at a time.
static int member_record_cycles(uint8_t m) { return 145 + 24 * m; }
static int member_object_cycles(uint8_t m) { return 155 + 24 * m; }
static int popcount8(uint8_t v) { int n = 0; for (; v; v >>= 1) n += v & 1; return n; }

// divide_8 ($2AFC): L = L / H, H = A = L % H (H must not be 0).
static void nat_divide_8(void) {
	uint8_t q = cpu.l / cpu.h, r = cpu.l % cpu.h;
	cpu.l = q;
	cpu.h = r;
	cpu.a = r;
	cpu.cycles += 242 + 3 * popcount8(q);   // each 1 bit of the quotient costs a subtract
}
Native native_divide_8 = { "divide_8", 0x2AFC, 0, nat_divide_8, OUT_A | OUT_HL, 0, 0 };

// member_take_damage ($467F): party member C loses HL hit points. At zero
// or below the member dies: hit points 0, status 'D', and their figure on
// the screen becomes the corpse graphic ($38). Carry is set if they died.
static void nat_member_take_damage(void) {
	uint16_t rec = PARTY + cpu.c * 29;
	uint16_t hp = rd16(rec), damage = HL;
	cpu.cycles += member_record_cycles(cpu.c);
	if (damage > hp) {
		cpu.cycles += 620 - 145 - 155 + member_object_cycles(cpu.c);
		wr16(rec, 0);
		mem_write(rec + 0x1C, STATUS_DEAD);
		uint16_t obj = PARTY_OBJECTS + cpu.c * 5;
		mem_write(obj + 2, 0x38);
		mem_write(obj + 3, 0x38);
		cpu.f |= FC;
	} else {
		wr16(rec, hp - damage);
		cpu.f &= ~FC;
		cpu.cycles += 354 - 145;
	}
}
Native native_member_take_damage = { "member_take_damage", 0x467F, 1, nat_member_take_damage, OUT_CARRY, 0, 0 };

// combat_strength_index ($3ACE in bank 0, same code at $A42A in bank 3):
// turn the graphic number of a figure on the map into its row of the
// combat_strength table. Monsters have graphics $80 and up, two animation
// frames each: index = graphic/2 & $3F, folding the 8 sea monsters' extra
// frames in (indexes 8 and up are halved and moved up by 4). People (graphics
// below $80) get rows $24-$43: index = (graphic & $1F) + $24.
static uint8_t combat_strength_index(uint8_t graphic) {
	if (graphic < 0x80) return (graphic & 0x1F) + 0x24;
	uint8_t i = (uint8_t)((graphic >> 1) | (graphic << 7)) & 0x3F;   // rrca
	if (i < 8) return i;
	return (i >> 1) + 4;
}

static void nat_combat_strength_index(void) {
	uint8_t g = cpu.a;
	cpu.a = combat_strength_index(g);
	cpu.cycles += g >= 0x80 && cpu.a >= 8 ? 62 : 43;   // halving the sea monsters' index costs more
}
Native native_combat_strength_index = { "combat_strength_index", 0x3ACE, 0, nat_combat_strength_index, OUT_A, 0, 0 };
Native native_combat_strength_index_b3 = { "combat_strength_index_b3", 0xA42A, 3, nat_combat_strength_index, OUT_A, 0, 0 };

// cap_9999 ($36DA): HL = min(HL, 9999); carry set if it was capped.
static int cap_cycles(uint16_t v) { return v < 9999 ? 87 : 94; }

static void nat_cap_9999(void) {
	cpu.cycles += cap_cycles(HL);
	if (HL > 9999) { SET_HL(9999); cpu.f |= FC; }
	else cpu.f &= ~FC;
}
Native native_cap_9999 = { "cap_9999", 0x36DA, 0, nat_cap_9999, OUT_HL | OUT_CARRY, 0, 0 };

// member_gain_xp ($4205): party member C gains HL experience, at most 9999.
static void nat_member_gain_xp(void) {
	uint16_t rec = PARTY + cpu.c * 29;
	uint32_t xp = rd16(rec + 4) + HL;
	wr16(rec + 4, xp > 9999 ? 9999 : (uint16_t)xp);
	cpu.cycles += 347 - 145 + member_record_cycles(cpu.c) + cap_cycles((uint16_t)xp);
}
Native native_member_gain_xp = { "member_gain_xp", 0x4205, 1, nat_member_gain_xp, 0, 0, 0 };

// member_heal ($4221): party member C regains HL hit points, up to their
// maximum. (The sum is 16-bit, as in the original.)
static void nat_member_heal(void) {
	uint16_t rec = PARTY + cpu.c * 29;
	uint32_t sum = rd16(rec) + HL;
	uint16_t max = rd16(rec + 2);
	// the original subtracts with the carry of the 16-bit add still set
	int32_t room = (int32_t)max - (int32_t)(sum & 0xFFFF) - (sum > 0xFFFF);
	wr16(rec, room >= 0 ? (uint16_t)sum : (uint16_t)(max - (sum > 0xFFFF)));
	cpu.cycles += 387 - 145 + member_record_cycles(cpu.c) + (room >= 0 ? 12 : 22);
}
Native native_member_heal = { "member_heal", 0x4221, 1, nat_member_heal, 0, 0, 0 };
