@echo off
rem Fetch the phaw base tree (Supermium/Chromium) at the pinned tag, using
rem cmd.exe (no PowerShell). Equivalent to fetch-base.ps1.
rem
rem Run on the BUILD box (Windows 10/11 or Linux), NOT on Windows 8.1. Requires
rem Chromium depot_tools on PATH. Pulls tens of GB and takes a long time.
rem
rem Usage:  build\fetch-base.bat <checkout-root>
rem   e.g.  build\fetch-base.bat C:\src\phaw-base
setlocal

if "%~1"=="" (
  echo Usage: %~nx0 ^<checkout-root^>
  echo   e.g. %~nx0 C:\src\phaw-base
  exit /b 1
)
set "ROOT=%~1"
set "HERE=%~dp0"
set "GCLIENTFILE=%HERE%config\phaw.gclient"

where gclient >nul 2>nul || (
  echo ERROR: depot_tools not found on PATH. See docs\win81-build.md.
  exit /b 1
)

findstr /c:"REPLACE_WITH_SUPERMIUM_TAG" "%GCLIENTFILE%" >nul && (
  echo ERROR: phaw.gclient still has REPLACE_WITH_SUPERMIUM_TAG.
  echo        Pin the Supermium tag first ^(Phase 0^).
  exit /b 1
)

if not exist "%ROOT%" mkdir "%ROOT%"
copy /Y "%GCLIENTFILE%" "%ROOT%\.gclient" >nul

pushd "%ROOT%"
echo ==^> gclient sync ^(large and slow; reads Supermium's DEPS^) ...
call gclient sync --no-history
set "RC=%ERRORLEVEL%"
popd

if not "%RC%"=="0" (
  echo ERROR: gclient sync failed ^(exit %RC%^).
  exit /b %RC%
)
echo ==^> Base fetched into %ROOT%. Next: apply patches\, then build\build.bat
endlocal
