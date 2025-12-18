# MultiTimer Update Script
# Usage: .\update_multitimer.ps1 -SourcePath "path\to\MultiTimer\repo"

param(
    [string]$SourcePath
)

$ScriptDir = $PSScriptRoot
$CsrcDir = Join-Path $ScriptDir "csrc"

if (-not $SourcePath) {
    Write-Host "Please provide -SourcePath to the MultiTimer repository."
    Write-Host "Example: git clone https://github.com/0x1abin/MultiTimer.git"
    exit 1
}

if (-not (Test-Path "$SourcePath\MultiTimer.c")) {
    Write-Error "MultiTimer.c not found in source path."
    exit 1
}

# Copy files
Copy-Item "$SourcePath\MultiTimer.c" -Destination $CsrcDir -Force
Copy-Item "$SourcePath\MultiTimer.h" -Destination $CsrcDir -Force

Write-Host "MultiTimer updated successfully."
