# coreMQTT Update Script
# This script updates the coreMQTT library from the official AWS repository
# Usage: .\update_coreMQTT.ps1

# Configuration
$repoUrl = "https://github.com/FreeRTOS/coreMQTT.git"
$branch = "main"
$tempDir = "temp_coreMQTT"
$targetDir = "csrc"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "coreMQTT Library Update Script" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Check if git is available
if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
    Write-Host "Error: git is not installed or not in PATH" -ForegroundColor Red
    exit 1
}

# Remove temp directory if it exists
if (Test-Path $tempDir) {
    Write-Host "Removing existing temp directory..." -ForegroundColor Yellow
    Remove-Item -Path $tempDir -Recurse -Force
}

# Clone the repository
Write-Host "Cloning coreMQTT repository..." -ForegroundColor Green
git clone --depth 1 --branch $branch $repoUrl $tempDir

if ($LASTEXITCODE -ne 0) {
    Write-Host "Error: Failed to clone repository" -ForegroundColor Red
    exit 1
}

# Create target directory if it doesn't exist
if (-not (Test-Path $targetDir)) {
    New-Item -ItemType Directory -Path $targetDir | Out-Null
}

# Copy source files
Write-Host "Copying source files..." -ForegroundColor Green
$sourceFiles = @(
    "source\core_mqtt.c",
    "source\core_mqtt_serializer.c",
    "source\core_mqtt_state.c"
)

foreach ($file in $sourceFiles) {
    $srcPath = Join-Path $tempDir $file
    if (Test-Path $srcPath) {
        Copy-Item $srcPath $targetDir -Force
        Write-Host "  Copied: $file" -ForegroundColor Gray
    } else {
        Write-Host "  Warning: $file not found" -ForegroundColor Yellow
    }
}

# Copy header files from include directory
Write-Host "Copying header files from include..." -ForegroundColor Green
$includeDir = Join-Path $tempDir "source\include"
if (Test-Path $includeDir) {
    Get-ChildItem -Path $includeDir -Filter "*.h" | ForEach-Object {
        Copy-Item $_.FullName $targetDir -Force
        Write-Host "  Copied: source\include\$($_.Name)" -ForegroundColor Gray
    }
}

# Copy header files from interface directory
Write-Host "Copying header files from interface..." -ForegroundColor Green
$interfaceDir = Join-Path $tempDir "source\interface"
if (Test-Path $interfaceDir) {
    Get-ChildItem -Path $interfaceDir -Filter "*.h" | ForEach-Object {
        Copy-Item $_.FullName $targetDir -Force
        Write-Host "  Copied: source\interface\$($_.Name)" -ForegroundColor Gray
    }
}

# Preserve core_mqtt_config.h if it exists
$configFile = Join-Path $targetDir "core_mqtt_config.h"
if (Test-Path $configFile) {
    Write-Host "Preserving existing core_mqtt_config.h" -ForegroundColor Cyan
} else {
    Write-Host "Note: core_mqtt_config.h was not found in target directory" -ForegroundColor Yellow
    Write-Host "      You may need to create it based on core_mqtt_config_defaults.h" -ForegroundColor Yellow
}

# Clean up
Write-Host "Cleaning up..." -ForegroundColor Green
Remove-Item -Path $tempDir -Recurse -Force

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Update completed successfully!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Files copied to: $targetDir" -ForegroundColor White
Write-Host ""
Write-Host "Important Notes:" -ForegroundColor Yellow
Write-Host "1. Review core_mqtt_config.h for any new configuration options" -ForegroundColor White
Write-Host "2. Check the CHANGELOG.md in the upstream repository for breaking changes" -ForegroundColor White
Write-Host "3. Test the integration thoroughly before deploying" -ForegroundColor White
Write-Host "4. Update your CMakeLists.txt if new source files were added" -ForegroundColor White
Write-Host ""
