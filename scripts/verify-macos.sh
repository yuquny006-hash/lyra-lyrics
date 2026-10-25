#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
cd "$root"
package="${1:?Pass the extracted macOS package directory}"
tools="${2:?Pass the validation tools directory}"
for bundle in CueFlow.app CueFlow.vst3 CueFlow.component CueFlow.clap; do
 lipo -verify_arch arm64 x86_64 "$package/$bundle/Contents/MacOS/CueFlow"
 codesign --verify --deep --strict "$package/$bundle"
done
"$tools/CueFlowTests_artefacts/Release/CueFlowTests" tests/fixtures
"$tools/CueFlowIntegration_artefacts/Release/CueFlowIntegration"
"$tools/CueFlowKaraokeTests_artefacts/Release/CueFlowKaraokeTests"
"$tools/tests/clap-smoke/Release/CueFlowClapSmoke" "$package/CueFlow.clap/Contents/MacOS/CueFlow"
bash "$package/install-macos.sh"
auval -v aufx Cfp1 Cufl
