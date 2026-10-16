'use strict';
// Coverage-guided random play: keeps a pool of save states that reached new
// code and keeps poking at them with random input.
//   node explore.js <rom> <cov.bin> <minutes> <state.json>... [--keep dir]
const fs = require('fs'), path = require('path');
const { SMS } = require('./sms');

const args = process.argv.slice(2);
const keepIdx = args.indexOf('--keep');
const keepDir = keepIdx >= 0 ? args.splice(keepIdx, 2)[1] : null;
const [romFile, covFile, minutes, ...seeds] = args;
const rom = fs.readFileSync(romFile);
const sms = new SMS(rom);
const cov = fs.existsSync(covFile) ? fs.readFileSync(covFile) : Buffer.alloc(rom.length);
const countOps = c => { let n = 0; for (const v of c) if (v & 1) n++; return n; };

// skip rendering: exploration only needs execution
sms.vdp.renderLine = function () {};

const pool = seeds.map(f => ({ state: JSON.parse(fs.readFileSync(f, 'utf8')), score: 1, name: path.basename(f) }));
const rnd = n => Math.floor(Math.random() * n);
const BUTTONS = [0x01, 0x02, 0x04, 0x08, 0x10, 0x20];

function randomInput() {
  const r = Math.random();
  if (r < 0.45) return BUTTONS[rnd(4)];          // walk
  if (r < 0.75) return 0x20;                      // button 2 (confirm / menu)
  if (r < 0.88) return 0x10;                      // button 1 (cancel)
  if (r < 0.95) return BUTTONS[rnd(4)] | 0x20;
  return 0;
}

const deadline = Date.now() + minutes * 60000;
let iter = 0, total = countOps(cov), found = 0;
console.log(`start: ${total} opcode bytes, pool ${pool.length}`);
while (Date.now() < deadline) {
  // prefer recent / productive states
  const pick = pool[Math.random() < 0.5 ? pool.length - 1 - rnd(Math.min(8, pool.length)) : rnd(pool.length)];
  sms.loadState(pick.state);
  sms.cov.fill(0);
  const frames = 300 + rnd(900);
  let f = 0;
  while (f < frames) {
    const btn = randomInput(), hold = 3 + rnd(20);
    sms.pad1 = 0xFF & ~btn;
    for (let i = 0; i < hold; i++) sms.runFrame();
    sms.pad1 = 0xFF;
    const gap = 2 + rnd(12);
    for (let i = 0; i < gap; i++) sms.runFrame();
    f += hold + gap;
    if (Math.random() < 0.02) { sms.pause(); }
  }
  let fresh = 0;
  for (let i = 0; i < rom.length; i++) {
    if ((sms.cov[i] & 1) && !(cov[i] & 1)) fresh++;
    cov[i] |= sms.cov[i];
  }
  iter++;
  if (fresh > 0) {
    found += fresh; total += fresh;
    const entry = { state: sms.saveState(), score: fresh, name: `it${iter}` };
    pool.push(entry);
    if (keepDir && fresh >= 40) fs.writeFileSync(path.join(keepDir, `x_${iter}_${fresh}.json`), JSON.stringify(entry.state));
    console.log(`iter ${iter}: +${fresh} (total ${total}, pool ${pool.length}) from ${pick.name}`);
    fs.writeFileSync(covFile, cov);
  }
}
fs.writeFileSync(covFile, cov);
console.log(`done: ${iter} runs, +${found}, total ${total} opcode bytes`);
