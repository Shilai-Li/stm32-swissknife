# cJSON Update Script
# 用于从官方 cJSON 仓库更新此组件
#
# 使用方法:
#   1. 下载最新 cJSON: git clone https://github.com/DaveGamble/cJSON.git
#      或下载 release: https://github.com/DaveGamble/cJSON/releases
#   2. 运行此脚本: .\update_cjson.ps1 -SourcePath "path\to\cJSON"
#
# 脚本会:
#   - 备份当前的 csrc/ (如果需要)
#   - 替换 csrc/ 目录中的核心文件 (cJSON.c, cJSON.h)
#   - 保留你的包装头文件 cjson.h
#
# 注意: cJSON 官方仓库结构很简单:
#   cJSON/
#   ├── cJSON.c         ← 我们需要这个
#   ├── cJSON.h         ← 我们需要这个
#   ├── cJSON_Utils.c   ← 可选 (JSON Patch/Pointer 支持)
#   ├── cJSON_Utils.h   ← 可选
#   └── ...

param(
    [Parameter(Mandatory=$true)]
    [string]$SourcePath,
    
    [Parameter(Mandatory=$false)]
    [switch]$IncludeUtils = $false
)

$ScriptDir = $PSScriptRoot
$BackupDir = Join-Path $ScriptDir "backup_$(Get-Date -Format 'yyyyMMdd_HHmmss')"
$CsrcDir = Join-Path $ScriptDir "csrc"

Write-Host "=== cJSON Update Script ===" -ForegroundColor Cyan
Write-Host "Source: $SourcePath"
Write-Host "Target: $ScriptDir"
Write-Host "Include Utils: $IncludeUtils"
Write-Host ""

# Validate source - check for cJSON.c and cJSON.h
if (-not (Test-Path (Join-Path $SourcePath "cJSON.c"))) {
    Write-Error "Invalid cJSON source path. 'cJSON.c' not found."
    Write-Host "Make sure you have the official cJSON repository or release:"
    Write-Host "  git clone https://github.com/DaveGamble/cJSON.git"
    Write-Host "  or download from: https://github.com/DaveGamble/cJSON/releases"
    exit 1
}

if (-not (Test-Path (Join-Path $SourcePath "cJSON.h"))) {
    Write-Error "Invalid cJSON source path. 'cJSON.h' not found."
    exit 1
}

# 1. Create backup of csrc
Write-Host "[1/4] Creating backup..." -ForegroundColor Yellow
if (Test-Path $CsrcDir) {
    New-Item -ItemType Directory -Path $BackupDir -Force | Out-Null
    Copy-Item -Path $CsrcDir -Destination $BackupDir -Recurse
    Write-Host "   Backup saved to: $BackupDir" -ForegroundColor Green
} else {
    Write-Host "   No existing csrc folder to backup." -ForegroundColor DarkGray
    New-Item -ItemType Directory -Path $CsrcDir -Force | Out-Null
}

# 2. Remove old source files
Write-Host "[2/4] Removing old source files..." -ForegroundColor Yellow
if (Test-Path $CsrcDir) {
    Remove-Item -Path (Join-Path $CsrcDir "*") -Force -ErrorAction SilentlyContinue
}
Write-Host "   Done." -ForegroundColor Green

# 3. Copy new source files
Write-Host "[3/4] Copying new source files..." -ForegroundColor Yellow

# Core files (always copy)
Copy-Item -Path (Join-Path $SourcePath "cJSON.c") -Destination $CsrcDir -Force
Copy-Item -Path (Join-Path $SourcePath "cJSON.h") -Destination $CsrcDir -Force
Write-Host "   Copied: cJSON.c, cJSON.h" -ForegroundColor Green

# Optional Utils files
if ($IncludeUtils) {
    $utilsC = Join-Path $SourcePath "cJSON_Utils.c"
    $utilsH = Join-Path $SourcePath "cJSON_Utils.h"
    
    if ((Test-Path $utilsC) -and (Test-Path $utilsH)) {
        Copy-Item -Path $utilsC -Destination $CsrcDir -Force
        Copy-Item -Path $utilsH -Destination $CsrcDir -Force
        Write-Host "   Copied: cJSON_Utils.c, cJSON_Utils.h" -ForegroundColor Green
    } else {
        Write-Host "   Warning: cJSON_Utils files not found in source." -ForegroundColor Yellow
    }
}

# Get version from header
$headerPath = Join-Path $CsrcDir "cJSON.h"
$headerContent = Get-Content $headerPath -Raw
$versionMajor = [regex]::Match($headerContent, 'CJSON_VERSION_MAJOR\s+(\d+)').Groups[1].Value
$versionMinor = [regex]::Match($headerContent, 'CJSON_VERSION_MINOR\s+(\d+)').Groups[1].Value
$versionPatch = [regex]::Match($headerContent, 'CJSON_VERSION_PATCH\s+(\d+)').Groups[1].Value
$version = "$versionMajor.$versionMinor.$versionPatch"

Write-Host "   Version: $version" -ForegroundColor Cyan

# 4. Reminder
Write-Host "[4/4] Post-update tasks..." -ForegroundColor Yellow
Write-Host ""
Write-Host "=== UPDATE COMPLETE ===" -ForegroundColor Green
Write-Host ""
Write-Host "cJSON has been updated to version $version" -ForegroundColor Cyan
Write-Host ""
Write-Host "IMPORTANT: Please review the following:" -ForegroundColor Cyan
Write-Host ""
Write-Host "  1. Check the cJSON changelog for any breaking changes:"
Write-Host "     https://github.com/DaveGamble/cJSON/blob/master/CHANGELOG.md"
Write-Host ""
Write-Host "  2. Your wrapper header was preserved:"
Write-Host "     - cjson.h"
Write-Host ""
Write-Host "  3. If you use cJSON_Utils and didn't include them, run:"
Write-Host "     .\update_cjson.ps1 -SourcePath `"$SourcePath`" -IncludeUtils"
Write-Host ""
if (Test-Path $BackupDir) {
    Write-Host "Backup location: $BackupDir" -ForegroundColor DarkGray
}
