#!/usr/bin/env python3
"""Recompute and bind all unaudited PA33 handoff observations at their commit."""
import hashlib,json,os,pathlib,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa32'))
from audit214_history import measurement
def sha(data):return hashlib.sha256(data).hexdigest()
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT)
folder=ROOT/'student.tests/pa33/evidence219'
b=json.loads((folder/'binding.json').read_text())
entries={line.split('\t')[1]:line.split()[0] for line in git('ls-tree','-r','c1328ca0').decode().splitlines()}
for name,digest in b['sources'].items():
    resolved=name
    while entries[resolved]=='120000':resolved=os.path.normpath(os.path.join(os.path.dirname(resolved),git('show','c1328ca0:'+resolved).decode()))
    assert sha(git('show','c1328ca0:'+resolved))==digest,name
for name,digest in b['evidence'].items():assert sha((folder/name).read_bytes())==digest,name
for name,digest in b['artifacts'].items():assert sha(pathlib.Path(name).read_bytes())==digest,name
observations=0
for file in sorted(folder.glob('*.json')):
    data=json.loads(file.read_text())
    if not isinstance(data,dict) or not data.get('runs') or not data.get('summary'):continue
    for binary in data['binaries'].values():assert sha(pathlib.Path(binary['path']).read_bytes())==binary['sha256']
    observations+=measurement(data)
for path,tree in b['fixture_trees'].items():assert git('rev-parse','HEAD:'+path).decode().strip()==tree
result=dict(handoff='c1328ca0',sources=len(b['sources']),artifacts=len(b['artifacts']),observations=observations,fixtures_unchanged=True)
pathlib.Path(sys.argv[1]).write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps(result))
