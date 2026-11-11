<#
.SYNOPSIS
  Builds dummy_ecu. Uses CMake if available, otherwise calls g++/gcc directly.
.EXAMPLE
  powershell -ExecutionPolicy Bypass -File .\build.ps1
.EXAMPLE
  powershell -ExecutionPolicy Bypass -File .\build.ps1 -Run
#>
param([switch]$Run, [switch]$Clean)
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
elseif (Get-Command g++ -ErrorAction SilentlyContinue) {
    Write-Host 'CMake not found - building with g++ directly.'
    $flags = @('-std=c++17', '-g', '-Wall', '-Wextra', '-Iinclude')
    Invoke-Checked gcc @('-std=c11', '-g', '-Wall', '-Wextra', '-Iinclude', '-c', 'src_c/crc8.c', '-o', 'build/crc8.o')
    Invoke-Checked g++ ($flags + @('src/main.cpp', 'src/sensor.cpp', 'src/stats.cpp', 'src/controller.cpp', 'build/crc8.o', '-o', 'build/dummy_ecu.exe'))
    $exePath = (Resolve-Path build/dummy_ecu.exe).Path
}
else {
    throw 'No compiler found. Install CMake + a C++ compiler (see README.md).'
}

Write-Host "OK  Built $exePath" -ForegroundColor Green
if ($Run) { & $exePath }
