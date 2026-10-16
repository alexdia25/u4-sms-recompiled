'use strict';
// Z80 CPU core. The bus supplies fetch(a) (opcode/operand), read(a), write(a,v),
// inp(port), out(port,v). Documented behaviour plus the undocumented X/Y flags
// and IXH/IXL forms; enough for commercial SMS software.

const FS = 0x80, FZ = 0x40, FY = 0x20, FH = 0x10, FX = 0x08, FP = 0x04, FN = 0x02, FC = 0x01;

const SZ = new Uint8Array(256), SZP = new Uint8Array(256);
for (let i = 0; i < 256; i++) {
  let p = i; p ^= p >> 4; p ^= p >> 2; p ^= p >> 1;
  SZ[i] = (i & 0x80) | (i === 0 ? FZ : 0) | (i & (FX | FY));
  SZP[i] = SZ[i] | ((p & 1) ? 0 : FP);
}

const CYC = [
  4,10,7,6,4,4,7,4,4,11,7,6,4,4,7,4,
  8,10,7,6,4,4,7,4,12,11,7,6,4,4,7,4,
  7,10,16,6,4,4,7,4,7,11,16,6,4,4,7,4,
  7,10,13,6,11,11,10,4,7,11,13,6,4,4,7,4,
  4,4,4,4,4,4,7,4,4,4,4,4,4,4,7,4,
  4,4,4,4,4,4,7,4,4,4,4,4,4,4,7,4,
  4,4,4,4,4,4,7,4,4,4,4,4,4,4,7,4,
  7,7,7,7,7,7,4,7,4,4,4,4,4,4,7,4,
  4,4,4,4,4,4,7,4,4,4,4,4,4,4,7,4,
  4,4,4,4,4,4,7,4,4,4,4,4,4,4,7,4,
  4,4,4,4,4,4,7,4,4,4,4,4,4,4,7,4,
  4,4,4,4,4,4,7,4,4,4,4,4,4,4,7,4,
  5,10,10,10,10,11,7,11,5,10,10,0,10,17,7,11,
  5,10,10,11,10,11,7,11,5,4,10,11,10,0,7,11,
  5,10,10,19,10,11,7,11,5,4,10,4,10,0,7,11,
  5,10,10,4,10,11,7,11,5,6,10,4,10,0,7,11,
];

class Z80 {
  constructor(bus) {
    this.bus = bus;
    this.reset();
  }

  reset() {
    this.a = 0xFF; this.f = 0xFF; this.b = 0; this.c = 0; this.d = 0; this.e = 0; this.h = 0; this.l = 0;
    this.a_ = 0; this.f_ = 0; this.b_ = 0; this.c_ = 0; this.d_ = 0; this.e_ = 0; this.h_ = 0; this.l_ = 0;
    this.ix = 0xFFFF; this.iy = 0xFFFF; this.sp = 0xDFF0; this.pc = 0;
    this.i = 0; this.r = 0; this.iff1 = 0; this.iff2 = 0; this.im = 0;
    this.halted = false; this.eiDelay = false;
    this.irqLine = false; this.nmiPending = false;
    this.cycles = 0;
  }

  // ---- register pairs ----
  get bc() { return (this.b << 8) | this.c; } set bc(v) { this.b = (v >> 8) & 0xFF; this.c = v & 0xFF; }
  get de() { return (this.d << 8) | this.e; } set de(v) { this.d = (v >> 8) & 0xFF; this.e = v & 0xFF; }
  get hl() { return (this.h << 8) | this.l; } set hl(v) { this.h = (v >> 8) & 0xFF; this.l = v & 0xFF; }
  get af() { return (this.a << 8) | this.f; } set af(v) { this.a = (v >> 8) & 0xFF; this.f = v & 0xFF; }

  f8() { const v = this.bus.fetch(this.pc); this.pc = (this.pc + 1) & 0xFFFF; return v; }
  f16() { const lo = this.f8(); return lo | (this.f8() << 8); }
  rd16(a) { return this.bus.read(a) | (this.bus.read((a + 1) & 0xFFFF) << 8); }
  wr16(a, v) { this.bus.write(a, v & 0xFF); this.bus.write((a + 1) & 0xFFFF, (v >> 8) & 0xFF); }
  push(v) { this.sp = (this.sp - 2) & 0xFFFF; this.wr16(this.sp, v); }
  pop() { const v = this.rd16(this.sp); this.sp = (this.sp + 2) & 0xFFFF; return v; }

