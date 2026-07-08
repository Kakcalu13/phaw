<#
.SYNOPSIS
  Fetch the phaw base tree (Supermium/Chromium) at the pinned tag.

.DESCRIPTION
  Runs gclient with build/config/phaw.gclient. Requires depot_tools on PATH.
  Run on the build box (Windows/Linux) — not the authoring host. This pulls
  tens of GB and takes a long time; it is Phase 0, Spike A, step 1.

.EXAMPLE
  pwsh build/fetch-base.ps1 -Root C:\src\phaw-base
#>
param(
  [Parameter(Mandatory = $true)]
  [string]$Root                      # where to check out the base tree
)

$ErrorActionPreference = "Stop"
$here = Split-Path -Parent $MyInvocation.MyCommand.Path
$gclientFile = Join-Path $here "config\phaw.gclient"

if (-not (Get-Command gclient -ErrorAction SilentlyContinue)) {
  throw "depot_tools not found on PATH. Install it and re-run. See docs/win81-build.md."
}

$pin = Select-String -Path $gclientFile -Pattern "REPLACE_WITH_SUPERMIUM_TAG"
if ($pin) {
  throw "phaw.gclient still has REPLACE_WITH_SUPERMIUM_TAG. Pin the Supermium tag first (Phase 0)."
}

New-Item -ItemType Directory -Force -Path $Root | Out-Null
Push-Location $Root
try {
  Copy-Item $gclientFile (Join-Path $Root ".gclient") -Force
  # Supermium is a full fork; gclient sync reads its in-tree DEPS to pull deps.
  # --no-history keeps the checkout smaller. Confirm exact flags against
  # Supermium's docs/ (some deps dislike shallow); adjust here if their docs differ.
  Write-Host "==> gclient sync (large and slow; reads Supermium's DEPS) ..."
  & gclient sync --no-history
  Write-Host "==> Base fetched into $Root. Next: apply patches/, then build/build.ps1"
}
finally {
  Pop-Location
}
