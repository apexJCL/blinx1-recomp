@echo off
rem blinx1: the project CLI (blinx1.py). Python 3.9 or newer.
where py >nul 2>nul
if %errorlevel%==0 (
    py -3 "%~dp0blinx1.py" %*
) else (
    python "%~dp0blinx1.py" %*
)
exit /b %errorlevel%
