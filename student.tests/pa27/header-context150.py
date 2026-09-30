#!/usr/bin/env python3
"""Header probe context follows #if/#elif through every macro expansion layer."""
import hashlib, json, pathlib, subprocess, sys
root = pathlib.Path(__file__).resolve().parents[2]
out = pathlib.Path(sys.argv[1]).resolve()
out.mkdir(parents=True,exist_ok=True)
compiler = root/'dev/cppgm++'
cases = {
    'conditional': (True, '''
#if !defined __has_include || !defined __has_include_next
#error missing operators
#endif
#define ID(x) x
#define PROBE(x) __has_include(x)
#if !ID(PROBE(__FILE__))
#error condition argument lost its context
#endif
#if 0
#elif PROBE("absent-audit150.h")
#error missing file exists
#elif !__has_include(__FILE__)
#error elif lost its context
#endif
int main() { return 0; }
'''),
    'direct': (False, 'int n=__has_include("absent-audit150.h");'),
    'replacement': (False, '#define HAS(x) __has_include(x)\nint n=HAS("absent-audit150.h");'),
    'argument': (False, '#define ID(x) x\nint n=ID(__has_include("absent-audit150.h"));'),
    'next-direct': (False, 'int n=__has_include_next("absent-audit150.h");'),
    'next-argument': (False, '#define ID(x) x\nint n=ID(__has_include_next("absent-audit150.h"));'),
    'line-directive': (False, '#line __has_include("absent-audit150.h")\nint n;'),
}
records = []
for name, (okay, body) in cases.items():
    source = out/(name+'.cpp'); source.write_text(body+'\n')
    for binary in (compiler,'g++'):
        # GCC also permits #line, beyond its documented #if/#elif contract.
        # Preserve that observation; host agreement is not the rule's proof.
        expected = okay or (binary == 'g++' and name == 'line-directive')
        args = list(map(str,[binary,'-std=c++11','-c',source,'-o',out/'case.o']))
        p = subprocess.run(args,capture_output=True,text=True,timeout=30)
        records.append(dict(case=name,expected_success=expected,command=args,status=p.returncode,
                            stdout=p.stdout,stderr=p.stderr))
        (out/'controls.json').write_text(json.dumps(dict(binary_sha256=hashlib.sha256(compiler.read_bytes()).hexdigest(),
            cases=cases,commands=records),indent=2)+'\n')
        assert (p.returncode==0) == expected, records[-1]
print('Header context controls PASS:',len(records),'commands')
