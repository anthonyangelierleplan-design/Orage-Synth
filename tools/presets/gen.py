import re,sys,json
exec(open('presets.py').read())
OUT = json.load(open('output.json')) if len(sys.argv)>1 and sys.argv[1]=='cal' else {}
def ident(n): return 'v18_' + re.sub(r'[^A-Za-z0-9]','_',n)
def f(v): 
    if isinstance(v,int) or float(v)==int(v): return str(int(v))
    return ('%g'%v)+'f'
code=[]; table=[]
for x in P:
    fn=ident(x['name']); d=dict(x['d']); d['output']=OUT.get(x['name'],-9)
    kv=','.join('{"%s",%s}'%(k,f(v)) for k,v in d.items())
    body=['    S (p, { '+kv+' });']
    for i,(s,dd,a) in enumerate(x['mods']): body.append('    M (p, %d, %s, %s, %s);'%(i+1,s,dd,f(a)))
    if x['steps']:
        st=x['steps']; 
        if isinstance(st,tuple): body.append('    setSteps (p, "%s", "%s");'%st)
        else: body.append('    setSteps (p, "%s");'%st)
    if x['seq']:
        notes,ties=x['seq']; body.append('    setSeq (p, { %s }%s);'%(', '.join(map(str,notes)), (', "%s"'%ties) if ties else ''))
    for (ln,dst,amt,gl,vals) in x['lanes']:
        body.append('    setLane (p, %d, %s, %s, %s, { %s });'%(ln,dst,f(amt),f(gl),', '.join(f(v) for v in vals)))
    code.append('void %s (OrageAudioProcessor& p)\n{\n%s\n}'%(fn,'\n'.join(body)))
    table.append('{ "%s", "%s", %s }'%(x['name'],x['cat'],fn))
open('presets_fn.inc','w').write('\n'.join(code)+'\n')
lines=[]; cur=[]
for t in table:
    cur.append(t)
    if len(cur)==4: lines.append('    '+', '.join(cur)+','); cur=[]
if cur: lines.append('    '+', '.join(cur)+',')
open('presets_tab.inc','w').write('\n'.join(lines)+'\n')
print(len(P),'presets')
