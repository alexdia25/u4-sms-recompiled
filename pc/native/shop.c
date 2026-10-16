// Shops, rewritten from bank 2 as C tasks (see talk.c for how C tasks work).
//
// The Talk command, facing a shop counter, picks the shop's handler from
// shop_handlers ($9741) by its type and jumps there with the shop's number
// (0-based, per type) in shop_number. Each handler greets, offers Buy / Sell
// (or its own services), and ends at $96B2: a farewell message, music, and
// back to the Talk command ($8084).
//
// The cycle counts are the original's (tools/cycles.py), added where the
// original spends them. Routines not rewritten yet run with call_original.
#include "../runtime/native.h"
#include "../runtime/sms.h"
#include "../runtime/z80.h"
#include "game.h"

#define SLOT           cpu.ix
#define SHOP_NUMBER    0xC983
#define GOLD           0xC909
#define PARTY_SIZE     0xC957

// the original's ways of calling things, with their cycles
static void call(uint16_t addr) { cpu.cycles += 17; call_original(addr); }
static void msg(uint8_t n) { cpu.cycles += 7; cpu.a = n; call(0x5582); }        // ld a,n / call print_message
static void msg_a(void) { call(0x5582); }                                        // call print_message
static void msg_shop(uint8_t add) {    // ld a,(shop_number) / add a,n / call print_message
	cpu.cycles += 13 + 7;
	cpu.a = (uint8_t)(mem_read(SHOP_NUMBER) + add);
	call(0x5582);
}
static void newline(void) { cpu.cycles += 17; text_newline(); }                  // call print_newline
static void blank_line(void) { cpu.cycles += 17 + 17; text_newline(); text_newline(); }   // call $54CB
static void quote(void) { cpu.cycles += 17 + 7; text_char('"'); }                // call $548D
static void put(uint8_t ch) { cpu.cycles += 7 + 17; text_char(ch); }             // ld a,ch / call print_char
static void shop_sound(uint8_t n) { cpu.cycles += 7; cpu.a = n; call(0x7552); }  // ld a,n / call $7552
static uint16_t word_at(uint16_t table, uint8_t i) {   // ld hl,table / rst $20: HL = word table[i]
	cpu.cycles += 10 + (i < 0x80 ? 91 : 90);
	uint16_t at = (uint16_t)(table + 2 * i);
	SET_HL(rd16(at));
	cpu.a = mem_read(at);
	return HL;
}
static uint16_t rst30(uint16_t hl, uint8_t a) {      // rst $30: HL += A
	uint16_t r = (uint16_t)(hl + a);
	cpu.cycles += 11 + 4 + 4 + ((uint8_t)(hl + a) < a ? 5 + 4 + 10 : 11);
	SET_HL(r);
	cpu.a = (uint8_t)r;
	return r;
}

// a menu or prompt: frame_tick at site, then call `check` until it says done
// (the loop `call frame_tick / call check / jr z|c,loop`)
static void until_z_clear(uint16_t site, uint16_t check) {   // jr z,loop
	for (;;) {
		task_frame_tick(site);
		call(check);
		cpu.cycles += 7;
		if (!(cpu.f & FZ)) return;
		cpu.cycles += 5;
	}
}
static void until_c_clear(uint16_t site, uint16_t check) {   // jr c,loop
	for (;;) {
		task_frame_tick(site);
		call(check);
		cpu.cycles += 7;
		if (!(cpu.f & FC)) return;
		cpu.cycles += 5;
	}
}

// $96B2: the last message (A), music, and back to the Talk command
static void leave_shop(void) {
	msg_a();
	newline();
	call(0x33E5);                      // which music for this place
	call(0x33AF);                      // play_music_later
	cpu.cycles += 10;
	native_continue_at(0x8084);
}

// ---------------------------------------------------- weapons, armour --
// The weapon shop ($97D5) and the armour shop ($9AE3) are the same code with
// different messages and tables. The armour shops leave empty slots out of
// their lists (a shop may sell fewer than 4 kinds) and don't add an "s".
typedef struct ArmsShop {
	uint8_t sound;          // $7552 sound/picture number
	uint8_t greet, shop_add, ask, bye;   // messages, and shop_number + shop_add
	uint8_t intro, list, which, about_add, cant_buy, how_many, none, poor, sold, more;
	uint8_t sell_first, sell_nothing, sell_which, sell_how_many, sell_how_many2, too_many;
	uint8_t offer, offer_n, offer_some, offer_one, offer_single, deal, no_deal, done_deal;
	uint8_t menu_add;       // added to the item number for its menu line ($340F)
	uint16_t stock, prices, owned;
	uint16_t name;          // prints an item's name: $AB85 weapons, $AB89 armour
	uint16_t has_any;       // anything to sell: $ABB7 / $ABBE
	uint16_t sell_menu_de;
	int plural;             // the list says "Daggers"
	int skip_empty;         // the list leaves out empty stock slots
	// the addresses of the original's waits (the slot's resume state)
	uint16_t at_greet_wait, at_menu, at_intro_wait, at_list_wait, at_pick, at_about_wait, at_count,
	         at_said_wait, at_more, at_sell_pick, at_sell_count, at_deal_wait, at_deal, at_sell_said_wait;
} ArmsShop;

static const ArmsShop weapons = {
	.sound = 8, .greet = 0x31, .shop_add = 6, .ask = 0x32, .bye = 0x33,
	.intro = 0x1C, .list = 0x1D, .which = 0x1E, .about_add = 0x0B, .cant_buy = 0x1F, .how_many = 0x20,
	.none = 0x21, .poor = 0x1A, .sold = 0x1B, .more = 0x22,
	.sell_first = 0x23, .sell_nothing = 0x25, .sell_which = 0x24, .sell_how_many = 0x26,
	.sell_how_many2 = 0x27, .too_many = 0x28,
	.offer = 0x29, .offer_n = 0x2A, .offer_some = 0x2B, .offer_one = 0x2C, .offer_single = 0x2D,
	.deal = 0x2E, .no_deal = 0x2F, .done_deal = 0x30,
	.menu_add = 0x25, .stock = 0x9829, .prices = 0x9841, .owned = 0xC91F, .name = 0xAB85,
	.has_any = 0xABB7, .sell_menu_de = 0x0101, .plural = 1, .skip_empty = 0,
	.at_greet_wait = 0x97E8, .at_menu = 0x97FE, .at_intro_wait = 0x9869, .at_list_wait = 0x989F,
	.at_pick = 0x98D6, .at_about_wait = 0x990D, .at_count = 0x991D, .at_said_wait = 0x9974,
	.at_more = 0x9982, .at_sell_pick = 0x99C9, .at_sell_count = 0x9A26, .at_deal_wait = 0x9A8D,
	.at_deal = 0x9A9B, .at_sell_said_wait = 0x9ADD,
};
static const ArmsShop armour = {
	.sound = 9, .greet = 0x29, .shop_add = 5, .ask = 0x2A, .bye = 0x2B,
	.intro = 0x13, .list = 0x14, .which = 0x15, .about_add = 0x0A, .cant_buy = 0x16, .how_many = 0x17,
	.none = 0x18, .poor = 0x11, .sold = 0x12, .more = 0x19,
	.sell_first = 0x1A, .sell_nothing = 0x1C, .sell_which = 0x1B, .sell_how_many = 0x1D,
	.sell_how_many2 = 0x1E, .too_many = 0x1F,
	.offer = 0x20, .offer_n = 0x21, .offer_some = 0x22, .offer_one = 0x23, .offer_single = 0x25,
	.deal = 0x26, .no_deal = 0x27, .done_deal = 0x28,
	.menu_add = 0x35, .stock = 0x9B37, .prices = 0x9B4B, .owned = 0xC917, .name = 0xAB89,
	.has_any = 0xABBE, .sell_menu_de = 0x0202, .plural = 0, .skip_empty = 1,
	.at_greet_wait = 0x9AF6, .at_menu = 0x9B0C, .at_intro_wait = 0x9B63, .at_list_wait = 0x9B9A,
	.at_pick = 0x9BDB, .at_about_wait = 0x9C13, .at_count = 0x9C23, .at_said_wait = 0x9C7A,
	.at_more = 0x9C88, .at_sell_pick = 0x9CCF, .at_sell_count = 0x9D2C, .at_deal_wait = 0x9D93,
	.at_deal = 0x9DA1, .at_sell_said_wait = 0x9DE3,
};

static uint8_t in_stock(const ArmsShop *sh) {   // $9999 / $9C9F: A = item at slot (ix+$1A)
	cpu.cycles += 17 + 13 + 4 + 4 + 19 + 10 + 11 + 4 + 4 + 4 + 4 + 4 + 7 + 10 + 10;
	uint8_t i = (uint8_t)(mem_read(SHOP_NUMBER) * 4 + mem_read(SLOT + 0x1A));
	SET_HL(sh->stock + i);
	cpu.a = mem_read(HL);
	return cpu.a;
}

enum { LEAVE_FIRST, LEAVE_SECOND };    // leave with two newlines ($9816) or one ($9819)

static void arms_leave(const ArmsShop *sh, int how) {
	if (how == LEAVE_FIRST) newline();
	newline();
	msg_shop(sh->shop_add);
	cpu.cycles += 7 + 10;
	cpu.a = sh->bye;
	leave_shop();
}

