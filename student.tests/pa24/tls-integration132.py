#!/usr/bin/env python3
"""Check TLS MIR/native identity, debug, multifile and runtime image boundaries."""
import hashlib, json, pathlib, struct, subprocess, sys
root=pathlib.Path(__file__).resolve().parents[2]
cc=pathlib.Path(sys.argv[1]).resolve(); out=pathlib.Path(sys.argv[2]).resolve()
out.mkdir(parents=True,exist_ok=True)
def compile(name, sources, dump=True):
    exe=out/name; mir=out/(name+'.mir')
    cmd=[str(cc),'-o',str(exe)]
    if dump: cmd+=['--dump-machine-ir',str(mir)]
    subprocess.run([*cmd,*map(str,sources)],check=True,capture_output=True)
    subprocess.run([str(exe)],check=True)
    return exe,mir
source=out/'debug.lowir'
source.write_text('''global @g : i64 [storage=thread_local] = 1
declare function @access() -> ptr [tls_for=@g]
function @init() -> void [role=init] {
block ^entry:
store i64 42, @g
return void
}
function @main() -> i64 [role=entry] !dbg("tls.cpp", 3, 1) {
block ^entry:
%p = call ptr @access() !dbg("tls.cpp", 4, 1)
%v = load i64 %p
%same = load i64 @g !dbg("tls.cpp", 6, 1)
%bad0 = cmp ne i64 %v, 42
%bad1 = cmp ne i64 %same, 42
%bad = binary or i64 %bad0, %bad1
return i64 %bad
}
''')
exe,mir=compile('debug',[source]); text=mir.read_text()
assert 'tls_addr r11, @access !dbg("tls.cpp", 6, 1)' in text
other,_=compile('without-view',[source],False)
assert exe.read_bytes()==other.read_bytes()
image=exe.read_bytes(); ph=struct.unpack_from('<Q',image,32)[0]
segments=[struct.unpack_from('<IIQQQQQQ',image,ph+56*k) for k in range(2)]
assert [s[1] for s in segments]==[5,6]
# Initial thread pointer is self-referential; accessors use FS:0 and signed
# thread offsets. Check these actual native facts without pinning register IDs.
data=segments[1]; cells=image[data[2]:data[2]+data[5]]
assert struct.unpack_from('<Q',cells,16)[0]==data[3]+16
assert b'\x0f\x05' in image[176:segments[0][5]]
assert b'\x64\x48\x8b\x04\x25\x00\x00\x00\x00' in image
a=out/'a.lowir'; b=out/'b.lowir'
a.write_text('declare function @get() -> ptr [tls_for=@g]\n'
 'function @main() -> i64 [role=entry] { block ^entry: %p = call ptr @get() '
 '%v = load i64 %p %bad = cmp ne i64 %v, 73 return i64 %bad }\n')
b.write_text('global @g : i64 [storage=thread_local] = 73\n')
compile('multifile',[a,b])
helper=out/'helper.lowir'; helper.write_text('global @g : i64 [storage=thread_local] = 4\n'
 'function @helper() -> ptr { block ^entry: %p = addr @g return ptr %p }\n')
helpermir=out/'helper.mir'
subprocess.run([str(cc),'--dump-machine-ir',str(helpermir),str(helper)],check=True)
assert 'startup' not in helpermir.read_text() and 'tls_addr' in helpermir.read_text()
rows=[]
for name in ('600-thread-local-direct-native-runtime','600-defined-and-declared-tls-wrappers',
             '700-thread-local-store-register-pressure'):
    lane='strict' if name.startswith('600-thread-local') else 'behavior'
    src=root/'pa24/tests'/lane/(name+'.t')
    # These programs intentionally return a nonzero success result.
    paths=[]
    for view in (False,True):
        target=out/(name+str(view)); cmd=[str(cc),'-o',str(target)]
        if view: cmd+=['--dump-machine-ir',str(target)+'.mir']
        subprocess.run([*cmd,str(src)],check=True); paths.append(target)
    assert paths[0].read_bytes()==paths[1].read_bytes()
    rows.append({'input':str(src),'sha256':hashlib.sha256(paths[0].read_bytes()).hexdigest()})
(out/'identities.json').write_text(json.dumps(rows,indent=2)+'\n')
print('TLS debug, ELF, startup initialization, multifile, helper-only and four view-identity checks pass')
