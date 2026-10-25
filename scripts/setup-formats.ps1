$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
$path=Join-Path $root 'third_party/clap-juce-extensions'
$revision='7adee3a1bd4684d4caa5601100e364abccff4b4d'
$archive=Join-Path $root 'third_party/clap-juce-extensions-source.zip'
if(!(Test-Path -LiteralPath $path) -and (Test-Path -LiteralPath $archive)){Expand-Archive -LiteralPath $archive -DestinationPath (Join-Path $root 'third_party')}
if((Test-Path "$path/.cueflow-revision") -and ((Get-Content "$path/.cueflow-revision" -Raw).Trim() -eq $revision) -and (Test-Path "$path/clap-libs/clap/include/clap/clap.h")){return}
if(!(Test-Path -LiteralPath $path)){
 & git clone https://github.com/free-audio/clap-juce-extensions.git $path
 if($LASTEXITCODE){throw 'CLAP clone failed'}
}
if((& git -C $path status --porcelain)){throw 'CLAP dependency has local changes; preserved without modification.'}
& git -C $path checkout --detach $revision
if($LASTEXITCODE){throw 'Pinned CLAP revision unavailable'}
& git -C $path submodule update --init --recursive
if($LASTEXITCODE){throw 'CLAP dependencies failed'}
Write-Host "CLAP ready: $revision"