static int arms_buy(const ArmsShop *sh);
static int arms_sell(const ArmsShop *sh);

static void arms_shop(const ArmsShop *sh) {
	msg(sh->greet);
	cpu.cycles += 13;
	cpu.a = mem_read(SHOP_NUMBER);
	msg_a();                           // the shop's name
	quote();
	shop_sound(sh->sound);
	task_wait_button(sh->at_greet_wait);
	blank_line();
	msg_shop(sh->shop_add);
	msg(sh->ask);                      // buy or sell?
	call(0xAB4B);
	until_z_clear(sh->at_menu, 0xAB55);
	uint8_t a = cpu.a, f = cpu.f;
	cpu.cycles += 11;                  // push af
	shop_sound(sh->sound);
	cpu.cycles += 10 + 7;              // pop af, jr c
	cpu.a = a; cpu.f = f;
	int how;
	if (f & FC) { cpu.cycles += 5; how = LEAVE_FIRST; }
	else {
		cpu.cycles += 4 + 10;
		op_or(a);
		if (a == 0) how = arms_buy(sh);
		else { cpu.cycles += 10; how = arms_sell(sh); }
	}
	arms_leave(sh, how);
}

// the list of what the shop sells, then pick one and how many
static int arms_buy(const ArmsShop *sh) {
	newline();
	msg(sh->intro);
	task_wait_button(sh->at_intro_wait);
	blank_line();
	msg(sh->list);
	cpu.cycles += 19 + 19;
	mem_write(SLOT + 0x1A, 0);
	mem_write(SLOT + 0x16, 4);
	for (;;) {                         // "  <item>s" for each in stock
		int show = 1;
		if (sh->skip_empty) {
			in_stock(sh);
			cpu.cycles += 4 + 7;
			if (cpu.a == 0) { cpu.cycles += 5; show = 0; }
		}
		if (show) {
			newline();
			put(' ');
			put(' ');
			in_stock(sh);
			call(sh->name);
			if (sh->plural) put('s');
		}
		cpu.cycles += 23 + 23 + 7;
		mem_write(SLOT + 0x1A, mem_read(SLOT + 0x1A) + 1);
		uint8_t left = (uint8_t)(mem_read(SLOT + 0x16) - 1);
		mem_write(SLOT + 0x16, left);
		if (!left) break;
		cpu.cycles += 5;
	}
	quote();
	task_wait_button(sh->at_list_wait);
	newline();
	for (;;) {                         // which one?
		newline();
		msg(sh->which);
		call(0x2E1B);                  // menu window
		cpu.cycles += 19 + 7 + 10;
		mem_write(SLOT + 0x1A, 0);
		if (sh->skip_empty) { cpu.cycles += 19; mem_write(SLOT + 0x1B, 0); }
		cpu.b = 4;
		SET_DE(0x0213);
		do {
			cpu.cycles += 11 + 11;
			uint8_t b = cpu.b;
			uint16_t de = DE;
			in_stock(sh);
			int show = 1;
			if (sh->skip_empty) {
				cpu.cycles += 4 + 7;
				if (cpu.a == 0) { cpu.cycles += 5; show = 0; }
				else { cpu.cycles += 23; mem_write(SLOT + 0x1B, mem_read(SLOT + 0x1B) + 1); }
			}
			if (show) {
				cpu.cycles += 7;
				cpu.a = (uint8_t)(cpu.a + sh->menu_add);
				call(0x340F);
				call(0x2D69);          // menu line
			}
			cpu.cycles += 10 + 10 + 4 + 4 + 23 + 8;
			SET_DE(de + 0x0200);
			cpu.b = b;
			mem_write(SLOT + 0x1A, mem_read(SLOT + 0x1A) + 1);
			if (--cpu.b) cpu.cycles += 5;
		} while (cpu.b);
		call(0x2E54);
		cpu.cycles += 19;
		mem_write(SLOT + 0x1A, 0);
		for (;;) {
			task_frame_tick(sh->at_pick);
			cpu.cycles += (sh->skip_empty ? 19 : 7) + 10;
			cpu.c = sh->skip_empty ? mem_read(SLOT + 0x1B) : 4;
			SET_DE(0x0102);
			call(0x49F4);
			cpu.cycles += 7;
			if (!(cpu.f & FZ)) break;
			cpu.cycles += 5;
		}
		uint8_t f = cpu.f;
		shop_sound(sh->sound);
		cpu.cycles += 10;
		if (f & FC) return LEAVE_FIRST;
		uint8_t item = in_stock(sh);
		cpu.cycles += 19;
		mem_write(SLOT + 0x1B, item);
		call(sh->name);
		blank_line();
		cpu.cycles += 19;
		word_at(sh->prices, mem_read(SLOT + 0x1B));
		call(0x4439);                  // can the party use it?
		cpu.cycles += 7 + 7;
		uint8_t m = sh->cant_buy;
		if (cpu.f & FC) { cpu.cycles += 5; goto said; }
		cpu.cycles += 19 + 7;
		cpu.a = (uint8_t)(mem_read(SLOT + 0x1B) + sh->about_add);
		msg_a();                       // about it, and its price
		task_wait_button(sh->at_about_wait);
		blank_line();
		msg(sh->how_many);
		cpu.cycles += 7;
		cpu.a = 2;
		call(0x4A66);
		until_c_clear(sh->at_count, 0x4A9F);
		cpu.cycles += 19;
		mem_write(SLOT + 0x21, cpu.l);
		blank_line();
		cpu.cycles += 19 + 4 + 7 + 7;
		m = sh->none;
		if (mem_read(SLOT + 0x21) == 0) { cpu.cycles += 5; goto said; }
		cpu.cycles += 19;
		word_at(sh->prices, mem_read(SLOT + 0x1B));
		cpu.cycles += 4 + 16 + 19 + 4 + 7;
		{
			uint16_t price = HL, gold = rd16(GOLD);
			uint8_t n = mem_read(SLOT + 0x21);
			m = sh->poor;
			int ok = 1;
			do {
				cpu.cycles += 15 + 7;
				if (gold < price) { cpu.cycles += 5; ok = 0; break; }
				gold -= price;
				cpu.cycles += 8;
				if (--n) cpu.cycles += 5;
			} while (n);
			if (!ok) goto said;
			cpu.cycles += 16;
			wr16(GOLD, gold);
		}
		call(0x43DA);                  // show the gold
		cpu.cycles += 19;
		{
			uint16_t at = rst30(sh->owned, mem_read(SLOT + 0x1B));
			cpu.cycles += 10 + 7 + 19 + 7 + 7;
			uint8_t v = (uint8_t)(mem_read(at) + mem_read(SLOT + 0x21));
			if (v < 0x64) cpu.cycles += 5;
			else { cpu.cycles += 7; v = 0x63; }
			cpu.cycles += 7;
			mem_write(at, v);
		}
		shop_sound(sh->sound);
		msg_shop(sh->shop_add);
		cpu.cycles += 7;
		m = sh->sold;
	said:
		cpu.a = m;
		msg_a();
		task_wait_button(sh->at_said_wait);
		blank_line();
		msg(sh->more);                 // anything else?
		call(0x4B5E);
		until_c_clear(sh->at_more, 0x4B69);
		shop_sound(sh->sound);
		cpu.cycles += 20 + 10;
		if (mem_read(SLOT + 0x07) & 1) { cpu.cycles += 10; return LEAVE_SECOND; }
	}
}

