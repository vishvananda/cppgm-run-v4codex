#!/usr/bin/env python3
"""Explicit source-EH controls through the supplied LowIR backend and host ABI."""
from pathlib import Path
import hashlib, json, subprocess, sys
ROOT = Path(__file__).resolve().parents[2]
CC, WORK = [Path(x).resolve() for x in sys.argv[1:]]
WORK.mkdir(parents=True, exist_ok=True)
cases = {
    'scalar_rethrow': 'int f(){try{throw 9;}catch(int){throw;}}int main(){try{f();}catch(int n){return n!=9;}return 1;}',
    'pointer_value': 'int n;int main(){try{throw &n;}catch(int*p){*p=7;}return n!=7;}',
    'pointer_function': 'int n;void f(){n=7;}int main(){try{throw f;}catch(void(*p)()){p();}return n!=7;}',
    'pointer_string': 'int main(){try{throw "word";}catch(const char*p){return *p!=\'w\';}return 1;}',
    'local_prefix': 'int trace;struct G{int n;G(int n):n(n){}~G(){trace=trace*10+n;}};void f(){G a(1);G b(2);throw 7;}int main(){try{f();}catch(int n){return n!=7||trace!=21;}return 1;}',
    'nested_miss': 'int trace;struct G{~G(){++trace;}};int main(){try{G g;try{throw 7;}catch(long){return 2;}}catch(int n){return n!=7||trace!=1;}return 1;}',
    'handler_escape': 'int trace;struct G{int n;G(int n):n(n){}~G(){trace=trace*10+n;}};void f(){G a(1);try{throw 7;}catch(int){G b(2);throw 8;}}int main(){try{f();}catch(int n){return n!=8||trace!=21;}return 1;}',
    'nested_handler_miss': 'int trace;struct G{~G(){++trace;}};void f(){throw 7L;}int main(){try{try{throw 1;}catch(...){G g;try{f();}catch(int){return 2;}}}catch(long n){return n!=7||trace!=1;}return 1;}',
    'template_capture_handler': 'template<class T>int f(T x){auto a=[&](){try{throw x;}catch(const T&n){return x+n;}};return a();}int main(){return f(4)!=8;}',
    'exception_object': 'int live;struct E{int n;E(int n):n(n){++live;}E(const E&e):n(e.n){++live;}~E(){--live;}};int main(){try{E e(7);throw e;}catch(const E&e){if(e.n!=7||live!=1)return 2;}return live;}',
    'constructor_prefix': 'int trace;struct G{int n;G(int n):n(n){if(n==3)throw n;}~G(){trace=trace*10+n;}};struct B{G a,b,c;B():a(1),b(2),c(3){}};int main(){try{B b;}catch(int n){return n!=3||trace!=21;}return 1;}',
    'conditional_throw': 'int live;struct E{E(){++live;}~E(){--live;}};int f(bool yes){E e;return yes?3:(throw 7);}int main(){if(f(true)!=3||live)return 1;try{f(false);}catch(int n){return n!=7||live;}return 2;}',
}
required = [
 '100-class-array-constructor-failure-cleanup', '100-source-nested-catch-miss-cleans-active-handler',
 '100-throw-class-template-move-constructor-definition', '200-constructor-early-return-cleanup-region',
 '200-constructor-unwind-shares-generated-suffix', '200-destructor-body-unwind-runs-base-destruction',
 '200-destructor-subobject-unwind-runs-later-base-destruction', '200-destructor-unwind-shares-generated-suffix',
 '200-source-handler-branch-call-cleans-outer-scope', '200-source-base-ref-catch',
 '200-throw-operand-temporary-retired-before-sibling-unwind', '400-handler-context-cleanup-continuation',
 '200-conditional-init-throw-does-not-clean-destination', '200-guarded-local-static-initializer-temporary-cleanup',
]
for name in required:
    cases[name] = (ROOT/'pa21/tests/general'/f'{name}.t').read_text()
rows = []
def run(cmd):
    try:
        p = subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=30)
        return dict(argv=list(map(str,cmd)),exit=p.returncode,stdout=p.stdout,stderr=p.stderr)
    except subprocess.TimeoutExpired:
        return dict(argv=list(map(str,cmd)),exit=124,stderr='timeout')
reference = run([ROOT/'dev/lowir2native-ref','-O0','-o',WORK/'reference-int',
                 ROOT/'pa21/tests/general/100-source-try-catch-int.ref'])
backend_limitation = reference['exit'] == 1 and reference['stderr'].strip() == 'ERROR: duplicate native object-symbol label'
for name, source in cases.items():
    src = WORK/(name+'.cpp'); src.write_text(source)
    ir = src.with_suffix('.lowir'); obj = src.with_suffix('.o'); exe = WORK/name
    commands = [run([CC,'--emit-lowir','-O0','--validate-lowir','-o',ir,src])]
    row = dict(name=name,source=source,commands=commands,runtimes={})
    if commands[-1]['exit'] == 0:
        row['lowir_sha256'] = hashlib.sha256(ir.read_bytes()).hexdigest()
        for mode, cmds in {
            'native': [[ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],[exe]],
            'host': [[ROOT/'dev/cppgm++-ref','-c','-O0','-o',obj,ir],['g++','-no-pie',obj,'-o',str(exe)+'-host'],[str(exe)+'-host']],
        }.items():
            results = []
            for cmd in cmds:
                results.append(run(cmd))
                if results[-1]['exit']: break
            row['runtimes'][mode] = results
    native = row['runtimes'].get('native',[])
    row['native_reference_limitation'] = bool(backend_limitation and native and len(native)==1 and
        native[0]['exit']==1 and native[0]['stderr']==reference['stderr'])
    row['passed'] = len(row['runtimes']) == 2 and row['runtimes']['host'][-1]['exit']==0 and (
        native[-1]['exit']==0 or row['native_reference_limitation'])
    rows.append(row)
    print(name,'PASS' if row['passed'] else 'FAIL',commands[-1]['stderr'].strip(),{k:v[-1]['exit'] for k,v in row['runtimes'].items()},flush=True)
    (WORK/'results.json').write_text(json.dumps(dict(compiler_sha256=hashlib.sha256(CC.read_bytes()).hexdigest(),
        native_reference_probe=reference,execution_requirement='Host runtime must pass every case; retain native failures reproduced on the checked-in reference',rows=rows),indent=2)+'\n')
sys.exit(0 if all(r['passed'] for r in rows) else 1)