  // ---- ALU ----
  add8(v, cy) {
    const a = this.a, r = a + v + cy;
    this.f = SZ[r & 0xFF] | ((r >> 8) & FC) | ((a ^ v ^ r) & FH) |
      ((((a ^ ~v) & (a ^ r)) & 0x80) ? FP : 0);
    this.a = r & 0xFF;
  }
  sub8(v, cy, store) {
    const a = this.a, r = a - v - cy;
    const f = (SZ[r & 0xFF] & ~(FX | FY)) | ((r >> 8) & FC) | FN | ((a ^ v ^ r) & FH) |
      ((((a ^ v) & (a ^ r)) & 0x80) ? FP : 0);
    if (store) { this.a = r & 0xFF; this.f = f | (r & (FX | FY)); }
    else this.f = f | (v & (FX | FY));
  }
  alu(op, v) {
    switch (op) {
      case 0: this.add8(v, 0); break;
      case 1: this.add8(v, this.f & FC); break;
      case 2: this.sub8(v, 0, true); break;
      case 3: this.sub8(v, this.f & FC, true); break;
      case 4: this.a &= v; this.f = SZP[this.a] | FH; break;
      case 5: this.a ^= v; this.f = SZP[this.a]; break;
      case 6: this.a |= v; this.f = SZP[this.a]; break;
      case 7: this.sub8(v, 0, false); break;
    }
  }
  inc8(v) {
    const r = (v + 1) & 0xFF;
    this.f = (this.f & FC) | SZ[r] | ((r & 0x0F) === 0 ? FH : 0) | (v === 0x7F ? FP : 0);
    return r;
  }
  dec8(v) {
    const r = (v - 1) & 0xFF;
    this.f = (this.f & FC) | SZ[r] | FN | ((v & 0x0F) === 0 ? FH : 0) | (v === 0x80 ? FP : 0);
    return r;
  }
  add16(a, b) {
    const r = a + b;
    this.f = (this.f & (FS | FZ | FP)) | ((r >> 16) & FC) | (((a ^ b ^ r) >> 8) & FH) | ((r >> 8) & (FX | FY));
    return r & 0xFFFF;
  }
  adc16(b) {
    const a = this.hl, c = this.f & FC, r = a + b + c;
    this.f = ((r >> 16) & FC) | (((a ^ b ^ r) >> 8) & FH) | ((r >> 8) & (FS | FX | FY)) |
      ((r & 0xFFFF) === 0 ? FZ : 0) | ((((a ^ ~b) & (a ^ r)) & 0x8000) ? FP : 0);
    this.hl = r & 0xFFFF;
  }
  sbc16(b) {
    const a = this.hl, c = this.f & FC, r = a - b - c;
    this.f = ((r >> 16) & FC) | FN | (((a ^ b ^ r) >> 8) & FH) | ((r >> 8) & (FS | FX | FY)) |
      ((r & 0xFFFF) === 0 ? FZ : 0) | ((((a ^ b) & (a ^ r)) & 0x8000) ? FP : 0);
    this.hl = r & 0xFFFF;
  }
  rot(op, v) {
    let r, c;
    switch (op) {
      case 0: c = v >> 7; r = ((v << 1) | c) & 0xFF; break;
      case 1: c = v & 1; r = (v >> 1) | (c << 7); break;
      case 2: c = v >> 7; r = ((v << 1) | (this.f & FC)) & 0xFF; break;
      case 3: c = v & 1; r = (v >> 1) | ((this.f & FC) << 7); break;
      case 4: c = v >> 7; r = (v << 1) & 0xFF; break;
      case 5: c = v & 1; r = (v >> 1) | (v & 0x80); break;
      case 6: c = v >> 7; r = ((v << 1) | 1) & 0xFF; break;
      default: c = v & 1; r = v >> 1; break;
    }
    this.f = SZP[r] | c;
    return r;
  }
  daa() {
    let a = this.a, corr = 0, c = this.f & FC;
    if ((this.f & FH) || (a & 0x0F) > 9) corr = 6;
    if (c || a > 0x99) { corr |= 0x60; c = FC; }
    const n = this.f & FN;
    const r = n ? (a - corr) & 0xFF : (a + corr) & 0xFF;
    const h = n ? ((this.f & FH) && (a & 0x0F) < 6 ? FH : 0) : ((a & 0x0F) > 9 ? FH : 0);
    this.a = r;
    this.f = SZP[r] | c | n | h;
  }

