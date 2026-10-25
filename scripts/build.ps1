param([string]$Configuration='Release', [string[]]$Targets=@('CueFlowTests','CueFlowKaraokeTests','CueFlowIntegration','CueFlowPlugin_VST3','CueFlowPlugin_CLAP','CueFlowClapSmoke','CueFlowStandalone'))
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
Push-Location $root
try {
 $cmake=Join-Path $root 'third_party/build-tools/cmake/data/bin/cmake.exe'
 if(!(Test-Path $cmake)){$cmake=(Get-Command cmake -ErrorAction Stop).Source}
 & $cmake -S . -B build/native -G 'Visual Studio 17 2022' -A x64
 if($LASTEXITCODE -ne 0){throw 'CMake configuration failed'}
 # JUCE's helper consumes narrow argv on Windows. Use relative arguments in generated build commands.
 $prefix=($root.Replace('\','/')+'/build/native/')
 foreach($project in Get-ChildItem 'build/native/*_rc_lib.vcxproj'){
  [xml]$xml=Get-Content -LiteralPath $project.FullName -Raw -Encoding utf8
  foreach($command in $xml.SelectNodes('//*[local-name()="Command"]')){
   if($command.InnerText.Contains(' rcfile ')){$command.InnerText=$command.InnerText.Replace($prefix,'')}
   $command.InnerText=$command.InnerText.Replace($root.Replace('\','/'),'../..').Replace($root,'..\..')
  }
  $xml.Save($project.FullName)
 }
 & $cmake --build build/native --config $Configuration --target @Targets -j 4
 if($LASTEXITCODE -ne 0){throw 'Build failed'}
} finally {Pop-Location}
