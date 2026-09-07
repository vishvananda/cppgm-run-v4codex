#!/usr/bin/env python3
"""Independent typed-tree oracle and boundary/stress checks; run explicitly."""
import argparse
import json
import os
from pathlib import Path
import random
import subprocess

ROOT = Path(__file__).resolve().parents[2]
SOURCES = [ROOT / ('dev/src/' + name + '.cpp') for name in (
    'preprocess/source', 'preprocess/identifier_table', 'preprocess/token_cursor',
    'posttoken/token_types', 'posttoken/number', 'posttoken/literal',
    'preprocess/expression_value', 'preprocess/expression')]
MASK = 2**64 - 1
SIGN = 2**63
BOOL_OPS = ('<', '>', '<=', '>=', '==', '!=', '&&', '||')
BINARY_OPS = ('+', '-', '*', '/', '%', '<<', '>>', '&', '|', '^', *BOOL_OPS)


def unsigned(tree):
    op, *args = tree
    if op == 'leaf':
        return args[2]
    if op == '?:':
        return unsigned(args[1]) or unsigned(args[2])
    if op in BOOL_OPS or op == '!':
        return False
    if len(args) == 1 or op in ('<<', '>>'):
        return unsigned(args[0])
    return any(map(unsigned, args))


def convert(value, uns):
    value %= 2**64
    return value if uns or value < SIGN else value - 2**64


def evaluate(tree):
    """Evaluate a separate AST lazily using Python mathematical integers."""
    op, *args = tree
    if op == 'leaf':
        return args[1]
    a = evaluate(args[0])
    if op == '?:':
        result = evaluate(args[1] if a else args[2])
    elif len(args) == 1:
        result = {'+': lambda: a, '-': lambda: -a, '~': lambda: ~a, '!': lambda: int(not a)}[op]()
    elif op == '&&':
        result = int(bool(a) and bool(evaluate(args[1])))
    elif op == '||':
        result = int(bool(a) or bool(evaluate(args[1])))
    else:
        b = evaluate(args[1])
        if op in ('<<', '>>'):
            if not 0 <= b < 64:
                raise ArithmeticError()
            result = a << b if op == '<<' else a >> b
        else:
            uns = unsigned(args[0]) or unsigned(args[1])
            a, b = convert(a, uns), convert(b, uns)
            if op in ('/', '%'):
                if b == 0 or (not uns and a == -SIGN and b == -1):
                    raise ArithmeticError()
                q = abs(a) // abs(b) * (-1 if (a < 0) != (b < 0) else 1)
                result = q if op == '/' else a - q * b
            else:
                result = {'+': lambda: a + b, '-': lambda: a - b, '*': lambda: a * b,
                          '&': lambda: a & b, '|': lambda: a | b, '^': lambda: a ^ b,
                          '<': lambda: a < b, '>': lambda: a > b, '<=': lambda: a <= b,
                          '>=': lambda: a >= b, '==': lambda: a == b, '!=': lambda: a != b}[op]()
    return convert(int(result), unsigned(tree))


