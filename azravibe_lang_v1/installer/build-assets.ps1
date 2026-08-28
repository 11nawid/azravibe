param(
    [Parameter(Mandatory = $true)]
    [string]$OutputDir
)

$ErrorActionPreference = 'Stop'

Add-Type -AssemblyName System.Drawing

function New-Color {
    param(
        [int]$R,
        [int]$G,
        [int]$B,
        [int]$A = 255
    )

    return [System.Drawing.Color]::FromArgb($A, $R, $G, $B)
}

function Save-WizardBitmap {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path,
        [Parameter(Mandatory = $true)]
        [int]$Width,
        [Parameter(Mandatory = $true)]
        [int]$Height,
        [Parameter(Mandatory = $true)]
        [scriptblock]$Painter
    )

    $bitmap = New-Object System.Drawing.Bitmap($Width, $Height)
    $graphics = [System.Drawing.Graphics]::FromImage($bitmap)

    try {
        $graphics.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
        $graphics.TextRenderingHint = [System.Drawing.Text.TextRenderingHint]::ClearTypeGridFit
        & $Painter $graphics $Width $Height
        $bitmap.Save($Path, [System.Drawing.Imaging.ImageFormat]::Bmp)
    }
    finally {
        $graphics.Dispose()
        $bitmap.Dispose()
    }
}

New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null

$wizardPath = Join-Path $OutputDir 'wizard.bmp'
$smallPath = Join-Path $OutputDir 'wizard-small.bmp'

Save-WizardBitmap -Path $wizardPath -Width 164 -Height 314 -Painter {
    param($graphics, $width, $height)

    $rect = New-Object System.Drawing.Rectangle(0, 0, $width, $height)
    $brush = New-Object System.Drawing.Drawing2D.LinearGradientBrush(
        $rect,
        (New-Color -R 7 -G 28 -B 46),
        (New-Color -R 7 -G 109 -B 112),
        90.0
    )

    $overlayBrush = New-Object System.Drawing.SolidBrush((New-Color -R 255 -G 255 -B 255 -A 28))
    $titleBrush = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::White)
    $bodyBrush = New-Object System.Drawing.SolidBrush((New-Color -R 220 -G 242 -B 245))
    $logoBrush = New-Object System.Drawing.SolidBrush((New-Color -R 255 -G 214 -B 102))
    $titleFont = New-Object System.Drawing.Font('Segoe UI', 22, [System.Drawing.FontStyle]::Bold)
    $bodyFont = New-Object System.Drawing.Font('Segoe UI', 10, [System.Drawing.FontStyle]::Regular)
    $logoFont = New-Object System.Drawing.Font('Segoe UI', 54, [System.Drawing.FontStyle]::Bold)

    try {
        $graphics.FillRectangle($brush, $rect)
        $graphics.FillEllipse($overlayBrush, -30, 180, 180, 180)
        $graphics.FillEllipse($overlayBrush, 55, -40, 120, 120)
        $graphics.DrawString('A', $logoFont, $logoBrush, 36, 32)
        $graphics.DrawString('Azravibe', $titleFont, $titleBrush, 18, 132)
        $graphics.DrawString('Persian-first programming language', $bodyFont, $bodyBrush, 20, 178)
        $graphics.DrawString('Compiler and runtime tools', $bodyFont, $bodyBrush, 20, 198)
        $graphics.DrawString('with a real Windows setup flow.', $bodyFont, $bodyBrush, 20, 218)
    }
    finally {
        $brush.Dispose()
        $overlayBrush.Dispose()
        $titleBrush.Dispose()
        $bodyBrush.Dispose()
        $logoBrush.Dispose()
        $titleFont.Dispose()
        $bodyFont.Dispose()
        $logoFont.Dispose()
    }
}

Save-WizardBitmap -Path $smallPath -Width 55 -Height 55 -Painter {
    param($graphics, $width, $height)

    $rect = New-Object System.Drawing.Rectangle(0, 0, $width, $height)
    $brush = New-Object System.Drawing.Drawing2D.LinearGradientBrush(
        $rect,
        (New-Color -R 11 -G 41 -B 66),
        (New-Color -R 13 -G 138 -B 124),
        45.0
    )
    $logoBrush = New-Object System.Drawing.SolidBrush((New-Color -R 255 -G 214 -B 102))
    $ringPen = New-Object System.Drawing.Pen((New-Color -R 255 -G 255 -B 255 -A 90), 2)
    $font = New-Object System.Drawing.Font('Segoe UI', 22, [System.Drawing.FontStyle]::Bold)

    try {
        $graphics.FillRectangle($brush, $rect)
        $graphics.DrawEllipse($ringPen, 5, 5, 44, 44)
        $graphics.DrawString('A', $font, $logoBrush, 11, 8)
    }
    finally {
        $brush.Dispose()
        $logoBrush.Dispose()
        $ringPen.Dispose()
        $font.Dispose()
    }
}
