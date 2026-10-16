'use strict';
// Log where the game reads graphics/data streams from, by watching the entry
// of the routines that unpack or copy them. Plays each state with random
// input.   node datalog.js <rom> <frames> <out.json> <state.json>...
const fs = require('fs');
const { SMS } = require('./sms');
const [romF, frames, outF, ...states] = process.argv.slice(2);
const sms = new SMS(fs.readFileSync(romF));
sms.vdp.renderLine = () => {};
const seen = new Map();
const note = (kind, bank, addr, extra) => {
  const k = `${kind} ${bank} ${addr.toString(16)} ${extra || ''}`;
  seen.set(k, (seen.get(k) || 0) + 1);
};
const c = sms.cpu;
const slot2 = () => sms.mapper[3];
sms.execHook = pc => {
  switch (pc) {
    case 0x075F: note('planes4', slot2(), c.hl); break;            // one plane stream -> VRAM
    case 0x079E: note('planes4', slot2(), c.hl); break;
    case 0x0729: note('rle_skip', slot2(), c.hl); break;           // name table, every other byte
    case 0x06FF: note('rle', slot2(), c.hl); break;
    case 0x07EC: note('copy', c.a, c.hl, c.bc); break;             // ldir from bank A
    case 0x0465: note('vram_queue', c.a, c.hl, c.bc); break;       // queued VRAM upload
    case 0x0807: note('table_word', c.a, c.hl); break;
  }
};
for (const f of states) {
  sms.loadState(JSON.parse(fs.readFileSync(f, 'utf8')));
  for (let i = 0; i < +frames; i++) {
    if (i % 12 === 0) { const r = Math.random(); sms.pad1 = 0xFF & ~(r < 0.5 ? [1, 2, 4, 8][Math.floor(Math.random() * 4)] : r < 0.8 ? 0x20 : 0x10); }
    if (i % 12 === 6) sms.pad1 = 0xFF;
    sms.runFrame();
  }
}
fs.writeFileSync(outF, JSON.stringify([...seen.keys()].sort(), null, 1));
console.log(seen.size, 'distinct reads');