  cond(y) {
    const f = this.f;
    switch (y) {
      case 0: return !(f & FZ); case 1: return !!(f & FZ);
      case 2: return !(f & FC); case 3: return !!(f & FC);
      case 4: return !(f & FP); case 5: return !!(f & FP);
      case 6: return !(f & FS); default: return !!(f & FS);
    }
  }

  // r[i] access; idx: 0 = hl, 1 = ix, 2 = iy; ea = address for (hl)/(ix+d)
  getR(i, idx, ea) {
    switch (i) {
      case 0: return this.b; case 1: return this.c; case 2: return this.d; case 3: return this.e;
      case 4: return idx === 0 ? this.h : idx === 1 ? this.ix >> 8 : this.iy >> 8;
      case 5: return idx === 0 ? this.l : idx === 1 ? this.ix & 0xFF : this.iy & 0xFF;
      case 6: return this.bus.read(ea);
      default: return this.a;
    }
  }
  setR(i, idx, ea, v) {
    switch (i) {
      case 0: this.b = v; break; case 1: this.c = v; break; case 2: this.d = v; break; case 3: this.e = v; break;
      case 4: if (idx === 0) this.h = v; else if (idx === 1) this.ix = (this.ix & 0xFF) | (v << 8); else this.iy = (this.iy & 0xFF) | (v << 8); break;
      case 5: if (idx === 0) this.l = v; else if (idx === 1) this.ix = (this.ix & 0xFF00) | v; else this.iy = (this.iy & 0xFF00) | v; break;
      case 6: this.bus.write(ea, v); break;
      default: this.a = v;
    }
  }
  getRP(p, idx) {
    switch (p) {
      case 0: return this.bc; case 1: return this.de;
      case 2: return idx === 0 ? this.hl : idx === 1 ? this.ix : this.iy;
      default: return this.sp;
    }
  }
  setRP(p, idx, v) {
    switch (p) {
      case 0: this.bc = v; break; case 1: this.de = v; break;
      case 2: if (idx === 0) this.hl = v; else if (idx === 1) this.ix = v; else this.iy = v; break;
      default: this.sp = v;
    }
  }

  interrupt() {
    if (this.nmiPending) {
      this.nmiPending = false;
      this.halted = false;
      this.iff2 = this.iff1; this.iff1 = 0;
      this.push(this.pc); this.pc = 0x66;
      this.cycles += 11;
      return;
    }
    if (this.irqLine && this.iff1 && !this.eiDelay) {
      this.halted = false;
      this.iff1 = this.iff2 = 0;
      this.push(this.pc);
      if (this.im === 2) this.pc = this.rd16((this.i << 8) | 0xFF);
      else this.pc = 0x38;
      this.cycles += 13;
    }
  }

  step() {
    this.interrupt();
    this.bus.opPC = this.pc;
    this.eiDelay = false;
    if (this.halted) { this.cycles += 4; this.r = (this.r & 0x80) | ((this.r + 1) & 0x7F); return; }
    this.r = (this.r & 0x80) | ((this.r + 1) & 0x7F);
    const op = this.f8();
    if (op === 0xDD) return this.indexed(1);
    if (op === 0xFD) return this.indexed(2);
    if (op === 0xCB) return this.cb(0, 0);
    if (op === 0xED) return this.ed();
    this.main(op, 0, 0);
  }

  indexed(idx) {
    this.r = (this.r & 0x80) | ((this.r + 1) & 0x7F);
    const op = this.f8();
    if (op === 0xCB) {
      const d = this.f8();
      const base = idx === 1 ? this.ix : this.iy;
      const ea = (base + ((d ^ 0x80) - 0x80)) & 0xFFFF;
      return this.cb(idx, ea);
    }
    if (op === 0xDD || op === 0xFD || op === 0xED) {
      this.cycles += 4;
      this.pc = (this.pc - 1) & 0xFFFF; // prefix acts as NOP
      return;
    }
    this.main(op, idx, 1);
  }

