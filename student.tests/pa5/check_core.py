#!/usr/bin/env python3
"""Independent PA5 parser properties; run explicitly from repository root."""
import pathlib
import subprocess
import tempfile

compiler = pathlib.Path('dev/cppgm++').resolve()
cases = [
    ('int f(int a,int b,int c){return a+b*c;}',
     ['binary-expression OP_PLUS:+\n          id-expression a\n          binary-expression OP_STAR:*']),
    ('int f(int a,int b){return a=b=3;}',
     ['assignment-expression OP_ASS:=\n          id-expression a\n          assignment-expression OP_ASS:=']),
    ('typedef int word; int f(word x){ word y=x; return y; }',
     ['decl-specifier TT_IDENTIFIER:word', 'return-statement\n        id-expression y']),
    ('template<class T> struct box {T value;}; box<int> x;',
     ['class-specifier box', 'decl-specifier box<int>', 'decl-specifier TT_IDENTIFIER:T']),
    ('int f(){if(true) return 3; else return 4;}', ['then\n', 'else\n']),
    ('#define DECL int value\nDECL = 3;\n', ['identifier value', 'literal 3']),
]
with tempfile.TemporaryDirectory(prefix='pa5-core-') as tmp:
    root = pathlib.Path(tmp)
    for index, (source, expected) in enumerate(cases):
        src, out = root / f'{index}.cpp', root / f'{index}.ast'
        src.write_text(source)
        subprocess.run([compiler, '--emit-ast', '-o', out, src], check=True)
        text = out.read_text()
        for fragment in expected:
            assert fragment in text, (index, fragment, text)
    a, b, out = root/'a.cpp', root/'b.cpp', root/'multi.ast'
    a.write_text('#define PRIVATE_MACRO 1\nint a=PRIVATE_MACRO;')
    b.write_text('int b=PRIVATE_MACRO;')
    subprocess.run([compiler, '--emit-ast', '-o', out, a, b], check=True)
    text = out.read_text()
    assert text.startswith('2 translation units\n')
    assert text.count('translation-unit\n') == 2
    assert 'literal 1' in text and 'id-expression PRIVATE_MACRO' in text
    for source in ('int f( {', 'int f(){co_return 1;}', 'int f(){return ;'):
        a.write_text(source)
        result = subprocess.run([compiler, '--emit-ast', '-o', out, a], capture_output=True)
        assert result.returncode == 1, (source, result)
print('PA5 core: 10 independent cases pass')
