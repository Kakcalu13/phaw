@echo off
rem Configure and build phaw for a CPU target, using cmd.exe (no PowerShell).
rem Equivalent to build.ps1. Run inside/against the base tree's src\ directory.
rem The output is a plain chrome.exe you copy to Win8.1 and double-click.
rem
rem Usage:  build\build.bat <src-dir> [x86^|x64]
rem   <src-dir> = the base tree's src\ directory (see fetch-base.bat)
rem   e.g.  build\build.bat C:\src\phaw-base\src x86
setlocal

if "%~1"=="" (
  echo Usage: %~nx0 ^<src-dir^> [x86^|x64]
  echo   e.g. %~nx0 C:\src\phaw-base\src x86
  exit /b 1
)
set "SRC=%~1"
set "CPU=%~2"
if "%CPU%"=="" set "CPU=x64"
if /I not "%CPU%"=="x86" if /I not "%CPU%"=="x64" (
  echo ERROR: CPU must be x86 or x64.
  exit /b 1
)

set "HERE=%~dp0"
set "ARGSGN=%HERE%config\args.gn"
set "OUTDIR=out\phaw-%CPU%"

pushd "%SRC%" || (
  echo ERROR: src dir not found: %SRC%
  exit /b 1
)
if not exist "%OUTDIR%" mkdir "%OUTDIR%"

rem Materialize args.gn: strip the pinned target_cpu line, append the requested
rem one. (GN forbids duplicate assignment, so we replace rather than add.)
findstr /v /b /c:"target_cpu" "%ARGSGN%" > "%OUTDIR%\args.gn"
>>"%OUTDIR%\args.gn" echo target_cpu = "%CPU%"

if not exist "%OUTDIR%\build.ninja" (
  echo ==^> gn gen %OUTDIR%
  call gn gen "%OUTDIR%" || ( popd & exit /b 1 )
)

echo ==^> autoninja -C %OUTDIR% chrome  ^(hours on a clean build^)
call autoninja -C "%OUTDIR%" chrome
set "RC=%ERRORLEVEL%"
popd

if not "%RC%"=="0" (
  echo ERROR: build failed ^(exit %RC%^).
  exit /b %RC%
)
echo ==^> Built. Binary in %SRC%\%OUTDIR%.
echo     Copy that folder to Win8.1 and run chrome.exe (no PowerShell needed).
endlocal