  main(op, idx, pre) {
    const x = op >> 6, y = (op >> 3) & 7, z = op & 7, p = y >> 1, q = y & 1;
    let cyc = CYC[op];
    // effective address for (hl) / (ix+d)
    const usesMem = (x === 1 && (y === 6 || z === 6) && op !== 0x76) || (x === 2 && z === 6) ||
      (x === 0 && (z === 4 || z === 5 || z === 6) && y === 6);
    let ea = 0, extra = pre ? 4 : 0;
    if (usesMem) {
      if (idx) {
        const d = this.f8();
        ea = ((idx === 1 ? this.ix : this.iy) + ((d ^ 0x80) - 0x80)) & 0xFFFF;
        cyc = (x === 0 && z !== 6) ? 23 : 19;
        extra = 0;
      } else ea = this.hl;
    }
    cyc += extra;
    // with a memory operand, the register operand is plain h/l
    const ridx = usesMem ? 0 : idx;
    switch (x) {
      case 0:
        switch (z) {
          case 0:
            switch (y) {
              case 0: break;
              case 1: { let t = this.a; this.a = this.a_; this.a_ = t; t = this.f; this.f = this.f_; this.f_ = t; break; }
              case 2: { const d = this.f8(); this.b = (this.b - 1) & 0xFF; if (this.b) { this.pc = (this.pc + ((d ^ 0x80) - 0x80)) & 0xFFFF; cyc += 5; } break; }
              case 3: { const d = this.f8(); this.pc = (this.pc + ((d ^ 0x80) - 0x80)) & 0xFFFF; break; }
              default: { const d = this.f8(); if (this.cond(y - 4)) { this.pc = (this.pc + ((d ^ 0x80) - 0x80)) & 0xFFFF; cyc += 5; } }
            }
            break;
          case 1:
            if (q === 0) this.setRP(p, idx, this.f16());
            else this.setRP(2, idx, this.add16(this.getRP(2, idx), this.getRP(p, idx)));
            break;
          case 2:
            switch (y) {
              case 0: this.bus.write(this.bc, this.a); break;
              case 1: this.a = this.bus.read(this.bc); break;
              case 2: this.bus.write(this.de, this.a); break;
              case 3: this.a = this.bus.read(this.de); break;
              case 4: this.wr16(this.f16(), this.getRP(2, idx)); break;
              case 5: this.setRP(2, idx, this.rd16(this.f16())); break;
              case 6: this.bus.write(this.f16(), this.a); break;
              default: this.a = this.bus.read(this.f16());
            }
            break;
          case 3: this.setRP(p, idx, (this.getRP(p, idx) + (q ? -1 : 1)) & 0xFFFF); break;
          case 4: this.setR(y, ridx, ea, this.inc8(this.getR(y, ridx, ea))); break;
          case 5: this.setR(y, ridx, ea, this.dec8(this.getR(y, ridx, ea))); break;
          case 6: this.setR(y, ridx, ea, this.f8()); break;
          default:
            switch (y) {
              case 0: { const c = this.a >> 7; this.a = ((this.a << 1) | c) & 0xFF; this.f = (this.f & (FS | FZ | FP)) | c | (this.a & (FX | FY)); break; }
              case 1: { const c = this.a & 1; this.a = (this.a >> 1) | (c << 7); this.f = (this.f & (FS | FZ | FP)) | c | (this.a & (FX | FY)); break; }
              case 2: { const c = this.a >> 7; this.a = ((this.a << 1) | (this.f & FC)) & 0xFF; this.f = (this.f & (FS | FZ | FP)) | c | (this.a & (FX | FY)); break; }
              case 3: { const c = this.a & 1; this.a = (this.a >> 1) | ((this.f & FC) << 7); this.f = (this.f & (FS | FZ | FP)) | c | (this.a & (FX | FY)); break; }
              case 4: this.daa(); break;
              case 5: this.a ^= 0xFF; this.f = (this.f & (FS | FZ | FP | FC)) | FH | FN | (this.a & (FX | FY)); break;
              case 6: this.f = (this.f & (FS | FZ | FP)) | FC | (this.a & (FX | FY)); break;
              default: this.f = ((this.f & (FS | FZ | FP | FC)) | ((this.f & FC) ? FH : 0) | (this.a & (FX | FY))) ^ FC;
            }
        }
        break;
      case 1:
        if (op === 0x76) { this.halted = true; break; }
        this.setR(y, z === 6 ? 0 : ridx, ea, this.getR(z, y === 6 ? 0 : ridx, ea));
        break;
      case 2:
        this.alu(y, this.getR(z, ridx, ea));
        break;
      default:
        switch (z) {
          case 0: if (this.cond(y)) { this.pc = this.pop(); cyc += 6; } break;
          case 1:
            if (q === 0) {
              const v = this.pop();
              if (p === 3) this.af = v; else this.setRP(p, idx, v);
            } else {
              switch (p) {
                case 0: this.pc = this.pop(); break;
                case 1: {
                  let t;
                  t = this.b; this.b = this.b_; this.b_ = t; t = this.c; this.c = this.c_; this.c_ = t;
                  t = this.d; this.d = this.d_; this.d_ = t; t = this.e; this.e = this.e_; this.e_ = t;
                  t = this.h; this.h = this.h_; this.h_ = t; t = this.l; this.l = this.l_; this.l_ = t;
                  break;
                }
                case 2: this.pc = this.getRP(2, idx); break;
                default: this.sp = this.getRP(2, idx);
              }
            }
            break;
          case 2: { const t = this.f16(); if (this.cond(y)) this.pc = t; break; }
          case 3:
            switch (y) {
              case 0: this.pc = this.f16(); break;
              case 2: this.bus.out(this.f8() | (this.a << 8), this.a); break;
              case 3: this.a = this.bus.inp(this.f8() | (this.a << 8)); break;
              case 4: { const v = this.rd16(this.sp); this.wr16(this.sp, this.getRP(2, idx)); this.setRP(2, idx, v); break; }
              case 5: { const t = this.de; this.de = this.hl; this.hl = t; break; }
              case 6: this.iff1 = this.iff2 = 0; break;
              default: this.iff1 = this.iff2 = 1; this.eiDelay = true;
            }
            break;
          case 4: { const t = this.f16(); if (this.cond(y)) { this.push(this.pc); this.pc = t; cyc += 7; } break; }
          case 5:
            if (q === 0) this.push(p === 3 ? this.af : this.getRP(p, idx));
            else { const t = this.f16(); this.push(this.pc); this.pc = t; }
            break;
          case 6: this.alu(y, this.f8()); break;
          default: this.push(this.pc); this.pc = y * 8;
        }
    }
    this.cycles += cyc;
  }

