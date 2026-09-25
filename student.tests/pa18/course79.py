#!/usr/bin/env python3
"""Validate and execute every newly repaired positive course source: CC WORK."""
from pathlib import Path
import sys
import ordering_controls as runner
ROOT=Path(__file__).resolve().parents[2]
runner.GOOD={};runner.BAD={}
for path in ('general/300-using-directive-overloaded-function-template-arg.t',
             'general/400-alias-template-function-argument-cv.t',
             'general/300-pack-expanded-explicit-member-template-arguments.t',
             'spec/100-explicit-member-specialization-dependent-alias-parameter.t'):
 runner.GOOD[Path(path).stem]=(ROOT/'pa18/tests'/path).read_text()
if __name__=='__main__':sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
