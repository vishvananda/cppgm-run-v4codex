#!/usr/bin/env python3
"""Check every measured counter and compare linked ELF section bytes."""
import fractions, hashlib, json, pathlib, struct, sys
out = pathlib.Path(sys.argv[1]).resolve()
def save(name, value):
    (out / (name + '.json')).write_text(json.dumps(value, indent=2) + '\n')
def sha(data):
    return hashlib.sha256(data).hexdigest()
def sections(path):
    data = path.read_bytes()
    assert data[:6] == b'\x7fELF\x02\x01'
    offset = struct.unpack_from('<Q', data, 40)[0]
    width, count, names = struct.unpack_from('<HHH', data, 58)
    rows = [struct.unpack_from('<IIQQQQIIQQ', data, offset + i * width) for i in range(count)]
    strings = data[rows[names][4]:rows[names][4] + rows[names][5]]
    return {strings[r[0]:].split(b'\0')[0].decode(): data[r[4]:r[4] + r[5]] for r in rows if r[1] != 8}
elf = {}
for group in ['cumulative-performance', 'owner-performance']:
    measurements = json.loads((out / group / 'performance.json').read_text())
    assert all(r['status'] == 0 for r in measurements['runs'])
    for name, images in measurements['images'].items():
        if 'A' not in images:
            continue
        paths = {label: out / group / (name + label) for label in 'AB'}
        for label, path in paths.items():
            assert sha(path.read_bytes()) == images[label]['executable_sha256']
        a, b = [sections(paths[label]) for label in 'AB']
        row = {}
        for section in ['.text', '.rodata', '.data', '.eh_frame']:
            row[section] = dict(equal=a[section] == b[section], bytes=len(a[section]), A_sha256=sha(a[section]), B_sha256=sha(b[section]))
            assert row[section]['equal'], (name, section)
        elf[group + '/' + name] = row
save('elf-comparison', elf)
j = json.loads((out / 'owner-performance' / 'performance.json').read_text())
keys = ['parsed_nodes', 'nodes', 'template_body_transitions', 'semantic_specializations',
        'semantic_substitution_frames', 'inline_variable_initializers', 'inline_variable_hits',
        'inline_variable_demands', 'inline_variable_dependencies', 'instructions',
        'native_functions', 'native_instructions', 'text_bytes']
scaling = {}
for family in ['source', 'declaration', 'selection', 'bound']:
    values = {}
    for n in [600, 1200, 2400]:
        rows = [r for r in j['runs'] if r['workload'] == family + str(n) and r['mode'] == 'compile' and r['label'] == 'B']
        counters = [{k: v for phase in r['counters'] for k, v in phase.items()} for r in rows]
        first = {k: counters[0][k] for k in keys}
        assert all({k: c[k] for k in keys} == first for c in counters)
        values[n] = dict(samples=len(rows), counters=first)
    equations = {}
    for k in keys:
        slope = fractions.Fraction(values[1200]['counters'][k] - values[600]['counters'][k], 600)
        intercept = values[600]['counters'][k] - 600 * slope
        assert all(v['counters'][k] == n * slope + intercept for n, v in values.items()), (family, k)
        equations[k] = dict(slope=str(slope), intercept=str(intercept))
    scaling[family] = dict(values=values, exact_linear=equations)
save('scaling', scaling)
print('Compared', len(elf), 'paired images; all sampled owner counters follow recorded linear equations.')
