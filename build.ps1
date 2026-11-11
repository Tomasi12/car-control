<#
.SYNOPSIS
  Builds dummy_ecu (C99). Uses CMake + Ninja if available, otherwise calls gcc directly.
.PARAMETER Run
  Run the normal drive cycle after building.
.PARAMETER Fault
  Run with a planted defect (1..15). See README.md or "dummy_ecu.exe --list".
.PARAMETER Clean
  Delete build/ first.
.EXAMPLE
  powershell -ExecutionPolicy Bypass -File .\build.ps1 -Run
.EXAMPLE
  powershell -ExecutionPolicy Bypass -File .\build.ps1 -Fault 2
#>
param([switch]$Run, [int]$Fault = 0, [switch]$Clean)
$ErrorActionPreference = 'Stop'
Set-Location $PSScriptRoot

# Use the project-local toolchain in third_party/ first (falls back to whatever is on PATH).
$tpBin = Join-Path $PSScriptRoot 'third_party\mingw64\bin'
if (Test-Path $tpBin) {
    $env:PATH = "$tpBin;$env:PATH"
    Write-Host "Using toolchain from $tpBin"
}

if ($Clean -and (Test-Path build)) { Remove-Item -Recurse -Force build }
New-Item -ItemType Directory -Force build | Out-Null

function Invoke-Checked([string]$exe, [string[]]$argv) {
    & $exe @argv
    if ($LASTEXITCODE -ne 0) { throw "$exe failed (exit $LASTEXITCODE)" }
}

$exePath = $null
if (Get-Command cmake -ErrorAction SilentlyContinue) {
    $gen = @()
    if (Get-Command ninja -ErrorAction SilentlyContinue) { $gen = @('-G', 'Ninja') }
    elseif (Get-Command mingw32-make -ErrorAction SilentlyContinue) { $gen = @('-G', 'MinGW Makefiles') }
    Invoke-Checked cmake (@('-S', '.', '-B', 'build', '-DCMAKE_BUILD_TYPE=Debug') + $gen)
    Invoke-Checked cmake @('--build', 'build')
    $exePath = Get-ChildItem build -Recurse -Filter dummy_ecu.exe | Select-Object -First 1 -ExpandProperty FullName
}
elseif (Get-Command gcc -ErrorAction SilentlyContinue) {
    Write-Host 'CMake not found - building with gcc directly.'
    $sources = Get-ChildItem src -Filter *.c | ForEach-Object { $_.FullName }
    Invoke-Checked gcc (@('-std=c99', '-g', '-Wall', '-Wextra', '-Iinclude') + $sources + @('-o', 'build/dummy_ecu.exe'))
    $exePath = (Resolve-Path build/dummy_ecu.exe).Path
}
else {
    throw 'No compiler found. Get the toolchain into third_party/ (see third_party/README.md).'
}

Write-Host "OK  Built $exePath" -ForegroundColor Green
if ($Fault -gt 0) { & $exePath --fault $Fault; Write-Host "exit code $LASTEXITCODE" }
elseif ($Run) { & $exePath }
