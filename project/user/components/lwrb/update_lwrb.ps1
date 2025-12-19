# lwrb Update Script
# 用于从官方 lwrb 仓库更新此组件
#
# 使用方法:
#   1. 下载 lwrb: git clone https://github.com/MaJerle/lwrb.git
#   2. 运行此脚本: .\update_lwrb.ps1 -SourcePath "path\to\lwrb"
#
# 脚本会:
#   - 备份当前的 csrc/ (如果需要)
#   - 替换 csrc/ 目录中的核心文件
#   - 保留你的端口文件 lwrb_port.c/h
#
# 注意: lwrb 官方仓库结构:
#   lwrb/
#   ├── lwrb/src/
#   │   ├── lwrb/lwrb.c     ← 我们需要这个
#   │   └── include/lwrb/lwrb.h  ← 我们需要这个
#   └── ...

param(
    [Parameter(Mandatory=$true)]
    [string]$SourcePath
)

$ScriptDir = $PSScriptRoot
$BackupDir = Join-Path $ScriptDir "backup_$(Get-Date -Format 'yyyyMMdd_HHmmss')"
$CsrcDir = Join-Path $ScriptDir "csrc"

Write-Host "=== lwrb Update Script ===" -ForegroundColor Cyan
Write-Host "Source: $SourcePath"
Write-Host "Target: $ScriptDir"
Write-Host ""

# Validate source - check for lwrb.c and lwrb.h
$lwrbC = Join-Path $SourcePath "lwrb\src\lwrb\lwrb.c"
$lwrbH = Join-Path $SourcePath "lwrb\src\include\lwrb\lwrb.h"

if (-not (Test-Path $lwrbC)) {
    Write-Error "Invalid lwrb source path. 'lwrb/src/lwrb/lwrb.c' not found."
    Write-Host "Make sure you have the official lwrb repository:"
    Write-Host "  git clone https://github.com/MaJerle/lwrb.git"
    exit 1
}

if (-not (Test-Path $lwrbH)) {
    Write-Error "Invalid lwrb source path. 'lwrb/src/include/lwrb/lwrb.h' not found."
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

Copy-Item -Path $lwrbC -Destination $CsrcDir -Force
Copy-Item -Path $lwrbH -Destination $CsrcDir -Force
Write-Host "   Copied: lwrb.c, lwrb.h" -ForegroundColor Green

# Get version from header
$headerPath = Join-Path $CsrcDir "lwrb.h"
$headerContent = Get-Content $headerPath -Raw
$versionMatch = [regex]::Match($headerContent, 'LWRB_VERSION\s+"([^"]+)"')
if ($versionMatch.Success) {
    $version = $versionMatch.Groups[1].Value
    Write-Host "   Version: $version" -ForegroundColor Cyan
}

# 4. Reminder
Write-Host "[4/4] Post-update tasks..." -ForegroundColor Yellow
Write-Host ""
Write-Host "=== UPDATE COMPLETE ===" -ForegroundColor Green
Write-Host ""
if ($version) {
    Write-Host "lwrb has been updated to version $version" -ForegroundColor Cyan
} else {
    Write-Host "lwrb has been updated successfully!" -ForegroundColor Cyan
}
Write-Host ""
Write-Host "IMPORTANT: Please review the following:" -ForegroundColor Cyan
Write-Host ""
Write-Host "  1. Check the lwrb changelog for any breaking changes:"
Write-Host "     https://github.com/MaJerle/lwrb/blob/master/CHANGELOG.md"
Write-Host ""
Write-Host "  2. Your port files were preserved:"
Write-Host "     - lwrb_port.h"
Write-Host "     - lwrb_port.c"
Write-Host ""
Write-Host "  3. Rebuild your project to test the updated library."
Write-Host ""
if (Test-Path $BackupDir) {
    Write-Host "Backup location: $BackupDir" -ForegroundColor DarkGray
}
