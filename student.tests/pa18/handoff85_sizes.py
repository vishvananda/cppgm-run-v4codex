#!/usr/bin/env python3
"""Host-only record-layout inspection; not compiler implementation: WORK."""
from pathlib import Path
import json,subprocess,sys
R=Path(__file__).resolve().parents[2];W=Path(sys.argv[1]).resolve();W.mkdir(parents=True,exist_ok=True)
rows=[]
for label,revision in [('entry','f953e42a875bc8f7e3b9882a324db843f0c4191a'),('final','HEAD')]:
 h=W/(label+'.h');h.write_bytes(subprocess.check_output(['git','show',revision+':dev/src/semantic/model.h'],cwd=R))
 p=W/(label+'.cpp');p.write_text('#include "'+str(h)+'"\n#include <iostream>\nint main(){std::cout<<sizeof(cppgm::semantic::Expression)<<"\\n";}\n')
 exe=W/label;subprocess.run(['g++','-std=c++11','-I'+str(R/'dev/src'),'-I'+str(R/'dev/src/semantic'),str(p),'-o',str(exe)],check=True)
 size=int(subprocess.check_output([exe]));rows.append(dict(revision=subprocess.check_output(['git','rev-parse',revision],cwd=R,text=True).strip(),expression_bytes=size))
assert rows[0]['expression_bytes']==rows[1]['expression_bytes']
(W/'sizes.json').write_text(json.dumps(rows,indent=2)+'\n');print(rows)
