#!/bin/bash
set -euo pipefail
package="$1"
bash "$package/setup-alignment-macos.sh"
base="$HOME/Library/AmuMusic/CueFlow"
python="$base/alignment-mac-v1/bin/python"
testdir="$(mktemp -d)"
say -o "$testdir/reference.aiff" "Welcome to the recording studio. We are testing the lyrics on this computer."
"$python" - "$base" "$testdir" <<'PY'
import json,sys,subprocess
from pathlib import Path
base,work=map(Path,sys.argv[1:])
c=json.loads((base/'alignment.json').read_text())
r={'audio':str(work/'reference.aiff'),'lyrics':'Welcome to the recording studio.\nWe are testing the lyrics on this computer.','language':'en','model':'base','model_dir':c['models'],'result':str(work/'result.json'),'progress':str(work/'progress.json')}
(work/'request.json').write_text(json.dumps(r))
p=subprocess.run([c['python'],c['script'],'--request',str(work/'request.json')])
if p.returncode:
 print((work/'result.log').read_text());raise SystemExit(p.returncode)
result=json.loads((work/'result.json').read_text())
assert len(result['lines'])==2 and all(row['words'] for row in result['lines'])
assert result['duration']>1
print('MAC REAL ALIGNMENT PASS',result['duration'],result['elapsed'])
PY