// sell one of the things the party has
static int arms_sell(const ArmsShop *sh) {
	newline();
	cpu.cycles += 19;
	mem_write(SLOT + 0x1A, sh->sell_first);
	call(sh->has_any);
	cpu.cycles += 7 + 10;
	uint8_t m = sh->sell_nothing;
	if (cpu.f & FZ) goto said;
	for (;;) {
		shop_sound(sh->sound);
		cpu.cycles += 19;
		cpu.a = mem_read(SLOT + 0x1A);
		msg_a();
		msg(sh->sell_which);
		cpu.cycles += 19;
		mem_write(SLOT + 0x1A, 0);
		for (;;) {
			task_frame_tick(sh->at_sell_pick);
			cpu.cycles += 19 + 10;
			cpu.c = mem_read(SLOT + 0x1B);
			SET_DE(sh->sell_menu_de);
			call(0x49F4);
			cpu.cycles += 7;
			if (!(cpu.f & FZ)) break;
			cpu.cycles += 5;
		}
		uint8_t f = cpu.f;
		shop_sound(sh->sound);
		cpu.cycles += 10;
		if (f & FC) return LEAVE_FIRST;
		// the (ix+$1A)+1-th kind the party has
		cpu.cycles += 19 + 4 + 4 + 4 + 10;
		uint8_t b = (uint8_t)(mem_read(SLOT + 0x1A) + 1), c = 0;
		uint16_t hl = sh->owned;
		for (;;) {
			hl++; c++;
			cpu.cycles += 6 + 4 + 7 + 7;
			if (mem_read(hl) == 0) { cpu.cycles += 5; continue; }
			cpu.cycles += 8;
			if (--b == 0) break;
			cpu.cycles += 5;
		}
		cpu.cycles += 19 + 19 + 19 + 4;
		mem_write(SLOT + 0x1B, c);
		mem_write(SLOT + 0x1C, (uint8_t)hl);
		mem_write(SLOT + 0x1D, hl >> 8);
		SET_HL(hl); cpu.b = 0; cpu.c = c;
		cpu.a = c;
		call(sh->name);
		blank_line();
		cpu.cycles += 19 + 19 + 19 + 19 + 7 + 4 + 7;
		mem_write(SLOT + 0x06, 0);
		mem_write(SLOT + 0x21, 1);
		uint16_t owned = (uint16_t)(mem_read(SLOT + 0x1C) | mem_read(SLOT + 0x1D) << 8);
		if (mem_read(owned) == 1) cpu.cycles += 5;   // only one: no "how many"
		else {
			msg(sh->sell_how_many);
			cpu.cycles += 19;
			cpu.a = mem_read(SLOT + 0x1B);
			call(sh->name);
			msg(sh->sell_how_many2);
			cpu.cycles += 7;
			cpu.a = 2;
			call(0x4A66);
			until_c_clear(sh->at_sell_count, 0x4A9F);
			cpu.cycles += 19;
			mem_write(SLOT + 0x21, cpu.l);
			blank_line();
			cpu.cycles += 23 + 19 + 4 + 7 + 10;
			mem_write(SLOT + 0x06, mem_read(SLOT + 0x06) | 1);
			m = sh->none;
			if (mem_read(SLOT + 0x21) == 0) goto said;
			cpu.cycles += 19 + 19 + 7 + 19 + 7 + 10;
			m = sh->too_many;
			if (mem_read(owned) < mem_read(SLOT + 0x21)) goto said;
		}
		msg(sh->offer);                // "I'll give you ..."
		cpu.cycles += 10;
		SET_HL(sh->prices);
		call(0xAB92);                  // the offer (into ix+$1F)
		cpu.cycles += 20 + 7;
		if (mem_read(SLOT + 0x06) & 1) {
			cpu.cycles += 19 + 4 + 7;
			if (mem_read(SLOT + 0x21) != 1) { msg(sh->offer_n); msg(sh->offer_some); }
			else { cpu.cycles += 5; msg(sh->offer_n); msg(sh->offer_one); }
			cpu.cycles += 12;
		} else {
			cpu.cycles += 5;
			msg(sh->offer_single);
			cpu.cycles += 19;
			cpu.a = mem_read(SLOT + 0x1B);
			call(sh->name);
			quote();
		}
		task_wait_button(sh->at_deal_wait);
		blank_line();
		msg(sh->deal);                 // deal?
		call(0x4B5E);
		until_c_clear(sh->at_deal, 0x4B69);
		shop_sound(sh->sound);
		cpu.cycles += 20 + 7 + 7;
		uint8_t next = sh->no_deal;
		if (mem_read(SLOT + 0x07) & 1) cpu.cycles += 5;   // no
		else {
			cpu.cycles += 19 + 19 + 7 + 19 + 7 + 19 + 19;
			mem_write(owned, (uint8_t)(mem_read(owned) - mem_read(SLOT + 0x21)));
			SET_HL(rd16(SLOT + 0x1F));
			call(0x4412);              // the gold
			shop_sound(sh->sound);
			cpu.cycles += 7;
			next = sh->done_deal;
		}
		cpu.cycles += 19;
		mem_write(SLOT + 0x1A, next);
		call(sh->has_any);
		cpu.cycles += 10;
		if (cpu.f & FZ) return LEAVE_SECOND;
		newline();
		cpu.cycles += 10;
	}
said:
	cpu.a = m;
	msg_a();
	task_wait_button(sh->at_sell_said_wait);
	cpu.cycles += 10;
	return LEAVE_FIRST;
}

static void weapon_shop(void) { arms_shop(&weapons); }   // $97D5
static void armour_shop(void) { arms_shop(&armour); }    // $9AE3
Native native_weapon_shop = { "weapon_shop", 0x97D5, 2, weapon_shop, 0, 0, 0, .task = 1 };
Native native_armour_shop = { "armour_shop", 0x9AE3, 2, armour_shop, 0, 0, 0, .task = 1 };

// --------------------------------------------------------------- food --
// $9DE9: food is sold in packs of 25 rations at the shop's price per pack.
#define FOOD_PRICES 0x9ED8              // 16-bit, per food shop

static void food_shop(void) {
	msg(0x0A);
	cpu.cycles += 13;
	cpu.a = mem_read(SHOP_NUMBER);
	msg_a();                           // the shop's name
	quote();
	task_wait_button(0x9DF7);
	blank_line();
	msg_shop(5);
	msg(0x0B);                         // want some food?
	call(0x4B5E);
	until_c_clear(0x9E0D, 0x4B69);
	call(0x4324);
	cpu.cycles += 20 + 10;
	if (!(mem_read(SLOT + 0x07) & 1)) {
		newline();
		msg(0x0D);
		cpu.cycles += 13;
		word_at(FOOD_PRICES, mem_read(SHOP_NUMBER));
		cpu.cycles += 19 + 19;
		mem_write(SLOT + 0x1C, cpu.l);
		mem_write(SLOT + 0x1D, cpu.h);
		call(0x5627);                  // the price
		msg(0x0E);
		task_wait_button(0x9E3C);
		uint8_t m = 0;
		for (int ask = 1; ask;) {
			blank_line();              // $9E3F
			for (;;) {
				msg(0x0F);             // $9E42: how many packs?
				cpu.cycles += 7;
				cpu.a = 3;
				call(0x4A66);
				until_c_clear(0x9E4C, 0x4A9F);
				blank_line();
				cpu.cycles += 19 + 19 + 4 + 4 + 7 + 7;
				mem_write(SLOT + 0x20, cpu.l);
				mem_write(SLOT + 0x21, cpu.h);
				if (HL == 0) { cpu.cycles += 5; m = 0x10; ask = 0; break; }
				cpu.cycles += 19 + 19;
				SET_DE(rd16(SLOT + 0x1C));
				call(0x2AC8);          // HL = packs * price
				call(0x4423);          // pay
				cpu.cycles += 7;
				if (cpu.f & FC) {      // $9E9E: not enough gold
					cpu.cycles += 5 + 16 + 19 + 19 + 4 + 15 + 7 + 7;
					uint16_t price = rd16(SLOT + 0x1C);
					if (rd16(GOLD) < price) { cpu.cycles += 5; m = 0x11; ask = 0; break; }
					msg(0x12);
					cpu.cycles += 16 + 19 + 19;
					SET_HL(rd16(GOLD));
					SET_DE(price);
					call(0x2B3F);      // how many the party can afford
					call(0x5627);
					msg(0x13);
					task_wait_button(0x9EC7);
					cpu.cycles += 10;
					break;             // to $9E3F
				}
				cpu.cycles += 19 + 19 + 10;
				SET_HL(rd16(SLOT + 0x20));
				SET_DE(0x0019);
				call(0x2AC8);          // rations
				call(0xABD2);          // add them
				msg(0x14);             // more?
				call(0x4B5E);
				until_c_clear(0x9E88, 0x4B69);
				call(0x4324);
				cpu.cycles += 20 + 7;
				if (mem_read(SLOT + 0x07) & 1) { cpu.cycles += 5; ask = 0; m = 0; break; }
				newline();
				cpu.cycles += 12;
			}
		}
		if (m) { cpu.a = m; msg_a(); }    // $9ECD
	}
	newline();                         // $9ED0
	cpu.cycles += 7 + 10;
	cpu.a = 0x0C;
	leave_shop();
}
Native native_food_shop = { "food_shop", 0x9DE9, 2, food_shop, 0, 0, 0, .task = 1 };

// ---------------------------------------------------------------- pub --
// $9EE2: the pub sells food (by the plate) and ale. Paying for an ale with a
// tip of 3 or more gold, when the party knows of reagents it has not found
// yet, gets the keeper talking: pick a reagent, and if it is the one this
// pub knows about and the tips add up to its price, they say where it grows.
#define PUB_PLATE_PRICES 0x9F74         // per pub
#define PUB_SECRET_PRICE 0x9F6E         // tips needed, per pub
#define PUB_SECRET_FLAG  0xA1E5         // which flag at $C500 the secret sets, per pub
#define REAGENT_NAMES    0xA19F
#define REAGENT_HINTS    0xC9B1         // 6 reagents to ask about ($A173 fills them)

static uint8_t byte_at(uint16_t table, uint8_t i) {   // ld hl,table / rst $28: A = table[i]
	cpu.cycles += 10 + 48;
	SET_HL(table + i);
	cpu.a = mem_read(HL);
	return cpu.a;
}
static void say(uint16_t text) { cpu.cycles += 10 + 17; text_print(text); }   // ld hl,text / call print_text_hl

enum { PUB_MENU, PUB_FOOD_DONE, PUB_AGAIN, PUB_SAY_BYE, PUB_BYE, PUB_BYE_NOW };

// $9F34 / $9F39: "anything else?" - back to the menu, or leave
static int pub_anything_else(int first) {
	if (first) msg(0x32);
	call(0x2BBF);                      // random: which way to ask
	cpu.cycles += 7 + 7 + 7;
	uint8_t m = 0x33;
	if (!(cpu.a & 1)) cpu.cycles += 5;
	else { cpu.cycles += 7; m = 0x2F; }
	cpu.a = m;
	msg_a();
	call(0x4B5E);
	until_c_clear(0x9F4A, 0x4B69);
	call(0x4324);
	cpu.cycles += 20 + 7;
	if (!(mem_read(SLOT + 0x07) & 1)) { cpu.cycles += 5; return PUB_MENU; }
	cpu.cycles += 12;
	return PUB_BYE;
}

