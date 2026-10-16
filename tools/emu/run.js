'use strict';
// Drive the headless SMS from a command script.
//   node run.js <rom> <script> [--cov coverage.bin] [--out dir]
// Script commands (one per line, # comments):
//   load <state> | save <state> | wait <frames> | press <btns> [frames] |
//   hold <btns> <frames> | shot <name> | pause | sram <file>
// Buttons: U D L R 1 2 (combine like "R1").
const fs = require('fs'), path = require('path'), zlib = require('zlib');
const { SMS } = require('./sms');

function png(file, w, h, rgb) {
  const raw = Buffer.alloc((w * 3 + 1) * h);
  for (let y = 0; y < h; y++) rgb.copy(raw, y * (w * 3 + 1) + 1, y * w * 3, (y + 1) * w * 3);
  const chunk = (type, data) => {
    const len = Buffer.alloc(4); len.writeUInt32BE(data.length);
    const td = Buffer.concat([Buffer.from(type), data]);
    const crc = Buffer.alloc(4); crc.writeUInt32BE(zlib.crc32(td) >>> 0);
    return Buffer.concat([len, td, crc]);
  };
  const ihdr = Buffer.alloc(13);
  ihdr.writeUInt32BE(w, 0); ihdr.writeUInt32BE(h, 4); ihdr[8] = 8; ihdr[9] = 2;
  fs.writeFileSync(file, Buffer.concat([Buffer.from([137, 80, 78, 71, 13, 10, 26, 10]),
    chunk('IHDR', ihdr), chunk('IDAT', zlib.deflateSync(raw)), chunk('IEND', Buffer.alloc(0))]));
}

const args = process.argv.slice(2);
const opt = (k, d) => { const i = args.indexOf(k); return i >= 0 ? args[i + 1] : d; };
const rom = fs.readFileSync(args[0]);
const lines = fs.readFileSync(args[1], 'utf8').split(/\r?\n/);
const out = opt('--out', '.');
const covFile = opt('--cov', null);
const keylog = opt('--keylog', null);
const klog = [];
const sms = new SMS(rom);
const BTN = { U: 0x01, D: 0x02, L: 0x04, R: 0x08, '1': 0x10, '2': 0x20 };
const mask = s => [...s].reduce((m, c) => m | (BTN[c.toUpperCase()] || 0), 0);
let lastPad = 0xFF;
const frames = n => {
  for (let i = 0; i < n; i++) {
    if (sms.pad1 !== lastPad) { klog.push(`${sms.frame} ${padStr(sms.pad1)}`); lastPad = sms.pad1; }
    sms.runFrame();
  }
};
function padStr(p) { let s = ''; for (const [k, v] of Object.entries(BTN)) if (!(p & v)) s += k; return s || '-'; }
const hashFile = opt('--hashes', null);
const hashes = [];
if (hashFile) sms.frameHook = () => {
  let h = 2166136261; for (const b of sms.ram) h = Math.imul(h ^ b, 16777619) >>> 0;
  hashes.push(`${sms.frame} ${h.toString(16).toUpperCase().padStart(8, '0')}`);
};
const t0 = Date.now();
for (const raw of lines) {
  const ln = raw.replace(/#.*/, '').trim();
  if (!ln) continue;
  const [cmd, a, b] = ln.split(/\s+/);
  switch (cmd) {
    case 'load': sms.loadState(JSON.parse(fs.readFileSync(path.resolve(out, a), 'utf8'))); break;
    case 'save': fs.writeFileSync(path.resolve(out, a), JSON.stringify(sms.saveState())); break;
    case 'wait': frames(+a); break;
    case 'press': sms.pad1 = 0xFF & ~mask(a); frames(+(b || 6)); sms.pad1 = 0xFF; frames(6); break;
    case 'tap': for (let i = 0; i < +(b || 1); i++) { sms.pad1 = 0xFF & ~mask(a); frames(8); sms.pad1 = 0xFF; frames(8); } break;
    case 'hold': sms.pad1 = 0xFF & ~mask(a); frames(+b); sms.pad1 = 0xFF; break;
    case 'pause': klog.push(`${sms.frame} P`); sms.pause(); frames(6); break;
    case 'shot': png(path.resolve(out, a), 256, 192, sms.vdp.rgb()); break;
    case 'sram': fs.writeFileSync(path.resolve(out, a), sms.sram); break;
    case 'ram': fs.writeFileSync(path.resolve(out, a), sms.ram); break;
    case 'poke': sms.ram[parseInt(a, 16) & 0x1FFF] = parseInt(b, 16); break;
    default: throw new Error('unknown command: ' + ln);
  }
}
if (covFile) {
  let prev = fs.existsSync(covFile) ? fs.readFileSync(covFile) : Buffer.alloc(rom.length);
  for (let i = 0; i < rom.length; i++) prev[i] |= sms.cov[i];
  fs.writeFileSync(covFile, prev);
  let op = 0, rd = 0; for (const v of prev) { if (v & 1) op++; if (v & 4) rd++; }
  console.log(`coverage: ${op} opcode bytes, ${rd} data-read bytes (cumulative)`);
}
if (hashFile) fs.writeFileSync(hashFile, hashes.join('\n') + '\n');
if (keylog) fs.writeFileSync(keylog, klog.join('\n') + '\n');
let hash = 2166136261; for (const b of sms.ram) hash = Math.imul(hash ^ b, 16777619) >>> 0;
console.log(`ram-hash ${hash.toString(16).toUpperCase().padStart(8, '0')}`);
console.log(`frames ${sms.frame}, pc $${sms.cpu.pc.toString(16)}, ${((Date.now() - t0) / 1000).toFixed(1)}s`);
