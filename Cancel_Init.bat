@echo off
setlocal

REM ============================================
REM Cancel_Init.bat
REM   Clean up all build artifacts:
REM     - build.bat / fast_build.bat in study/ and test/
REM     - Release/ folders in study/ and test/
REM     - base/KF.lib
REM     - base/obj/
REM     - build/ (CMake, if present)
REM ============================================

echo ============================================
echo   KForge - Cancel Init (Cleanup)
echo ============================================
echo.

set "ROOT=%~dp0"
set "ROOT=%ROOT:~0,-1%"

echo [1/6] Deleting build.bat / fast_build.bat in study\ + test\ ...
for /r "%ROOT%\study" %%b in (build.bat fast_build.bat) do if exist "%%b" del "%%b"
for /r "%ROOT%\test"  %%b in (build.bat fast_build.bat) do if exist "%%b" del "%%b"
echo       Done.

echo [2/6] Deleting Release\ folders in study\ ...
for /d %%d in ("%ROOT%\study\*") do (
    for /d %%e in ("%%d\*") do (
        if exist "%%e\Release" rd /s /q "%%e\Release"
    )
    if exist "%%d\Release" rd /s /q "%%d\Release"
)
echo       Done.

echo [3/6] Deleting Release\ in test\ ...
if exist "%ROOT%\test\Release" rd /s /q "%ROOT%\test\Release"
echo       Done.

echo [4/6] Deleting base\KF.lib ...
if exist "%ROOT%\base\KF.lib" del "%ROOT%\base\KF.lib"
echo       Done.

echo [5/6] Deleting base\obj\ ...
if exist "%ROOT%\base\obj" rd /s /q "%ROOT%\base\obj"
echo       Done.

echo [6/6] Deleting build\ (CMake) ...
if exist "%ROOT%\build" rd /s /q "%ROOT%\build"
echo       Done.

echo.
echo All build artifacts cleaned up.
pause
endlocal
exit /b 0