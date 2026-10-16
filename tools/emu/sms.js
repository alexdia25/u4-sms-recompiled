'use strict';
// Headless Sega Master System: Sega mapper with battery RAM, mode-4 VDP with a
// scanline renderer, joypads, and a coverage log of every ROM byte touched.
const { Z80 } = require('./z80');

const LINES = 262, CYCLES_PER_LINE = 228;

// coverage flags per ROM byte
const COV_OP = 1, COV_ARG = 2, COV_READ = 4;

class VDP {
  constructor() {
    this.vram = new Uint8Array(0x4000);
    this.cram = new Uint8Array(32);
    this.reg = new Uint8Array(16);
    this.reg.set([0x36, 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFB, 0x00, 0x00, 0x00, 0xFF]);
    this.addr = 0; this.code = 0; this.latch = false; this.first = 0; this.buffer = 0;
    this.status = 0; this.line = 0; this.lineCounter = 0xFF; this.linePending = false;
    this.fb = new Uint8Array(256 * 192); // CRAM indices (0..31)
  }
  get irq() {
    return ((this.status & 0x80) && (this.reg[1] & 0x20)) || (this.linePending && (this.reg[0] & 0x10));
  }
  readData() {
    this.latch = false;
    const v = this.buffer;
    this.buffer = this.vram[this.addr];
    this.addr = (this.addr + 1) & 0x3FFF;
    return v;
  }
  writeData(v) {
    this.latch = false;
    if (this.code === 3) this.cram[this.addr & 0x1F] = v & 0x3F;
    else this.vram[this.addr] = v;
    this.buffer = v;
    this.addr = (this.addr + 1) & 0x3FFF;
  }
  readStatus() {
    const v = this.status | 0x1F;
    this.status = 0; this.linePending = false; this.latch = false;
    return v;
  }
  writeCtrl(v) {
    if (!this.latch) {
      this.latch = true; this.first = v;
      this.addr = (this.addr & 0x3F00) | v;
      return;
    }
    this.latch = false;
    this.code = v >> 6;
    this.addr = ((v & 0x3F) << 8) | this.first;
    if (this.code === 0) { this.buffer = this.vram[this.addr]; this.addr = (this.addr + 1) & 0x3FFF; }
    else if (this.code === 2) this.reg[v & 0x0F] = this.first;
  }
  vcounter() {
    const l = this.line;
    return l <= 0xDA ? l : l - 6;
  }
  // called at the end of each scanline
  endLine(line) {
    if (line < 192) this.renderLine(line);
    if (line <= 192) {
      if (this.lineCounter === 0) { this.lineCounter = this.reg[10]; this.linePending = true; }
      else this.lineCounter = (this.lineCounter - 1) & 0xFF;
    } else this.lineCounter = this.reg[10];
    if (line === 192) this.status |= 0x80;
  }
  renderLine(y) {
    const fb = this.fb, row = y * 256, reg = this.reg, vram = this.vram;
    const backdrop = 16 + (reg[7] & 0x0F);
    if (!(reg[1] & 0x40)) { fb.fill(backdrop, row, row + 256); return; }
    const prio = new Uint8Array(256), bgOpaque = new Uint8Array(256);
    const nameBase = (reg[2] & 0x0E) << 10;
    const sx = (y < 16 && (reg[0] & 0x40)) ? 0 : reg[8];
    for (let x = 0; x < 256; x++) {
      const lockV = (reg[0] & 0x80) && x >= 192;
      const vy = lockV ? y : (y + reg[9]) % 224;
      const px = (x - sx) & 0xFF;
      const col = px >> 3, trow = vy >> 3;
      const ent = nameBase + ((trow * 32 + col) << 1);
      const w = vram[ent] | (vram[ent + 1] << 8);
      const tile = w & 0x1FF, hf = w & 0x200, vf = w & 0x400, pal = (w & 0x800) ? 16 : 0, pr = w & 0x1000;
      let ty = vy & 7; if (vf) ty = 7 - ty;
      let tx = px & 7; if (!hf) tx = 7 - tx;
      const a = (tile << 5) + (ty << 2);
      const c = ((vram[a] >> tx) & 1) | (((vram[a + 1] >> tx) & 1) << 1) |
        (((vram[a + 2] >> tx) & 1) << 2) | (((vram[a + 3] >> tx) & 1) << 3);
      fb[row + x] = pal + c;
      bgOpaque[x] = c ? 1 : 0;
      prio[x] = pr ? 1 : 0;
    }
    // sprites
    const sat = (reg[5] & 0x7E) << 7;
    const tall = reg[1] & 0x02, zoom = reg[1] & 0x01;
    const h = (tall ? 16 : 8) << (zoom ? 1 : 0);
    const patBase = (reg[6] & 0x04) ? 256 : 0;
    const drawn = new Uint8Array(256);
    let count = 0;
    for (let i = 0; i < 64; i++) {
      let sy = vram[sat + i];
      if (sy === 0xD0) break;
      sy = sy + 1; if (sy > 240) sy -= 256;
      if (y < sy || y >= sy + h) continue;
      if (++count > 8) { this.status |= 0x40; break; }
      let sxp = vram[sat + 128 + i * 2];
      if (reg[0] & 0x08) sxp -= 8;
      let n = vram[sat + 129 + i * 2];
      if (tall) n &= 0xFE;
      let ly = (y - sy) >> (zoom ? 1 : 0);
      const tile = patBase + n + (ly >> 3);
      const a = (tile << 5) + ((ly & 7) << 2);
      for (let k = 0; k < (zoom ? 16 : 8); k++) {
        const x = sxp + k;
        if (x < 0 || x > 255) continue;
        const bit = 7 - (zoom ? k >> 1 : k);
        const c = ((vram[a] >> bit) & 1) | (((vram[a + 1] >> bit) & 1) << 1) |
          (((vram[a + 2] >> bit) & 1) << 2) | (((vram[a + 3] >> bit) & 1) << 3);
        if (!c) continue;
        if (drawn[x]) { this.status |= 0x20; continue; }
        drawn[x] = 1;
        if (prio[x] && bgOpaque[x]) continue;
        fb[row + x] = 16 + c;
      }
    }
    if (reg[0] & 0x20) fb.fill(backdrop, row, row + 8);
  }
  rgb() {
    const out = Buffer.alloc(256 * 192 * 3);
    for (let i = 0; i < 256 * 192; i++) {
      const c = this.cram[this.fb[i]];
      out[i * 3] = (c & 3) * 85; out[i * 3 + 1] = ((c >> 2) & 3) * 85; out[i * 3 + 2] = ((c >> 4) & 3) * 85;
    }
    return out;
  }
}