  cb(idx, ea) {
    let op;
    if (idx) op = this.f8();
    else { this.r = (this.r & 0x80) | ((this.r + 1) & 0x7F); op = this.f8(); ea = this.hl; }
    const x = op >> 6, y = (op >> 3) & 7, z = op & 7;
    const reg = idx ? 6 : z;
    const v = this.getR(reg, 0, ea);
    switch (x) {
      case 0: { const r = this.rot(y, v); this.setR(reg, 0, ea, r); if (idx && z !== 6) this.setR(z, 0, 0, r); break; }
      case 1: {
        const b = v & (1 << y);
        this.f = (this.f & FC) | FH | (b ? 0 : (FZ | FP)) | (b & FS) | (reg === 6 ? ((ea >> 8) & (FX | FY)) : (v & (FX | FY)));
        break;
      }
      case 2: { const r = v & ~(1 << y); this.setR(reg, 0, ea, r); if (idx && z !== 6) this.setR(z, 0, 0, r); break; }
      default: { const r = v | (1 << y); this.setR(reg, 0, ea, r); if (idx && z !== 6) this.setR(z, 0, 0, r); }
    }
    this.cycles += idx ? (x === 1 ? 20 : 23) : (reg === 6 ? (x === 1 ? 12 : 15) : 8);
  }

