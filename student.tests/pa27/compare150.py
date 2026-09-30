#!/usr/bin/env python3
"""Compare frozen audit150 images; normalization preserves symbols and opcodes."""
import difflib, hashlib, json, pathlib, re, subprocess, sys

out = pathlib.Path(sys.argv[1]).resolve()

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def functions(path):
    text = subprocess.check_output(['objdump','-d','--no-show-raw-insn',str(path)],text=True)
    (out/(path.name+'.final.dis')).write_text(text)
    result = {}; name = None
    for line in text.splitlines():
        label = re.fullmatch(r'[0-9a-f]+ <(.*)>:',line)
        if label:
            name = label[1]; result[name] = []
        instruction = re.match(r'\s+[0-9a-f]+:\s+(.*)',line)
        if instruction and name is not None:
            value = instruction[1]
            value = re.sub(r'[-+]?0x[0-9a-f]+\(%rip\)', 'DISP(%rip)',value)
            value = re.sub(r'\b[0-9a-f]+ (?=<)', 'ADDR ',value)
            result[name].append(' '.join(value.split()))
    return result

result = dict(normalization='Remove instruction/target addresses and RIP displacements; retain symbolic targets, opcodes and other operands.', workloads={})
for group in ('common','storage','hosted'):
    before = json.loads((out/group/'performance.json').read_text())
    final = json.loads((out/(group+'-final')/'performance.json').read_text())
    assert before['inputs'] == final['inputs']
    for name, images in final['images'].items():
        assert images == before['images'][name], 'header-context fix changed benchmark image'
        row = dict(pre_context_images_identical=True,images=images)
        result['workloads'][name] = row
        texts = {}; insns = {}
        for label in images:
            binary = out/(group+'-final')/(name+label)
            assert sha(binary) == images[label]['executable_sha256']
            data = out/(name+label+'.final.text')
            subprocess.run(['objcopy','--dump-section','.text='+str(data),str(binary)],check=True)
            texts[label] = sha(data)
            insns[label] = functions(binary)
        row['text_sha256'] = texts
        if 'A' not in insns:
            continue
        a,b = insns['A'],insns['B']
        row['shared_functions'] = len(a.keys() & b.keys())
        row['removed'] = sorted(a.keys()-b.keys())
        row['added'] = sorted(b.keys()-a.keys())
        row['changed'] = {n:list(difflib.unified_diff(a[n],b[n],fromfile='A',tofile='B',lineterm=''))
                          for n in sorted(a.keys() & b.keys()) if a[n] != b[n]}
        row['instruction_counts'] = {k:sum(map(len,v.values())) for k,v in insns.items()}
        if name == 'exceptions':
            assert set(row['changed']) == {'_Z4stepi'}
        else:
            assert not row['changed']
        assert not row['added']
        assert len(row['removed']) == (1200 if name == 'pruning' else 0)
        if name in ('construction','constant','signature'):
            assert texts['A'] == texts['B']
        if name == 'constant':
            symbols = [subprocess.check_output(['readelf','-sW',str(out/(group+'-final')/(name+k))],text=True).splitlines() for k in ('A','B')]
            row['symbol_difference'] = list(difflib.unified_diff(*symbols,fromfile='A',tofile='B',lineterm=''))
            assert '\n'.join(symbols[0]).replace('constantA.o','constantB.o') == '\n'.join(symbols[1])
(out/'image-comparison.json').write_text(json.dumps(result,indent=2)+'\n')
print('Frozen image comparison PASS:',len(result['workloads']),'workloads')
