#!/bin/bash
set -euo pipefail
[[ "$(uname -s)" == Darwin ]] || { echo "This installer requires macOS."; exit 1; }
here="$(cd "$(dirname "$0")" && pwd)"
base="$HOME/Library/AmuMusic/CueFlow"
mkdir -p "$base"
exec > >(tee -a "$base/alignment-install.log") 2>&1
trap 'echo "Installation failed. 安装失败，请发送日志 ~/Library/AmuMusic/CueFlow/alignment-install.log"; exit 1' ERR
echo "Installing local alignment engine / 正在安装本地对齐引擎，需要联网"
worker="$here/engine/align.py"
[[ -f "$worker" ]] || worker="$here/../engine/align.py"
[[ -f "$worker" ]] || { echo "Please extract the complete package / 请完整解压发行包"; exit 1; }
archive="$here/CueFlow-Engine-$(uname -m).tar.gz"
if [[ -f "$archive" ]]; then
 echo "Offline engine found / 找到离线引擎，正在校验解压，无需联网"
 (cd "$here" && shasum -a 256 -c SHA256SUMS)
 stage="$(mktemp -d "$base/offline-XXXXXXXX")"
 tar -xzf "$archive" -C "$stage"
 python="$stage/runtime/python/bin/python3.11"
 "$python" -c 'import torch,torchaudio,stable_whisper,soundfile,numpy'
 cp "$worker" "$stage/runtime/align.py"
 "$python" - "$base" "$stage/runtime" <<'PY'
import json,os,sys
from pathlib import Path
base,runtime=map(Path,sys.argv[1:])
c={'version':1,'python':str(runtime/'python/bin/python3.11'),'script':str(runtime/'align.py'),'models':str(runtime/'models')}
p=base/'alignment.json.tmp'
p.write_text(json.dumps(c),encoding='utf-8')
os.replace(p,base/'alignment.json')
PY
 echo "Ready / 离线安装成功，返回插件开始对齐即可。base 与 small 模型已包含。"
 exit 0
fi
echo "未找到离线引擎，尝试联网安装；网络受限请使用与电脑芯片匹配的离线包。"
export UV_HTTP_TIMEOUT=60
export UV_HTTP_RETRIES=2
export UV_INSTALL_DIR="$base/tools"
export UV_NO_MODIFY_PATH=1
export UV_PYTHON_INSTALL_DIR="$base/python"
export UV_CACHE_DIR="$base/cache"
curl --fail --location --connect-timeout 15 --max-time 180 --retry 2 https://astral.sh/uv/0.9.7/install.sh -o "$base/install-uv.sh"
export UV_INSTALLER_DOWNLOAD_TIMEOUT=120
sh "$base/install-uv.sh"
uv="$base/tools/uv"
"$uv" python install 3.11
envdir="$base/alignment-mac-v1"
if [[ ! -x "$envdir/bin/python" ]]; then
 "$uv" venv --python 3.11 --managed-python "$envdir"
fi
python="$envdir/bin/python"
"$uv" pip install --python "$python" 'numpy==1.26.4' 'torch==2.2.2' 'torchaudio==2.2.2' 'stable-ts==2.19.1' 'openai-whisper==20250625' 'soundfile==0.13.1' 'numba==0.61.2' 'llvmlite==0.44.0'
"$python" -c 'import torch,torchaudio,stable_whisper,soundfile,numpy; print("Dependencies ready:",torch.__version__)'
cp "$worker" "$envdir/align.py"
mkdir -p "$base/models"
"$python" - "$base" "$envdir" <<'PY'
import json,os,sys
from pathlib import Path
base,env=map(Path,sys.argv[1:])
config={'version':1,'python':str(env/'bin/python'),'script':str(env/'align.py'),'models':str(base/'models')}
temp=base/'alignment.json.tmp'
temp.write_text(json.dumps(config),encoding='utf-8')
os.replace(temp,base/'alignment.json')
PY
echo "Ready / 安装完成。返回软件重新开始自动对齐；首次模型下载需要联网。音频留在本机。"
