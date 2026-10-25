$ErrorActionPreference='Stop'
$source=Join-Path $PSScriptRoot 'CueFlow.vst3'
if(!(Test-Path -LiteralPath $source)){throw 'CueFlow.vst3 must be next to this script.'}
$hosts=Get-Process | Where-Object {($_.ProcessName -match '^(Cubase|Nuendo|Studio One|Studio Pro)') -and -not $_.HasExited}
if($hosts){throw 'Save your projects and close Cubase / Nuendo / Studio One before updating. No files changed.'}
$system=Join-Path $env:CommonProgramFiles 'VST3/CueFlow.vst3'
$user=Join-Path $env:LOCALAPPDATA 'Programs/Common/VST3/CueFlow.vst3'
if((Test-Path -LiteralPath $system) -and (Test-Path -LiteralPath $user)){throw 'Two CueFlow installations found. Keep one VST3 location before updating.'}
$destination=if(Test-Path -LiteralPath $system){$system}else{$user}
if(Test-Path -LiteralPath $destination){
 $backup=Join-Path $env:LOCALAPPDATA ('AmuMusic/CueFlow/plugin-backups/'+(Get-Date -Format 'yyyyMMdd-HHmmss'))
 New-Item -ItemType Directory -Path $backup -Force | Out-Null
 Copy-Item -LiteralPath $destination -Destination $backup -Recurse
 Write-Host "Backup: $backup"
}else{New-Item -ItemType Directory -Path $destination -Force | Out-Null}
Copy-Item -Path (Join-Path $source '*') -Destination $destination -Recurse -Force
Write-Host "Updated CueFlow 0.5.4: $destination"
Write-Host 'Reopen Cubase and rescan VST plug-ins. If access is denied, run PowerShell as administrator.'
