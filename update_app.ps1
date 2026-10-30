# Update script for Liz Wegner Proprietary Software / Therapist Documentation Tool
# This script downloads the latest release / build from GitHub or updates the local git repository.

[CmdletBinding()]
param (
    [string]$RepoUrl = "https://raw.githubusercontent.com/Travishulse/therapist-documentation-tool/release/Therapist-Documentation-Tool.html",
    [string]$DestinationPath = "$PSScriptRoot\Therapist-Documentation-Tool.html"
)

$ErrorActionPreference = "Stop"

Write-Host "=====================================================" -ForegroundColor Cyan
Write-Host " Liz Wegner Proprietary Software - Update Manager    " -ForegroundColor Cyan
Write-Host "=====================================================" -ForegroundColor Cyan
Write-Host ""

# Check if running in a git clone
$gitDir = Join-Path $PSScriptRoot ".git"
if (Test-Path $gitDir) {
    Write-Host "[1/2] Git repository detected. Checking for updates via Git..." -ForegroundColor Yellow
    try {
        git fetch origin
        $local = git rev-parse HEAD
        $remote = git rev-parse "@{u}" 2>$null
        
        if ($local -ne $remote -and $remote) {
            Write-Host "New updates found on remote repository. Pulling changes..." -ForegroundColor Green
            git pull
            if (Get-Command npm -ErrorAction SilentlyContinue) {
                Write-Host "Rebuilding single-file HTML bundle..." -ForegroundColor Yellow
                npm run build
            }
            Write-Host "Update completed successfully via Git!" -ForegroundColor Green
            exit 0
        } else {
            Write-Host "Repository is already up to date." -ForegroundColor Green
        }
    } catch {
        Write-Warning "Git update check failed or was not connected. Falling back to direct HTTP download..."
    }
}

# Standalone single-file HTML updater
Write-Host "[2/2] Checking remote release build from GitHub..." -ForegroundColor Yellow
try {
    [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
    $backupPath = "$DestinationPath.bak"
    if (Test-Path $DestinationPath) {
        Copy-Item -Path $DestinationPath -Destination $backupPath -Force
    }

    Write-Host "Downloading latest version to '$DestinationPath'..." -ForegroundColor Cyan
    Invoke-WebRequest -Uri $RepoUrl -OutFile $DestinationPath -UseBasicParsing

    Write-Host "Success! The application has been updated to the latest version." -ForegroundColor Green
    if (Test-Path $backupPath) {
        Remove-Item $backupPath -Force
    }
} catch {
    Write-Error "Failed to download update: $_"
    if (Test-Path "$DestinationPath.bak") {
        Move-Item -Path "$DestinationPath.bak" -Destination $DestinationPath -Force
        Write-Warning "Restored previous working version from backup."
    }
    exit 1
}

Write-Host ""
Write-Host "Update completed! You can now launch Therapist-Documentation-Tool.html." -ForegroundColor Green
