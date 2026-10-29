// Rewritten routines that other rewritten code calls directly (pc/native).
// Each takes the original's cycles, including its ret, and leaves registers
// as the original does; a caller adds the cycles of its own call or jp.
#ifndef U4_GAME_H
#define U4_GAME_H

#include <stdint.h>

// text.c: the text window. These can wait for frames (scrolling, the
// "press a button" stop), so only blocking natives and C tasks may call them.
uint16_t text_print(uint16_t text);   // print_text_hl ($5519): returns HL after the text
void text_newline(void);              // print_newline ($54CE)
void text_char(uint8_t ch);           // print_char ($548F)

// math.c: what multiply_8x8 ($2AA1) takes for H * L, with its ret
int multiply_cycles(uint8_t h, uint8_t l);

// tasks.c: for C tasks (Native.task).
void frame_tick_work(void);           // what frame_tick ($2FFB) does before it yields
void task_frame_tick(uint16_t site);  // "call frame_tick" at site: ends the turn
int use_things(uint16_t site);         // the special things (items.c)
int ready_thing(int armour, uint16_t site);   // Ready's lists, bank 1 $57D7/$5855 (commands.c); 1: cancelled
void status_pages(uint16_t site);      // the status pages, bank 1 $7588 (commands.c)
int window_anim_call(uint16_t addr, uint16_t site);   // the window animations from a C task (window_anim.c)
void aim_at(uint16_t site);            // "call $37DD" (Aim:) at site, from a C task (commands.c)
void missile_step_at(uint16_t site);   // "call $392F": the missile a step on
void burst_at(uint16_t site);          // "call $3A91": its burst
void task_wait_button(uint16_t site); // "call wait_button" at site: waits for button 1 or 2
void task_wait_frames(uint16_t site, uint8_t n);   // "ld a,n / call $3046" at site: n turns
void task_wait_flag(uint16_t site);   // "call wait_flag" at site ($0A82): until $C0C7 is set
void task_next_pass(uint16_t site);   // "call $7F59" at site ($305A): the next turn, no frame wait

// creatures.c: the creatures on the map, at the end of the party's turn
int creatures_move(uint16_t back);   // $B5B3; back: where "call $B4CA" returns. 1: a fight began
void creatures_appear(void);          // $BB17

// vram.c, instruction by instruction (the machine catches up after each)
void vram_bank_push(uint16_t site);   // "call bank_push" ($07CE) at site, bank A
void vram_bank_pop(void);             // "jp bank_pop" ($07DE), with the ret
void vram_set_write_address(void);    // rst $10: DE as the VDP's write address

#endif
