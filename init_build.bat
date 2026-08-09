@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

echo ============================================
echo   KForge - Init Build
echo ============================================
echo.

REM ============================================
REM --setup : only MSVC environment
REM ============================================
if "%~1"=="--setup" goto :setup_only

REM ============================================
REM --build-dir <dir> : build one directory
REM ============================================
if "%~1"=="--build-dir" goto :build_dir

REM ============================================
REM --gen : only generate per-dir build.bat files
REM ============================================
if "%~1"=="--gen" goto :gen_only

REM ============================================
REM Full mode: setup + gen + build all
REM ============================================
call :msvc_setup
if errorlevel 1 exit /b 1

set "ROOT=%~dp0"
set "ROOT=%ROOT:~0,-1%"

echo [GEN] Generating per-directory build.bat ...
call :gen_builds "%ROOT%\test"
call :gen_builds "%ROOT%\study"
echo.

set "total_built=0"
set "total_skipped=0"

echo [BUILD] Building test\ ...
call :traverse "%ROOT%\test"
echo.
echo [BUILD] Building study\ ...
call :traverse "%ROOT%\study"

echo.
echo ============================================
echo   Build Summary: !total_built! built, !total_skipped! skipped
echo ============================================
pause
endlocal
exit /b 0

REM ============================================
REM --gen only
REM ============================================
:gen_only
set "ROOT=%~dp0"
set "ROOT=%ROOT:~0,-1%"
echo [GEN] Generating per-directory build.bat ...
call :gen_builds "%ROOT%\test"
call :gen_builds "%ROOT%\study"
echo [DONE] Build scripts generated.
pause
endlocal
exit /b 0

REM ============================================
REM --setup only
REM ============================================
:setup_only
call :msvc_setup
exit /b %errorlevel%

REM ============================================
REM --build-dir: build a single directory
REM ============================================
:build_dir
call :msvc_setup
if errorlevel 1 exit /b 1
set "dir=%~2"
set "dir=%dir:~0,-1%"
set "ROOT=%~dp0"
set "ROOT=%ROOT:~0,-1%"
set "BASE=%ROOT%\base"
set "CXXFLAGS=/O2 /std:c++17 /utf-8 /EHsc /MD /I%BASE%"
set "SOURCES=%BASE%\KSON.cpp %BASE%\KLOGGER.cpp %BASE%\KFIO.cpp %BASE%\KCLI.cpp %BASE%\KTIMER.cpp %BASE%\KBIGNUM.cpp"
set "LINKFLAGS=/OPT:REF /OPT:ICF"
set "has=0"
for %%f in ("%dir%\*.cpp") do set "has=1"
if "!has!"=="0" (
    echo [INFO] No .cpp files found.
    pause
    exit /b 0
)
echo.
echo ===== [%dir%] =====
pushd "%dir%"
for %%f in (*.cpp) do (
    set "build=1"
    if exist "%%~nf.exe" (
        for %%a in ("%%f") do for %%b in ("%%~nf.exe") do (
            if "%%~ta" LEQ "%%~tb" set "build=0"
        )
    )
    if "!build!"=="1" (
        if exist "%%~nf.exe" del "%%~nf.exe"
        echo   [BUILD] %%~nxf
        cl %CXXFLAGS% %SOURCES% "%%f" /Fe:"%%~nf.exe" /link %LINKFLAGS%
        if errorlevel 1 (echo   [FAIL] %%~nxf)
    ) else (
        echo   [SKIP] %%~nxf
    )
)
del /q *.obj 2>nul
popd
echo [DONE]
pause
endlocal
exit /b 0

REM ============================================
REM :msvc_setup - MSVC + SDK environment
REM ============================================
:msvc_setup
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" set "VSWHERE=%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
    echo [ERROR] vswhere.exe not found - install VS 2022
    exit /b 1
)
for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -property installationPath`) do set "VSROOT=%%i"
if not defined VSROOT (
    echo [ERROR] VS installation not found
    exit /b 1
)
call "%VSROOT%\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul
if errorlevel 1 (
    echo [ERROR] vcvarsall.bat failed
    exit /b 1
)
set "SDKROOT=%ProgramFiles(x86)%\Windows Kits\10"
if not exist "%SDKROOT%\Include" set "SDKROOT=%ProgramFiles%\Windows Kits\10"
for /f "usebackq delims=" %%s in (`dir "%SDKROOT%\Include" /b /ad /o-n 2^>nul`) do (
    set "SDKVER=%%s"
    goto :sdkdone
)
:sdkdone
if defined SDKVER (
    if exist "%SDKROOT%\Include\%SDKVER%\ucrt" (
        set "INCLUDE=%SDKROOT%\Include\%SDKVER%\ucrt;%SDKROOT%\Include\%SDKVER%\um;%SDKROOT%\Include\%SDKVER%\shared;%INCLUDE%"
        set "LIB=%SDKROOT%\Lib\%SDKVER%\ucrt\x64;%SDKROOT%\Lib\%SDKVER%\um\x64;%LIB%"
    )
)
exit /b 0

REM ============================================
REM :gen_builds - generate build.bat in each dir with .cpp files
REM ============================================
:gen_builds
set "dir=%~1"
set "has=0"
for %%f in ("%dir%\*.cpp") do set "has=1"
if "!has!"=="1" (
    set "out=%dir%\build.bat"
    echo   [GEN] !out!
    > "!out!" echo @echo off
    >>"!out!" echo call "%ROOT%\init_build.bat" --build-dir "%%~dp0"
    >>"!out!" echo pause
)
for /d %%d in ("%dir%\*") do call :gen_builds "%%d"
exit /b 0

REM ============================================
REM :traverse - recursively build all .cpp in dir
REM ============================================
:traverse
set "dir=%~1"
set "has=0"
for %%f in ("%dir%\*.cpp") do set "has=1"
if "!has!"=="1" (
    echo.
    echo ===== [!dir!] =====
    pushd "!dir!"
    for %%f in (*.cpp) do (
        set "build=1"
        if exist "%%~nf.exe" (
            for %%a in ("%%f") do for %%b in ("%%~nf.exe") do (
                if "%%~ta" LEQ "%%~tb" set "build=0"
            )
        )
        if "!build!"=="1" (
            if exist "%%~nf.exe" del "%%~nf.exe"
            echo   [BUILD] %%~nxf
            cl %CXXFLAGS% %SOURCES% "%%f" /Fe:"%%~nf.exe" /link %LINKFLAGS%
            if not errorlevel 1 (
                set /a total_built+=1
            ) else (
                echo   [FAIL] %%~nxf
            )
        ) else (
            echo   [SKIP] %%~nxf
            set /a total_skipped+=1
        )
    )
    del /q *.obj 2>nul
    popd
)
for /d %%d in ("%dir%\*") do call :traverse "%%d"
exit /b 0