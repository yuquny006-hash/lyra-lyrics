$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
Push-Location $root
try {
 $cmake=Join-Path $root 'third_party/build-tools/cmake/data/bin/cmake.exe'
 if(!(Test-Path $cmake)){$cmake=(Get-Command cmake -ErrorAction Stop).Source}
 & $cmake -S . -B build/aax-development -G 'Visual Studio 17 2022' -A x64 -DCUEFLOW_BUILD_AAX=ON -DCUEFLOW_BUILD_CLAP=OFF
 if($LASTEXITCODE){throw 'AAX configuration failed'}
 # JUCE resource helper requires relative paths in non-ASCII workspaces.
 foreach($project in Get-ChildItem 'build/aax-development/*_rc_lib.vcxproj'){
  [xml]$xml=Get-Content -LiteralPath $project.FullName -Raw -Encoding utf8
  foreach($command in $xml.SelectNodes('//*[local-name()="Command"]')){
   $command.InnerText=$command.InnerText.Replace($root.Replace('\','/'),'../..').Replace($root,'..\..')
  }
  $xml.Save($project.FullName)
 }
 & $cmake --build build/aax-development --config Release --target CueFlowPlugin_AAX -j 4
 if($LASTEXITCODE){throw 'AAX development build failed'}
 Write-Host 'UNSIGNED development AAX only. Avid/PACE signing and Pro Tools testing are required before release.'
}finally{Pop-Location}
