#!/usr/bin/env python3
"""Final-audit edge topology corpus, frozen separately from the stage corpus.
audit_performance.py measure|verify CHECKPOINT FINAL record.json
audit_performance.py report record.json
Uses the same frozen ABBA, A/A, startup, RSS, text and scaling budgets.
"""
import json
import pathlib
import subprocess
import sys
import measure

def workloads():
    cases={}
    for scale in (1,4):
        count=6000*scale
        source=''.join(f'namespace N_{i}{{}}\n' for i in range(count))
        # Qualified targets isolate edge insertion from required unqualified
        # nomination work. Repeated directives must reuse the same edge.
        source+=''.join(f'using namespace ::N_{i};using namespace ::N_{i};\n' for i in range(count))
        cases[f'types-edges-{scale}']=dict(source=source,mode='--emit-types',repeats=4,scale=scale,group='edges')
    return cases

if __name__=='__main__':
    measure.workloads=workloads
    if sys.argv[1]=='report':
        measure.report(json.loads(pathlib.Path(sys.argv[2]).read_text()))
    else:
        a,b=map(pathlib.Path,sys.argv[2:4]);output=pathlib.Path(sys.argv[4])
        if sys.argv[1]=='measure':
            measure.measure([a,a,b],output,['1edcbe5dbbc7af3300ffd117cf05191571e4395d']*2+
                            [subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip()])
        else:
            data=json.loads(output.read_text());measure.verify([a,a,b],data)
            for row in data['work']:
                if row['variant']!='B':continue
                count=6000*data['inputs'][row['input']]['scale']
                for phase in row['phases']:
                    assert phase['semantic_edges']==count
                    assert phase['semantic_lookup_work']==2*count
            print('Edge identity deduplication and linear qualified lookup work passed')
