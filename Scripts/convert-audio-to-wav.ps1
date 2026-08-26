<#
  convert-audio-to-wav.ps1

  Batch-converts every music file dropped into RawAssets/Audio/Music/incoming/
  (MP3, OGG, FLAC, M4A, AAC, or WAV) into a normalized 48kHz/16-bit PCM WAV in
  RawAssets/Audio/Music/wav/ — the only format UE's editor natively imports as
  a Sound Wave asset.

  Requires ffmpeg on PATH (confirmed installed via WinGet on this machine).

  Usage — just drop downloaded tracks into RawAssets/Audio/Music/incoming/,
  then from the project root:

      powershell -File Scripts/convert-audio-to-wav.ps1

  Filenames are normalized to lowercase-kebab (spaces/underscores -> hyphens,
  anything else stripped) to match the RawAssets naming convention used for
  the 3D asset set. Safe to rerun — already-converted files are skipped
  unless -Force is passed.
#>

param(
    [switch]$Force
)

$ErrorActionPreference = "Stop"

if (-not (Get-Command ffmpeg -ErrorAction SilentlyContinue)) {
    Write-Error "ffmpeg not found on PATH. Install it (e.g. 'winget install Gyan.FFmpeg') and retry."
    exit 1
}

$root     = Split-Path -Parent $PSScriptRoot
$incoming = Join-Path $root "RawAssets\Audio\Music\incoming"
$outDir   = Join-Path $root "RawAssets\Audio\Music\wav"

New-Item -ItemType Directory -Force -Path $incoming | Out-Null
New-Item -ItemType Directory -Force -Path $outDir   | Out-Null

$extensions = @("*.mp3", "*.ogg", "*.flac", "*.m4a", "*.aac", "*.wav")
$files = Get-ChildItem -Path $incoming -Include $extensions -File -Recurse

if ($files.Count -eq 0) {
    Write-Host "No audio files found in $incoming — drop your downloads there and rerun."
    exit 0
}

$converted = 0
$skipped   = 0
$failed    = 0

foreach ($file in $files) {
    $normalized = $file.BaseName.ToLower() -replace '[\s_]+', '-' -replace '[^a-z0-9\-]', ''
    $outPath = Join-Path $outDir "$normalized.wav"

    if ((Test-Path $outPath) -and -not $Force) {
        Write-Host "Skip (already converted): $($file.Name)"
        $skipped++
        continue
    }

    Write-Host "Converting: $($file.Name) -> $normalized.wav"
    & ffmpeg -y -i $file.FullName -ar 48000 -sample_fmt s16 -acodec pcm_s16le $outPath *>$null

    if ($LASTEXITCODE -eq 0) {
        $converted++
    } else {
        Write-Warning "FAILED: $($file.Name)"
        $failed++
    }
}

Write-Host ""
Write-Host "Done — $converted converted, $skipped skipped, $failed failed."
Write-Host "WAVs ready to import from: $outDir"
