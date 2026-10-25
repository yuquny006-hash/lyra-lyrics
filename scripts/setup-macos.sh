#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
dep="$root/third_party/JUCE-8.0.12"
revision=29396c22c93392d6738e021b83196283d6e4d850
if [[ ! -e "$dep" ]]; then
 mkdir -p "$root/third_party"
 git clone --depth 1 --branch 8.0.12 https://github.com/juce-framework/JUCE.git "$dep"
fi
if [[ -d "$dep/.git" ]]; then
 [[ "$(git -C "$dep" rev-parse HEAD)" == "$revision" ]] || { echo 'Unexpected JUCE revision; existing source preserved.' >&2; exit 1; }
 [[ -z "$(git -C "$dep" status --porcelain)" ]] || { echo 'JUCE has local modifications; existing source preserved.' >&2; exit 1; }
fi
[[ -f "$dep/CMakeLists.txt" ]] || { echo 'JUCE source missing.' >&2; exit 1; }
bash "$root/scripts/setup-formats.sh"
