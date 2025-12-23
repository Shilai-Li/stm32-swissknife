# update_nanomodbus.ps1
# Script to update nanoMODBUS library from official repository

param(
    [string]$SourceDir = "..\..\..\example\nanoMODBUS-master",
    [switch]$Help
)

if ($Help) {
    Write-Host @"
Usage: .\update_nanomodbus.ps1 [-SourceDir <path>] [-Help]

This script updates the nanoMODBUS library from the official repository.

Parameters:
  -SourceDir  Path to the nanoMODBUS source directory (default: ..\..\..\example\nanoMODBUS-master)
  -Help       Show this help message

Example:
  .\update_nanomodbus.ps1
  .\update_nanomodbus.ps1 -SourceDir "C:\path\to\nanoMODBUS"

Before running this script:
  1. Clone or download nanoMODBUS from https://github.com/debevv/nanoMODBUS
  2. Place it in HAL/user/example/ or specify custom path with -SourceDir

"@
    exit 0
}

# Check if source directory exists
if (-not (Test-Path $SourceDir)) {
    Write-Error "Source directory not found: $SourceDir"
    Write-Host "Please clone nanoMODBUS from https://github.com/debevv/nanoMODBUS first"
    exit 1
}

Write-Host "Updating nanoMODBUS from: $SourceDir" -ForegroundColor Cyan

# Copy core files
Write-Host "Copying core files..." -ForegroundColor Green
$coreFiles = @(
    "nanomodbus.c",
    "nanomodbus.h"
)

foreach ($file in $coreFiles) {
    $src = Join-Path $SourceDir $file
    $dst = Join-Path "csrc" $file
    
    if (Test-Path $src) {
        Copy-Item $src $dst -Force
        Write-Host "  ✓ Copied: $file" -ForegroundColor Gray
    } else {
        Write-Warning "  ✗ File not found: $file"
    }
}

Write-Host "`nnanoMODBUS update complete!" -ForegroundColor Green
Write-Host "Note: Porting layer (nanomodbus_port.c/h) is NOT updated to preserve customizations." -ForegroundColor Yellow