static int pub_food(void) {            // $9F89
	say(0x9F7C);                       // "FOOD"
	blank_line();
	msg(0x1F);
	msg_shop(0x0C);
	msg(0x20);
	cpu.cycles += 13;
	uint8_t price = byte_at(PUB_PLATE_PRICES, mem_read(SHOP_NUMBER));
	cpu.cycles += 19 + 4 + 7;
	mem_write(SLOT + 0x1C, price);
	SET_HL(price);
	call(0x5627);                      // the price
	msg(0x21);                         // how many plates?
	cpu.cycles += 7;
	cpu.a = 2;
	call(0x4A66);
	until_c_clear(0x9FBE, 0x4A9F);
	cpu.cycles += 19 + 4 + 4 + 10;
	mem_write(SLOT + 0x1D, cpu.l);
	if (cpu.l == 0) return PUB_BYE_NOW;
	blank_line();
	cpu.cycles += 19;
	cpu.h = mem_read(SLOT + 0x1C);
	call(0x2AA1);                      // HL = plates * price
	call(0x4423);                      // pay
	cpu.cycles += 7;
	if (!(cpu.f & FC)) {
		cpu.cycles += 19 + 7;
		SET_HL(mem_read(SLOT + 0x1D));
		call(0xABD2);                  // the food
		cpu.cycles += 10;
		return PUB_FOOD_DONE;
	}
	cpu.cycles += 5 + 16 + 19 + 7 + 4 + 15 + 7 + 10;   // $9FE7: not enough
	if (rd16(GOLD) < mem_read(SLOT + 0x1C)) { cpu.a = 0x22; return PUB_SAY_BYE; }
	msg(0x23);
	cpu.cycles += 16 + 19 + 7;
	SET_HL(rd16(GOLD));
	SET_DE(mem_read(SLOT + 0x1C));
	call(0x2B3F);                      // how many the party can afford
	cpu.cycles += 11;
	uint16_t n = HL;
	call(0x5627);
	cpu.cycles += 10 + 6 + 4 + 4 + 7 + 7;
	uint8_t m = 0x24;
	if ((uint16_t)(n - 1) == 0) cpu.cycles += 5;
	else { cpu.cycles += 7; m = 0x35; }
	cpu.a = m;
	msg_a();
	task_wait_button(0xA018);
	blank_line();
	cpu.cycles += 10;
	return PUB_AGAIN;
}

static int pub_ale(void) {             // $A021
	say(0x9F83);                       // "GROG"
	blank_line();
	cpu.cycles += 23 + 19 + 7 + 7;
	uint8_t ales = (uint8_t)(mem_read(SLOT + 0x17) + 1);
	mem_write(SLOT + 0x17, ales);
	if (ales >= 4) {                   // enough for today
		msg_shop(6);
		cpu.cycles += 7 + 10;
		cpu.a = 0x25;
		return PUB_SAY_BYE;
	}
	cpu.cycles += 5;
	msg(0x26);                         // what will you pay?
	cpu.cycles += 7;
	cpu.a = 2;
	call(0x4A66);
	until_c_clear(0xA04B, 0x4A9F);
	cpu.cycles += 19 + 4 + 4 + 10;
	mem_write(SLOT + 0x21, cpu.l);
	if (cpu.l == 0) return PUB_BYE_NOW;
	blank_line();
	cpu.cycles += 19 + 7 + 7 + 10;
	if (mem_read(SLOT + 0x21) < 2) { cpu.a = 0x27; return PUB_SAY_BYE; }
	cpu.cycles += 19 + 7;
	SET_HL(mem_read(SLOT + 0x21));
	call(0x4423);                      // pay
	cpu.cycles += 7 + 10;
	if (cpu.f & FC) { cpu.a = 0x28; return PUB_SAY_BYE; }
	cpu.cycles += 19 + 7 + 10;
	if (mem_read(SLOT + 0x21) < 3) return PUB_FOOD_DONE;
	call(0xA173);                      // reagents the party could ask about
	cpu.cycles += 10;
	if (cpu.f & FZ) return PUB_FOOD_DONE;
	msg(0x29);
	call(0x2E1B);
	cpu.cycles += 19 + 10 + 10 + 10;
	mem_write(SLOT + 0x1B, 0);
	SET_HL(REAGENT_HINTS);
	SET_DE(0x0213);
	SET_BC(0x0600);
	do {                               // a menu of them
		cpu.cycles += 11 + 11 + 7 + 4 + 7;
		uint16_t hl = HL, bc = BC;
		if (mem_read(hl) == 0) cpu.cycles += 5;
		else {
			cpu.cycles += 23 + 4;
			mem_write(SLOT + 0x1B, mem_read(SLOT + 0x1B) + 1);
			word_at(REAGENT_NAMES, cpu.c);
			cpu.cycles += 11;
			uint16_t de = DE;
			call(0x2D69);
			cpu.cycles += 10 + 4 + 4;
			SET_DE(de + 0x0200);
		}
		cpu.cycles += 10 + 10 + 6 + 4 + 8;
		SET_HL(hl + 1);
		SET_BC(bc);
		cpu.c++;
		if (--cpu.b) cpu.cycles += 5;
	} while (cpu.b);
	call(0x2E54);
	cpu.cycles += 19;
	mem_write(SLOT + 0x1A, 0);
	for (;;) {
		task_frame_tick(0xA0BA);
		cpu.cycles += 19 + 10;
		cpu.c = mem_read(SLOT + 0x1B);
		SET_DE(0x0102);
		call(0x49F4);
		cpu.cycles += 7;
		if (!(cpu.f & FZ)) break;
		cpu.cycles += 5;
	}
	cpu.cycles += 19 + 7;
	mem_write(SLOT + 0x1B, 0xFF);
	if (cpu.f & FC) cpu.cycles += 5;   // none
	else {
		cpu.cycles += 19 + 4 + 4 + 7 + 10;
		uint8_t b = (uint8_t)(mem_read(SLOT + 0x1A) + 1), c = 0xFF;
		uint16_t hl = REAGENT_HINTS - 1;
		for (;;) {
			hl++; c++;
			cpu.cycles += 6 + 4 + 7 + 7;
			if (mem_read(hl) == 0) { cpu.cycles += 5; continue; }
			cpu.cycles += 8;
			if (--b == 0) break;
			cpu.cycles += 5;
		}
		cpu.cycles += 19 + 4;
		mem_write(SLOT + 0x1B, c);
		word_at(REAGENT_NAMES, c);
		cpu.cycles += 17;
		text_print(HL);                // its name
	}
	blank_line();                      // $A0EA
	cpu.cycles += 19 + 10 + 7 + 7;
	uint8_t asked = mem_read(SLOT + 0x1B);
	if (asked != mem_read(SHOP_NUMBER)) {   // not this pub's secret
		msg(0x2A);
		task_wait_button(0xA0FB);
		blank_line();
		cpu.cycles += 10;
		return PUB_AGAIN;
	}
	cpu.cycles += 5;
	cpu.a = asked;
	uint8_t wanted = byte_at(PUB_SECRET_PRICE, asked);   // $A104
	cpu.cycles += 19;
	mem_write(SLOT + 0x1C, wanted);
	for (;;) {                         // $A10B: more tips until enough
		cpu.cycles += 19 + 19 + 7;
		if (mem_read(SLOT + 0x21) >= mem_read(SLOT + 0x1C)) { cpu.cycles += 5; break; }
		msg(0x2B);
		cpu.cycles += 7;
		cpu.a = 2;
		call(0x4A66);
		until_c_clear(0xA11D, 0x4A9F);
		blank_line();
		cpu.cycles += 4 + 4 + 7 + 10;
		if (cpu.l == 0) { cpu.a = 0x2C; return PUB_SAY_BYE; }
		cpu.cycles += 4 + 19 + 19;
		mem_write(SLOT + 0x21, (uint8_t)(cpu.l + mem_read(SLOT + 0x21)));
		call(0x4423);                  // pay
		cpu.cycles += 7;
		if (cpu.f & FC) {
			msg(0x2D);
			newline();
			cpu.cycles += 7 + 10;
			cpu.a = 0x2C;
			return PUB_SAY_BYE;
		}
		cpu.cycles += 5;
	}
	msg_shop(6);                       // $A148: the secret
	msg(0x2E);
	msg_shop(0x19);
	task_wait_button(0xA15D);
	blank_line();
	cpu.cycles += 13;
	uint8_t flag = byte_at(PUB_SECRET_FLAG, mem_read(SHOP_NUMBER));
	cpu.cycles += 10 + 10;
	uint16_t at = rst30(0xC500, flag);
	mem_write(at, 1);
	cpu.cycles += 10;
	return PUB_AGAIN;
}

