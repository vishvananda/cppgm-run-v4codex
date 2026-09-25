#!/usr/bin/env python3
"""Execute the nested/lookup course paths repaired since the last audit."""
from pathlib import Path
import sys
import ordering_controls as runner
root=Path(__file__).resolve().parents[2]
runner.GOOD={};runner.BAD={}
for name in ('300-lazy-nested-class-in-member-template','300-lazy-nested-member-class-instantiation','300-ambiguous-inherited-member-type-sfinae'):
 source=(root/'pa18/tests/general'/(name+'.t')).read_text()
 if 'int main(' not in source:source+='\nint main(){}\n'
 runner.GOOD[name]=source
if __name__=='__main__':sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
