#!/usr/bin/env python3
"""Check exact-layout adapters, EH input bounds and the revised private ABI."""
import json, os, pathlib, struct, subprocess, sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
records=[]
def run(name,args,expected=0):
    p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=30,
                     env={**os.environ,'PATH':''})
    records.append(dict(name=name,args=list(map(str,args)),status=p.returncode,
                        expected=expected,stderr=p.stderr,passed=p.returncode==expected))
    return p.returncode==expected
main='function @main() -> i32 { block ^entry: return i32 0 }\n'
for name,text,expected in [
    ('packed','global @a : obj<5x1> = { i8 3 i32 7 }\n'+main,0),
    ('too-small','global @a : obj<4x1> = { i8 3 i32 7 }\n'+main,1),
    ('too-large','global @a : obj<6x1> = { i8 3 i32 7 }\n'+main,1),
    ('wrong-layout-type','global @a : i32 = { i32 7 }\n'+main,1),
    ('bad-binding','declare global @t\nfunction @main() -> i32 { block ^entry: eh_catch @t, 1 [binding=bad] return i32 0 }\n',1),
]:
    src=out/(name+'.lowir');src.write_text(text)
    if run(name,[root/'dev/lowir','-o',out/(name+'.roundtrip'),src],expected) and not expected:
        run(name+'-native',[root/'dev/lowir2native','--dump-machine-ir',out/(name+'.mir'),'-o',out/name,src]);run(name+'-execute',[out/name])
src=out/'selected-handler.lowir'
src.write_text('function @main() -> i32 { block ^entry: eh_try ^handler throw i32 9 block ^handler: eh_catch_all, 7 %s = exception_selector i32 %r = cmp ne i32 %s, 7 return i32 %r }\n')
if run('selected-handler-native',[root/'dev/lowir2native','--dump-machine-ir',out/'selected-handler.mir','-o',out/'selected-handler',src]):
    run('selected-handler-execute',[out/'selected-handler'])
for name,clause in [('missing-catch-selector','eh_catch @t'),('missing-all-selector','eh_catch_all')]:
    src=out/(name+'.lowir');src.write_text('declare global @t\nfunction @main() -> i32 { block ^entry: '+clause+' return i32 0 }\n')
    # Generic views accept omitted selectors; PA25's native typed dispatch needs
    # an explicit value and must diagnose it before reading a missing operand.
    run(name+'-view',[root/'dev/lowir','-o',out/(name+'.roundtrip'),src])
    run(name+'-native',[root/'dev/lowir2native','--dump-machine-ir',out/(name+'.mir'),src],1)
src=out/'object.cc';src.write_text('int main(){return 0;}\n');obj=out/'object.obj'
if run('version4-object',[root/'dev/cppgm++','-c','-o',obj,src]):
    raw=bytearray(obj.read_bytes());assert raw[:8]==b'CPPGMOBJ' and struct.unpack_from('<Q',raw,8)[0]==4
    struct.pack_into('<Q',raw,8,3);old=out/'version3.obj';old.write_bytes(raw)
    run('reject-version3',[root/'dev/cppgm++','-o',out/'old',old],1)
(out/'results.json').write_text(json.dumps(records,indent=2)+'\n')
print(json.dumps([r for r in records if not r['passed']],indent=2))
print(f"{sum(r['passed'] for r in records)}/{len(records)} adapter checks passed")
sys.exit(any(not r['passed'] for r in records))
