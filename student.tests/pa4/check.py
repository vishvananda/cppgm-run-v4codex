#!/usr/bin/env python3
"""Explicit PA4 semantic tests; course tests and oracles remain untouched."""
import json
import os
from pathlib import Path
import random
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
APP = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else ROOT / 'dev/preproc'
POST = ROOT / 'dev/posttoken'
count = 0

with tempfile.TemporaryDirectory(prefix='pa4-personal-') as temporary:
    directory = Path(temporary)
    source, output = directory / 'input.cc', directory / 'output'

    def check(text, expected=None, reject=False, stats=False):
        global count
        source.write_text(text)
        run = subprocess.run([str(APP)] + (['--stats'] if stats else []) +
                             ['-o', str(output), str(source)], capture_output=True, timeout=30)
        count += 1
        if reject:
            assert run.returncode == 1, (text[:500], run.returncode, run.stderr)
            return
        assert run.returncode == 0, (text[:500], run.returncode, run.stderr)
        if expected is not None:
            oracle = subprocess.run([str(POST)], input=expected.encode(), capture_output=True, check=True)
            actual = b'\n'.join(output.read_bytes().splitlines()[2:]) + b'\n'
            assert actual == oracle.stdout, (text[:500], actual[:2000], oracle.stdout[:2000])
        if stats:
            return json.loads(run.stderr)

    for text, expected in [
        ('#define A 2\n#define B A + A\nB\n#undef A\nB\n', '2 + 2 A + A'),
        ('#define X 8\n#define STR(x) #x\nSTR(X /*x*/ +\n X)\n', '"X + X"'),
        ('#define X 8\n#define ID(x) x\n#define STR(x) #x\n#define S(x) STR(x)\nS(ID(X))', '"8"'),
        ('#define CAT(a,b) a ## b\n#define xy 19\nCAT(x,y) CAT(,xy) CAT(xy,) CAT(,)\n', '19 19 19'),
        ('#define F(x) x+x\nF((a,b))', '(a,b)+(a,b)'),
        ('#define F() 9\nF\n#define Q 1\n() F()', 'F () 9'),
        ('#define F() 9\n#define OPEN (\nF OPEN )', 'F ( )'),
        ('#define f(x) 1 g(x)\n#define g(x) 2 f(x)\nf(f(z))', '1 2 f(1 2 f(z))'),
        ('#define z z[0]\n#define f(x) x x\nf(z)', 'z[0] z[0]'),
        ('#define f(x) 1 x\n#define g(x) 2 x\ng(f)(g)(3)', '2 1 g(3)'),
        ('#define D(x) x x x\nD(__COUNTER__) __COUNTER__', '0 0 0 1'),
        ('#define S(x) #x\nS(__COUNTER__) __COUNTER__', '"__COUNTER__" 0'),
        ('#define UNUSED(x) ok\n#define TWO(a,b) a\nUNUSED(TWO(1))', 'ok'),
        ('#define M 1\n#if defined(M) && !defined(Z) && (1 || 1/0)\nyes\n#else\n#error wrong\n#endif', 'yes'),
        ('#define D defined\n#define X 1\n#if D(X) && (1 ? 3 : 1/0)\nyes\n#endif', 'yes'),
        ('#if 0\n#bad @\n#if malformed tokens\n#else @\n#endif @\n#else\nyes\n#endif', 'yes'),
        ('#define V(a,...) a __VA_ARGS__\nV(1,2,3)', '1 2,3'),
        ('#define C(...) foo(1, ##__VA_ARGS__)\nC() C(a,b)', 'foo(1) foo(1,a,b)'),
        ('#define L __LINE__\nL\n#line 80 "virtual"\nL __FILE__', '2 80 "virtual"'),
        ('#define P _Pragma("cppgm_mock_unknown")\n"a" P "b"', '"a" "b"'),
        ('#define \u03b1 42\n\u03b1', '42'),
    ]:
        check(text, expected)

    check('#define S(x) #x\nS(R"(??=)")')
    assert '5222283F3F3D292200' in output.read_text()

    for text in [
        '#define', '#define F(', '#define F(a,)', '#define F(a,a) a',
        '#define F(...,x) x', '#define F(a,...,x) x', '#define F(__VA_ARGS__) x',
        '#define __VA_ARGS__ x', '#define F __VA_ARGS__', '__VA_ARGS__',
        '#define F(a) #b', '#define F ## a', '#define F a ##', '#undef F 2',
        '#define F 1\n#define F 2', '#define F(x) x\n#define F(y) y',
        '#define F(x) x\nF(1,2)', '#define F(x) x\nF(',
        '#define P(a,b) a##b\nP(+,*)', '#define P(a,b) a##b\nP(/,*)',
        '#if 1/0\n#endif', '#if 1\n', '#else', '#endif',
        '#if 0\n#else\n#elif 1\n#endif', '#if 1\n#else extra\n#endif',
        '#if 1\n#endif extra', '#include "absent.h"', '#error fail', '#unknown',
        '_Pragma', '_Pragma(3)', '@', '0xG',
    ]:
        check(text, reject=True)

    # Stateful inclusion uses physical identity for aliases and resets per TU.
    header = directory / 'header.h'
    alias = directory / 'alias.h'
    header.write_text('#pragma once\n#define FROM_HEADER 27\nFROM_HEADER\n')
    os.link(header, alias)
    text = '#include "header.h"\n#include "alias.h"\nFROM_HEADER\n'
    check(text, '27 27')
    companion = directory / 'second.cc'
    companion.write_text(text)
    run = subprocess.run([str(APP), '-o', str(output), str(source), str(companion)], capture_output=True)
    assert run.returncode == 0, run.stderr
    assert output.read_text().count('literal 27 int 1B000000') == 4
    count += 1
    header.write_text('#if 1\n')
    check('#if 1\n#include "header.h"\n#endif', reject=True)
    header.write_text('from_header\n')
    check('#line 40 "' + str(directory / 'virtual.cc') + '"\n#include "header.h"', 'from_header')

    rng = random.Random(4204)
    for _ in range(100):
        values = [rng.randrange(1000) for _ in range(10)]
        definitions = ''.join('#define M%d %d\n' % (i, n) for i, n in enumerate(values))
        definitions += '#define CAT(a,b) a##b\n#define DUP(x) x x\n'
        chosen = [rng.randrange(10) for _ in range(30)]
        check(definitions + ' '.join('DUP(CAT(M,%d))' % i for i in chosen),
              ' '.join('%d %d' % (values[i], values[i]) for i in chosen))

    depth = 16000
    chain = '#define F0() done\n' + ''.join('#define F%d() F%d()\n' % (i, i-1) for i in range(1, depth+1))
    check(chain + 'F%d()' % depth, 'done')
    stats = check(chain + '#define D(x) ' + 'x '*64 + '\nD(F%d())' % depth,
                  'done '*64, stats=True)
    assert stats['invocations'] == depth + 2, stats
    assert stats['argument_prescans'] == 1, stats
    check('#define I(x) x\n' + 'I('*2000 + '42' + ')'*2000, '42')

print('PA4 personal checks passed:', count)
