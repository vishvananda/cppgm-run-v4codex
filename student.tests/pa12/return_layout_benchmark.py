#!/usr/bin/env python3
"""Diagnostic only: isolate a counter from executable cache lines in frozen PA8 images.

No compiler, source fixture, reference, or production image is changed. The fixed
one-counter reducer has four data-address references. Their displacements and the
ELF segment sizes are the only existing bytes changed; instruction positions and
opcodes remain identical. Padding is inserted before the counter, after all code.
"""
from pathlib import Path
import importlib.util
import json
import os
import statistics
import struct
import sys
import time

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('prior',ROOT/'student.tests/pa10/benchmark.py')
prior = importlib.util.module_from_spec(spec); spec.loader.exec_module(prior)
work, output = map(Path,sys.argv[1:3])
cpu=min(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
result=dict(protocol='per compiler: warmups, four A/A observations and two ABBA blocks; original vs data relocated by 64 bytes',
            cpu=cpu,harness_sha256=prior.sha(__file__),images=[],
            pmu='machine_clears.smc is unavailable; native address relocation supplies the causal control')

def relocate(original):
    data=bytearray(original.read_bytes())
    entry,phoff=struct.unpack_from('<QQ',data,24)
    assert data[:5]==b'\x7fELF\x02' and struct.unpack_from('<H',data,56)[0]==1
    assert struct.unpack_from('<Q',data,40)[0]==0
    kind,flags,offset,base,physical,size,memory,align=struct.unpack_from('<IIQQQQQQ',data,phoff)
    assert kind==1 and flags==7 and offset==0 and size==memory==len(data)
    counter=len(data)-4; address=base+counter
    assert data[counter:]==bytes(4)
    ir=original.with_suffix('.lowir').read_text()
    globals=[line for line in ir.splitlines() if line.startswith('global ')]
    assert len(globals)==1 and ' : i32 ' in globals[0] and globals[0].endswith('= zero')
    patched=bytearray(data); fixups=[]
    for at in range(entry-base,counter-7):
        if data[at:at+3] in (bytes.fromhex('4c6305'),bytes.fromhex('448905')):
            displacement=struct.unpack_from('<i',data,at+3)[0]
            if base+at+7+displacement==address:
                struct.pack_into('<i',patched,at+3,displacement+64);fixups.append((at+3,4,'rip-relative'))
        if data[at:at+8]==struct.pack('<Q',address):
            assert data[at-2:at]==bytes.fromhex('49bb')
            struct.pack_into('<Q',patched,at,address+64);fixups.append((at,8,'absolute'))
    assert sorted(kind for _,_,kind in fixups)==['absolute','absolute','rip-relative','rip-relative']
    masked=[bytearray(data[entry-base:counter]),bytearray(patched[entry-base:counter])]
    for at,length,_ in fixups:
        for code in masked:code[at-(entry-base):at-(entry-base)+length]=bytes(length)
    assert masked[0]==masked[1]
    patched[counter:counter]=bytes(64)
    struct.pack_into('<Q',patched,phoff+32,size+64);struct.pack_into('<Q',patched,phoff+40,memory+64)
    path=original.with_name(original.name+'-separated');path.write_bytes(patched);path.chmod(0o755)
    return path,dict(original_sha256=prior.sha(original),relocated_sha256=prior.sha(path),
        lowir_sha256=prior.sha(original.with_suffix('.lowir')),code_entry=hex(entry),
        code_bytes=counter-(entry-base),original_payload_bytes=size-(entry-base),relocated_payload_bytes=size+64-(entry-base),
        original_counter=hex(address),relocated_counter=hex(address+64),
        original_counter_line=hex(address&~63),relocated_counter_line=hex((address+64)&~63),
        unchanged_instruction_positions=True,unchanged_instruction_bytes_except_data_addresses=True,
        fixups=[dict(file_offset=at,bytes=length,kind=kind) for at,length,kind in fixups])

def observe(path):
    usage=work/'layout-usage.txt';start=time.perf_counter_ns()
    prior.run(['/usr/bin/time','-f','%M','-o',usage,path])
    return dict(wall_s=(time.perf_counter_ns()-start)/1e9,rss_kib=int(usage.read_text()),checked_exit=0)

for label in (0,1):
    original=work/f'loop-runtime-{label}'
    separated,record=relocate(original)
    record['compiler']=label;record['warmups']=[observe(original),observe(separated)];rows=[]
    for index in prior.ORDER:
        row=observe((original,separated)[index]);row['image']=index;rows.append(row)
    record['observations']=rows
    record['paired_relocated_over_original']=[statistics.mean(x['wall_s'] for x in rows[k:k+4] if x['image']==1)/
        statistics.mean(x['wall_s'] for x in rows[k:k+4] if x['image']==0) for k in (4,8)]
    result['images'].append(record);output.write_text(json.dumps(result,indent=2)+'\n')
    print('compiler',label,'complete',flush=True)
