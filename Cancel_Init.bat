@echo off
setlocal enabledelayedexpansion

REM ============================================
REM Cancel_Init.bat
REM   Deletes all build.bat, base/KF.lib, base/obj/,
REM   .exe, and .ktemp in test/ and study/.
REM ============================================

echo ============================================
echo   KForge - Cancel Init (Cleanup)
echo ============================================
echo.

set "ROOT=%~dp0"
set "ROOT=%ROOT:~0,-1%"

echo [1/5] Deleting all build.bat ...
for /r "%ROOT%\test" %%b in (build.bat) do if exist "%%b" del "%%b"
for /r "%ROOT%\study" %%b in (build.bat) do if exist "%%b" del "%%b"
echo       Done.

echo [2/5] Deleting base\KF.lib ...
if exist "%ROOT%\base\KF.lib" del "%ROOT%\base\KF.lib"
echo       Done.

echo [3/5] Deleting base\obj\ ...
if exist "%ROOT%\base\obj" rd /s /q "%ROOT%\base\obj"
echo       Done.

echo [4/5] Deleting all .exe ...
for /r "%ROOT%\test" %%e in (*.exe) do if exist "%%e" del "%%e"
for /r "%ROOT%\study" %%e in (*.exe) do if exist "%%e" del "%%e"
echo       Done.

echo [5/5] Deleting .ktemp folders ...
for /r "%ROOT%\test" %%d in (.ktemp) do if exist "%%d" rd /s /q "%%d"
for /r "%ROOT%\study" %%d in (.ktemp) do if exist "%%d" rd /s /q "%%d"
echo       Done.

echo.
echo All build artifacts cleaned up.
pause
endlocal
exit /b 0
