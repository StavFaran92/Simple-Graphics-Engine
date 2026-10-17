# Launches the editor on a project, lets it run N frames, saves the Editor View as a PNG and the console log, then quits.
# Usage: powershell -File scripts/run_capture.ps1 [-Project samples/game_2] [-Frames 120] [-Config Debug] [-TimeoutSec 180]
param(
    [string]$Project = "samples/game_2",
    [int]$Frames = 120,
    [string]$Config = "Debug",
    [int]$TimeoutSec = 180
)

$root = Split-Path -Parent $PSScriptRoot
$exe = Join-Path $root "build/bin/$Config/EditorApp.exe"
# Same working directory as the Visual Studio debugger; the editor finds its resources relative to it
$workDir = Join-Path $root "build/EditorApp"
$outDir = Join-Path $root "build/capture"
$png = Join-Path $outDir "frame.png"
$logFile = Join-Path $outDir "log.txt"
$projectDir = (Resolve-Path (Join-Path $root $Project)).Path

if (-not (Test-Path $exe)) { Write-Output "No editor binary at $exe - run scripts/build.ps1 first"; exit 1 }
New-Item -ItemType Directory -Force $outDir | Out-Null
Remove-Item $png, $logFile -ErrorAction SilentlyContinue

$editorArgs = @("-ec", "`"$projectDir`"", "-frames", $Frames, "-capture", "`"$png`"", "-log", "`"$logFile`"")
$proc = Start-Process -FilePath $exe -ArgumentList $editorArgs -WorkingDirectory $workDir -PassThru
$null = $proc.Handle  # caching the handle keeps ExitCode readable after exit

if (-not $proc.WaitForExit($TimeoutSec * 1000)) {
    Stop-Process -Id $proc.Id -Force
    Write-Output "TIMEOUT after $TimeoutSec s - editor killed (it may be stuck on a dialog or still loading)"
    exit 2
}

Write-Output "Editor exited with code $($proc.ExitCode)"
if (Test-Path $png) { Write-Output "Screenshot: $png" } else { Write-Output "Screenshot: MISSING" }
if (Test-Path $logFile) {
    Write-Output "Log: $logFile"
    $issues = Select-String -Path $logFile -Pattern "^\[[^\]]+\] \[(warning|error|critical)\]" | ForEach-Object { $_.Line }
    Write-Output "Warnings/errors in log: $(@($issues).Count)"
    $issues | Select-Object -First 30 | ForEach-Object { Write-Output "  $_" }
} else { Write-Output "Log: MISSING" }

if ($proc.ExitCode -ne 0) { exit $proc.ExitCode }
