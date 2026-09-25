#!/usr/bin/env python3
"""Validate and execute the four repaired course cases plus the cast reducer."""
from pathlib import Path
import sys
import ordering_controls as runner
ROOT=Path(__file__).resolve().parents[2]
runner.GOOD={};runner.BAD={}
for path,main in [
 ('general/100-dependent-braced-overload-signature.t','int main(){return invoke_probe()!=1||probe_size()!=sizeof(int);}'),
 ('general/300-alias-bool-explicit-pack-call-dependent-tag.t',''),
 ('spec/300-list-initialization-narrowing-sfinae.t','int main(){}'),
 ('spec/300-c-style-virtual-base-downcast-sfinae.t','int main(){}')]:
 runner.GOOD[Path(path).stem]=(ROOT/'pa18/tests'/path).read_text()+'\n'+main
runner.GOOD['cast-reducer']=(Path(__file__).parent/'cast75-reducer.cpp').read_text()
if __name__=='__main__':sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
