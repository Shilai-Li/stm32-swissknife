# TinyFrame Update Script
# 用于从官方 TinyFrame 仓库更新此组件
#
# 使用方法:
#   1. 下载 TinyFrame: git clone https://github.com/MightyPork/TinyFrame.git
#   2. 运行此脚本: .\update_tinyframe.ps1 -SourcePath "path\to\TinyFrame"
#
# 脚本会:
#   - 备份当前的 csrc/ (如果需要)
#   - 替换 csrc/ 目录中的核心文件
#   - 保留你的端口文件 tinyframe_port.c/h
#
# 注意: TinyFrame 官方仓库结构:
#   TinyFrame/
#   ├── TinyFrame.c      ← 我们需要这个
#   ├── TinyFrame.h      ← 我们需要这个
#   └── ...

param(
    [Parameter(Mandatory=$true)]
    [string]$SourcePath
)

$ScriptDir = $PSScriptRoot
$BackupDir = Join-Path $ScriptDir "backup_$(Get-Date -Format 'yyyyMMdd_HHmmss')"
$CsrcDir = Join-Path $ScriptDir "csrc"

Write-Host "=== TinyFrame Update Script ===" -ForegroundColor Cyan
Write-Host "Source: $SourcePath"
Write-Host "Target: $ScriptDir"
Write-Host ""

# Validate source - check for TinyFrame.c and TinyFrame.h
if (-not (Test-Path (Join-Path $SourcePath "TinyFrame.c"))) {
    Write-Error "Invalid TinyFrame source path. 'TinyFrame.c' not found."
    Write-Host "Make sure you have the official TinyFrame repository:"
    Write-Host "  git clone https://github.com/MightyPork/TinyFrame.git"
    exit 1
}

if (-not (Test-Path (Join-Path $SourcePath "TinyFrame.h"))) {
    Write-Error "Invalid TinyFrame source path. 'TinyFrame.h' not found."
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

Copy-Item -Path (Join-Path $SourcePath "TinyFrame.c") -Destination $CsrcDir -Force
Copy-Item -Path (Join-Path $SourcePath "TinyFrame.h") -Destination $CsrcDir -Force
Write-Host "   Copied: TinyFrame.c, TinyFrame.h" -ForegroundColor Green

# 4. Reminder
Write-Host "[4/4] Post-update tasks..." -ForegroundColor Yellow
Write-Host ""
Write-Host "=== UPDATE COMPLETE ===" -ForegroundColor Green
Write-Host ""
Write-Host "TinyFrame has been updated successfully!" -ForegroundColor Cyan
Write-Host ""
Write-Host "IMPORTANT: Please review the following:" -ForegroundColor Cyan
Write-Host ""
Write-Host "  1. Check the TinyFrame GitHub for any breaking changes:"
Write-Host "     https://github.com/MightyPork/TinyFrame"
Write-Host ""
Write-Host "  2. Your port files were preserved:"
Write-Host "     - tinyframe_port.h"
Write-Host "     - tinyframe_port.c"
Write-Host ""
Write-Host "  3. Rebuild your project to test the updated library."
Write-Host ""
if (Test-Path $BackupDir) {
    Write-Host "Backup location: $BackupDir" -ForegroundColor DarkGray
}