class SMS {
  constructor(rom) {
    this.rom = rom;
    this.nbanks = rom.length >> 14;
    this.ram = new Uint8Array(0x2000);
    this.sram = new Uint8Array(0x8000);
    this.sramUsed = false;
    this.mapper = new Uint8Array([0, 0, 1, 2]); // FFFC..FFFF
    this.vdp = new VDP();
    this.cpu = new Z80(this);
    this.pad1 = 0xFF; this.pad2 = 0xFF; // active low; pad2 bit4 = reset
    this.ioctrl = 0xFF;
    this.cov = new Uint8Array(rom.length);
    this.frame = 0;
    this.execHook = null;
  }
  // ROM offset for a CPU address, or -1
  romOffset(a) {
    if (a < 0x400) return a;
    if (a < 0x4000) return ((this.mapper[1] % this.nbanks) << 14) | a;
    if (a < 0x8000) return ((this.mapper[2] % this.nbanks) << 14) | (a & 0x3FFF);
    if (a < 0xC000) {
      if (this.mapper[0] & 0x08) return -1;
      return ((this.mapper[3] % this.nbanks) << 14) | (a & 0x3FFF);
    }
    return -1;
  }
  read(a) {
    if (a >= 0xC000) return this.ram[a & 0x1FFF];
    if (a >= 0x8000 && (this.mapper[0] & 0x08)) return this.sram[((this.mapper[0] & 0x04) << 12) | (a & 0x3FFF)];
    const o = this.romOffset(a);
    this.cov[o] |= COV_READ;
    return this.rom[o];
  }
  fetch(a) {
    if (a >= 0xC000) return this.ram[a & 0x1FFF];
    if (a >= 0x8000 && (this.mapper[0] & 0x08)) return this.sram[((this.mapper[0] & 0x04) << 12) | (a & 0x3FFF)];
    const o = this.romOffset(a);
    this.cov[o] |= (a === this.opPC) ? COV_OP : COV_ARG;
    return this.rom[o];
  }
  write(a, v) {
    if (a >= 0xC000) {
      this.ram[a & 0x1FFF] = v;
      if (a >= 0xFFFC) this.mapper[a - 0xFFFC] = v;
      return;
    }
    if (a >= 0x8000 && (this.mapper[0] & 0x08)) {
      this.sram[((this.mapper[0] & 0x04) << 12) | (a & 0x3FFF)] = v;
      this.sramUsed = true;
    }
  }
  inp(port) {
    port &= 0xFF;
    switch (port & 0xC1) {
      case 0x40: return this.vdp.vcounter();
      case 0x41: return 0;
      case 0x80: return this.vdp.readData();
      case 0x81: { const v = this.vdp.readStatus(); this.cpu.irqLine = this.vdp.irq; return v; }
      case 0xC0: return this.pad1;
      case 0xC1: {
        // TH outputs read back through bits 6/7 when configured as outputs
        let v = this.pad2 | 0xC0;
        if (!(this.ioctrl & 0x02)) v = (v & ~0x40) | ((this.ioctrl & 0x20) << 1);
        if (!(this.ioctrl & 0x08)) v = (v & ~0x80) | (this.ioctrl & 0x80);
        return v;
      }
      default: return 0xFF;
    }
  }
  out(port, v) {
    port &= 0xFF;
    switch (port & 0xC1) {
      case 0x01: this.ioctrl = v; break;
      case 0x80: this.vdp.writeData(v); break;
      case 0x81: this.vdp.writeCtrl(v); this.cpu.irqLine = this.vdp.irq; break;
      default: break; // memory control, PSG
    }
  }
  runFrame() {
    const cpu = this.cpu, vdp = this.vdp;
    for (let line = 0; line < LINES; line++) {
      vdp.line = line;
      const end = cpu.cycles + CYCLES_PER_LINE;
      while (cpu.cycles < end) {
        this.opPC = cpu.pc;
        if (this.execHook) this.execHook(cpu.pc);
        cpu.step();
      }
      vdp.endLine(line);
      cpu.irqLine = vdp.irq;
    }
    this.frame++;
    if (this.frameHook) this.frameHook();
  }
  pause() { this.cpu.nmiPending = true; }

