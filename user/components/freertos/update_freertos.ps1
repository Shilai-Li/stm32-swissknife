<#
.SYNOPSIS
    Updates the FreeRTOS Kernel library from a local Git repository or downloads it.

.DESCRIPTION
    This script updates the FreeRTOS kernel source files in the local component directory.
    It copies the core kernel files, include headers, and the specific portable files
    for Cortex-M3 (STM32F103) using the GCC compiler. It also copies heap_4.c.

.PARAMETER SourcePath
    Optional. Path to the local FreeRTOS-Kernel repository.
    If not provided, the script will look in temporary download directories.

.EXAMPLE
    .\update_freertos.ps1 -SourcePath "C:\Downloads\FreeRTOS-Kernel"
#>

param (
    [string]$SourcePath = ""
)

$ErrorActionPreference = "Stop"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$TargetDir = Join-Path $ScriptDir "csrc"

# Define source candidates if not provided
if ([string]::IsNullOrEmpty($SourcePath)) {
    $Candidates = @(
        "$ScriptDir\..\..\..\..\temp_downloads\FreeRTOS-Kernel",
        "$ScriptDir\..\..\..\..\temp_downloads\FreeRTOS"
    )
    
    foreach ($Candidate in $Candidates) {
        if (Test-Path $Candidate) {
            $SourcePath = $Candidate
            break
        }
    }
}

if (-not (Test-Path $SourcePath)) {
    Write-Error "Source path not found. Please clone the repository first or provide a valid path.`nUsage: .\update_freertos.ps1 -SourcePath <path_to_FreeRTOS_Kernel>"
}

Write-Host "Updating FreeRTOS Kernel from: $SourcePath"
Write-Host "Destination: $TargetDir"

# Ensure target directories exist
$DirsToCreate = @(
    "",
    "include",
    "portable\GCC\ARM_CM3",
    "portable\MemMang"
)

foreach ($Dir in $DirsToCreate) {
    $Path = Join-Path $TargetDir $Dir
    if (-not (Test-Path $Path)) {
        New-Item -ItemType Directory -Path $Path -Force | Out-Null
    }
}

# 1. Copy Core Source Files
$CoreFiles = @("tasks.c", "queue.c", "list.c", "timers.c", "event_groups.c", "stream_buffer.c")
foreach ($File in $CoreFiles) {
    $SrcFile = Join-Path $SourcePath $File
    if (Test-Path $SrcFile) {
        Copy-Item -Path $SrcFile -Destination $TargetDir -Force
        Write-Host "  [+] Copied $File"
    } else {
        Write-Warning "Core file not found: $File"
    }
}

# 2. Copy Headers
$IncludeDir = Join-Path $SourcePath "include"
Copy-Item -Path "$IncludeDir\*.h" -Destination (Join-Path $TargetDir "include") -Force
Write-Host "  [+] Copied headers from include/"

# 3. Copy Portable Files (GCC / ARM_CM3)
$PortableSrc = Join-Path $SourcePath "portable\GCC\ARM_CM3"
$PortableDst = Join-Path $TargetDir "portable\GCC\ARM_CM3"

if (Test-Path $PortableSrc) {
    Copy-Item -Path "$PortableSrc\*.*" -Destination $PortableDst -Force
    Write-Host "  [+] Copied portable layer (GCC/ARM_CM3)"
} else {
    Write-Error "Portable layer not found at $PortableSrc"
}

# 4. Copy Memory Manager (heap_4.c)
$HeapSrc = Join-Path $SourcePath "portable\MemMang\heap_4.c"
$HeapDst = Join-Path $TargetDir "portable\MemMang"
if (Test-Path $HeapSrc) {
    Copy-Item -Path $HeapSrc -Destination $HeapDst -Force
    Write-Host "  [+] Copied heap_4.c"
} else {
    Write-Warning "heap_4.c not found"
}

Write-Host "FreeRTOS update complete!"
