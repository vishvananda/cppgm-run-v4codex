#!/usr/bin/env python3
"""Check runtime outcomes even where a MIR mismatch stops the course comparator."""
import pathlib, collections
root=pathlib.Path(__file__).resolve().parents[2]/'pa24/tests'
compiled=0; failures=[]; missing=collections.Counter()
for lane in ('strict','structural','behavior'):
 for source in sorted((root/lane).glob('*.t')):
  base=source.with_suffix('')
  def read(suffix): return pathlib.Path(str(base)+suffix).read_bytes()
  if read('.my.impl.exit_status').strip()!=b'0':
   missing[read('.my.impl.stderr').decode().strip()]+=1
   continue
  compiled+=1
  for suffix in ('.program.exit_status','.program.stdout'):
   if read('.my'+suffix)!=read('.ref'+suffix): failures.append(str(source)+suffix)
assert not failures, failures
print(f'All {compiled} successfully compiled course programs match reference exit status/stdout, including MIR-failing fixtures')
for reason,count in sorted(missing.items()): print(count,reason)
