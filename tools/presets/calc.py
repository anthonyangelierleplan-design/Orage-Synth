import json,sys
m=json.load(open('/home/claude/hr/o7/t18.json')); cur=json.load(open('output.json'))
tr={'Pad':-19,'Bass':-17,'Lead':-18,'FX / Texture':-21}; tp=0.40
out={}
for n,(rms,pk,cat) in m.items():
    if n=='_': continue
    o=cur.get(n,-9)
    if cat in tr and n not in ('Seq Bass Line',): d=tr[cat]-rms
    else:
        import math; d=20*math.log10(tp/max(pk,1e-4))
    out[n]=round(max(-22,min(6,o+d)),1)
json.dump(out,open('output.json','w'),indent=0)
big=[(n,v) for n,v in out.items() if abs(v-cur.get(n,-9))>0.3]
print(len(big),'presets ajustes')
