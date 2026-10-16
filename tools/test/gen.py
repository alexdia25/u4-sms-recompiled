import sys; sys.path.insert(0, sys.argv[1])
from z80dec import decode
seqs=[]
for a in range(256):
    if a in (0xCB,0xED,0xDD,0xFD): continue
    seqs.append([a,0x05,0x12])
for a in range(256): seqs.append([0xCB,a])
for a in range(256): seqs.append([0xED,a,0x34,0x12])
for pre in (0xDD,0xFD):
    for a in range(256):
        if a==0xCB: continue
        seqs.append([pre,a,0x85,0x12])
    for a in range(256): seqs.append([pre,0xCB,0xF3,a])
lines=[];exp=[]
for s in seqs:
    m=lambda x: s[x-0x100] if 0<=x-0x100<len(s) else 0
    i=decode(m,0x100)
    if i.undoc: continue
    lines.append(i.text); exp.append(bytes(s[:i.length]))
open(sys.argv[2],'w').write('.MEMORYMAP\nSLOTSIZE $10000\nDEFAULTSLOT 0\nSLOT 0 $0000\n.ENDME\n.ROMBANKMAP\nBANKSTOTAL 1\nBANKSIZE $10000\nBANKS 1\n.ENDRO\n.BANK 0 SLOT 0\n'+''.join('.ORG $%04X\n%s\n'%(0x100+k*8 if False else 0x100,'') for k in [])+ '\n'.join('.ORGA $0100\n'*0 + l for l in []))
# one file per instruction is slow; instead place each at own org, jr targets relative to that
out=['.MEMORYMAP','SLOTSIZE $10000','DEFAULTSLOT 0','SLOT 0 $0000','.ENDME','.ROMBANKMAP','BANKSTOTAL 1','BANKSIZE $10000','BANKS 1','.ENDRO','.BANK 0 SLOT 0']
import pickle
recs=[]
for k,(s) in enumerate(seqs):
    base=0x1000+k*8
    m=lambda x,s=s,base=base: s[x-base] if 0<=x-base<len(s) else 0
    i=decode(m,base)
    if i.undoc: continue
    out.append('.ORGA $%04X'%base); out.append(i.fmt.replace('{T}','$%02X'%i.disp) if i.kind=='jr' else i.text); recs.append((base,bytes(s[:i.length]),i.text))
open(sys.argv[2],'w').write('\n'.join(out)+'\n')
pickle.dump(recs,open(sys.argv[3],'wb'))
print(len(recs),'documented encodings')
