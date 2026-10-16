'use strict';
// Convert a JS emulator save state (JSON) to the binary form the PC build loads
// with --state:  node tobin.js state.json state.bin
const fs = require('fs');
const s = JSON.parse(fs.readFileSync(process.argv[2], 'utf8'));
const c = s.cpu, v = s.vdp;
const parts = [Buffer.from('U4ST')];
parts.push(Buffer.from(['a', 'f', 'b', 'c', 'd', 'e', 'h', 'l', 'a_', 'f_', 'b_', 'c_', 'd_', 'e_', 'h_', 'l_'].map(k => c[k])));
const w = Buffer.alloc(8);
w.writeUInt16LE(c.ix, 0); w.writeUInt16LE(c.iy, 2); w.writeUInt16LE(c.sp, 4); w.writeUInt16LE(c.pc, 6);
parts.push(w);
parts.push(Buffer.from([c.i, c.r, c.iff1, c.iff2, c.im, c.halted ? 1 : 0, c.eiDelay ? 1 : 0, c.nmiPending ? 1 : 0]));
const cy = Buffer.alloc(8); cy.writeBigInt64LE(BigInt(Math.round(c.cycles))); parts.push(cy);
parts.push(Buffer.from(s.ram, 'base64'), Buffer.from(s.sram, 'base64'), Buffer.from(s.mapper), Buffer.from([s.ioctrl]));
parts.push(Buffer.from(v.vram, 'base64'), Buffer.from(v.cram), Buffer.from(v.reg));
const va = Buffer.alloc(2); va.writeUInt16LE(v.addr); parts.push(va);
parts.push(Buffer.from([v.code, v.latch ? 1 : 0, v.first, v.buffer, v.status, v.lineCounter, v.linePending ? 1 : 0]));
const fr = Buffer.alloc(4); fr.writeUInt32LE(s.frame); parts.push(fr);
fs.writeFileSync(process.argv[3], Buffer.concat(parts));
