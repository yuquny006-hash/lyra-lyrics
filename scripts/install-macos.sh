#!/usr/bin/env bash
set -euo pipefail
[[ "$(uname -s)" == Darwin ]] || { echo 'macOS only.' >&2; exit 1; }
root="$(cd "$(dirname "$0")/.." && pwd)"
src="$root/build/macos/CueFlowPlugin_artefacts/Release"
script_dir="$(cd "$(dirname "$0")" && pwd)"
if [[ -d "$script_dir/CueFlow.vst3" ]]; then
 src="$script_dir"
 items=('CueFlow.vst3:VST3' 'CueFlow.component:Components' 'CueFlow.clap:CLAP')
else
 items=('VST3/CueFlow.vst3:VST3' 'AU/CueFlow.component:Components' 'CLAP/CueFlow.clap:CLAP')
fi
for mapping in "${items[@]}"; do
 item="${mapping%%:*}"
 [[ -e "$src/$item" ]] || { echo "Missing built artifact: $src/$item" >&2; exit 1; }
done
for mapping in "${items[@]}"; do
 item="${mapping%%:*}"; folder="${mapping##*:}"; name="${item##*/}"
 for base in "$HOME/Library/Audio/Plug-Ins" '/Library/Audio/Plug-Ins'; do
  [[ ! -e "$base/$folder/$name" ]] || { echo "Existing installation preserved: $base/$folder/$name" >&2; exit 1; }
 done
done
for mapping in "${items[@]}"; do
 item="${mapping%%:*}"; folder="${mapping##*:}"
 mkdir -p "$HOME/Library/Audio/Plug-Ins/$folder"
 ditto "$src/$item" "$HOME/Library/Audio/Plug-Ins/$folder/${item##*/}"
done
echo 'Installed locally. Validate AU with: auval -v aufx Cfp1 Cufl'
