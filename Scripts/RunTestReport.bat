::
:: > Notice: Amélie Heinrich @ 2025
:: > Create Time: 2025-05-26 19:28:15
::
@echo off
setlocal

REM Go to the Data folder relative to the script location
cd Data

REM Start HTTP server
echo Starting HTTP server at http://localhost:8000
python -m http.server

REM Wait for exit, then go back (optional)
cd ../
endlocal
