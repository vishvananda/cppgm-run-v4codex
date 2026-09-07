#!/usr/bin/env python3
"""Independent PA2 boundary/property oracles; run explicitly from the root."""
import argparse
import itertools
import json
import os
from pathlib import Path
import random
import struct
import subprocess

ROOT = Path(__file__).resolve().parents[2]
SOURCES = [ROOT / ('dev/src/preprocess/' + n + '.cpp')
           for n in ('source', 'identifier_table', 'token_cursor')]
SOURCES += sorted((ROOT / 'dev/src/posttoken').glob('*.cpp'))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--sanitize', action='store_true')
    parser.add_argument('--tool', type=Path, default=ROOT / 'dev/posttoken')
    args = parser.parse_args()
    out = ROOT / 'obj/student-pa2'
    out.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c++11', '-Wall', '-Wextra', '-pedantic', '-I' + str(ROOT / 'dev/src')]
    flags += (['-O1', '-g', '-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-fno-pie', '-no-pie']
              if args.sanitize else ['-O2'])
    cxx = os.environ.get('CXX', 'g++')
    api = out / ('cursor-sanitized' if args.sanitize else 'cursor')
    subprocess.run([cxx, *flags, str(Path(__file__).with_name('cursor.cpp')),
                    *map(str, SOURCES), '-o', str(api)], check=True)
    subprocess.run([str(api)], check=True)
    tool = args.tool.resolve()
    if args.sanitize:
        tool = out / 'posttoken-sanitized'
        subprocess.run([cxx, *flags, str(ROOT / 'dev/posttoken.cpp'),
                        *map(str, SOURCES), '-o', str(tool)], check=True)
    count = 0

    def check(source, lines=None, failure=False, options=()):
        nonlocal count
        if isinstance(source, str):
            source = source.encode()
        p = subprocess.run([str(tool), *options], input=source, capture_output=True, timeout=30)
        count += 1
        assert b'Sanitizer' not in p.stderr and b'runtime error:' not in p.stderr, p.stderr
        if failure:
            assert p.returncode == 1, (source[:200], p.returncode, p.stderr)
            return
        assert p.returncode == 0, (source[:200], p.stderr)
        if lines is not None:
            expected = ('\n'.join(lines) + '\neof\n').encode() if lines else b'eof\n'
            assert p.stdout == expected, (source[:200], p.stdout[:2000], expected[:2000])
        if options:
            return json.loads(p.stderr)

    check('', [])
    check(' \n/*nothing*/', [])
    check('# ## %: %:%: @', ['invalid ' + s for s in ('#', '##', '%:', '%:%:', '@')])
    check('#include <x>', ['invalid #', 'identifier include', 'invalid <x>'])
    check('"a" \'\' "b"', ['literal "a" array of 2 char 6100', "invalid ''", 'literal "b" array of 2 char 6200'])
    stats = check("'ab' 'x'", ["invalid 'ab'", "literal 'x' char 78"], options=('--stats',))
    assert stats['decoded_elements'] == 3 and stats['invalid_tokens'] == 1
    for bad in (b'\xff', b'"bad\n', b'"\\x"', b'"\\q"', b'/*', b'R"no(', b'"\\uD800"'):
        check(bad, failure=True)

    # Independent candidate table, random magnitudes and all legal rank spellings.
    rng = random.Random(20260907)
    suffixes = ['', 'u', 'U', 'l', 'L', 'll', 'LL', 'ul', 'Lu', 'uLL', 'llU']
    values = [0, 1, *[2**b + d for b in (31, 32, 63, 64) for d in (-1, 0, 1)]]
    values += [rng.getrandbits(rng.randrange(1, 80)) for _ in range(200)]
    source, expected = [], []
    for value, base, suffix in itertools.product(values, (8, 10, 16), suffixes):
        digits = str(value) if base == 10 else '0' + format(value, 'o') if base == 8 else '0x' + format(value, 'X')
        text = digits + suffix
        source.append(text)
        rank = 2 if ('ll' in suffix or 'LL' in suffix) else 1 if ('l' in suffix or 'L' in suffix) else 0
        uns = 'u' in suffix.lower()
        candidates = []
        for r in range(rank, 3):
            for u in (False, True):
                if (uns and not u) or (not uns and u and base == 10):
                    continue
                bits = 32 if r == 0 else 64
                name = ('unsigned ' if u else '') + ('', 'long ', 'long long ')[r] + 'int'
                candidates.append((2**(bits - (not u)) - 1, bits, name))
        match = next(((bits, name) for limit, bits, name in candidates if value <= limit), None)
        expected.append('invalid ' + text if match is None else
                        'literal ' + text + ' ' + match[1] + ' ' + value.to_bytes(match[0] // 8, 'little').hex().upper())
    check(' '.join(source), expected)
    for text in ('09', '078_tag', '0x', '1f', '1lL', '1ulL', '1uu', '1e', '1e+', '1e-π',
                 '1.2.3', '1.._x', '1_tag.0', '0x1p2', '0b11', '1_ae+1', '1_é-', '123é'):
        if text == '1_é-':
            check(text, ['user-defined-literal 1_é _é integer 1', 'simple - OP_MINUS'])
        else:
            check(text, ['invalid ' + text])
    for prefix, category in (('9' * 20000, 'integer'), ('0x' + 'F' * 20000, 'integer'),
                             ('1.e+999999', 'floating'), ('.25', 'floating'), ('007', 'integer')):
        text = prefix + '_é3'
        check(text, ['user-defined-literal ' + text + ' _é3 ' + category + ' ' + prefix])
    check('9' * 20000, ['invalid ' + '9' * 20000])

    # Exactly representable decimal fractions, including the x87 ABI padding.
    for text, fmt, name in (('1.5', 'd', 'double'), ('.125f', 'f', 'float'), ('16e-1F', 'f', 'float'),
                            ('1.5L', None, 'long double')):
        raw = bytes.fromhex('00000000000000C0FF3F') + bytes(6) if fmt is None else struct.pack('<' + fmt, float(text.rstrip('fF')))
        check(text, ['literal ' + text + ' ' + name + ' ' + raw.hex().upper()])

    encodings = {'': ('char', 'utf-8', 1), 'u8': ('char', 'utf-8', 1),
                 'u': ('char16_t', 'utf-16le', 2), 'U': ('char32_t', 'utf-32le', 4), 'L': ('wchar_t', 'utf-32le', 4)}
    for prefix, (name, codec, width) in encodings.items():
        for cp in (0x7f, 0x80, 0x7ff, 0x800, 0xd7ff, 0xe000, 0xffff, 0x10000, 0x10ffff):
            text = prefix + '"' + chr(cp) + '"'
            data = (chr(cp) + '\0').encode(codec)
            check(text, ['literal ' + text + ' array of ' + str(len(data) // width) + ' ' + name + ' ' + data.hex().upper()])
        for value in (0, 127, 128, 255, 256, 0xd800, 65535, 65536, 0xffffffff, 0x100000000):
            text = prefix + '"\\x' + format(value, 'x') + '"'
            if value >= 2**(width * 8):
                check(text, ['invalid ' + text])
            else:
                data = value.to_bytes(width, 'little') + bytes(width)
                check(text, ['literal ' + text + ' array of 2 ' + name + ' ' + data.hex().upper()])
    for prefix in ('', 'u', 'U', 'L'):
        for cp in (0, 127, 128, 65535, 65536, 0x10ffff, 0xd800, 0xdfff, 0x110000, 0x100000000):
            text = prefix + "'\\x" + format(cp, 'x') + "'"
            valid = cp <= 0x10ffff and not 0xd800 <= cp <= 0xdfff and (prefix != 'u' or cp <= 65535)
            if valid:
                name = 'int' if not prefix and cp > 127 else encodings[prefix][0]
                width = 4 if name == 'int' else encodings[prefix][2]
                line = 'literal ' + text + ' ' + name + ' ' + cp.to_bytes(width, 'little').hex().upper()
            else:
                line = 'invalid ' + text
            check(text, [line])
    for a, b, c in itertools.product(encodings, repeat=3):
        parts = [p + '"é"' for p in (a, b, c)]
        text = ' '.join(parts)
        selected = {p for p in (a, b, c) if p}
        if len(selected) > 1:
            check(text, ['invalid ' + text])
        else:
            name, codec, width = encodings[next(iter(selected), '')]
            data = ('ééé\0').encode(codec)
            check(text, ['literal ' + text + ' array of ' + str(len(data) // width) + ' ' + name + ' ' + data.hex().upper()])
    check('"\\x3c0" u""', ['literal "\\x3c0" u"" array of 2 char16_t C0030000'])
    check('"\\x1" "F"', ['literal "\\x1" "F" array of 3 char 014600'])
    check('"\\1234"', ['literal "\\1234" array of 3 char 533400'])
    check('"\\\\u03C0"', ['literal "\\\\u03C0" array of 7 char 5C753033433000'])
    check('"\\u03C0"', ['literal "π" array of 3 char CF8000'])
    check('"x"_a "y"_b "z" "next";', ['invalid "x"_a "y"_b "z" "next"', 'simple ; OP_SEMICOLON'])
    check('"x"_π "y"_\\u03c0', ['user-defined-literal "x"_π "y"_π _π string array of 3 char 787900'])
    for suffix in ('sv', '_x', 'é'):
        check('operator/**/""' + suffix,
              ['simple operator KW_OPERATOR', 'literal "" array of 1 char 00', 'identifier ' + suffix])
    raw = 'R"1234567890abcdef(a\\\nb??/\\u03c0)1234567890abcdef"'
    data = b'a\\\nb??/\\u03c0\0'
    check(raw, ['literal ' + raw + ' array of ' + str(len(data)) + ' char ' + data.hex().upper()])

    # Maximal runs, late encoding, overflow with arbitrary leading zeros and
    # raw near-matches check linear work and recovery independent of fixtures.
    check('U"\\x' + '0' * 100000 + 'FFFFFFFF"',
          ['literal U"\\x' + '0' * 100000 + 'FFFFFFFF" array of 2 char32_t FFFFFFFF00000000'])
    check('U"\\x100000000' + '0' * 100000 + '"', ['invalid U"\\x100000000' + '0' * 100000 + '"'])
    run = '"a" ' * 50000 + 'u"𝄞"'
    stats = check(run, options=('--stats',))
    assert stats['string_parts'] == 50001 and stats['decoded_elements'] == 50001
    assert stats['encoded_bytes'] == 100006 and stats['post_tokens'] == 2
    assert stats['post_storage_growths'] < 100, stats
    assert stats['post_storage_bytes'] <= 32 * len(run.encode()), stats
    print(str(count) + ' personal cases passed, including ' + str(len(source)) + ' independent integer cases')


if __name__ == '__main__':
    main()
