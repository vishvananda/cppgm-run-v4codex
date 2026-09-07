#!/usr/bin/env python3
"""Build a frozen audit control with only the indentation reuse removed.

Run the ordinary dev build first. Output and temporary objects stay in the
specified directory, outside the implementation and course fixtures.
"""
import pathlib
import re
import subprocess
import sys

root = pathlib.Path(__file__).resolve().parents[2]
out = pathlib.Path(sys.argv[1]).resolve()
out.mkdir(parents=True, exist_ok=True)
source = (root/'dev/src/syntax/output.cpp').read_text()
before = "        indentation.resize(work.depth * 2, ' ');\n        out << indentation << kind_name(node.kind);"
after = "        out << std::string(work.depth * 2, ' ') << kind_name(node.kind);"
if before in source:
    assert source.count(before) == source.count('    std::string indentation;\n') == 1
    source = source.replace('    std::string indentation;\n', '').replace(before, after)
else:
    # The final audit dropped the optimization; its control is already present.
    assert source.count(after) == 1
control = out/'output-no-reuse.cpp'
control.write_text(source)
flags = ['g++', '-std=gnu++11', '-Wall', '-O3', '-I'+str(root/'dev/src')]
subprocess.run([*flags, '-c', control, '-o', out/'output-no-reuse.o'], check=True)
sets = dict(re.findall(r'^(FRONTEND_OBJ_BASENAMES_\S+)\s*:=\s*(.*)$',
                       (root/'dev/frontend_source_sets.mk').read_text(), re.M))
sources = sets['FRONTEND_OBJ_BASENAMES_cppgm++']
while '$(' in sources:
    sources = re.sub(r'\$\(([^)]+)\)', lambda m: sets[m[1]], sources)
objects = [out/'output-no-reuse.o' if name == 'syntax/output' else root/f'obj/dev/{name}.o'
           for name in sources.split()]
subprocess.run([*flags, '-o', out/'no-reuse', root/'obj/dev/entry/cppgm++.o',
                *objects, root/'obj/dev/test_runner_enabled.o'], check=True)
print(out/'no-reuse')
