::
:: > Notice: Floating Trees Inc. @ 2025
:: > Create Time: 2025-08-07 00:59:18
::

@echo off
setlocal

:: Validate argument
if "%~1"=="" (
    echo Usage: run_auto_test_runner.bat ^<debug|release|releasedbg^>
    exit /b 1
)

set CONFIG=%~1
if /i not "%CONFIG%"=="debug" if /i not "%CONFIG%"=="release" if /i not "%CONFIG%"=="releasedbg" (
    echo Invalid configuration: %CONFIG%
    echo Must be one of: debug, release, releasedbg
    exit /b 1
)

:: Paths
set SRC=code\auto_test_runner\Main.hs
set OUTDIR=build\windows\x64\%CONFIG%
set OUTEXE=auto_test_runner.exe

:: Save current directory
set ROOTDIR=%CD%

:: Create output directory if it doesn't exist
if not exist "%OUTDIR%" (
    mkdir "%OUTDIR%"
)

:: Compile the Haskell file
ghc -outputdir "%OUTDIR%" -o "%OUTDIR%\%OUTEXE%" "%SRC%"
if errorlevel 1 (
    echo Failed to compile Main.hs
    exit /b 1
)

:: Move into the build output directory
cd /d "%OUTDIR%"
if errorlevel 1 (
    echo Failed to enter output directory.
    exit /b 1
)

:: Run the test runner
"%OUTEXE%"
set EXITCODE=%ERRORLEVEL%

:: Return to root directory
cd /d "%ROOTDIR%"

:: Exit with the runner's exit code
exit /b %EXITCODE%

