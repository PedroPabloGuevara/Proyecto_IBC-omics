$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
$sourceFiles = Get-ChildItem -Path (Join-Path $root "src") -Recurse -Filter "*.c" |
    Where-Object { $_.Name -ne "main.c" } |
    ForEach-Object { $_.FullName }
foreach ($testName in @("pruebas_regresion", "main_flujo")) {
    $testFile = Join-Path $PSScriptRoot ($testName + ".c")
    $outputFile = Join-Path $env:TEMP ("proyecto-ibc-" + $testName + "-" + $PID + ".exe")
    $compilerArguments = @(
        "-std=c99",
        "-Wall",
        "-Wextra",
        "-Wpedantic",
        "-I$(Join-Path $root 'include')"
    ) + $sourceFiles + @($testFile, "-o", $outputFile)

    try {
        & gcc @compilerArguments
        if ($LASTEXITCODE -ne 0) {
            throw "La compilacion de $testName fallo con codigo $LASTEXITCODE."
        }

        & $outputFile
        if ($LASTEXITCODE -ne 0) {
            throw "Las pruebas de $testName fallaron con codigo $LASTEXITCODE."
        }
    }
    finally {
        if (Test-Path -LiteralPath $outputFile) {
            Remove-Item -LiteralPath $outputFile
        }
    }
}
