# SFUD Update Script
# 用于从官方 SFUD 仓库更新此组件
#
# 使用方法:
#   1. 下载 SFUD: git clone https://github.com/armink/SFUD.git
#   2. 运行此脚本: .\update_sfud.ps1 -SourcePath "path\to\SFUD"
#
# 脚本会:
#   - 备份当前的 csrc/ (如果需要)
#   - 替换 csrc/ 目录中的核心文件
#   - 保留你的端口文件 sfud_port.c/h
#   - 保留你的配置文件 csrc/sfud_cfg.h
#
# 注意: SFUD 官方仓库结构:
#   SFUD/
#   ├── sfud/src/
#   │   ├── sfud.c         ← 我们需要这个
#   │   └── sfud_sfdp.c    ← 我们需要这个
#   ├── sfud/inc/
#   │   ├── sfud.h          ← 我们需要这个
#   │   ├── sfud_cfg.h      ← 我们需要这个（但会保留自定义版本）
#   │   ├── sfud_def.h      ← 我们需要这个
#   │   └── sfud_flash_def.h ← 我们需要这个
#   └── ...

param(
    [Parameter(Mandatory=$true)]
    [string]$SourcePath
)

$ScriptDir = $PSScriptRoot
$BackupDir = Join-Path $ScriptDir "backup_$(Get-Date -Format 'yyyyMMdd_HHmmss')"
$CsrcDir = Join-Path $ScriptDir "csrc"
$ConfigBackup = Join-Path $ScriptDir "sfud_cfg_backup.h"

Write-Host "=== SFUD Update Script ===" -ForegroundColor Cyan
Write-Host "Source: $SourcePath"
Write-Host "Target: $ScriptDir"
Write-Host ""

# Validate source paths
$sfudSrc = Join-Path $SourcePath "sfud\src"
$sfudInc = Join-Path $SourcePath "sfud\inc"

if (-not (Test-Path $sfudSrc)) {
    Write-Error "Invalid SFUD source path. 'sfud/src' not found."
    Write-Host "Make sure you have the official SFUD repository:"
    Write-Host "  git clone https://github.com/armink/SFUD.git"
    exit 1
}

if (-not (Test-Path $sfudInc)) {
    Write-Error "Invalid SFUD source path. 'sfud/inc' not found."
    exit 1
}

# 1. Backup existing csrc and sfud_cfg.h
Write-Host "[1/5] Creating backup..." -ForegroundColor Yellow
if (Test-Path $CsrcDir) {
    New-Item -ItemType Directory -Path $BackupDir -Force | Out-Null
    Copy-Item -Path $CsrcDir -Destination $BackupDir -Recurse
    
    # Backup custom config
    $customConfig = Join-Path $CsrcDir "sfud_cfg.h"
    if (Test-Path $customConfig) {
        Copy-Item -Path $customConfig -Destination $ConfigBackup -Force
        Write-Host "   Custom sfud_cfg.h backed up" -ForegroundColor Green
    }
    
    Write-Host "   Backup saved to: $BackupDir" -ForegroundColor Green
} else {
    Write-Host "   No existing csrc folder to backup." -ForegroundColor DarkGray
    New-Item -ItemType Directory -Path $CsrcDir -Force | Out-Null
}

# 2. Remove old source files (except our custom config backup)
Write-Host "[2/5] Removing old source files..." -ForegroundColor Yellow
if (Test-Path $CsrcDir) {
    Remove-Item -Path (Join-Path $CsrcDir "*") -Force -ErrorAction SilentlyContinue
}
Write-Host "   Done." -ForegroundColor Green

# 3. Copy new source files
Write-Host "[3/5] Copying new source files..." -ForegroundColor Yellow

# Copy source files (.c)
$srcFiles = Get-ChildItem -Path $sfudSrc -Filter "*.c"
foreach ($file in $srcFiles) {
    Copy-Item -Path $file.FullName -Destination $CsrcDir -Force
    Write-Host "   Copied: $($file.Name)" -ForegroundColor Green
}

# Copy header files (.h)
$incFiles = Get-ChildItem -Path $sfudInc -Filter "*.h"
foreach ($file in $incFiles) {
    Copy-Item -Path $file.FullName -Destination $CsrcDir -Force
    Write-Host "   Copied: $($file.Name)" -ForegroundColor Green
}

# 4. Restore custom configuration
Write-Host "[4/5] Restoring custom configuration..." -ForegroundColor Yellow
if (Test-Path $ConfigBackup) {
    Copy-Item -Path $ConfigBackup -Destination (Join-Path $CsrcDir "sfud_cfg.h") -Force
    Remove-Item -Path $ConfigBackup -Force
    Write-Host "   Custom sfud_cfg.h restored" -ForegroundColor Green
} else {
    Write-Host "   No custom config to restore, using default" -ForegroundColor DarkGray
}

# 5. Get version info
Write-Host "[5/5] Checking version..." -ForegroundColor Yellow
$headerPath = Join-Path $CsrcDir "sfud.h"
if (Test-Path $headerPath) {
    $headerContent = Get-Content $headerPath -Raw
    $versionMatch = [regex]::Match($headerContent, 'SFUD_SW_VERSION\s+\"([^\"]+)\"')
    if ($versionMatch.Success) {
        $version = $versionMatch.Groups[1].Value
        Write-Host "   Version: $version" -ForegroundColor Cyan
    }
}

# Summary
Write-Host ""
Write-Host "=== UPDATE COMPLETE ===" -ForegroundColor Green
Write-Host ""
if ($version) {
    Write-Host "SFUD has been updated to version $version" -ForegroundColor Cyan
} else {
    Write-Host "SFUD has been updated successfully!" -ForegroundColor Cyan
}
Write-Host ""
Write-Host "IMPORTANT: Please review the following:" -ForegroundColor Cyan
Write-Host ""
Write-Host "  1. Check the SFUD changelog for any breaking changes:"
Write-Host "     https://github.com/armink/SFUD/blob/master/docs/zh-cn/api.md"
Write-Host ""
Write-Host "  2. Your port files were preserved:"
Write-Host "     - sfud_port.h"
Write-Host "     - sfud_port.c"
Write-Host "     - csrc/sfud_cfg.h (custom configuration)"
Write-Host ""
Write-Host "  3. Rebuild your project to test the updated library."
Write-Host ""
if (Test-Path $BackupDir) {
    Write-Host "Backup location: $BackupDir" -ForegroundColor DarkGray
}
Write-Host ""
