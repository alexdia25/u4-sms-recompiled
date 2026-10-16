'use strict';
// Report which code writes a RAM range: node watch.js <rom> <state> <frames> <lo> <hi> [buttons-at-start]
const fs = require('fs');
const { SMS } = require('./sms');
const [romF, stateF, frames, lo, hi, btn] = process.argv.slice(2);
const sms = new SMS(fs.readFileSync(romF));
sms.loadState(JSON.parse(fs.readFileSync(stateF, 'utf8')));
const L = parseInt(lo, 16), H = parseInt(hi, 16);
const hits = new Map();
const w = sms.write.bind(sms);
sms.write = (a, v) => {
  if (a >= L && a <= H) {
    const k = `${sms.opPC.toString(16)} bank${sms.mapper[3]}`;
    hits.set(k, (hits.get(k) || 0) + 1);
  }
  w(a, v);
};
const BTN = { U: 1, D: 2, L: 4, R: 8, '1': 16, '2': 32 };
if (btn) { let m = 0; for (const c of btn) m |= BTN[c]; sms.pad1 = 0xFF & ~m; }
for (let f = 0; f < +frames; f++) { if (f === 10) sms.pad1 = 0xFF; sms.runFrame(); }
for (const [k, n] of [...hits].sort((a, b) => b[1] - a[1]).slice(0, 25)) console.log(k, n);
