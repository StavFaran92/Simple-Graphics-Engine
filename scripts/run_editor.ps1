# Starts the editor on a project and waits until its MCP server answers. For restarting it after a crash.
# Usage: powershell -File scripts/run_editor.ps1 -Project <projectDir> [-Config Debug] [-TimeoutSec 120]
param(
    [Parameter(Mandatory = $true)][string]$Project,
    [string]$Config = "Debug",
    [int]$TimeoutSec = 120
)

$root = Split-Path -Parent $PSScriptRoot
$exe = Join-Path $root "build/bin/$Config/EditorApp.exe"
# Same working directory as the Visual Studio debugger; the editor finds its resources relative to it
$workDir = Join-Path $root "build/EditorApp"
$mcpUrl = "http://127.0.0.1:7777/mcp"
$ping = '{"jsonrpc":"2.0","id":1,"method":"ping"}'

function Test-Mcp {
    try {
        $null = Invoke-WebRequest -Uri $mcpUrl -Method Post -Body $ping -ContentType "application/json" -UseBasicParsing -TimeoutSec 2
        return $true
    } catch { return $false }
}

if (-not (Test-Path $exe)) { Write-Output "No editor binary at $exe - run scripts/build.ps1 first"; exit 1 }
if (Test-Mcp) { Write-Output "An editor is already serving MCP at $mcpUrl"; exit 0 }

$projectDir = if ([System.IO.Path]::IsPathRooted($Project)) { $Project } else { Join-Path $root $Project }
if (-not (Test-Path $projectDir)) { Write-Output "No project at $projectDir"; exit 1 }
$projectDir = (Resolve-Path $projectDir).Path

# The editor moves the report aside when it starts, so check before launching
$crashReport = Join-Path (Split-Path -Parent $exe) "crash/last_crash.txt"
$crashed = Test-Path $crashReport

$proc = Start-Process -FilePath $exe -ArgumentList @("-ec", "`"$projectDir`"") -WorkingDirectory $workDir -PassThru
$null = $proc.Handle  # caching the handle keeps ExitCode readable after exit

$deadline = (Get-Date).AddSeconds($TimeoutSec)
while ((Get-Date) -lt $deadline) {
    if ($proc.HasExited) { Write-Output "Editor exited during startup with code $($proc.ExitCode)"; exit 2 }
    if (Test-Mcp) {
        Write-Output "Editor running (pid $($proc.Id)), MCP at $mcpUrl"
        if ($crashed) { Write-Output "The previous run crashed - call get_last_crash" }
        exit 0
    }
    Start-Sleep -Milliseconds 500
}

Write-Output "TIMEOUT after $TimeoutSec s - the editor (pid $($proc.Id)) is running but MCP doesn't answer"
exit 3
