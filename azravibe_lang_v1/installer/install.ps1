param(
    [string]$SourceRoot = (Split-Path -Parent $MyInvocation.MyCommand.Path)
)

$ErrorActionPreference = 'Stop'

Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;

namespace Win32 {
    public static class NativeMethods {
        [DllImport("user32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        public static extern IntPtr SendMessageTimeout(
            IntPtr hWnd,
            int Msg,
            IntPtr wParam,
            string lParam,
            int fuFlags,
            int uTimeout,
            out IntPtr lpdwResult);

        [DllImport("gdi32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        public static extern int AddFontResourceEx(string name, uint fl, IntPtr pdv);
    }
}
'@

function Get-VersionValue {
    param([string]$Path)
    if (-not (Test-Path $Path)) {
        return 'v.0.1'
    }
    foreach ($line in Get-Content $Path) {
        $trimmed = $line.Trim()
        if ($trimmed) {
            return $trimmed
        }
    }
    return 'v.0.1'
}

function Ensure-UserPathEntry {
    param([string]$Entry)
    $current = [Environment]::GetEnvironmentVariable('Path', 'User')
    $parts = @()
    if ($current) {
        $parts = $current.Split(';', [System.StringSplitOptions]::RemoveEmptyEntries)
    }
    $normalizedEntry = $Entry.Trim().TrimEnd('\')
    foreach ($part in $parts) {
        if ($part.Trim().TrimEnd('\').ToLowerInvariant() -eq $normalizedEntry.ToLowerInvariant()) {
            return
        }
    }
    $updated = if ($current) { "$current;$normalizedEntry" } else { $normalizedEntry }
    [Environment]::SetEnvironmentVariable('Path', $updated, 'User')
    $result = [IntPtr]::Zero
    [void][Win32.NativeMethods]::SendMessageTimeout([IntPtr]0xffff, 0x001A, [IntPtr]::Zero, 'Environment', 0x0002, 5000, [ref]$result)
}

function Install-UserFontIfPresent {
    param(
        [string]$FontSource,
        [string]$FontFileName,
        [string]$FontRegistryName
    )

    if (-not (Test-Path $FontSource)) {
        return
    }

    $fontsDir = Join-Path $env:LOCALAPPDATA 'Microsoft\Windows\Fonts'
    $fontsKey = 'HKCU:\Software\Microsoft\Windows NT\CurrentVersion\Fonts'
    $fontDestination = Join-Path $fontsDir $FontFileName

    New-Item -ItemType Directory -Force -Path $fontsDir | Out-Null
    Copy-Item -LiteralPath $FontSource -Destination $fontDestination -Force

    if (-not (Test-Path $fontsKey)) {
        New-Item -Path $fontsKey -Force | Out-Null
    }

    New-ItemProperty -Path $fontsKey -Name $FontRegistryName -Value $FontFileName -PropertyType String -Force | Out-Null
    [void][Win32.NativeMethods]::AddFontResourceEx($fontDestination, 0, [IntPtr]::Zero)
}

$payloadRoot = $SourceRoot
$installRoot = Join-Path $env:LOCALAPPDATA 'Programs\Azravibe'
$stdlibArchive = Join-Path $payloadRoot 'stdlib.zip'
$versionFile = Join-Path $payloadRoot 'versions.txt'
$version = Get-VersionValue -Path $versionFile

if (-not (Test-Path (Join-Path $payloadRoot 'azravibe.exe'))) {
    throw "Missing azravibe.exe in installer payload: $payloadRoot"
}

New-Item -ItemType Directory -Force -Path $installRoot | Out-Null

@(
    'azravibe.exe',
    'azr.exe',
    'versions.txt',
    'README.md'
) | ForEach-Object {
    $source = Join-Path $payloadRoot $_
    if (Test-Path $source) {
        Copy-Item -Path $source -Destination (Join-Path $installRoot $_) -Force
    }
}

$installedStdlib = Join-Path $installRoot 'stdlib'
if (Test-Path $installedStdlib) {
    Remove-Item -LiteralPath $installedStdlib -Recurse -Force
}
Expand-Archive -LiteralPath $stdlibArchive -DestinationPath $installedStdlib -Force

Ensure-UserPathEntry -Entry $installRoot
Install-UserFontIfPresent -FontSource (Join-Path $payloadRoot 'fonts\Vazir-Code.ttf') -FontFileName 'Vazir-Code.ttf' -FontRegistryName 'Vazir Code (TrueType)'

Write-Host "Azravibe $version installed to $installRoot"
Write-Host 'Open a new terminal and run: azravibe --version'
