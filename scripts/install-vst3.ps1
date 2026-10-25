$ErrorActionPreference='Stop'
$source=Join-Path $PSScriptRoot 'CueFlow.vst3'
if(!(Test-Path -LiteralPath $source)){$source=Join-Path (Split-Path $PSScriptRoot -Parent) 'build/native/CueFlowPlugin_artefacts/Release/VST3/CueFlow.vst3'}
if(!(Test-Path -LiteralPath $source)){throw 'CueFlow.vst3 was not found next to this script.'}
$directory=Join-Path $env:LOCALAPPDATA 'Programs/Common/VST3'
$destination=Join-Path $directory 'CueFlow.vst3'
New-Item -ItemType Directory -Force -Path $directory | Out-Null
if(Test-Path -LiteralPath $destination){throw "Existing installation kept. Close your DAW and rename $destination before installing another version."}
Copy-Item -LiteralPath $source -Destination $destination -Recurse
Write-Host "Installed to $destination"
Write-Host 'Add this VST3 path in your DAW if necessary, then rescan plugins.'
