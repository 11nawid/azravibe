param(
    [string]$InstallRoot = (Join-Path $env:LOCALAPPDATA 'Programs\Azravibe')
)

$ErrorActionPreference = 'Stop'

Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;

namespace AzravibeInstallerReset {
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
    }
}
'@

function Broadcast-EnvironmentChange {
    $result = [IntPtr]::Zero
    [void][AzravibeInstallerReset.NativeMethods]::SendMessageTimeout(
        [IntPtr]0xffff,
        0x001A,
        [IntPtr]::Zero,
        'Environment',
        0x0002,
        5000,
        [ref]$result
    )
}

function Remove-UserPathEntry {
    param([string]$Entry)

    $current = [Environment]::GetEnvironmentVariable('Path', 'User')
    if (-not $current) {
        return
    }

    $normalizedEntry = $Entry.Trim().TrimEnd('\')
    $updatedParts = New-Object System.Collections.Generic.List[string]

    foreach ($part in $current.Split(';', [System.StringSplitOptions]::RemoveEmptyEntries)) {
        $normalizedPart = $part.Trim().TrimEnd('\')
        if (-not $normalizedPart) {
            continue
        }
        if ($normalizedPart.Equals($normalizedEntry, [System.StringComparison]::OrdinalIgnoreCase)) {
            continue
        }
        $updatedParts.Add($part.Trim())
    }

    $updated = [string]::Join(';', $updatedParts)
    [Environment]::SetEnvironmentVariable('Path', $updated, 'User')
    Broadcast-EnvironmentChange
}

function Invoke-RegisteredUninstall {
    $entries = @(
        Get-ItemProperty 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Uninstall\*' -ErrorAction SilentlyContinue
        Get-ItemProperty 'HKLM:\Software\Microsoft\Windows\CurrentVersion\Uninstall\*' -ErrorAction SilentlyContinue
    ) | Where-Object { $_.DisplayName -eq 'Azravibe' }

    foreach ($entry in $entries) {
        $quiet = $entry.QuietUninstallString
        $cmd = if ($quiet) { $quiet } else { $entry.UninstallString }
        if (-not $cmd) {
            continue
        }

        if ($cmd -match '^\s*"([^"]+)"\s*(.*)$') {
            $exe = $Matches[1]
            $args = $Matches[2]
        }
        elseif ($cmd -match '^\s*([^\s]+)\s*(.*)$') {
            $exe = $Matches[1]
            $args = $Matches[2]
        }
        else {
            continue
        }

        $extraArgs = '/VERYSILENT /SUPPRESSMSGBOXES /NORESTART'
        if ($args -notmatch '/VERYSILENT') {
            $args = ($args.Trim() + ' ' + $extraArgs).Trim()
        }

        if (Test-Path $exe) {
            Start-Process -FilePath $exe -ArgumentList $args -Wait -NoNewWindow
        }
    }
}

Invoke-RegisteredUninstall

Remove-UserPathEntry -Entry $InstallRoot

$startMenuDir = Join-Path $env:APPDATA 'Microsoft\Windows\Start Menu\Programs\Azravibe'
$desktopShortcut = Join-Path ([Environment]::GetFolderPath('Desktop')) 'Azravibe.lnk'

foreach ($path in @($InstallRoot, $startMenuDir, $desktopShortcut)) {
    if (Test-Path $path) {
        Remove-Item -LiteralPath $path -Recurse -Force
    }
}

Write-Host "Azravibe reset complete. Cleaned install root: $InstallRoot"
