#!/usr/bin/env python3
"""Independent PA5 audit regressions; check syntax choices, not just acceptance."""
import pathlib
import subprocess
import sys
import tempfile

cases = [
    ('scoped-enum', 'struct item{}; enum class flag{item,next=sizeof(item)}; item value;',
     ['decl-specifier TT_IDENTIFIER:item', 'sizeof-expression\n        id-expression item']),
    ('while-scope', 'struct item{}; void f(){while(int item=0){} item value;}',
     ['condition-declaration', 'decl-specifier TT_IDENTIFIER:item']),
    ('branch-scope', 'struct item{}; void f(){if(true) int item=0; else item value;}',
     ['else', 'decl-specifier TT_IDENTIFIER:item']),
    ('do-body', 'struct item{}; void f(){do int item=0; while(false); item value;}',
     ['do-statement', 'decl-specifier TT_IDENTIFIER:item']),
    ('for-body', 'struct item{}; void f(){for(;;) int item=0; item value;}',
     ['for-statement', 'decl-specifier TT_IDENTIFIER:item']),
    ('comma-parameters', 'struct item{}; void f(int n), g(int item); item value;',
     ['identifier g', 'decl-specifier TT_IDENTIFIER:item']),
    ('nested-parameters', 'struct item{}; void f(int callback(int item)){item value;}',
     ['identifier callback', 'decl-specifier TT_IDENTIFIER:item']),
    ('parameter-default', 'struct item{}; void f(int item=1,int x=sizeof(item)); item value;',
     ['default-argument', 'id-expression item', 'decl-specifier TT_IDENTIFIER:item']),
    ('nested-prototype', 'struct item{}; void f(int callback(int item),item value);',
     ['identifier callback', 'decl-specifier TT_IDENTIFIER:item']),
    ('for-condition', 'void f(){for(;int x=1;){}}', ['condition-declaration']),
    ('braced-objects', 'int (*p)(int){}; int a[2]{1,2}; int *f(int x){return 0;}',
     ['braced-init-list', 'array-suffix', 'function-definition']),
    ('global-value', 'int T=0; void f(){::T<2;}',
     ['binary-expression OP_LT:<', 'id-expression ::T']),
    ('imported-name', 'namespace n{struct item{};} namespace m{using namespace n;}'
     'using m::item; item value;', ['target m::item', 'decl-specifier TT_IDENTIFIER:item']),
    ('alias-list', 'namespace n{struct cell{typedef int word;};}'
     'typedef n::cell first,second; void f(){second::word value;}',
     ['decl-specifier second::word']),
    ('member-scope', 'struct C{typedef int item; int f(item); C(item);};'
     'int C::f(item p){return p;} C::C(item p){}',
     ['function-definition', 'special-member-definition C::C']),
    ('returned-function', 'int (*f(int T))(int){return T<2;}',
     ['function-definition', 'binary-expression OP_LT:<']),
    ('unnamed-reopening', 'namespace {struct item{};} namespace {item value;} item outside;',
     ['identifier value', 'identifier outside']),
    ('qualifier-category', 'namespace target{typedef int item;} namespace alias=target;'
     'void f(){int alias; alias::item value;}', ['decl-specifier alias::item']),
    ('unnamed-pack', 'template<int...> struct pack {};',
     ['non-type-template-parameter', 'parameter-pack ...']),
]


def check(compiler):
    with tempfile.TemporaryDirectory(prefix='pa5-audit-regressions-') as tmp:
        root = pathlib.Path(tmp)
        for name, source, fragments in cases:
            src, out = root/(name+'.cpp'), root/'tree.ast'
            src.write_text(source)
            run = subprocess.run([compiler, '--emit-ast', '-o', out, src],
                                 capture_output=True, text=True, timeout=15)
            assert run.returncode == 0, (name, run.stderr)
            assert 'Sanitizer' not in run.stderr and 'runtime error:' not in run.stderr, run.stderr
            tree = out.read_text()
            for fragment in fragments:
                assert fragment in tree, (name, fragment, tree)
            if name == 'braced-objects':
                assert tree.count('function-definition') == 1, tree
                assert tree.count('braced-init-list') == 2, tree
    print(f'PA5 final audit: {len(cases)} ownership and structured-syntax regressions pass')


if __name__ == '__main__':
    check(pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else 'dev/cppgm++').resolve())