static void pub(void) {
	cpu.cycles += 19;
	mem_write(SLOT + 0x17, 0);         // ales served
	msg_shop(6);
	msg(0x30);
	cpu.cycles += 13;
	cpu.a = mem_read(SHOP_NUMBER);
	msg_a();                           // the pub's name
	quote();
	task_wait_button(0x9EFC);
	newline();
	int r = PUB_MENU;
	for (;;) {
		if (r == PUB_MENU) {           // $9F02
			newline();
			msg_shop(6);
			msg(0x31);
			cpu.cycles += 10;
			SET_HL(0x9F7A);
			call(0xAB4E);              // food / ale
			for (;;) {
				task_frame_tick(0x9F18);
				cpu.cycles += 7 + 10;
				cpu.c = 2;
				SET_DE(0x0102);
				call(0x49F4);
				cpu.cycles += 7;
				if (!(cpu.f & FZ)) break;
				cpu.cycles += 5;
			}
			call(0x4324);
			cpu.cycles += 7;
			if (cpu.f & FC) { cpu.cycles += 5; r = PUB_BYE_NOW; break; }
			cpu.cycles += 20 + 10;
			if (!(mem_read(SLOT + 0x1A) & 1)) r = pub_food();
			else { cpu.cycles += 10; r = pub_ale(); }
			continue;
		}
		if (r == PUB_FOOD_DONE) { r = pub_anything_else(1); continue; }
		if (r == PUB_AGAIN) { r = pub_anything_else(0); continue; }
		break;
	}
	if (r == PUB_SAY_BYE) {            // $9F5D
		msg_a();
		task_wait_button(0x9F60);
	}
	if (r != PUB_BYE) newline();       // $9F63
	newline();                         // $9F66
	cpu.cycles += 7 + 10;
	cpu.a = 0x34;
	leave_shop();
}
Native native_pub = { "pub", 0x9EE2, 2, pub, 0, 0, 0, .task = 1 };

// ----------------------------------------------------------- reagents --
// $A1EB: the reagent shop. The player names their own price: paying less
// than asked costs Honesty, Justice and Honor karma (a third of the
// shortfall, at least 4); paying at all gains 2 of each.
#define REAGENT_PRICES 0xA3B7          // 6 per reagent shop
#define REAGENTS_OWNED 0xC92F

static void reagent_shop(void) {
	msg(0x08);
	cpu.cycles += 13;
	cpu.a = mem_read(SHOP_NUMBER);
	msg_a();                           // the shop's name
	quote();
	shop_sound(0x0C);
	task_wait_button(0xA1FE);
	blank_line();
	msg(0x09);
	msg_shop(4);
	msg(0x0A);                         // want reagents?
	call(0x4B5E);
	until_c_clear(0xA219, 0x4B69);
	shop_sound(0x0C);
	cpu.cycles += 20 + 10;
	int two_lines = 0;
	if (!(mem_read(SLOT + 0x07) & 1)) {
		newline();
		msg(0x0B);
		task_wait_button(0xA235);
		blank_line();
		msg(0x0C);
		task_wait_button(0xA240);
		newline();
		for (;;) {                     // $A246
			newline();
			msg(0x0D);                 // which one?
			call(0x2E1B);
			cpu.cycles += 10 + 10;
			SET_DE(0x0213);
			SET_BC(0x065D);
			do {
				cpu.cycles += 11 + 11 + 4;
				uint16_t bc = BC, de = DE;
				cpu.a = cpu.c;
				call(0x340F);
				call(0x2D69);          // menu line
				cpu.cycles += 10 + 10 + 4 + 4 + 4 + 8;
				SET_DE(de + 0x0200);
				SET_BC(bc + 1);
				if (--cpu.b) cpu.cycles += 5;
			} while (cpu.b);
			call(0x2E54);
			cpu.cycles += 19;
			mem_write(SLOT + 0x1A, 0);
			for (;;) {
				task_frame_tick(0xA26E);
				cpu.cycles += 7 + 10;
				cpu.c = 6;
				SET_DE(0x0102);
				call(0x49F4);
				cpu.cycles += 7;
				if (!(cpu.f & FZ)) break;
				cpu.cycles += 5;
			}
			uint8_t f = cpu.f;
			shop_sound(0x0C);
			cpu.cycles += 10;
			if (f & FC) { two_lines = 1; break; }
			uint8_t item = mem_read(SLOT + 0x1A);
			cpu.cycles += 19;
			cpu.a = item;
			call(0xAB8D);              // its name
			blank_line();
			msg(0x0E);
			cpu.cycles += 19;
			cpu.a = item;
			call(0xAB8D);
			msg(0x0F);
			cpu.cycles += 13 + 4 + 4 + 4 + 4 + 19;
			uint8_t price = byte_at(REAGENT_PRICES, (uint8_t)(mem_read(SHOP_NUMBER) * 6 + item));
			cpu.cycles += 19 + 4 + 7;
			mem_write(SLOT + 0x1C, price);
			SET_HL(price);
			call(0x5627);              // the price
			msg(0x10);                 // how many?
			cpu.cycles += 7;
			cpu.a = 2;
			call(0x4A66);
			until_c_clear(0xA2BD, 0x4A9F);
			cpu.cycles += 19;
			mem_write(SLOT + 0x1D, cpu.l);
			blank_line();
			cpu.cycles += 4 + 4 + 7;
			uint8_t m;
			if (cpu.l == 0) goto none;
			cpu.cycles += 5;
			msg(0x11);                 // that comes to ...
			cpu.cycles += 19 + 19;
			cpu.l = mem_read(SLOT + 0x1C);
			cpu.h = mem_read(SLOT + 0x1D);
			call(0x2AA1);              // HL = price * count
			cpu.cycles += 19 + 19;
			mem_write(SLOT + 0x1E, cpu.l);
			mem_write(SLOT + 0x1F, cpu.h);
			call(0x5627);
			msg(0x12);                 // what will you pay?
			cpu.cycles += 7;
			cpu.a = 3;
			call(0x4A66);
			until_c_clear(0xA2F8, 0x4A9F);
			cpu.cycles += 19 + 19;
			mem_write(SLOT + 0x20, cpu.l);
			mem_write(SLOT + 0x21, cpu.h);
			blank_line();
			cpu.cycles += 4 + 4 + 7;
			if (HL == 0) { cpu.cycles += 5; goto none; }
			cpu.cycles += 19 + 19 + 4 + 15 + 7;
			{
				uint16_t paid = HL, asked = rd16(SLOT + 0x1E);
				if (paid < asked) {    // less than asked: karma
					cpu.cycles += 4 + 10 + 4 + 15 + 10;
					SET_HL((uint16_t)(asked - paid));
					SET_DE(3);
					call(0x2B3F);      // shortfall / 3
					cpu.cycles += 10 + 4 + 15 + 7;
					uint16_t q = HL;
					if (q >= 4) cpu.cycles += 5 + 11;
					else { cpu.cycles += 4 + 12; q = 4; }
					cpu.cycles += 4 + 7;
					uint8_t loss = (uint8_t)q;
					for (int v = 0; v < 3; v++) {   // Honesty, Justice, Honor
						if (v) cpu.cycles += 7;
						cpu.a = loss;
						cpu.c = (uint8_t)(v == 0 ? 0 : v == 1 ? 3 : 5);
						call(0x426F);  // karma_lose
					}
				} else cpu.cycles += 5;
			}
			cpu.cycles += 19 + 19;     // $A341
			SET_HL(rd16(SLOT + 0x20));
			call(0x4423);              // pay
			cpu.cycles += 7;
			if (cpu.f & FC) { cpu.cycles += 7 + 12; m = 0x13; }
			else {
				cpu.cycles += 5 + 7;
				for (int v = 0; v < 3; v++) {
					cpu.cycles += 7;
					cpu.a = 2;
					cpu.c = (uint8_t)(v == 0 ? 0 : v == 1 ? 3 : 5);
					call(0x4255);      // karma_gain
				}
				cpu.cycles += 19;
				uint16_t at = rst30(REAGENTS_OWNED, mem_read(SLOT + 0x1A));
				cpu.cycles += 10 + 7 + 19 + 7 + 7;
				uint8_t v = (uint8_t)(mem_read(at) + mem_read(SLOT + 0x1D));
				if (v < 0x63) cpu.cycles += 5;
				else { cpu.cycles += 7; v = 0x63; }
				cpu.cycles += 7;
				mem_write(at, v);
				shop_sound(0x0C);
				cpu.cycles += 7;
				m = 0x14;
			}
			cpu.a = m;                 // $A37A
			msg_a();
			task_wait_button(0xA37D);
			blank_line();
			msg(0x16);                 // anything else?
			goto more;
		none:
			msg(0x15);
			cpu.cycles += 10;
		more:                          // $A388
			call(0x4B5E);
			until_c_clear(0xA38B, 0x4B69);
			shop_sound(0x0C);
			cpu.cycles += 20 + 10;
			if (mem_read(SLOT + 0x07) & 1) { cpu.cycles += 12; break; }
		}
	}
	if (two_lines) newline();          // $A3A1
	newline();                         // $A3A4
	call(0x4324);
	msg_shop(4);
	cpu.cycles += 7 + 10;
	cpu.a = 0x17;
	leave_shop();
}
Native native_reagent_shop = { "reagent_shop", 0xA1EB, 2, reagent_shop, 0, 0, 0, .task = 1 };

