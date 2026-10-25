#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
cd "$root"
package="${1:?Pass the extracted macOS package directory}"
tools="${2:?Pass the validation tools directory}"
for bundle in CueFlow.app CueFlow.vst3 CueFlow.component CueFlow.clap; do
 lipo "$package/$bundle/Contents/MacOS/CueFlow" -verify_arch arm64 x86_64
 codesign --verify --deep --strict "$package/$bundle"
done
"$tools/CueFlowTests_artefacts/Release/CueFlowTests" tests/fixtures
for pass in 1 2 3; do
 "$tools/CueFlowIntegration_artefacts/Release/CueFlowIntegration"
done
"$tools/CueFlowKaraokeTests_artefacts/Release/CueFlowKaraokeTests"
"$tools/tests/clap-smoke/Release/CueFlowClapSmoke" "$package/CueFlow.clap/Contents/MacOS/CueFlow"
bash "$package/install-macos.sh"
# Fresh CI machines may still have an AudioComponentRegistrar catalogue from
# before the bundle was installed. Refresh only that disposable runner session.
if [[ "${CI:-}" == true ]]; then
 killall AudioComponentRegistrar 2>/dev/null || true
fi
for attempt in {1..12}; do
 catalogue="$(auval -a 2>&1)"
 if [[ "$catalogue" == *Cfp1*Cufl* ]]; then break; fi
 sleep 1
done
printf '%s\n' "$catalogue"
auval -v aufx Cfp1 Cufl
