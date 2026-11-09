param(
    [Parameter(Mandatory = $true)][string]$QtRoot,
    [string]$CompilerBin,
    [string]$BuildDir = "$PSScriptRoot/../build/windows-native",
    [string]$InstallDir = "$PSScriptRoot/../dist/windows",
    [int]$Jobs = 4
)
$ErrorActionPreference = 'Stop'
$sourceDir = (Resolve-Path "$PSScriptRoot/..").Path
$QtRoot = (Resolve-Path $QtRoot).Path
if (!(Test-Path "$QtRoot/lib/cmake/Qt6/Qt6Config.cmake")) {
    throw 'QtRoot must point to a desktop Windows Qt kit, e.g. C:/Qt/6.11.1/mingw_64.'
}
$env:PATH = "$QtRoot/bin;$env:PATH"
if ($CompilerBin) { $env:PATH = "$( (Resolve-Path $CompilerBin).Path );$env:PATH" }
function Invoke-CMake {
    & cmake @args
    if ($LASTEXITCODE -ne 0) { throw "CMake failed ($LASTEXITCODE). See the output above." }
}
Invoke-CMake -S $sourceDir -B $BuildDir -G Ninja "-DCMAKE_PREFIX_PATH=$QtRoot" -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DAURORA_WITH_WEBENGINE=OFF
Invoke-CMake --build $BuildDir --parallel $Jobs
Invoke-CMake --install $BuildDir --prefix $InstallDir --config Release
Write-Host "Ready: $InstallDir/bin/AuroraPlayer.exe (copy the entire $InstallDir directory)."