// -------------------------------------------------------------- healer --
// $A3CF: the healer cures poison (100gp), heals wounds (200gp) and raises
// the dead (300gp). On the way out a healthy Avatar (400+ hit points) is
// asked to give blood: yes gains Sacrifice karma and costs 100 hit points,
// no loses Sacrifice karma.
#define AVATAR_HP 0xC751

enum { HEAL_SAID, HEAL_MENU };

// $A5F8 and the loop around $A606: "that will be n gold - pay?" (the price
// is in HL). Returns 1 when paid; 0 when not, with the message number in A.
static int healer_pay(uint16_t site) {
	call(0xA5F8);                      // ask
	for (;;) {
		task_frame_tick(site);
		cpu.cycles += 17;              // call $A606
		call(0x4B69);
		if (cpu.f & FC) { cpu.cycles += 11 + 12; continue; }
		cpu.cycles += 5;
		call(0x4324);
		newline();
		cpu.cycles += 20 + 7;
		if (mem_read(SLOT + 0x07) & 1) {   // no
			cpu.cycles += 10 + 7 + 10;
			cpu.a = 0x18;
			return 0;
		}
		cpu.cycles += 5 + 19 + 19;
		SET_HL(rd16(SLOT + 0x20));
		call(0x4423);                  // pay
		cpu.cycles += 4 + 10 + 7;
		op_or(cpu.a);
		return 1;
	}
}

// the member chosen for a service, in IY ($48B6)
static void healer_member(void) {
	cpu.cycles += 19;
	cpu.c = mem_read(SLOT + 0x08);
	call(0x48B6);
}

// $A567: pay, then the member is well ('G'ood)
static int healer_make_good(void) {
	if (!healer_pay(0xA56A)) return HEAL_SAID;
	call(0xA627);                      // sound, flash, a few frames
	cpu.cycles += 19;
	mem_write(cpu.iy + 0x1C, 'G');
	call(0x4324);
	cpu.cycles += 10;
	return HEAL_MENU;
}

static int healer_cure(void) {         // $A534
	healer_member();
	cpu.cycles += 19 + 7 + 7 + 10;
	if (mem_read(cpu.iy + 0x1C) != 'P') { cpu.a = 0x19; return HEAL_SAID; }
	msg(0x1A);
	task_wait_button(0xA549);
	blank_line();
	cpu.cycles += 10;
	SET_HL(100);
	call(0x4439);                      // can the party pay?
	cpu.cycles += 7;
	if (cpu.f & FC) {
		msg(0x1B);
		task_wait_button(0xA55C);
		blank_line();
		cpu.cycles += 7 + 10;
		cpu.a = 0x1C;
		return HEAL_SAID;
	}
	cpu.cycles += 5;
	return healer_make_good();
}

static int healer_heal(void) {         // $A57F
	healer_member();
	cpu.cycles += 19 + 19 + 19 + 19 + 4 + 15 + 7 + 10;
	if (rd16(cpu.iy) == rd16(cpu.iy + 2)) { cpu.a = 0x1D; return HEAL_SAID; }
	msg(0x1E);
	task_wait_button(0xA59E);
	blank_line();
	cpu.cycles += 10;
	SET_HL(200);
	call(0x4439);
	cpu.cycles += 7 + 10;
	if (cpu.f & FC) { cpu.a = 0x1F; return HEAL_SAID; }
	if (!healer_pay(0xA5B2)) return HEAL_SAID;
	call(0xA627);
	cpu.cycles += 19 + 19 + 19 + 19;
	wr16(cpu.iy, rd16(cpu.iy + 2));    // full hit points
	call(0x4324);
	cpu.cycles += 10;
	return HEAL_MENU;
}

static int healer_raise(void) {        // $A5CF
	healer_member();
	cpu.cycles += 19 + 7 + 7 + 10;
	if (mem_read(cpu.iy + 0x1C) != 'D') { cpu.a = 0x20; return HEAL_SAID; }
	msg(0x21);
	task_wait_button(0xA5E4);
	blank_line();
	cpu.cycles += 10;
	SET_HL(300);
	call(0x4439);
	cpu.cycles += 10;
	if (cpu.f & FC) { cpu.cycles += 7 + 10; cpu.a = 0x1F; return HEAL_SAID; }
	return healer_make_good();
}

static void healer(void) {
	msg(0x24);
	cpu.cycles += 13;
	cpu.a = mem_read(SHOP_NUMBER);
	msg_a();                           // the healer's name
	quote();
	task_wait_button(0xA3DD);
	blank_line();
	cpu.cycles += 19;
	mem_write(SLOT + 0x17, 0);
	msg_shop(0x0A);
	cpu.cycles += 7;
	cpu.a = 0x25;
	for (;;) {                         // $A3F1: need help?
		msg_a();
		call(0x4B5E);
		until_c_clear(0xA3F7, 0x4B69);
		call(0x4324);
		newline();
		cpu.cycles += 20 + 7;
		if (mem_read(SLOT + 0x07) & 1) break;   // no: on to the blood
		cpu.cycles += 5 + 20 + 7;      // $A465
		if (!(mem_read(SLOT + 0x17) & 1)) {
			msg_shop(0x0A);
			msg(0x26);
			task_wait_button(0xA478);
			blank_line();
			cpu.cycles += 23;
			mem_write(SLOT + 0x17, mem_read(SLOT + 0x17) | 1);
		} else cpu.cycles += 5;
		msg(0x27);                     // which service?
		cpu.cycles += 10;
		SET_HL(0xA511);
		call(0xAB4E);
		for (;;) {
			task_frame_tick(0xA48D);
			cpu.cycles += 7 + 10;
			cpu.c = 3;
			SET_DE(0x0102);
			call(0x49F4);
			cpu.cycles += 7;
			if (!(cpu.f & FZ)) break;
			cpu.cycles += 5;
		}
		call(0x4324);
		cpu.cycles += 7;
		if (cpu.f & FC) {              // none: on to the blood
			blank_line();
			cpu.cycles += 10;
			break;
		}
		cpu.cycles += 5 + 19;
		uint8_t service = mem_read(SLOT + 0x1A);
		word_at(0xA50B, service);
		cpu.cycles += 17;
		text_print(HL);                // its name
		blank_line();
		msg_shop(0x0A);
		msg(0x14);
		newline();
		msg(0x15);                     // for whom?
		call(0x4BCB);
		for (;;) {
			task_frame_tick(0xA4CA);
			call(0x4BD2);
			cpu.cycles += 7;
			if (!(cpu.f & FZ)) break;
			cpu.cycles += 5;
		}
		uint8_t f = cpu.f;
		cpu.cycles += 11 + 7;          // push af, jr nc
		if (f & FC) {
			msg(0x16);
			newline();
		} else cpu.cycles += 5;
		call(0x4324);
		newline();
		cpu.cycles += 10 + 7;          // pop af, jr c
		int r = HEAL_MENU;
		if (f & FC) cpu.cycles += 5;   // nobody picked
		else {
			cpu.cycles += 19 + 10 + 10 + 10 + 52;   // ld a / ld hl / jp $0018: the table jump
			if (service == 0) r = healer_cure();
			else if (service == 1) r = healer_heal();
			else r = healer_raise();
		}
		if (r == HEAL_SAID) {          // $A4F5
			msg_a();
			task_wait_button(0xA4F8);
			blank_line();
		}
		msg_shop(0x0A);                // $A4FE
		cpu.cycles += 7 + 10;
		cpu.a = 0x28;                  // anything more?
	}
	// $A40B: the blood
	cpu.cycles += 16 + 10 + 11 + 7;
	if (rd16(AVATAR_HP) >= 400) {
		msg(0x22);
		call(0x4B5E);
		until_c_clear(0xA41C, 0x4B69);
		call(0x4324);
		newline();
		cpu.cycles += 20 + 7;
		if (!(mem_read(SLOT + 0x07) & 1)) {
			cpu.cycles += 7 + 7;
			cpu.a = 5; cpu.c = 4;
			call(0x4255);              // karma_gain: Sacrifice
			cpu.cycles += 16 + 10 + 11 + 16;
			wr16(AVATAR_HP, (uint16_t)(rd16(AVATAR_HP) - 100));
			call(0x4324);
			msg(0x23);
			task_wait_button(0xA449);
			blank_line();
			cpu.cycles += 12;
		} else {
			cpu.cycles += 5 + 7 + 7;
			cpu.a = 5; cpu.c = 4;
			call(0x426F);              // karma_lose: Sacrifice
		}
	} else cpu.cycles += 5;
	msg_shop(0x0A);                    // $A458
	cpu.cycles += 7 + 10;
	cpu.a = 0x29;
	leave_shop();
}
Native native_healer = { "healer", 0xA3CF, 2, healer, 0, 0, 0, .task = 1 };

// ----------------------------------------------------------------- inn --
// $A647: the inn. A night's stay moves the party to the room, fades the
// screen out and in, heals everyone a little, and sometimes brings a night
// visitor: an ambush (combat) or, at one inn, a ghost.
#define TIME_OF_DAY  0xC959            // (16-bit) $1F when the inn is open?
#define PARTY_X      0xC953            // also view_x at $C985
#define INN_ROOMS    0xA83E            // x, y, price per inn
#define INN_ROOMS_3  0xA853            // inn 3 has three kinds of room

