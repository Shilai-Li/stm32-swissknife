# U8G2 Update Script
# 用于从官方 U8G2 仓库更新此组件
#
# 使用方法:
#   1. 下载最新 U8G2: git clone https://github.com/olikraus/u8g2.git
#   2. 运行此脚本: .\update_u8g2.ps1 -SourcePath "path\to\u8g2"
#
# 脚本会:
#   - 备份当前的 port 文件
#   - 替换 csrc/ 目录 (官方 U8G2 的核心代码)
#   - 保留你的 port 文件
#
# 注意: U8G2 官方仓库结构:
#   u8g2/
#   ├── csrc/          ← 我们需要这个目录
#   ├── cppsrc/
#   ├── sys/
#   └── ...

param(
    [Parameter(Mandatory=$true)]
    [string]$SourcePath
)

$ScriptDir = $PSScriptRoot
$BackupDir = Join-Path $ScriptDir "backup_$(Get-Date -Format 'yyyyMMdd_HHmmss')"

Write-Host "=== U8G2 Update Script ===" -ForegroundColor Cyan
Write-Host "Source: $SourcePath"
Write-Host "Target: $ScriptDir"
Write-Host ""

# Validate source - U8G2 uses 'csrc' folder
if (-not (Test-Path (Join-Path $SourcePath "csrc"))) {
    Write-Error "Invalid U8G2 source path. 'csrc' folder not found."
    Write-Host "Make sure you cloned the official U8G2 repository:"
    Write-Host "  git clone https://github.com/olikraus/u8g2.git"
    exit 1
}

# 1. Create backup
Write-Host "[1/4] Creating backup..." -ForegroundColor Yellow
New-Item -ItemType Directory -Path $BackupDir -Force | Out-Null
Copy-Item -Path (Join-Path $ScriptDir "u8g2_port.c") -Destination $BackupDir -ErrorAction SilentlyContinue
Copy-Item -Path (Join-Path $ScriptDir "u8g2_port.h") -Destination $BackupDir -ErrorAction SilentlyContinue
Write-Host "   Backup saved to: $BackupDir" -ForegroundColor Green

# 2. Remove old csrc
Write-Host "[2/4] Removing old csrc folder..." -ForegroundColor Yellow
$oldCsrc = Join-Path $ScriptDir "csrc"
if (Test-Path $oldCsrc) {
    Remove-Item -Path $oldCsrc -Recurse -Force
}
Write-Host "   Done." -ForegroundColor Green

# 3. Copy new csrc
Write-Host "[3/4] Copying new csrc folder..." -ForegroundColor Yellow
Copy-Item -Path (Join-Path $SourcePath "csrc") -Destination $ScriptDir -Recurse
Write-Host "   Done." -ForegroundColor Green

# Count files
$fileCount = (Get-ChildItem -Path (Join-Path $ScriptDir "csrc") -File -Recurse | Measure-Object).Count
Write-Host "   Copied $fileCount files." -ForegroundColor DarkGray

# 4. Reminder
Write-Host "[4/4] Post-update tasks..." -ForegroundColor Yellow
Write-Host ""
Write-Host "=== UPDATE COMPLETE ===" -ForegroundColor Green
Write-Host ""
Write-Host "IMPORTANT: Please review the following:" -ForegroundColor Cyan
Write-Host ""
Write-Host "  1. Check for API changes in U8G2 changelog:"
Write-Host "     https://github.com/olikraus/u8g2/blob/master/ChangeLog"
Write-Host ""
Write-Host "  2. Your port files were preserved:"
Write-Host "     - u8g2_port.c"
Write-Host "     - u8g2_port.h"
Write-Host ""
Write-Host "  3. If you use specific display drivers, verify they still exist in:"
Write-Host "     csrc/u8x8_d_*.c"
Write-Host ""
Write-Host "  4. Update CMakeLists.txt if new drivers were added that you need."
Write-Host ""
Write-Host "Backup location: $BackupDir" -ForegroundColor DarkGray
