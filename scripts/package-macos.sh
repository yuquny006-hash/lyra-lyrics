#!/usr/bin/env bash
set -euo pipefail
[[ "$(uname -s)" == Darwin ]] || exit 1
root="$(cd "$(dirname "$0")/.." && pwd)"
cd "$root"
version="$(sed -n 's/^project(CueFlow VERSION \([^ ]*\).*/\1/p' CMakeLists.txt)"
[[ -n "$version" ]] || { echo 'Missing project version' >&2; exit 1; }
name="CueFlow-$version-macOS-Universal"
dest="$root/dist/$name"
[[ ! -e "$dest" ]] || { echo "Output exists: $dest" >&2; exit 1; }
mkdir -p "$dest"
for item in 'VST3/CueFlow.vst3' 'AU/CueFlow.component' 'CLAP/CueFlow.clap'; do
 ditto "build/macos/CueFlowPlugin_artefacts/Release/$item" "$dest/${item##*/}"
done
ditto build/macos/CueFlowStandalone_artefacts/Release/CueFlow.app "$dest/CueFlow.app"
for bundle in "$dest/CueFlow.app" "$dest/CueFlow.vst3" "$dest/CueFlow.component" "$dest/CueFlow.clap"; do
 lipo -verify_arch arm64 x86_64 "$bundle/Contents/MacOS/CueFlow"
 codesign --force --deep --sign - "$bundle"
 codesign --verify --deep --strict "$bundle"
done
cp scripts/install-macos.sh "$dest/install-macos.sh"
cp docs/MAC-INSTALL.md "$dest/README-MAC.md"
cp README.md THIRD-PARTY-NOTICES.md LICENSE "$dest/"
ditto LICENSES "$dest/LICENSES"
ditto samples "$dest/samples"
ditto integrations "$dest/integrations"
mkdir -p "$dest/Source/third_party"
git archive HEAD | tar -x -C "$dest/Source"
for dependency in JUCE-8.0.12 clap-juce-extensions; do
 tar -C third_party --exclude='.git' -cf - "$dependency" | tar -xf - -C "$dest/Source/third_party"
done
printf '%s' 7adee3a1bd4684d4caa5601100e364abccff4b4d > "$dest/Source/third_party/clap-juce-extensions/.cueflow-revision"
printf 'commit=%s\narchitecture=arm64,x86_64\nsigning=ad-hoc; not Developer ID notarized\n' "$(git rev-parse HEAD)" > "$dest/BUILD-INFO.txt"
ditto -c -k --sequesterRsrc --keepParent "$dest" "$root/dist/$name.zip"
(cd dist && shasum -a 256 "$name.zip" > "$name.zip.sha256")
tar -czf dist/mac-validation-tools.tar.gz -C build/macos \
 CueFlowTests_artefacts/Release/CueFlowTests \
 CueFlowIntegration_artefacts/Release/CueFlowIntegration \
 CueFlowKaraokeTests_artefacts/Release/CueFlowKaraokeTests \
 tests/clap-smoke/Release/CueFlowClapSmoke
