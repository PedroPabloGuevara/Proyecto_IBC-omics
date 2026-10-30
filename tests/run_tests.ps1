$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
$compiler = Get-Command gcc -ErrorAction SilentlyContinue
$useZig = $false

if ($null -eq $compiler) {
    $compiler = Get-Command python -ErrorAction SilentlyContinue
    if ($null -eq $compiler) {
        throw "Se requiere gcc o Python con el paquete ziglang para compilar las pruebas."
    }

    & $compiler.Source -m ziglang cc --version *> $null
    if ($LASTEXITCODE -ne 0) {
        throw "No se encontro gcc ni un compilador ziglang utilizable."
    }
    $useZig = $true
}

$sourceFiles = Get-ChildItem -Path (Join-Path $root "src") -Recurse -Filter "*.c" |
    Where-Object { $_.Name -ne "main.c" } |
    ForEach-Object { $_.FullName }

foreach ($testName in @("pruebas_regresion", "main_flujo", "pruebas_estres")) {
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
        if ($useZig) {
            & $compiler.Source -m ziglang cc @compilerArguments
        }
        else {
            & $compiler.Source @compilerArguments
        }
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

$mainFile = Join-Path $root "src\main.c"
$mainOutput = Join-Path $env:TEMP ("proyecto-ibc-main-" + $PID + ".exe")
$mainArguments = @(
    "-std=c99",
    "-Wall",
    "-Wextra",
    "-Wpedantic",
    "-I$(Join-Path $root 'include')"
) + $sourceFiles + @($mainFile, "-o", $mainOutput)

try {
    if ($useZig) {
        & $compiler.Source -m ziglang cc @mainArguments
    }
    else {
        & $compiler.Source @mainArguments
    }
    if ($LASTEXITCODE -ne 0) {
        throw "La compilacion de la aplicacion fallo con codigo $LASTEXITCODE."
    }

    $consoleInput = @(
        "29/10/2026", "09:00", "20:00",
        "1", "1", "Ana", "100",
        "1", "6", "100", "1", "3", "7001", "0", "10:00",
        "2", "1", "Sala", "10", "100",
        "3", "1", "10", "200", "Preventa", "500", "1", "Edicion",
        "1", "3", "Drama", "01/01/2020", "0",
        "4", "1", "10", "200", "100", "7001",
        "4", "6", "10", "200", "100",
        "4", "1", "10", "200", "100", "7001",
        "3", "6", "10", "200", "1",
        "4", "5", "10", "200", "100",
        "7", "0"
    ) -join [Environment]::NewLine
    $consoleInput += [Environment]::NewLine
    $consoleOutput = $consoleInput | & $mainOutput
    $consoleText = $consoleOutput -join "`n"
    if ($LASTEXITCODE -ne 0 -or $consoleText -notmatch "Reserva cancelada" -or
        $consoleText -notmatch "Venta de la reserva registrada; recaudacion: 500" -or
        $consoleText -notmatch "Recaudacion: 500" -or
        $consoleText -notmatch "Jornada cerrada") {
        throw "La aplicacion no completo el flujo integrado de reservas y cierre."
    }
}
finally {
    if (Test-Path -LiteralPath $mainOutput) {
        Remove-Item -LiteralPath $mainOutput
    }
}
