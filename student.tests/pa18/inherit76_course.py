#!/usr/bin/env python3
"""Execute inherited course cases, including the remaining O0-policy mismatch."""
from pathlib import Path
import sys
import ordering_controls as runner
ROOT=Path(__file__).resolve().parents[2]
runner.GOOD={};runner.BAD={}
for path in ('200-inherited-constructor-template-forwarding','400-partial-specialization-inherited-constructor-template','500-inherited-constructor-template-member-alias-pack'):
 runner.GOOD[path]=(ROOT/'pa18/tests/general'/(path+'.t')).read_text()
if __name__=='__main__':sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
