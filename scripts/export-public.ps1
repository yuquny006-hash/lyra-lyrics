param([string]$Destination='.local/github-source')
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
$target=[IO.Path]::GetFullPath((Join-Path $root $Destination))
$boundary=[IO.Path]::GetFullPath((Join-Path $root '.local'))+[IO.Path]::DirectorySeparatorChar
if(!$target.StartsWith($boundary,[StringComparison]::OrdinalIgnoreCase)){throw 'Export must remain under workspace .local/'}
New-Item -ItemType Directory -Path $target -Force | Out-Null
$files=@('.gitignore','.gitattributes','CMakeLists.txt','README.md','THIRD-PARTY-NOTICES.md','LICENSE')
foreach($folder in @('src','tests','engine','integrations','samples','LICENSES','scripts','.github')){
 $files+=Get-ChildItem -LiteralPath (Join-Path $root $folder) -File -Recurse | Where-Object {$_.FullName -notmatch '[\\/]__pycache__[\\/]'} | ForEach-Object {$_.FullName.Substring($root.Length+1)}
}
$files+=@('docs/CUBASE-QUICKSTART.md','docs/LYRIC-SEEK.md','docs/HOST-COMPATIBILITY.md','docs/MAC-INSTALL.md')
$files+=Get-ChildItem (Join-Path $root 'research/format-reference/*.py') | ForEach-Object {$_.FullName.Substring($root.Length+1)}
foreach($relative in $files){
 $source=Join-Path $root $relative
 $out=Join-Path $target $relative
 New-Item -ItemType Directory -Path (Split-Path $out -Parent) -Force | Out-Null
 Copy-Item -LiteralPath $source -Destination $out -Force
}
Write-Host "Exported $($files.Count) source files to $target. Original Git history was not copied."
