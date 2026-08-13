@echo off
setlocal enabledelayedexpansion

REM ============================================
REM init_build.bat  -  KForge one-click build (CMake)
REM   no args              : cfgure + build all
REM   --cfgure          : cfgure only
REM   --build <target>     : build specific target
REM   --clean              : remove build\ then re-run
REM ============================================

set "ROOT=%~dp0"
set "ROOT=%ROOT:~0,-1%"
set "BUILD=%ROOT%\build"

if "%~1"=="--cfgure" goto :cfgure
if "%~1"=="--build"     goto :build_target
if "%~1"=="--clean"     goto :clean

REM ========== default: cfgure + build all ==========
echo ============================================
echo   KForge - Init Build (CMake)
echo ============================================
echo.

call :detect_generator
if errorlevel 1 goto :err

if not exist "%BUILD%" (
    echo [1/2] Configuring ...
    cmake -S "%ROOT%" -B "%BUILD%" -G "%CMAKE_GEN%" -A x64
    if errorlevel 1 goto :err
) else (
    echo [1/2] [SKIP] build\ already cfgured
)
echo.

echo [2/2] Building all targets ...
cmake --build "%BUILD%" --cfg Release
if errorlevel 1 goto :err

echo.
echo All targets built. Executables are next to their sources.
pause
goto :end

REM ========== --cfgure ==========
:cfgure
call :detect_generator
if errorlevel 1 goto :err
cmake -S "%ROOT%" -B "%BUILD%" -G "%CMAKE_GEN%" -A x64
if errorlevel 1 goto :err
echo [OK] Configured.
pause
goto :end

REM ========== --build <target> ==========
:build_target
if "%~2"=="" (
    echo [ERROR] Usage: init_build.bat --build ^<target^>
    goto :err
)
if not exist "%BUILD%" (
    call :detect_generator
    if errorlevel 1 goto :err
    echo [INFO] build\ not found, cfguring first ...
    cmake -S "%ROOT%" -B "%BUILD%" -G "%CMAKE_GEN%" -A x64
    if errorlevel 1 goto :err
)
cmake --build "%BUILD%" --target %~2 --cfg Release
if errorlevel 1 goto :err
goto :end

REM ========== --clean ==========
:clean
if exist "%BUILD%" rd /s /q "%BUILD%"
echo [OK] build\ removed. Re-running default flow ...
endlocal
call "%~dp0init_build.bat"
exit /b 0

REM ========== detect generator ==========
:detect_generator
where cmake >nul 2>nul
if errorlevel 1 (
    echo [ERROR] cmake not found in PATH.
    goto :err
)
set "CMAKE_GEN=Visual Studio 17 2022"
exit /b 0

REM ========== error handler ==========
:err
echo [ERROR] Task failed.
pause
exit /b 1

:end
endlocal
exit /b 0