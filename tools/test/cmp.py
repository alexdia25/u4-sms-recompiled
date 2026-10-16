import pickle,sys
recs=pickle.load(open(sys.argv[1],'rb')); d=open(sys.argv[2],'rb').read()
bad=[(t,e.hex(),d[b:b+len(e)].hex()) for b,e,t in recs if d[b:b+len(e)]!=e]
print(len(bad),'mismatches'); [print(x) for x in bad[:40]]
