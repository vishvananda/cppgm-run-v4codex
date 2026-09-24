#!/usr/bin/env python3
"""Check emitted dependent expression ABI names against the published grammar.
Run after query69_controls.py: CONTROLS_DIR OUT.
No host compiler or demangler supplies expected names.
"""
from pathlib import Path
import hashlib,json,sys
work,out=map(Path,sys.argv[1:]);rows=[]
# Itanium ABI 5.1.5: DT <expression> E, cl <expression> ... E,
# pt <expression> <unresolved-name>, dn <destructor-name>, pL +=,
# fp_ first function parameter; template type T_ enters substitutions.
expected={
 'assign_runtime':'_Z3addIiEDTpLfp_Li2EERT_',
 'destructor_trailing_abi':'_Z7destroyIiEDTclptfp_dnT_EEPS0_',
 'destructor_template_id_abi':'_Z7destroyIiEDTclptfp_dn1AIT_EEEP1AIS0_E',
 'destructor_base_runtime':'_Z7destroyI1DEDTclptfp_sr1BEdn1BEEPT_',
}
for name,symbol in expected.items():
 source=work/(name+'.lowir');data=source.read_text()
 rows.append(dict(name=name,path=str(source),sha256=hashlib.sha256(source.read_bytes()).hexdigest(),expected=symbol,passed=('object='+symbol+']') in data))
out.write_text(json.dumps(rows,indent=2)+'\n')
assert all(r['passed'] for r in rows)
print('Four emitted ABI identities match the Itanium dependent-expression grammar.')
