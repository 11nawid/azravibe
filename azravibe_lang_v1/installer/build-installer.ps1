param(
    [string]$OutputDir = (Join-Path $PSScriptRoot '..\\dist'),
    [switch]$SkipBuild,
    [switch]$NoToolBootstrap
)

$ErrorActionPreference = 'Stop'

function Get-VersionValue {
    param([string]$Path)
    foreach ($line in Get-Content $Path) {
        $trimmed = $line.Trim()
        if ($trimmed) {
            return $trimmed
        }
    }
    throw "No version found in $Path"
}

function Convert-ToNumericVersion {
    param([string]$Version)

    $parts = [regex]::Matches($Version, '\d+') | ForEach-Object { $_.Value }
    if (-not $parts -or $parts.Count -eq 0) {
        return '0.0.0.0'
    }

    $numeric = New-Object System.Collections.Generic.List[string]
    foreach ($part in $parts) {
        $numeric.Add($part)
        if ($numeric.Count -eq 4) {
            break
        }
    }
    while ($numeric.Count -lt 4) {
        $numeric.Add('0')
    }

    return [string]::Join('.', $numeric)
}

function Get-IsccPath {
    $candidates = @(
        (Join-Path $env:LOCALAPPDATA 'Programs\Inno Setup 6\ISCC.exe'),
        'C:\Program Files (x86)\Inno Setup 6\ISCC.exe',
        'C:\Program Files\Inno Setup 6\ISCC.exe'
    )

    foreach ($candidate in $candidates) {
        if (Test-Path $candidate) {
            return $candidate
        }
    }

    $command = Get-Command ISCC.exe -ErrorAction SilentlyContinue
    if ($command) {
        return $command.Source
    }

    return $null
}

function Ensure-InnoSetup {
    $existing = Get-IsccPath
    if ($existing) {
        return $existing
    }

    if ($NoToolBootstrap) {
        throw 'Inno Setup 6 is required. Install JRSoftware.InnoSetup or rerun without -NoToolBootstrap.'
    }

    Write-Host 'Inno Setup 6 was not found. Installing it with winget...'
    & winget install --id JRSoftware.InnoSetup --exact --accept-package-agreements --accept-source-agreements
    if ($LASTEXITCODE -ne 0) {
        throw "winget install JRSoftware.InnoSetup failed with exit code $LASTEXITCODE"
    }

    $installed = Get-IsccPath
    if (-not $installed) {
        throw 'Inno Setup 6 was installed, but ISCC.exe could not be located afterward.'
    }

    return $installed
}

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$ideRoot = (Resolve-Path (Join-Path $repoRoot '..\azravibe_ide')).Path
$outputRoot = (Resolve-Path $OutputDir -ErrorAction SilentlyContinue)
if (-not $outputRoot) {
    New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null
    $outputRoot = Resolve-Path $OutputDir
}
$outputRoot = $outputRoot.Path
$version = Get-VersionValue -Path (Join-Path $repoRoot 'versions.txt')
$numericVersion = Convert-ToNumericVersion -Version $version
$workDir = Join-Path $outputRoot 'installer-work'
$targetExe = Join-Path $outputRoot ("Azravibe-Setup-{0}.exe" -f $version)
$assetsDir = Join-Path $workDir 'assets'
$consoleAppRoot = Join-Path $ideRoot 'dist\win-unpacked'
$consoleExe = Join-Path $consoleAppRoot 'Azravibe IDE.exe'
$fontFile = Join-Path $repoRoot 'installer\fonts\Vazir-Code.ttf'
$fontLicenseFile = Join-Path $repoRoot 'installer\fonts\Vazir-Code-LICENSE.txt'
$isccPath = Ensure-InnoSetup

if (-not $SkipBuild) {
    Push-Location $repoRoot
    try {
        mingw32-make clean
        if ($LASTEXITCODE -ne 0) {
            throw 'mingw32-make clean failed.'
        }
        mingw32-make
        if ($LASTEXITCODE -ne 0) {
            throw 'mingw32-make failed.'
        }
    }
    finally {
        Pop-Location
    }

    Push-Location $ideRoot
    try {
        npm run build
        if ($LASTEXITCODE -ne 0) {
            throw 'npm run build failed for azravibe_ide.'
        }

        npx electron-builder --win --dir --config
        if ($LASTEXITCODE -ne 0) {
            throw 'electron-builder --win --dir failed for azravibe_ide.'
        }
    }
    finally {
        Pop-Location
    }
}

if (Test-Path $workDir) {
    Remove-Item -LiteralPath $workDir -Recurse -Force
}
New-Item -ItemType Directory -Force -Path $assetsDir | Out-Null

@(
    'azravibe.exe',
    'azr.exe',
    'versions.txt',
    'README.md',
    'LICENSE'
) | ForEach-Object {
    $requiredPath = Join-Path $repoRoot $_
    if (-not (Test-Path $requiredPath)) {
        throw "Required installer input is missing: $requiredPath"
    }
}

if (-not (Test-Path (Join-Path $repoRoot 'stdlib'))) {
    throw "Required installer input is missing: $(Join-Path $repoRoot 'stdlib')"
}

if (-not (Test-Path $consoleExe)) {
    throw "Required console app is missing: $consoleExe"
}

if (-not (Test-Path $fontFile)) {
    throw "Required installer font is missing: $fontFile"
}

if (-not (Test-Path $fontLicenseFile)) {
    throw "Required installer font license is missing: $fontLicenseFile"
}

& (Join-Path $PSScriptRoot 'build-assets.ps1') -OutputDir $assetsDir

$scriptPath = Join-Path $PSScriptRoot 'azravibe.iss'
$arguments = @(
    '/Qp',
    "/DMyAppVersion=$version",
    "/DMyAppVersionNumeric=$numericVersion",
    "/DSourceRoot=$repoRoot",
    "/DOutputRoot=$outputRoot",
    "/DInstallerAssetsDir=$assetsDir",
    "/DConsoleAppRoot=$consoleAppRoot",
    $scriptPath
)

& $isccPath @arguments
if ($LASTEXITCODE -ne 0) {
    throw "Inno Setup build failed with exit code $LASTEXITCODE"
}

if (-not (Test-Path $targetExe)) {
    throw "Expected installer output was not created: $targetExe"
}

Write-Host "Installer created: $targetExe"
