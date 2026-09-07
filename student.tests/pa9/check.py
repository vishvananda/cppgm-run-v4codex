#!/usr/bin/env python3
"""Independent exact contract comparison, API roundtrips and semantic probes."""
import argparse
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
parser = argparse.ArgumentParser()
parser.add_argument('--api', default='/tmp/pa9-evidence/check-api')
parser.add_argument('--tool', default=str(ROOT / 'dev/abimangle'))
a = parser.parse_args()
subprocess.run([a.api], check=True)
count = 0
for test in sorted((ROOT / 'pa9/tests').rglob('*.t')):
    success = test.with_suffix('.ref.exit_status').read_text().strip() == 'EXIT_SUCCESS'
    result = subprocess.run([a.api, str(test)], capture_output=True)
    assert (result.returncode == 0) == success, (test, result.stderr.decode())
    if success:
        assert result.stdout == test.with_suffix('.ref').read_bytes(), (test, result.stdout)
    count += 1

valid = [
    ('variable lone\n', 'lone\n'),
    ('type template-param 4294967296\n', 'T4294967295_\n'),
    ('let-arg I type int\nlet-type Box template ns::Box I\nlet-type Tagged tagged Box z a\nfunction f Tagged Tagged\n', '_Z1fN2ns3BoxB1aB1zIiEES1_\n'),
    ('let-type C named:C\nlet-type Tagged tagged C tag\nfunction f C Tagged Tagged\n', '_Z1f1C1CB3tagS0_\n'),
    ('function ping\nparam int\n', '_Z4pingi\n'),
    ('function f named:One named:One named:Two\n', '_Z1f3OneS_3Two\n'),
    ('let-arg A value uint -1\nlet-arg B value uint 4294967295\nlet-type X template Box A\nlet-type Y template Box B\nfunction f X Y\n', '_Z1f3BoxILj4294967295EES0_\n'),
    ('function encoding\nname-source C\nname-source operator\noperator-terminal minus\n', '_ZN1CngEv\n'),
    ('let-expr X function-param 0\nlet-expr Y unary de X\nlet-type T decltype Y\nfunction f T T\n', '_Z1fDTdefp_ES_\n'),
    ('let-context Host function path outer int\nlet-type L local-type Host Worker 2\ntype L\n', 'Z5outeriE6Worker_1\n'),
    ('type ' + 'ptr:' * 20000 + 'int\n', 'P' * 20000 + 'i\n'),
]
invalid = [
    '', 'type unknown:int\n', 'type ns::\n', 'type template-param -2\n', 'type template-param 1x\n',
 'type template-param 18446744073709551616\n',
    'let-type T int\nlet-arg T type int\ntype T\n',
    'let-arg A type int\ntype A\n', 'let-type C int\n',
    'type int\nabi-tag bad\n', 'function f\nqualifier lvalue-ref rvalue-ref\n',
    'let-arg A expression absent\ntype template T A\n',
    'type array:-1:int\n', 'type memberptr:C\n',
    'function f\noperator-terminal nonexistent\n',
]
with tempfile.TemporaryDirectory(prefix='pa9-personal-') as tmp:
    p, output = Path(tmp) / 'input.t', Path(tmp) / 'names'
    for text, expected in valid:
        p.write_text(text)
        result = subprocess.run([a.tool, '--stats', '-o', str(output), str(p)], capture_output=True)
        assert result.returncode == 0, result.stderr.decode()
        assert output.read_text() == expected, (text, output.read_text(), expected)
        roundtrip = subprocess.run([a.api, str(p)], capture_output=True)
        assert roundtrip.returncode == 0 and roundtrip.stdout.decode() == expected, roundtrip.stderr.decode()
    for text in invalid:
        p.write_text(text)
        result = subprocess.run([a.tool, '-o', str(output), str(p)], capture_output=True)
        assert result.returncode != 0, text
    # Preserve file and case order and ensure contexts/substitutions reset.
    paths = []
    for i in range(3):
        case = Path(tmp) / f'file{i}.t'
        case.write_text(f'case a\nfunction f{i} named:C named:C\ncase b\nfunction z{i}\n')
        paths.append(str(case))
    subprocess.run([a.tool, '-o', str(output), *paths], check=True)
    assert output.read_text() == ''.join(f'_Z2f{i}1CS_\n_Z2z{i}v\n' for i in range(3))
print(f'{count} contract status/output and serialization checks; {len(valid)} valid/{len(invalid)} invalid personal probes; batch order pass')
