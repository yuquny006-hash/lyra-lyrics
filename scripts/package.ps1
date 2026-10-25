$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
$destination=Join-Path $root 'dist/CueFlow-0.5.4-Windows-x64'
New-Item -ItemType Directory -Force $destination | Out-Null
$app=Join-Path $root 'build/native/CueFlowStandalone_artefacts/Release/CueFlow.exe'
$plugin=Join-Path $root 'build/native/CueFlowPlugin_artefacts/Release/VST3/CueFlow.vst3'
$clap=Join-Path $root 'build/native/CueFlowPlugin_artefacts/Release/CLAP/CueFlow.clap'
if(!(Test-Path -LiteralPath $clap)){throw 'Build CLAP Release first.'}
Copy-Item -LiteralPath $clap -Destination $destination -Force
if(!(Test-Path -LiteralPath $app) -or !(Test-Path -LiteralPath $plugin)){throw 'Build Release app and VST3 first.'}
Copy-Item -LiteralPath $app -Destination $destination -Force
if(!(Test-Path (Join-Path $destination 'CueFlow.vst3'))){Copy-Item -LiteralPath $plugin -Destination $destination -Recurse}else{Copy-Item -Path (Join-Path $plugin '*') -Destination (Join-Path $destination 'CueFlow.vst3') -Recurse -Force}
foreach($folder in @('samples','LICENSES','integrations')){Copy-Item -LiteralPath (Join-Path $root $folder) -Destination $destination -Recurse -Force}
foreach($name in @('README.md','THIRD-PARTY-NOTICES.md')){Copy-Item -LiteralPath (Join-Path $root $name) -Destination $destination -Force}
Copy-Item -LiteralPath (Join-Path $root 'scripts/install-vst3.ps1') -Destination $destination -Force
Copy-Item -LiteralPath (Join-Path $root 'scripts/install-plugins.ps1') -Destination $destination -Force
Copy-Item -LiteralPath (Join-Path $root 'docs/HOST-COMPATIBILITY.md') -Destination $destination -Force
Copy-Item -LiteralPath (Join-Path $root 'scripts/update-vst3.ps1') -Destination $destination -Force
Copy-Item -LiteralPath (Join-Path $root 'scripts/setup-alignment.ps1') -Destination $destination -Force
Copy-Item -LiteralPath (Join-Path $root 'scripts/setup-cubase-seek.ps1') -Destination $destination -Force
$releaseEngine=Join-Path $destination 'engine'
New-Item -ItemType Directory -Force $releaseEngine | Out-Null
Copy-Item -LiteralPath (Join-Path $root 'engine/align.py') -Destination $releaseEngine -Force
Copy-Item -LiteralPath (Join-Path $root 'docs/CUBASE-QUICKSTART.md') -Destination $destination -Force
if(Test-Path (Join-Path $root 'docs/TEST-REPORT.md')){Copy-Item -LiteralPath (Join-Path $root 'docs/TEST-REPORT.md') -Destination $destination -Force}
Copy-Item -LiteralPath (Join-Path $root 'docs/LYRIC-SEEK.md') -Destination $destination -Force
$source=Join-Path $destination 'Source'
New-Item -ItemType Directory -Force $source | Out-Null
foreach($folder in @('src','tests','scripts','docs','samples','LICENSES','integrations','.github')){Copy-Item -LiteralPath (Join-Path $root $folder) -Destination $source -Recurse -Force}
foreach($name in @('README.md','THIRD-PARTY-NOTICES.md','LICENSE','.gitignore','.gitattributes')){Copy-Item -LiteralPath (Join-Path $root $name) -Destination $source -Force}
Copy-Item -LiteralPath (Join-Path $root 'CMakeLists.txt') -Destination $source -Force
$sourceEngine=Join-Path $source 'engine'
New-Item -ItemType Directory -Force $sourceEngine | Out-Null
Copy-Item -LiteralPath (Join-Path $root 'engine/align.py') -Destination $sourceEngine -Force
$references=Join-Path $source 'research/format-reference'
New-Item -ItemType Directory -Force $references | Out-Null
Copy-Item -Path (Join-Path $root 'research/format-reference/*.py') -Destination $references -Force
$dependencies=Join-Path $source 'third_party'
New-Item -ItemType Directory -Force $dependencies | Out-Null
Copy-Item -LiteralPath (Join-Path $root 'third_party/juce.zip') -Destination (Join-Path $dependencies 'JUCE-8.0.12-source.zip') -Force
& (Join-Path $root 'scripts/archive-clap.ps1') -Destination (Join-Path $dependencies 'clap-juce-extensions-source.zip')
Compress-Archive -Path (Join-Path $destination '*') -DestinationPath (Join-Path $root 'dist/CueFlow-0.5.4-Windows-x64.zip') -Force
Get-FileHash -LiteralPath $app -Algorithm SHA256
Write-Host "Packaged to $destination"
