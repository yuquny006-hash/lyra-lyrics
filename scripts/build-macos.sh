#!/usr/bin/env bash
set -euo pipefail
[[ "$(uname -s)" == Darwin ]] || { echo 'Run this script on macOS with Xcode and CMake.' >&2; exit 1; }
root="$(cd "$(dirname "$0")/.." && pwd)"
cd "$root"
bash scripts/setup-macos.sh
targets=(CueFlowPlugin_VST3 CueFlowPlugin_AU CueFlowPlugin_CLAP CueFlowStandalone CueFlowTests CueFlowIntegration CueFlowKaraokeTests CueFlowClapSmoke)
aax=OFF
if [[ "${1:-}" == --aax-development ]]; then aax=ON; targets+=(CueFlowPlugin_AAX); fi
cmake -S . -B build/macos -G Xcode -DCMAKE_OSX_ARCHITECTURES='arm64;x86_64' -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 -DCUEFLOW_BUILD_AAX="$aax"
cmake --build build/macos --config Release --target "${targets[@]}" --parallel 4
ctest --test-dir build/macos -C Release --output-on-failure --timeout 120
echo 'Build complete. AU must pass auval and Logic validation on this Mac. AAX is unsigned development output, not a Pro Tools release.'
