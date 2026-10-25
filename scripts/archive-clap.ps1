param([Parameter(Mandatory=$true)][string]$Destination)
$ErrorActionPreference='Stop'
$root=Join-Path (Split-Path $PSScriptRoot -Parent) 'third_party/clap-juce-extensions'
$root=(Resolve-Path -LiteralPath $root).Path.TrimEnd('\','/')
Add-Type -AssemblyName System.IO.Compression
$stream=[IO.File]::Open($Destination,[IO.FileMode]::Create)
$zip=[IO.Compression.ZipArchive]::new($stream,[IO.Compression.ZipArchiveMode]::Create)
try {
 foreach($file in Get-ChildItem -LiteralPath $root -File -Recurse -Force | Where-Object { $_.FullName -notmatch '[\\/]\.git([\\/]|$)' }){
  $relative=$file.FullName.Substring($root.Length+1).Replace('\','/')
  $entry=$zip.CreateEntry('clap-juce-extensions/'+$relative)
  $output=$entry.Open();$input=$file.OpenRead()
  try {$input.CopyTo($output)} finally {$input.Dispose();$output.Dispose()}
 }
 $entry=$zip.CreateEntry('clap-juce-extensions/.cueflow-revision');$writer=[IO.StreamWriter]::new($entry.Open())
 try {$writer.Write('7adee3a1bd4684d4caa5601100e364abccff4b4d')}finally{$writer.Dispose()}
}finally{$zip.Dispose();$stream.Dispose()}