static void say_inline(uint16_t text) {   // call print_inline / the text follows
	uint16_t hl = HL;
	cpu.cycles += 17 + 19 + 17;
	text_print(text);
	cpu.cycles += 19 + 10;
	SET_HL(hl);
}

static void leave_shop_quietly(void) {    // $96B5: no last message
	newline();
	call(0x33E5);
	call(0x33AF);
	cpu.cycles += 10;
	native_continue_at(0x8084);
}

static void inn(void) {
	cpu.cycles += 13 + 7 + 7 + 10;
	if (mem_read(TIME_OF_DAY) != 0x1F) { cpu.a = 0x17; leave_shop(); return; }
	msg(0x18);
	cpu.cycles += 13;
	cpu.a = mem_read(SHOP_NUMBER);
	msg_a();
	msg(0x19);
	msg_shop(7);
	say_inline(0xA66C);                // ."
	task_wait_button(0xA66F);
	blank_line();
	msg(0x1A);                         // a room for the night?
	call(0x4B5E);
	until_c_clear(0xA67D, 0x4B69);
	call(0x4324);
	newline();
	cpu.cycles += 20 + 7;
	if (mem_read(SLOT + 0x07) & 1) {
		msg_shop(7);
		cpu.cycles += 7 + 10;
		cpu.a = 0x21;
		leave_shop();
		return;
	}
	cpu.cycles += 5;
	msg_shop(0x0E);
	task_wait_button(0xA6A6);
	blank_line();
	cpu.cycles += 13 + 7 + 7;
	uint8_t which;
	uint16_t rooms;
	if (mem_read(SHOP_NUMBER) == 3) {  // which room?
		msg(0x1B);
		cpu.cycles += 10;
		SET_HL(0xA85C);
		call(0xAB4E);
		for (;;) {
			task_frame_tick(0xA6BE);
			cpu.cycles += 7 + 10;
			cpu.c = 3;
			SET_DE(0x0102);
			call(0x49F4);
			cpu.cycles += 7;
			if (!(cpu.f & FZ)) break;
			cpu.cycles += 5;
		}
		call(0x4324);
		cpu.cycles += 10;
		if (cpu.f & FC) { leave_shop_quietly(); return; }
		cpu.cycles += 19 + 7 + 17;
		text_char((uint8_t)(mem_read(SLOT + 0x1A) + '1'));
		blank_line();
		cpu.cycles += 19 + 10 + 12;
		which = mem_read(SLOT + 0x1A);
		rooms = INN_ROOMS_3;
	} else {
		cpu.cycles += 5;
		msg(0x1C);                     // the price; take it?
		call(0x4B5E);
		until_c_clear(0xA6EC, 0x4B69);
		call(0x4324);
		newline();
		cpu.cycles += 20 + 7 + 10;
		if (mem_read(SLOT + 0x07) & 1) { cpu.a = 0x1D; leave_shop(); return; }
		cpu.cycles += 13 + 10;
		which = mem_read(SHOP_NUMBER);
		rooms = INN_ROOMS;
	}
	cpu.cycles += 4 + 4 + 4;           // $A709: the room's x, y and price
	uint16_t room = rst30(rooms, (uint8_t)(which * 3));
	cpu.cycles += 7 + 19 + 6 + 7 + 19 + 6 + 7 + 7;
	mem_write(SLOT + 0x0A, mem_read(room));
	mem_write(SLOT + 0x0C, mem_read(room + 1));
	SET_HL(mem_read(room + 2));
	call(0x4423);                      // pay
	cpu.cycles += 7 + 10;
	if (cpu.f & FC) { cpu.a = 0x1E; leave_shop(); return; }
	msg(0x1F);
	task_wait_button(0xA727);
	newline();
	call(0x2BBF);                      // random
	cpu.cycles += 7 + 7;
	if (cpu.a & 3) cpu.cycles += 5;
	else {
		newline();
		msg(0x20);
		task_wait_button(0xA73C);
		newline();
	}
	cpu.cycles += 7;                   // $A742: a lullaby, then to the room
	cpu.a = 9;
	call(0x33AF);
	task_wait_frames(0xA749, 5);
	cpu.cycles += 19 + 19 + 16 + 16;
	uint16_t at = (uint16_t)(mem_read(SLOT + 0x0A) | mem_read(SLOT + 0x0C) << 8);
	wr16(PARTY_X, at);
	wr16(0xC985, at);
	task_wait_frames(0xA75A, 5);
	cpu.cycles += 10 + 16;
	wr16(TIME_OF_DAY, 0x0038);
	static const uint8_t fade[3] = { 0x03, 0x0C, 0x30 };   // the screen goes dark
	static const uint16_t fade_at[3] = { 0xA76A, 0xA774, 0xA77E };
	for (int i = 0; i < 3; i++) {
		cpu.cycles += 7;
		cpu.a = fade[i];
		call(0xA81A);
		task_wait_frames(fade_at[i], i < 2 ? 1 : 0xC8);
	}
	static const uint16_t light_at[3] = { 0xA786, 0xA78E, 0xA796 };
	for (int i = 0; i < 3; i++) {     // and light again
		call(0xA836);
		task_wait_frames(light_at[i], i < 2 ? 1 : 5);
	}
	cpu.cycles += 10 + 16 + 13 + 4 + 7;
	wr16(TIME_OF_DAY, 0x8A1F);
	cpu.b = mem_read(PARTY_SIZE);
	cpu.c = 0;
	do {                               // everyone awake heals a little
		call(0x4445);                  // asleep or dead?
		cpu.cycles += 7;
		if (cpu.f & FC) cpu.cycles += 5;
		else {
			cpu.cycles += 7;
			cpu.a = 0x32;
			call(0x2BEB);              // random_below
			cpu.cycles += 4 + 10;
			cpu.a = (uint8_t)(cpu.a * 2);
			rst30(100, cpu.a);
			call(0x4221);              // member_heal
		}
		cpu.cycles += 4 + 8;
		cpu.c++;
		if (--cpu.b) cpu.cycles += 5;
	} while (cpu.b);
	call(0x4776);
	call(0x4324);
	cpu.cycles += 7;
	cpu.c = 0;
	call(0x4445);
	cpu.cycles += 7;
	if (!(cpu.f & FC)) {
		call(0x2BBF);
		cpu.cycles += 7 + 7;
		if ((cpu.a & 7) == 0) {        // ambushed in the night
			newline();
			msg(0x15);
			task_wait_button(0xA7D6);
			newline();
			cpu.cycles += 7 + 13 + 13 + 10;
			mem_write(0xC97D, 0xC8);
			mem_write(0xC97E, 0xC8);
			native_continue_at(0x3B94);
			return;
		}
		cpu.cycles += 5;
	} else cpu.cycles += 5;
	cpu.cycles += 13 + 7 + 7;          // $A7E7
	if (mem_read(SHOP_NUMBER) == 5) {
		call(0x2BBF);
		cpu.cycles += 7 + 7;
		if ((cpu.a & 3) == 0) {        // a visitor at the bedside
			call(0x4933);              // a free figure -> IY
			cpu.cycles += 19 + 19 + 16 + 4 + 19 + 19 + 19 + 19;
			mem_write(cpu.iy + 3, 0x9C);
			mem_write(cpu.iy + 0, 0x9C);
			uint16_t p = rd16(PARTY_X);
			mem_write(cpu.iy + 1, (uint8_t)(p - 1));
			mem_write(cpu.iy + 2, p >> 8);
			mem_write(cpu.iy + 6, 0x01);
			mem_write(cpu.iy + 7, 0x10);
		} else cpu.cycles += 5;
	} else cpu.cycles += 5;
	newline();                         // $A812
	cpu.cycles += 7 + 10;
	cpu.a = 0x16;
	leave_shop();
}
Native native_inn = { "inn", 0xA647, 2, inn, 0, 0, 0, .task = 1 };

// --------------------------------------------------------------- guild --
// $A869: the thieves' guild sells torches, gems and keys - and a sextant,
// once the party has learnt to ask for one ($C545).
#define GUILD_PRICES   0xA9CE           // 16-bit, per item
#define GUILD_AMOUNTS  0xA9D6           // how many come in one purchase
#define GUILD_NAMES    0xA9DA
#define GUILD_COUNTERS 0xAA0C           // where the party's count of each is kept

