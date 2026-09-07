#!/usr/bin/env python3
"""Explicit PA1 boundary/property checks. No course fixtures are modified."""
import argparse
import json
import os
from pathlib import Path
import random
import subprocess

ROOT = Path(__file__).resolve().parents[2]
SOURCES = [ROOT / ('dev/src/preprocess/' + name + '.cpp')
           for name in ('source', 'identifier_table', 'token_cursor', 'token_output')]


def decode(data):
    tokens = []
    offset = 0
    while data[offset:] != b'eof\n':
        space = data.index(b' ', offset)
        kind = data[offset:space].decode()
        end = data.index(b' ', space + 1)
        length = int(data[space + 1:end])
        start = end + 1
        assert data[start + length:start + length + 1] == b'\n', data
        tokens.append((kind, data[start:start + length]))
        offset = start + length + 1
    return tokens


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--sanitize', action='store_true')
    parser.add_argument('--tool', type=Path, default=ROOT / 'dev/pptoken')
    args = parser.parse_args()
    out = ROOT / 'obj/student-pa1'
    out.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c++11', '-Wall', '-Wextra', '-pedantic', '-I' + str(ROOT / 'dev/src')]
    flags += (['-O1', '-g', '-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-fno-pie', '-no-pie']
              if args.sanitize else ['-O2'])
    compiler = os.environ.get('CXX', 'g++')
    api = out / ('cursor-sanitized' if args.sanitize else 'cursor')
    subprocess.run([compiler, *flags, str(Path(__file__).with_name('cursor.cpp')),
                    *map(str, SOURCES), '-o', str(api)], check=True)
    subprocess.run([str(api)], check=True)
    tool = args.tool.resolve()
    if args.sanitize:
        tool = out / 'pptoken-sanitized'
        subprocess.run([compiler, *flags, str(ROOT / 'dev/pptoken.cpp'),
                        *map(str, SOURCES), '-o', str(tool)], check=True)

    count = 0

    def check(source, expected=None, failure=False):
        nonlocal count
        if isinstance(source, str):
            source = source.encode()
        p = subprocess.run([str(tool)], input=source, capture_output=True, timeout=15)
        count += 1
        if failure:
            assert p.returncode == 1 and p.stderr.startswith(b'ERROR: '), (source, p.returncode, p.stderr)
            assert b'runtime error:' not in p.stderr and b'Sanitizer' not in p.stderr, p.stderr
            return
        assert p.returncode == 0, (source, p.stderr)
        result = decode(p.stdout)
        if expected is not None:
            normalized = [(kind, text.encode() if isinstance(text, str) else text) for kind, text in expected]
            assert result == normalized, (source, result, normalized)
        return result

    nl = ('new-line', '')
    ws = ('whitespace-sequence', '')
    ident = lambda text: ('identifier', text)
    punct = lambda text: ('preprocessing-op-or-punc', text)
    string = lambda text: ('string-literal', text)
    for source in (b'', b'\xef\xbb\xbf'):
        check(source, [])
    check('a', [ident('a'), nl])
    check('a\n\\\n', [ident('a'), nl, nl])
    check('\n??/\n', [nl, nl])
    check('\\\n', [nl])
    check('a\\\nb\\\nc', [ident('abc'), nl])
    check('a\\\\\nb', [ident('a'), ('non-whitespace-character', '\\'), ident('b'), nl])
    check('//x\\\ny', [ws, nl])
    check('a /*\nx\n*/\t/**/ b', [ident('a'), ws, ident('b'), nl])
    check('??/u03C0 ?π', [ident('π'), ws, punct('?'), ident('π'), nl])
    check('\\u0300a\\u0300 \\U000EFFFD \\U000F0000',
          [('non-whitespace-character', '̀'), ident('à'), ws, ident('\U000efffd'),
           ws, ('non-whitespace-character', '\U000f0000'), nl])
    check('1p+3 1e+3 ..4 ...4', [('pp-number', '1p'), punct('+'), ('pp-number', '3'), ws,
          ('pp-number', '1e+3'), ws, punct('.'), ('pp-number', '.4'), ws, punct('...'), ('pp-number', '4'), nl])
    check('<::x <::: <::> %:%:', [punct('<'), punct('::'), ident('x'), ws, punct('<:'), punct('::'), ws,
          punct('<:'), punct(':>'), ws, punct('%:%:'), nl])
    for text in (r'"\\u03C0"', r'"\\UFFFFFFFF"', r'"\04y\xabcg\08"'):
        check(text, [string(text), nl])
    raw = 'u8R"tag(\\UFFFFFFFF??/\n\\\n/*x*/)tag"'
    check(raw, [string(raw), nl])
    check(raw + '\\u03C0', [('user-defined-string-literal', raw + 'π'), nl])
    # Raw delimiters use physical punctuation, including quotes and trigraphs.
    for delimiter in ('', 'abcdefghijklmnop', '?', '"', '??/'):
        text = 'R"' + delimiter + '(text)other"partial)' + delimiter + '"'
        check(text, [string(text), nl])
    check('R\\\n"(x)"', [string('R"(x)"'), nl])
    check('%:/**/inc??/\nlude /*x*/ <a//b> "c"', [punct('%:'), ws, ident('include'), ws,
          ('header-name', '<a//b>'), ws, string('"c"'), nl])
    check('x #include <a>', [ident('x'), ws, punct('#'), ident('include'), ws, punct('<'), ident('a'), punct('>'), nl])
    check('template<class T> struct Box { T value; };\nBox<::Tag> bo\\\nx;',
          [ident('template'), punct('<'), ident('class'), ws, ident('T'), punct('>'), ws,
           ident('struct'), ws, ident('Box'), ws, punct('{'), ws, ident('T'), ws,
           ident('value'), punct(';'), ws, punct('}'), punct(';'), nl,
           ident('Box'), punct('<'), punct('::'), ident('Tag'), punct('>'), ws,
           ident('box'), punct(';'), nl])
    # Exercise the memory/work owners at sizes beyond the course fixtures.
    long_name = 'a' * (2 * 1024 * 1024)
    check(long_name + '\\\nx', [ident(long_name + 'x'), nl])
    raw = 'R"abcdefghijklmnop(' + ')abcdefghijklmnoQ' * 131072 + ')abcdefghijklmnop"'
    check(raw, [string(raw), nl])
    check('/*' + '*' * (2 * 1024 * 1024) + '/x', [ws, ident('x'), nl])
    check('a' + '\\\n' * 1048576 + 'b', [ident('ab'), nl])
    for bad in (b'\x80', b'\xc0\x80', b'\xc2', b'\xe2\x28\xa1', b'\xed\xa0\x80',
                b'\xf0\x80\x80\x80', b'\xf4\x90\x80\x80', b'\xff', b'\xfe'):
        check(bad, failure=True)
        check(b'/*' + bad + b'*/', failure=True)
    for bad in ('\\uD800', '\\U00110000', '\\UFFFFFFFF', '/*', '"abc', "''", "'a\n'",
                r'"\xg"', r'"\8"', 'R"a b(x)a b"', 'R"12345678901234567()12345678901234567"',
                'R"tag(x)ta"', 'R"é(x)é"'):
        check(bad, failure=True)

    # Independent maximal-munch oracle: whitespace separates independently
    # specified tokens. Physical rewrites preserve each expected logical token.
    rng = random.Random(1701)
    atoms = [ident('alpha'), ident('_x9'), ident('π'), ('pp-number', '1.E+2'),
             punct('->*'), punct('<<='), punct('and_eq'), string(r'"a\tb"'), string('R"(x??=)"')]
    expected = []
    physical = []
    for i in range(3000):
        kind, text = rng.choice(atoms)
        encoded = text
        if kind == 'identifier':
            encoded = encoded.replace('π', '\\u03C0')
            if encoded == 'alpha' and rng.randrange(2):
                encoded = 'al??/\npha'
        physical.append(encoded)
        expected.append((kind, text))
        if i != 2999:
            physical.append(rng.choice([' ', '\t', '/**/', ' /*multi\nline*/ ']))
            expected.append(ws)
    expected.append(nl)
    check(''.join(physical), expected)

    # Instrumentation must be separable from language behavior.
    source = b'alpha al\\\npha R"(raw)"_s\n'
    plain = subprocess.run([str(tool)], input=source, capture_output=True, check=True)
    measured = subprocess.run([str(tool), '--stats'], input=source, capture_output=True, check=True)
    assert plain.stdout == measured.stdout
    stats = json.loads(measured.stderr)
    assert stats['identifiers'] == 2 and stats['spelling_bytes'] == 5
    print(f'{count} PA1 boundary/property cases and telemetry equivalence: passed')


if __name__ == '__main__':
    main()
