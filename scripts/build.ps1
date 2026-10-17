# Builds one target and prints only compiler/linker errors, so agents and CI get a short answer.
# Usage: powershell -File scripts/build.ps1 [-Target EditorApp] [-Config Debug]
param(
    [string]$Target = "EditorApp",
    [string]$Config = "Debug"
)

$root = Split-Path -Parent $PSScriptRoot
$buildDir = Join-Path $root "build"
$log = Join-Path $buildDir "last_build.log"

cmake --build $buildDir --target $Target --config $Config -- /m /v:minimal /nologo *> $log
$code = $LASTEXITCODE

$errors = Select-String -Path $log -Pattern "error C\d+|error LNK\d+|fatal error|error MSB\d+" | ForEach-Object { $_.Line.Trim() } | Select-Object -Unique
$errors | Select-Object -First 40 | ForEach-Object { Write-Output $_ }

if ($code -ne 0) {
    Write-Output "BUILD FAILED (exit $code, $(@($errors).Count) errors) - full log: $log"
    exit $code
}
Write-Output "BUILD OK ($Target, $Config)"