  saveState() {
    const v = this.vdp;
    return {
      cpu: this.cpu.saveState(), ram: Buffer.from(this.ram).toString('base64'),
      sram: Buffer.from(this.sram).toString('base64'), mapper: Array.from(this.mapper),
      ioctrl: this.ioctrl, frame: this.frame,
      vdp: {
        vram: Buffer.from(v.vram).toString('base64'), cram: Array.from(v.cram), reg: Array.from(v.reg),
        addr: v.addr, code: v.code, latch: v.latch, first: v.first, buffer: v.buffer, status: v.status,
        lineCounter: v.lineCounter, linePending: v.linePending,
      },
    };
  }
  loadState(s) {
    this.cpu.loadState(s.cpu);
    this.ram.set(Buffer.from(s.ram, 'base64'));
    this.sram.set(Buffer.from(s.sram, 'base64'));
    this.mapper.set(s.mapper); this.ioctrl = s.ioctrl; this.frame = s.frame;
    const v = this.vdp, d = s.vdp;
    v.vram.set(Buffer.from(d.vram, 'base64')); v.cram.set(d.cram); v.reg.set(d.reg);
    Object.assign(v, { addr: d.addr, code: d.code, latch: d.latch, first: d.first, buffer: d.buffer,
      status: d.status, lineCounter: d.lineCounter, linePending: d.linePending });
  }
}

module.exports = { SMS, COV_OP, COV_ARG, COV_READ };
