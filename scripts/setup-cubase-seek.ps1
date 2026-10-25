$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
$driver=Join-Path $root 'integrations/cubase/AmuMusic/Lyra/AmuMusic_Lyra.js'
if(!(Test-Path -LiteralPath $driver)){$driver=Join-Path $PSScriptRoot 'integrations/cubase/AmuMusic/Lyra/AmuMusic_Lyra.js'}
if(!(Test-Path -LiteralPath $driver)){throw 'Lyra MIDI Remote script not found.'}
$destination=Join-Path ([Environment]::GetFolderPath('MyDocuments')) 'Steinberg/Cubase/MIDI Remote/Driver Scripts/Local/AmuMusic/Lyra'
New-Item -ItemType Directory -Force $destination | Out-Null
Copy-Item -LiteralPath $driver -Destination $destination -Force
$loop=Join-Path ${env:ProgramFiles(x86)} 'Tobias Erichsen/loopMIDI/loopMIDI.exe'
if(Test-Path -LiteralPath $loop){
 if(!(Get-Process loopMIDI -ErrorAction SilentlyContinue)){
  $ports='HKCU:/Software/Tobias Erichsen/loopMIDI/Ports'
  if(!(Test-Path -LiteralPath $ports)){New-Item -Path $ports -Force | Out-Null}
  foreach($name in @('Lyra Control','Lyra Feedback')){New-ItemProperty -Path $ports -Name $name -PropertyType DWord -Value 1 -Force | Out-Null}
  Start-Process -FilePath $loop -WindowStyle Hidden
 }else{Write-Host 'loopMIDI is already running. Verify it has Lyra Control and Lyra Feedback ports.'}
}else{Write-Host 'Install loopMIDI from its official vendor and create Lyra Control and Lyra Feedback ports.'}
Write-Host 'Driver installed. Restart Cubase, select Lyra in MIDI Remote, and set primary or secondary time display to Seconds. See LYRIC-SEEK.md.'
Write-Host 'Cubase MIDI Port Setup: for INPUT Lyra Feedback, uncheck BOTH Visible and In All MIDI Inputs. Keep INPUT Lyra Control and OUTPUT Lyra Feedback enabled for MIDI Remote.'
