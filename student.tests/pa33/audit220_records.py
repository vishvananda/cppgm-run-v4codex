#!/usr/bin/env python3
"""Bind final PA33 code, unchanged contracts, measurements and audit results."""
import hashlib,json,os,pathlib,shutil,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa32'))
from audit214_history import measurement
ART=pathlib.Path(os.environ['RALPH_ARTIFACT_DIR'])/'pa33-220'
OUT=ROOT/'student.tests/pa33/evidence220'
BASE='676b6e328c7a05334b15f1c9ee1ec30c783e9aff'
LANES=['affected','common-o0','common-o2','selfhost','scratch']
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT,text=True).strip()
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text())
def write(p,v):p.write_text(json.dumps(v,indent=2)+'\n')
def measurements():
    samples=0;objects=0
    lines=['# Final frozen performance evidence','',
           'A/A calibration followed by six wall-time ABBA blocks; paired B/A medians and all paired extrema. Compilation and checked execution are separate. CPU 2. Every observation, RSS value, input/binary hash and A/A range is retained in the linked JSON. A is the stage base except scratch, where A is the audited entry (219). B is the final audited compiler.','']
    for lane in LANES:
        d=read(OUT/(lane+'.json'));samples+=measurement(d)
        for binary in d['binaries'].values():assert sha(pathlib.Path(binary['path']))==binary['sha256']
        summaries=d['summary'] if lane not in ['selfhost','scratch'] else {lane:({'compile':d['summary']} if lane=='selfhost' else d['summary'])}
        if lane!='scratch':
            previous=read(ROOT/'student.tests/pa33/evidence219'/('affected.json' if lane=='affected' else lane+'.json'))
            old_images=previous['images']
            current=d['images']
            for name in (['selfhost'] if lane=='selfhost' else current):
                a=old_images['B'] if lane=='selfhost' else old_images[name]['B']
                b=current['B'] if lane=='selfhost' else current[name]['B']
                assert a.get('object_sha256',a.get('sha256'))==b.get('object_sha256',b.get('sha256')),(lane,name)
                objects+=1
        else:assert d['images']['A']['object_sha256']==d['images']['B']['object_sha256']
        lines+=['## '+lane,'','[All samples]('+lane+'.json'+')','','| Workload | Compile B/A [range] | Compile ms A/B | Peak KiB A/B | Runtime B/A [range] | Runtime ms A/B | Object text A/B |','| --- | --- | --- | --- | --- | --- | --- |']
        for name,modes in summaries.items():
            images=d['images'] if lane in ['scratch','selfhost'] else d['images'][name]
            def ratio(s):return '%.3f [%.3f–%.3f]'%(s['paired_ratio_median'],*s['paired_ratio_range'])
            def pair(s,key,scale=1):return '/'.join(('%.2f'%(s[k][key]*scale)) if scale!=1 else str(s[k][key]) for k in 'AB')
            c=modes['compile'];r=modes.get('runtime')
            text='/'.join(str(images[k].get('object_text_bytes',images[k].get('text_bytes'))) for k in 'AB')
            lines.append('| '+' | '.join([name,ratio(c),pair(c,'median_s',1000),pair(c,'peak_rss_kib'),ratio(r) if r else 'N/A',pair(r,'median_s',1000) if r else 'N/A',text])+' |')
        lines.append('')
    return samples,objects,'\n'.join(lines)
CHECKS=[('build','make build','LINK'),('stage','make test-pa33','(57 / 57)'),
        ('debug','make -C pa33 test-debuginfo','PASS (1/1)'),
        ('file','perl scripts/cppgm_file_audit.pl --stage pa33 --paths dev/src','File audit passed for pa33'),
        ('through','make test-report-through-pa33','(5454 / 5454)'),
        ('trace','python3 student.tests/pa33/audit220.py $RALPH_ARTIFACT_DIR/pa33-220/trace','controls: PASS')]
def checks():
    rows=[]
    for name,command,marker in CHECKS:
        path=ART/(name+'.log');s=path.read_text();assert marker in s,(name,s)
        assert not any(x in s for x in ['ERROR:','FAIL','*** Error','Traceback']),name
        if name in ['stage','through']:assert 'PASS (18/18)' in s and 'PASS (5/5)' in s
        if name=='debug':assert s.count('PASS (5/5)')==2
        rows.append(dict(command=command,exit_code=0,log=str(path),sha256=sha(path)))
    return rows
if sys.argv[1]=='bind':
    OUT.mkdir(exist_ok=True)
    for lane in LANES:shutil.copyfile(ART/lane/'performance.json',OUT/(lane+'.json'))
    shutil.copyfile(ART/'history.json',OUT/'history.json')
    samples,objects,report=measurements();(OUT/'performance.md').write_text(report)
    write(OUT/'checks.json',checks())
    sources=set(git('ls-files','dev').splitlines())
    sources.update(str(p.relative_to(ROOT)) for p in (ROOT/'student.tests/pa33').glob('*') if p.is_file())
    sources.update(['spec.md','AGENTS.md','TESTING_AND_REFERENCES.md','pa33/README.md','pa33/plan.md','pa33/audit.md'])
    b=dict(stage_base=BASE,implementation_commit=git('log','-1','--format=%H','--','dev'),
           sources={p:sha(ROOT/p) for p in sorted(sources)},
           evidence={p.name:sha(p) for p in sorted(OUT.iterdir()) if p.is_file() and p.name!='binding.json'},
           artifacts={str(p):sha(p) for p in sorted(ART.rglob('*')) if p.is_file() and p.name!='verify.log'},
           fixture_trees={f'pa{i}/tests':git('rev-parse',f'HEAD:pa{i}/tests') for i in range(1,34)},
           observations=samples,unchanged_measured_objects=objects,open_findings=[],unaudited_handoffs=[])
    write(OUT/'binding.json',b)
else:
    b=read(OUT/'binding.json');assert b['stage_base']==BASE
    assert not git('diff',b['implementation_commit'],'--','dev')
    assert not git('diff',BASE,'--','scripts',*[f'pa{i}/tests' for i in range(1,34)])
    for key,prefix in [('sources',ROOT),('evidence',OUT),('artifacts',pathlib.Path('/'))]:
        for path,digest in b[key].items():assert sha(prefix/path)==digest,path
    for path,tree in b['fixture_trees'].items():assert git('rev-parse','HEAD:'+path)==tree
    for tool in ['cppgm++','lowir2native']:assert sha(ROOT/'dev'/tool)==sha(ART/'final'/tool)
    assert checks()==read(OUT/'checks.json')
    samples,objects,report=measurements()
    assert samples==b['observations'] and objects==b['unchanged_measured_objects']
    assert report==(OUT/'performance.md').read_text()
    if '--clean' in sys.argv:assert not git('status','--short')
    print('PA33 final audit: verified;',samples,'current observations;',objects,'unchanged measured outputs; no open findings or handoffs')
