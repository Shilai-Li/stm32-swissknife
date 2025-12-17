# LVGL Update Script
# 用于从官方 LVGL 仓库更新此组件
#
# 使用方法:
#   1. 下载最新 LVGL: git clone https://github.com/lvgl/lvgl.git --branch v9.4.0
#   2. 运行此脚本: .\update_lvgl.ps1 -SourcePath "path\to\lvgl"
#
# 脚本会:
#   - 备份当前的 lv_conf.h 和 port 文件
#   - 替换 csrc/ 目录 (对应官方的 src/)
#   - 更新 lvgl.h (并自动修改 include 路径)
#   - 恢复你的配置文件

param(
    [Parameter(Mandatory=$true)]
    [string]$SourcePath
)

$ScriptDir = $PSScriptRoot
$BackupDir = Join-Path $ScriptDir "backup_$(Get-Date -Format 'yyyyMMdd_HHmmss')"

Write-Host "=== LVGL Update Script ===" -ForegroundColor Cyan
Write-Host "Source: $SourcePath"
Write-Host "Target: $ScriptDir"
Write-Host ""

# Validate source
if (-not (Test-Path (Join-Path $SourcePath "src"))) {
    Write-Error "Invalid LVGL source path. 'src' folder not found."
    exit 1
}

# 1. Create backup
Write-Host "[1/6] Creating backup..." -ForegroundColor Yellow
New-Item -ItemType Directory -Path $BackupDir -Force | Out-Null
Copy-Item -Path (Join-Path $ScriptDir "lv_conf.h") -Destination $BackupDir -ErrorAction SilentlyContinue
Copy-Item -Path (Join-Path $ScriptDir "lvgl_port.c") -Destination $BackupDir -ErrorAction SilentlyContinue
Copy-Item -Path (Join-Path $ScriptDir "lvgl_port.h") -Destination $BackupDir -ErrorAction SilentlyContinue
Write-Host "   Backup saved to: $BackupDir" -ForegroundColor Green

# 2. Remove old csrc
Write-Host "[2/6] Removing old csrc folder..." -ForegroundColor Yellow
$oldCsrc = Join-Path $ScriptDir "csrc"
if (Test-Path $oldCsrc) {
    Remove-Item -Path $oldCsrc -Recurse -Force
}
Write-Host "   Done." -ForegroundColor Green

# 3. Copy new src as csrc
Write-Host "[3/6] Copying new src folder as csrc..." -ForegroundColor Yellow
Copy-Item -Path (Join-Path $SourcePath "src") -Destination (Join-Path $ScriptDir "csrc") -Recurse
Write-Host "   Done." -ForegroundColor Green

# 4. Copy and modify lvgl.h
Write-Host "[4/6] Updating lvgl.h..." -ForegroundColor Yellow
$srcLvglH = Join-Path $SourcePath "lvgl.h"
$dstLvglH = Join-Path $ScriptDir "lvgl.h"

# Read content, replace "src/" with "csrc/", and write
$content = Get-Content $srcLvglH -Raw
$content = $content -replace '"src/', '"csrc/'
$content | Set-Content $dstLvglH -NoNewline
Write-Host "   Done. (src/ -> csrc/ paths updated)" -ForegroundColor Green

# 5. Copy lv_version.h if it exists at root
Write-Host "[5/6] Checking for lv_version.h..." -ForegroundColor Yellow
$srcVersion = Join-Path $SourcePath "lv_version.h"
if (Test-Path $srcVersion) {
    Copy-Item -Path $srcVersion -Destination $ScriptDir -Force
    Write-Host "   Copied lv_version.h" -ForegroundColor Green
} else {
    Write-Host "   lv_version.h not found at root (may be in src/)" -ForegroundColor DarkGray
}

# 6. Reminder
Write-Host "[6/6] Post-update tasks..." -ForegroundColor Yellow
Write-Host ""
Write-Host "=== UPDATE COMPLETE ===" -ForegroundColor Green
Write-Host ""
Write-Host "IMPORTANT: Please review the following:" -ForegroundColor Cyan
Write-Host "  1. Compare your lv_conf.h with the new lv_conf_template.h"
Write-Host "     New template: $SourcePath\lv_conf_template.h"
Write-Host "     Your config:  $ScriptDir\lv_conf.h"
Write-Host ""
Write-Host "  2. Check for API changes in the LVGL changelog:"
Write-Host "     https://github.com/lvgl/lvgl/blob/master/docs/CHANGELOG.md"
Write-Host ""
Write-Host "  3. Your port files were preserved:"
Write-Host "     - lvgl_port.c"
Write-Host "     - lvgl_port.h"
Write-Host ""
Write-Host "  4. Directory mapping:"
Write-Host "     Official: src/  ->  Local: csrc/"
Write-Host ""
Write-Host "Backup location: $BackupDir" -ForegroundColor DarkGray