  ed() {
    this.r = (this.r & 0x80) | ((this.r + 1) & 0x7F);
    const op = this.f8();
    const x = op >> 6, y = (op >> 3) & 7, z = op & 7, p = y >> 1, q = y & 1;
    let cyc = 8;
    if (x === 1) {
      switch (z) {
        case 0: { const v = this.bus.inp(this.bc); if (y !== 6) this.setR(y, 0, 0, v); this.f = (this.f & FC) | SZP[v]; cyc = 12; break; }
        case 1: this.bus.out(this.bc, y === 6 ? 0 : this.getR(y, 0, 0)); cyc = 12; break;
        case 2: if (q === 0) this.sbc16(this.getRP(p, 0)); else this.adc16(this.getRP(p, 0)); cyc = 15; break;
        case 3: { const a = this.f16(); if (q === 0) this.wr16(a, this.getRP(p, 0)); else this.setRP(p, 0, this.rd16(a)); cyc = 20; break; }
        case 4: { const v = this.a; this.a = 0; this.sub8(v, 0, true); break; }
        case 5: this.pc = this.pop(); this.iff1 = this.iff2; cyc = 14; break;
        case 6: this.im = [0, 0, 1, 2, 0, 0, 1, 2][y]; break;
        default:
          switch (y) {
            case 0: this.i = this.a; cyc = 9; break;
            case 1: this.r = this.a; cyc = 9; break;
            case 2: this.a = this.i; this.f = (this.f & FC) | SZ[this.a] | (this.iff2 ? FP : 0); cyc = 9; break;
            case 3: this.a = this.r; this.f = (this.f & FC) | SZ[this.a] | (this.iff2 ? FP : 0); cyc = 9; break;
            case 4: { const m = this.bus.read(this.hl); this.bus.write(this.hl, ((this.a << 4) | (m >> 4)) & 0xFF); this.a = (this.a & 0xF0) | (m & 0x0F); this.f = (this.f & FC) | SZP[this.a]; cyc = 18; break; }
            case 5: { const m = this.bus.read(this.hl); this.bus.write(this.hl, ((m << 4) | (this.a & 0x0F)) & 0xFF); this.a = (this.a & 0xF0) | (m >> 4); this.f = (this.f & FC) | SZP[this.a]; cyc = 18; break; }
            default: break;
          }
      }
    } else if (x === 2 && y >= 4 && z <= 3) {
      cyc = this.block(y, z);
    }
    this.cycles += cyc;
  }

  block(y, z) {
    const dec = y & 1, rep = y >= 6;
    const step = dec ? -1 : 1;
    let cyc = 16;
    switch (z) {
      case 0: { // ldi/ldd
        const v = this.bus.read(this.hl);
        this.bus.write(this.de, v);
        this.hl = (this.hl + step) & 0xFFFF; this.de = (this.de + step) & 0xFFFF; this.bc = (this.bc - 1) & 0xFFFF;
        const n = (v + this.a) & 0xFF;
        this.f = (this.f & (FS | FZ | FC)) | (this.bc ? FP : 0) | (n & FX) | ((n << 4) & FY);
        if (rep && this.bc) { this.pc = (this.pc - 2) & 0xFFFF; cyc = 21; }
        break;
      }
      case 1: { // cpi/cpd
        const v = this.bus.read(this.hl), r = (this.a - v) & 0xFF;
        const h = (this.a ^ v ^ r) & FH;
        this.hl = (this.hl + step) & 0xFFFF; this.bc = (this.bc - 1) & 0xFFFF;
        this.f = (this.f & FC) | (SZ[r] & ~(FX | FY)) | h | FN | (this.bc ? FP : 0);
        if (rep && this.bc && r !== 0) { this.pc = (this.pc - 2) & 0xFFFF; cyc = 21; }
        break;
      }
      case 2: { // ini/ind
        const v = this.bus.inp(this.bc);
        this.bus.write(this.hl, v);
        this.hl = (this.hl + step) & 0xFFFF; this.b = (this.b - 1) & 0xFF;
        this.f = SZ[this.b] | FN;
        if (rep && this.b) { this.pc = (this.pc - 2) & 0xFFFF; cyc = 21; }
        break;
      }
      default: { // outi/outd
        const v = this.bus.read(this.hl);
        this.b = (this.b - 1) & 0xFF;
        this.bus.out(this.bc, v);
        this.hl = (this.hl + step) & 0xFFFF;
        this.f = SZ[this.b] | FN;
        if (rep && this.b) { this.pc = (this.pc - 2) & 0xFFFF; cyc = 21; }
      }
    }
    return cyc;
  }

  saveState() {
    const s = {};
    for (const k of ['a', 'f', 'b', 'c', 'd', 'e', 'h', 'l', 'a_', 'f_', 'b_', 'c_', 'd_', 'e_', 'h_', 'l_',
      'ix', 'iy', 'sp', 'pc', 'i', 'r', 'iff1', 'iff2', 'im', 'halted', 'eiDelay', 'nmiPending', 'cycles']) s[k] = this[k];
    return s;
  }
  loadState(s) { Object.assign(this, s); }
}

module.exports = { Z80 };