static void guild(void) {
	shop_sound(0x0A);
	msg(0x08);
	msg_shop(2);
	msg(0x09);
	task_wait_button(0xA880);
	blank_line();
	msg_shop(2);
	msg(0x0A);
	cpu.cycles += 13;
	cpu.a = mem_read(SHOP_NUMBER);
	msg_a();
	quote();
	task_wait_button(0xA89C);
	blank_line();
	cpu.cycles += 19 + 19 + 7;
	mem_write(SLOT + 0x1A, 0);
	mem_write(SLOT + 0x06, 0);
	cpu.a = 0x0B;                      // buying?
	for (;;) {                         // $A8AC
		msg_a();
		call(0x4B5E);
		until_c_clear(0xA8B2, 0x4B69);
		shop_sound(0x0A);
		newline();
		cpu.cycles += 20 + 7;
		if (mem_read(SLOT + 0x07) & 1) break;
		cpu.cycles += 5 + 20 + 7;
		if (!(mem_read(SLOT + 0x06) & 1)) {   // the first time: what there is
			cpu.cycles += 23;
			mem_write(SLOT + 0x06, mem_read(SLOT + 0x06) | 1);
			msg_shop(2);
			msg(0x0C);
			task_wait_button(0xA8EC);
			blank_line();
			msg(0x0D);
			cpu.cycles += 19 + 13 + 4 + 7;
			mem_write(SLOT + 0x1B, 3);
			if (mem_read(0xC545)) {
				cpu.cycles += 23;
				mem_write(SLOT + 0x1B, 4);
				msg(0x0E);             // and a sextant
			} else cpu.cycles += 5;
			quote();
			task_wait_button(0xA90C);
			blank_line();
		} else cpu.cycles += 5;
		msg(0x0F);                     // which?
		call(0x2E1B);
		cpu.cycles += 10 + 19;
		SET_HL(GUILD_NAMES + 8);
		cpu.b = mem_read(SLOT + 0x1B);
		do {
			cpu.cycles += 11;
			uint8_t b = cpu.b;
			call(0x2D65);              // menu line (moves HL on)
			cpu.cycles += 10 + 8;
			cpu.b = b;
			if (--cpu.b) cpu.cycles += 5;
		} while (cpu.b);
		call(0x2E54);
		for (;;) {
			task_frame_tick(0xA92A);
			cpu.cycles += 7 + 10;
			cpu.c = 4;
			SET_DE(0x0102);
			call(0x49F4);
			cpu.cycles += 7;
			if (!(cpu.f & FZ)) break;
			cpu.cycles += 5;
		}
		uint8_t f = cpu.f;
		shop_sound(0x0A);
		cpu.cycles += 7;
		if (f & FC) {
			newline();
			cpu.cycles += 12;
			break;
		}
		cpu.cycles += 5 + 19;
		uint8_t item = mem_read(SLOT + 0x1A);
		word_at(GUILD_NAMES, item);
		cpu.cycles += 17;
		text_print(HL);
		blank_line();
		cpu.cycles += 19 + 7;
		cpu.a = (uint8_t)(item + 4);
		msg_a();                       // about it
		task_wait_button(0xA958);
		blank_line();
		msg(0x10);                     // buy it?
		call(0x4B5E);
		until_c_clear(0xA966, 0x4B69);
		shop_sound(0x0A);
		newline();
		cpu.cycles += 20 + 7;
		if (mem_read(SLOT + 0x07) & 1) {
			msg(0x11);
			task_wait_button(0xA981);
			blank_line();
			cpu.cycles += 10;
			break;
		}
		cpu.cycles += 5 + 19;
		word_at(GUILD_PRICES, item);
		call(0x4423);                  // pay
		cpu.cycles += 7 + 10;
		if (cpu.f & FC) { cpu.a = 0x12; leave_shop(); return; }
		cpu.cycles += 19;
		uint16_t counter = word_at(GUILD_COUNTERS, item);
		cpu.cycles += 4 + 19;
		uint8_t n = byte_at(GUILD_AMOUNTS, item);
		cpu.cycles += 4 + 7 + 7 + 7;
		uint8_t v = (uint8_t)(n + mem_read(counter));
		if (v < 0x64) cpu.cycles += 5;
		else { cpu.cycles += 7; v = 0x63; }
		cpu.cycles += 7;
		mem_write(counter, v);
		SET_HL(counter);
		shop_sound(0x0A);
		msg(0x13);
		task_wait_button(0xA9BB);
		blank_line();
		msg_shop(2);
		cpu.cycles += 7 + 10;
		cpu.a = 0x14;                  // anything else?
	}
	msg_shop(2);                       // $A8C8
	cpu.cycles += 7 + 10;
	cpu.a = 0x15;
	leave_shop();
}
Native native_guild = { "guild", 0xA869, 2, guild, 0, 0, 0, .task = 1 };

// -------------------------------------------------------------- horses --
// $AA14: horses for the whole party, 100gp a head.
static void stable(void) {
	msg(0x00);                         // want horses?
	call(0x4B5E);
	until_c_clear(0xAA1C, 0x4B69);
	newline();
	cpu.cycles += 20 + 7 + 10;
	if (mem_read(SLOT + 0x07) & 1) { cpu.a = 0x01; leave_shop(); return; }
	msg(0x02);
	cpu.cycles += 13 + 7 + 17;
	text_char((uint8_t)(mem_read(PARTY_SIZE) + '0'));
	msg(0x03);                         // n horses, n*100gp - deal?
	call(0x4B5E);
	until_c_clear(0xAA45, 0x4B69);
	newline();
	cpu.cycles += 20 + 7 + 10;
	if (mem_read(SLOT + 0x07) & 1) { cpu.a = 0x01; leave_shop(); return; }
	cpu.cycles += 13 + 4 + 7;
	cpu.l = mem_read(PARTY_SIZE);
	cpu.h = 100;
	call(0x2AA1);
	call(0x4423);                      // pay
	cpu.cycles += 7 + 10;
	if (cpu.f & FC) { cpu.a = 0x04; leave_shop(); return; }
	cpu.cycles += 10 + 16 + 7 + 10;
	wr16(TIME_OF_DAY, 0x8614);         // (the party's transport: horses)
	cpu.a = 0x05;
	leave_shop();
}
Native native_stable = { "stable", 0xAA14, 2, stable, 0, 0, 0, .task = 1 };

// ---------------------------------------------------------------- seer --
// $AA75: the seer tells the Avatar how far along each virtue is. Asking
// gains Spirituality karma, at most once a while ($C969: moves / 100).
static void seer(void) {
	cpu.cycles += 7;
	cpu.c = 0;
	call(0x4466);
	cpu.cycles += 7;
	if (cpu.f & FC) {                  // the Avatar can't be helped now
		msg(0x28);
		call(0x5650);
		msg(0x29);
		call(0x5650);
		cpu.cycles += 7 + 10;
		cpu.a = 0x2A;
		leave_shop();
		return;
	}
	cpu.cycles += 5;
	msg(0x2B);
	call(0x5650);
	newline();
	msg(0x2C);
	task_wait_button(0xAAA1);
	blank_line();
	call(0xABE7);                      // the virtues menu
	cpu.cycles += 7;
	cpu.a = 0x2D;
	for (;;) {                         // $AAAC
		msg_a();
		for (;;) {
			task_frame_tick(0xAAAF);
			cpu.cycles += 7 + 10;
			cpu.c = 8;
			SET_DE(0x0102);
			call(0x49F4);
			cpu.cycles += 7;
			if (!(cpu.f & FZ)) break;
			cpu.cycles += 5;
		}
		cpu.cycles += 7;
		if (cpu.f & FC) {              // done: karma for asking
			call(0x4324);
			msg(0x2F);
			blank_line();
			cpu.cycles += 16 + 10;
			SET_HL(rd16(0xC8FD));
			SET_DE(0x0064);
			call(0x2B3F);              // moves / 100
			cpu.cycles += 4 + 16 + 4 + 15 + 4 + 16 + 7 + 7 + 10;
			uint16_t now = HL, last = rd16(0xC969);
			SET_HL(last); SET_DE(now);
			op_or(cpu.a);
			cpu.f = (cpu.f & ~FZ) | (now == last ? FZ : 0);
			wr16(0xC969, now);
			SET_HL(now); SET_DE((uint16_t)(last - now));
			cpu.a = 3; cpu.c = 6;
			if (now != last) { cpu.cycles += 7; call_original(0x4255); }   // call nz,karma_gain
			cpu.cycles += 7 + 10;
			cpu.a = 0x32;
			leave_shop();
			return;
		}
		cpu.cycles += 5 + 19 + 7 + 17;   // $AAE9: the virtue's name
		uint8_t virtue = mem_read(SLOT + 0x1A);
		cpu.a = (uint8_t)(virtue + 0x97);
		call_original(0x5516);
		blank_line();
		cpu.cycles += 19;
		uint8_t karma = byte_at(0xC90B, virtue);
		cpu.cycles += 4 + 7;
		if (karma == 0) {              // that eighth is won
			msg(0x30);
			task_wait_button(0xAB03);
			blank_line();
			cpu.cycles += 12;
		} else {
			cpu.cycles += 5 + 7;
			static const uint8_t steps[4] = { 0x14, 0x28, 0x3C, 0x63 };
			uint8_t b = 0;
			for (int i = 0; i < 4; i++) {
				cpu.cycles += 7 + 7;
				if (karma < steps[i]) { cpu.cycles += 5; break; }
				cpu.cycles += 4;
				b++;
			}
			cpu.cycles += 19 + 4 + 4 + 4 + 4 + 19;
			mem_write(SLOT + 0x1B, b);
			cpu.b = b;
			cpu.a = (uint8_t)(b * 8 + virtue);
			msg_a();                   // how far along
			task_wait_button(0xAB2E);
			blank_line();
			cpu.cycles += 19 + 7 + 7;
			if (b == 4) {              // ready for the shrine
				msg(0x31);
				task_wait_button(0xAB40);
				blank_line();
			} else cpu.cycles += 5;
		}
		cpu.cycles += 7 + 10;          // $AB46
		cpu.a = 0x2E;
	}
}
Native native_seer = { "seer", 0xAA75, 2, seer, 0, 0, 0, .task = 1 };
