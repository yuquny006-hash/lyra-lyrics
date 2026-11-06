#!/bin/bash
set -euo pipefail
base="$HOME/Library/AmuMusic/CueFlow"
python="$base/alignment-mac-v1/bin/python"
out="$PWD/offline-output"
mkdir -p "$out/runtime/python" "$out/runtime/models"
"$python" - "$base/models" <<'PY'
import stable_whisper,sys
stable_whisper.load_model('small',device='cpu',download_root=sys.argv[1])
PY
prefix="$("$python" -c 'import sys;print(sys.base_prefix)')"
site="$("$python" -c 'import site;print(site.getsitepackages()[0])')"
ditto "$prefix/" "$out/runtime/python/"
ditto "$site/" "$out/runtime/python/lib/python3.11/site-packages/"
cp "$base/models/base.pt" "$base/models/small.pt" "$out/runtime/models/"
cp engine/align.py "$out/runtime/align.py"
"$out/runtime/python/bin/python3.11" -c 'import stable_whisper,torch,soundfile;print("Relocated runtime import PASS")'
tar -czf "$out/CueFlow-Engine-$(uname -m).tar.gz" -C "$out" runtime
(cd "$out" && shasum -a 256 CueFlow-Engine-*.tar.gz > SHA256SUMS)
