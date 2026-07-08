<#
.SYNOPSIS
  Configure and build phaw for a given CPU target.

.DESCRIPTION
  Copies build/config/args.gn into out/phaw-<cpu>/, runs `gn gen`, then
  `autoninja`. Run inside the base tree's `src/` directory (see fetch-base.ps1).

.EXAMPLE
  pwsh build/build.ps1 -Src C:\src\phaw-base\src -Cpu x86
#>
param(
  [Parameter(Mandatory = $true)][string]$Src,          # path to base tree's src/
  [ValidateSet("x86", "x64")][string]$Cpu = "x64",
  [switch]$CleanGen                                     # force gn re-gen
)

$ErrorActionPreference = "Stop"
$here    = Split-Path -Parent $MyInvocation.MyCommand.Path
$argsGn  = Join-Path $here "config\args.gn"
$outDir  = "out\phaw-$Cpu"

Push-Location $Src
try {
  New-Item -ItemType Directory -Force -Path $outDir | Out-Null

  # Materialize args.gn with the requested target_cpu.
  (Get-Content $argsGn) `
    -replace '^target_cpu\s*=.*', "target_cpu = `"$Cpu`"" |
    Set-Content (Join-Path $outDir "args.gn")

  if ($CleanGen -or -not (Test-Path (Join-Path $outDir "build.ninja"))) {
    Write-Host "==> gn gen $outDir"
    & gn gen $outDir
  }

  Write-Host "==> autoninja -C $outDir chrome  (hours on a clean build)"
  & autoninja -C $outDir chrome

  Write-Host "==> Built. Binary in $Src\$outDir. Copy to a Win8.1 VM to run (see phase-0-spikes.md)."
}
finally {
  Pop-Location
}