def render(tree):
    op, *args = tree
    if op == 'leaf':
        return args[0]
    if op == '?:':
        return '(' + render(args[0]) + ' ? ' + render(args[1]) + ' : ' + render(args[2]) + ')'
    if len(args) == 1:
        return '(' + op + ' ' + render(args[0]) + ')'
    return '(' + render(args[0]) + ' ' + op + ' ' + render(args[1]) + ')'


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--sanitize', action='store_true')
    parser.add_argument('--tool', type=Path, default=ROOT / 'dev/ppexpr')
    args = parser.parse_args()
    out = ROOT / 'obj/student-pa3'
    out.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c++11', '-Wall', '-Wextra', '-pedantic', '-I' + str(ROOT / 'dev/src')]
    flags += (['-O1', '-g', '-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-fno-pie', '-no-pie']
              if args.sanitize else ['-O2'])
    cxx = os.environ.get('CXX', 'g++')
    api = out / ('api-sanitized' if args.sanitize else 'api')
    subprocess.run([cxx, *flags, str(Path(__file__).with_name('api.cpp')),
                    *map(str, SOURCES), '-o', str(api)], check=True)
    subprocess.run([str(api)], check=True)
    tool = args.tool.resolve()
    if args.sanitize:
        tool = out / 'ppexpr-sanitized'
        subprocess.run([cxx, *flags, str(ROOT / 'dev/ppexpr.cpp'),
                        *map(str, SOURCES), '-o', str(tool)], check=True)
    count, expressions = 0, 0

    def check(source, expected=(), failure=False, stats=False):
        nonlocal count, expressions
        if isinstance(source, str):
            source = source.encode()
        p = subprocess.run([str(tool), *(['--stats'] if stats else [])], input=source,
                           capture_output=True, timeout=60)
        count += 1
        assert b'Sanitizer' not in p.stderr and b'runtime error:' not in p.stderr, p.stderr
        assert p.returncode == int(failure), (source[:200], p.returncode, p.stderr)
        if failure:
            assert b'eof\n' not in p.stdout
        else:
            actual = p.stdout.decode().splitlines()
            wanted = list(expected) + ['eof']
            if actual != wanted:
                bad = next((i for i, (a, b) in enumerate(zip(actual, wanted)) if a != b), 0)
                raise AssertionError((bad, source.splitlines()[bad][:500], actual[bad:bad+3], wanted[bad:bad+3]))
            expressions += len(expected)
        return json.loads(p.stderr) if stats else None

    check('')
    check(' /*a\nb*/\n\t// tail')
    check('1 + \\\n2\n4/* embedded\nnewline */+5\n6??/\n+7', ['3', '9', '13'])
    check('de\\\nfined (abc)\ntrue\r\nfalse', ['1', '1', '0'])
    check('defined é\ndefined π\ndefined(defined)\ndefined(true)\ndefined(false)\nnullptr\nsizeof',
          ['1', '1', '0', '0', '0', '0', '0'])
    check('0x80000000\n0xffffffff\n2147483648\n0xffffffffffffffff\nu\'a\' << 1\nL\'a\' << 1u',
          ['2147483648u', '4294967295u', '2147483648', '18446744073709551615u', '194u', '194'])
    check('1?0?2u:3:4\n0?2:1?3:4u\n1?2:0?3u:4\n(1?2:0)?3:4u\n'
          '1?2:3?4:5\n1 || 0 ? 4 : 8\n1 | 2 ^ 3 & 4 == 4 < 5 << 1 + 1 * 2',
          ['3u', '3u', '2u', '3u', '2', '4', '3'])
    for bad in ('1.0', '1_tag', "'a'_tag", '"x"', 'R"(x)"', '1uu', '09', '0x', '18446744073709551616u',
                '9223372036854775808', '()', 'defined', 'defined()', 'defined(1)', 'defined((a))',
                'defined(a+b)', 'defined and', 'defined(not)', 'sizeof(1)', '1,2', '1=2', '1++',
                '1--2', '1[0]', '1 2', '1?', '1?2:', '1?:3', '(1?2):3', '1?(2:3)', '1:2',
                '1 ? 2 : 3 : 4', '(1)(2)', '1 + ()', '(1', '1)', '@', '#'):
        check('\n'.join([bad, '7', '0 && (' + bad + ')', '7', '1 ? 7 : (' + bad + ')', '7']),
              ['error', '7', 'error', '7', 'error', '7'])
    for bad in ('1/0', '1%0', '1<<64', '1>>-1', '1>>18446744073709551615u', '(1<<63)/-1', '(1<<63)%-1'):
        check('\n'.join([bad, '1 || (' + bad + ')', '0 && (' + bad + ')',
                         '1 ? -7 : (' + bad + ')', '0 ? (' + bad + ') : -7',
                         '(' + bad + ') ? 0 : 0']), ['error', '1', '0', '-7', '-7', 'error'])
    for bad in (b'\xff', b'"bad\n', b'"\\x"', b'"\\q"', b'/*', b'R"no(', b'"\\uD800"', b"''"):
        check(b'1 + ) ' + bad, failure=True)  # lexical failure survives syntax rejection

    leaves = [('leaf', str(v) + ('u' if u else ''), v, u)
              for u in (False, True) for v in (0, 1, 2, 31, 63, 64, 2**31, 2**32-1, SIGN-1)]
    leaves += [('leaf', str(MASK) + 'u', MASK, True), ('leaf', "u'π'", 960, True),
               ('leaf', 'true', 1, False), ('leaf', 'unknown', 0, False)]
    rng = random.Random(3052026)

    def tree(depth):
        if not depth or rng.random() < .25:
            return rng.choice(leaves)
        op = rng.choice((*BINARY_OPS, '?:', 'unary'))
        if op == 'unary':
            return (rng.choice(('+', '-', '~', '!')), tree(depth-1))
        return (op, *(tree(depth-1) for _ in range(3 if op == '?:' else 2)))

    trees = [tree(5) for _ in range(6000)]
    # Exhaust all signed/unsigned boundary pairs for every binary operation.
    for op in BINARY_OPS:
        for a in leaves:
            for b in leaves:
                trees.append((op, ('-', a), b))
    expected = []
    for t in trees:
        try:
            expected.append(str(evaluate(t)) + ('u' if unsigned(t) else ''))
        except ArithmeticError:
            expected.append('error')
    check('\n'.join(map(render, trees)), expected)

    depth = 200000
    check('('*depth + '7' + ')'*depth, ['7'])
    check('! '*depth + '7', ['1'])
    check('0?1u:'*depth + '-7', [str(2**64-7) + 'u'])
    check('1?'*depth + '7' + ':0u'*depth, ['7u'])
    check('('*depth + '1\n7', ['error', '7'])
    stats = check('1' + '+1'*depth, [str(depth+1)], stats=True)
    assert stats['max_values'] == 2 and stats['max_operators'] == 1
    assert stats['reductions'] == depth and stats['expression_storage_bytes'] < 128
    print(f'PA3: {count} personal invocations, {expressions} expression results passed')


if __name__ == '__main__':
    main()
