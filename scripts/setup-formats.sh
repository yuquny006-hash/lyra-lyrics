#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
dep="$root/third_party/clap-juce-extensions"
revision=7adee3a1bd4684d4caa5601100e364abccff4b4d
if [[ ! -d "$dep" && -f "$root/third_party/clap-juce-extensions-source.zip" ]]; then
 unzip -q "$root/third_party/clap-juce-extensions-source.zip" -d "$root/third_party"
fi
if [[ -f "$dep/.cueflow-revision" && "$(cat "$dep/.cueflow-revision")" == "$revision" && -f "$dep/clap-libs/clap/include/clap/clap.h" ]]; then exit 0; fi
if [[ ! -d "$dep" ]]; then
 git clone https://github.com/free-audio/clap-juce-extensions.git "$dep"
fi
if [[ -n "$(git -C "$dep" status --porcelain)" ]]; then
 echo 'CLAP dependency has local changes; leaving it untouched.' >&2; exit 1
fi
git -C "$dep" checkout --detach "$revision"
git -C "$dep" submodule update --init --recursive
