import sys
import os; sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from z80dec import decode
d=open(sys.argv[1],'rb').read()
off=int(sys.argv[2],16); n=int(sys.argv[3]); base=int(sys.argv[4],16) if len(sys.argv)>4 else off
m=lambda a: d[off+a-base]
pc=base
for _ in range(n):
    i=decode(m,pc); print('%04X  %-12s %s'%(pc, d[off+pc-base:off+pc-base+i.length].hex(), i.text)); pc+=i.length
