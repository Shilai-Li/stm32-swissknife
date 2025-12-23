<#
.SYNOPSIS
Updates the FlexibleButton library from a local source directory.

.DESCRIPTION
This script copies the necessary FlexibleButton source files from a specified source directory
to the component's csrc directory. It preserves the directory structure required by the component.

.PARAMETER SourcePath
The path to the root of the FlexibleButton repository (containing flexible_button.c/h).

.EXAMPLE
.\update_flexible_button.ps1 -SourcePath "C:\Downloads\FlexibleButton"
#>

param (
    [Parameter(Mandatory=$true)]
    [string]$SourcePath
)

$ErrorActionPreference = "Stop"

# Configuration
$ComponentPath = $PSScriptRoot
$DestPath = Join-Path $ComponentPath "csrc"

# Verify source path
if (-not (Test-Path $SourcePath)) {
    Write-Error "Source path does not exist: $SourcePath"
}

# Ensure destination directory exists
if (-not (Test-Path $DestPath)) {
    New-Item -ItemType Directory -Path $DestPath -Force | Out-Null
}

# File copy list (Relative to SourcePath -> Relative to DestPath)
$FilesToCopy = @{
    "flexible_button.c" = "flexible_button.c"
    "flexible_button.h" = "flexible_button.h"
}

Write-Host "Updating FlexibleButton library..." -ForegroundColor Cyan
Write-Host "Source: $SourcePath"
Write-Host "Destination: $DestPath"

foreach ($SrcFile in $FilesToCopy.Keys) {
    $FullSrcPath = Join-Path $SourcePath $SrcFile
    $DestFileName = $FilesToCopy[$SrcFile]
    $FullDestPath = Join-Path $DestPath $DestFileName
    
    if (Test-Path $FullSrcPath) {
        Copy-Item -Path $FullSrcPath -Destination $FullDestPath -Force
        Write-Host "  [+] Copied $SrcFile" -ForegroundColor Green
    } else {
        Write-Warning "  [-] Source file not found: $SrcFile"
    }
}

Write-Host "FlexibleButton update complete!" -ForegroundColor Cyan
