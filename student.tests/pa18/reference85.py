#!/usr/bin/env python3
"""N3485 demand/discard corrections from entry oracles only: [--write]."""
from pathlib import Path
import hashlib,json,re,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
ENTRY='f953e42a875bc8f7e3b9882a324db843f0c4191a'
STATIC={
 'pa18/tests/general/300-boost-enable-if-type-condition-static-keyword-overload.ref':'instance',
 'pa18/tests/general/500-source-owner-member-template-sfinae-default.ref':'npos',
}
rows=[]
def sha(s):return hashlib.sha256(s.encode()).hexdigest()
def original(p):return subprocess.check_output(['git','show',ENTRY+':'+p],cwd=ROOT,text=True)
def save(path,old,new,reason):
 if '--write' in sys.argv:(ROOT/path).write_text(new)
 assert (ROOT/path).read_text()==new,path
 rows.append(dict(path=path,before_sha256=sha(old),after_sha256=sha(new),reason=reason))
for path,member in STATIC.items():
 old=original(path)
 pattern=r'\A(global @[^\n]*__'+member+r'(?: : i32)? \[[^\n]*\]) = (?:0\n|\{\n  zero 1\n}\n)'
 match=re.match(pattern,old);assert match,path
 symbol=match[1].split()[1]
 assert len(re.findall(re.escape(symbol)+r'(?=\s|,|\))',old))==1,(path,symbol)
 new='declare '+match[1]+'\n'+old[match.end():]
 save(path,old,new,'[temp.inst]/1,2,8,10: no definition or initializer demand for unused static member')
path='pa18/tests/general/300-explicit-template-call-transitive-base-deduction.ref';old=original(path)
m=re.search(r'^(    (%\w+) = call ptr @[^\n]+\n)    (%\w+) = load i32 (\2)\n',old,re.M);assert m
assert len(re.findall(re.escape(m[3])+r'\b',old))==1
new=old[:m.start()]+m[1]+old[m.end():]
save(path,old,new,'[expr]/11 and [expr.static.cast]/6: void-cast reference call result has no lvalue-to-rvalue conversion')
manifest=dict(entry=ENTRY,bundle='c2f713cd70d06170632bfde3e75dd6fe1aa44d98',bundle_sha256='c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7',revisions=rows)
p=ROOT/'student.tests/pa18/reference85-revisions.json'
if '--write' in sys.argv:p.write_text(json.dumps(manifest,indent=2)+'\n')
assert json.loads(p.read_text())==manifest
print('Verified: two unused static definitions become declarations; one discarded reference load removed. All other oracle bytes retained.')
