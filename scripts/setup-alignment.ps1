param([string]$EngineDirectory='',[string]$ModelDirectory='')
$ErrorActionPreference='Stop'
if(!$EngineDirectory){$EngineDirectory=Join-Path $env:APPDATA 'AmuMusic/CueFlow/engine'}
$engineRoot=[IO.Path]::GetFullPath($EngineDirectory)
New-Item -ItemType Directory -Force -Path $engineRoot | Out-Null
$pythonRoot=Join-Path $engineRoot 'python'
$python=Join-Path $pythonRoot 'python.exe'
if(!(Test-Path -LiteralPath $python)){
 $archive=Join-Path $engineRoot 'python-3.12.10-embed-amd64.zip'
 Invoke-WebRequest 'https://www.python.org/ftp/python/3.12.10/python-3.12.10-embed-amd64.zip' -OutFile $archive
 Expand-Archive -LiteralPath $archive -DestinationPath $pythonRoot -Force
}
@('python312.zip','.','Lib/site-packages','import site') | Set-Content -LiteralPath (Join-Path $pythonRoot 'python312._pth') -Encoding ascii
& $python -m pip --version 2>$null
if($LASTEXITCODE -ne 0){
 $bootstrap=Join-Path $engineRoot 'get-pip.py'
 Invoke-WebRequest 'https://bootstrap.pypa.io/get-pip.py' -OutFile $bootstrap
 & $python $bootstrap --disable-pip-version-check
 if($LASTEXITCODE -ne 0){throw 'pip bootstrap failed'}
}
& $python -m pip install torch==2.5.1 torchaudio==2.5.1 --index-url https://download.pytorch.org/whl/cpu --disable-pip-version-check
if($LASTEXITCODE -ne 0){throw 'PyTorch installation failed'}
& $python -m pip install stable-ts==2.19.1 openai-whisper==20250625 soundfile==0.13.1 numpy==2.5.4 numba==0.68.0 llvmlite==0.50.0 --disable-pip-version-check
if($LASTEXITCODE -ne 0){throw 'Alignment dependencies installation failed'}
$source=Join-Path (Split-Path $PSScriptRoot -Parent) 'engine/align.py'
if(!(Test-Path -LiteralPath $source)){$source=Join-Path $PSScriptRoot 'engine/align.py'}
if(!(Test-Path -LiteralPath $source)){throw 'engine/align.py not found next to release package or source tree.'}
$worker=Join-Path $engineRoot 'align.py'
Copy-Item -LiteralPath $source -Destination $worker -Force
if(!$ModelDirectory){$ModelDirectory=Join-Path $engineRoot 'models'}
$models=[IO.Path]::GetFullPath($ModelDirectory)
New-Item -ItemType Directory -Force -Path $models | Out-Null
$configFolder=Join-Path $env:APPDATA 'AmuMusic/CueFlow'
New-Item -ItemType Directory -Force -Path $configFolder | Out-Null
@{python=$python;script=$worker;models=$models;version=1} | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $configFolder 'alignment.json') -Encoding utf8
& $python -c 'import stable_whisper,torch,soundfile;print("CueFlow local alignment engine ready")'
if($LASTEXITCODE -ne 0){throw 'Alignment import check failed'}
Write-Output 'Setup complete. The small model is downloaded on first alignment; audio stays on this computer.'
