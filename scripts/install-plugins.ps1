param([ValidateSet('VST3','CLAP')][string[]]$Formats=@('VST3','CLAP'),[switch]$Update)
$ErrorActionPreference='Stop'
$running=Get-Process | Where-Object { !$_.HasExited -and $_.ProcessName -match '^(Cubase|Nuendo|Studio One|Studio Pro|Ableton|Live|FL64|FL|reaper|Bitwig|ProTools)' }
if($running){throw 'Save projects and close your DAWs before installation. No files changed.'}
$items=@()
foreach($format in $Formats){
 $name=if($format -eq 'VST3'){'CueFlow.vst3'}else{'CueFlow.clap'}
 $source=Join-Path $PSScriptRoot $name
 if(!(Test-Path -LiteralPath $source)){$source=Join-Path (Split-Path $PSScriptRoot -Parent) "build/native/CueFlowPlugin_artefacts/Release/$format/$name"}
 if(!(Test-Path -LiteralPath $source)){throw "Missing artifact: $source"}
 $system=Join-Path $env:CommonProgramFiles "$format/$name"
 $user=Join-Path $env:LOCALAPPDATA "Programs/Common/$format/$name"
 if((Test-Path -LiteralPath $system) -and (Test-Path -LiteralPath $user)){throw "Duplicate $format installations exist. Resolve duplicates first."}
 $target=if(Test-Path -LiteralPath $system){$system}else{$user}
 if((Test-Path -LiteralPath $target) -and !$Update){throw "Existing installation preserved: $target. Use -Update after closing DAWs."}
 $items+=@{Source=$source;Target=$target;Name=$name}
}
$backup=Join-Path $env:LOCALAPPDATA ('AmuMusic/CueFlow/plugin-backups/'+(Get-Date -Format 'yyyyMMdd-HHmmss-fff'))
foreach($item in $items){
 if(Test-Path -LiteralPath $item.Target){New-Item -ItemType Directory -Path $backup -Force | Out-Null;Copy-Item -LiteralPath $item.Target -Destination $backup -Recurse}
 New-Item -ItemType Directory -Path (Split-Path $item.Target -Parent) -Force | Out-Null
 if(Test-Path -LiteralPath $item.Source -PathType Container){
  if(!(Test-Path -LiteralPath $item.Target)){New-Item -ItemType Directory -Path $item.Target | Out-Null}
  Copy-Item -Path (Join-Path $item.Source '*') -Destination $item.Target -Recurse -Force
 }else{Copy-Item -LiteralPath $item.Source -Destination $item.Target -Force}
 Write-Host "Installed: $($item.Target)"
}
Write-Host 'Rescan plugins in your DAW. Use only one format per track. See HOST-COMPATIBILITY.md for tested capabilities.'
