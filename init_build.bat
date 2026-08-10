@echo off
setlocal enabledelayedexpansion

REM ============================================
REM init_build.bat
REM   no args  : delete all build.bat, precompile base/obj + KF.lib, generate build.bat
REM   --setup  : MSVC env setup only
REM   --build-dir "path" : setup + compile+link all cpp in path
REM ============================================

if "%~1"=="--setup"      goto :setup_only
if "%~1"=="--build-dir"  goto :build_dir

REM ========== Main: delete + precompile + generate ==========
echo ============================================
echo   KForge - Init Build
echo ============================================
echo.

set "ROOT=%~dp0"
set "ROOT=%ROOT:~0,-1%"
set "BASE=%ROOT%\base"

echo [1/4] Deleting all build.bat ...
for /r "%ROOT%\test" %%b in (build.bat) do if exist "%%b" del "%%b"
for /r "%ROOT%\study" %%b in (build.bat) do if exist "%%b" del "%%b"
echo       Done.
echo.

echo [2/4] MSVC environment setup ...
call :msvc_setup
if errorlevel 1 (
    echo [ERROR] MSVC setup failed.
    pause
    exit /b 1
)
echo       Done.
echo.

echo [3/4] Precompiling base\obj + KF.lib ...
call :precompile_base
if errorlevel 1 (
    echo [ERROR] Base precompilation failed.
    pause
    exit /b 1
)
echo.

echo [4/4] Generating per-directory build.bat ...
call :gen_builds "%ROOT%\test"
call :gen_builds "%ROOT%\study"
echo       Done.
echo.

echo All build scripts generated.
echo Run build.bat in any subdirectory of test\ or study\ to compile.
echo.
pause
endlocal
exit /b 0

REM ========== --build-dir: compile+link one directory ==========
:build_dir
call :msvc_setup
if errorlevel 1 exit /b 1

set "ROOT=%~dp0"
set "ROOT=%ROOT:~0,-1%"
set "BASE=%ROOT%\base"
set "KFLIB=%BASE%\KF.lib"
set "CXXFLAGS=/O2 /std:c++17 /utf-8 /EHsc /MD /I%BASE%"
set "LINKFLAGS=/OPT:REF /OPT:ICF /SUBSYSTEM:CONSOLE"

REM Ensure KF.lib exists - precompile if missing
if not exist "%KFLIB%" (
    echo   [INFO] KF.lib not found, precompiling ...
    call :precompile_base
    if errorlevel 1 exit /b 1
)

set "dir=%~2"
if "!dir:~-1!"=="\" set "dir=!dir:~0,-1!"

echo.
echo ===== [!dir!] =====

REM List .cpp files into numbered array
set "filecount=0"
for %%f in ("%dir%\*.cpp") do (
    set /a filecount+=1
    set "file_!filecount!=%%~nxf"
    set "filepath_!filecount!=%%f"
)

if "!filecount!"=="0" (
    echo   [INFO] No .cpp files found.
    pause
    exit /b 0
)

REM Show file list
echo   Files in this directory:
for /l %%i in (1,1,!filecount!) do (
    echo     %%i. !file_%%i!
)
echo.

REM Prompt for selection
set "choice="
set /p "choice=  Enter numbers (e.g. 1,3,5) or Enter for all: "

REM Determine mode: if choice undefined or only spaces/commas -> compile all
set "compileall=1"
if defined choice (
    set "sel=!choice:,= !"
    set "seltrimmed=!sel: =!"
    if not "!seltrimmed!"=="" set "compileall=0"
)

if "!compileall!"=="1" (
    REM Compile all - incremental (force=0)
    for /l %%i in (1,1,!filecount!) do (
        call :compile_one "!filepath_%%i!" 0
        if errorlevel 1 (
            pause
            exit /b 1
        )
    )
) else (
    REM Compile selected - force rebuild (force=1)
    for %%n in (!sel!) do (
        if defined filepath_%%n (
            call :compile_one "!filepath_%%n!" 1
            if errorlevel 1 (
                pause
                exit /b 1
            )
        ) else (
            echo   [WARN] Invalid number: %%n
        )
    )
)

echo   [DONE]
pause
endlocal
exit /b 0

REM ========== :compile_one ==========
REM %1 = file path, %2 = force (0=incremental, 1=force rebuild)
:compile_one
set "cf=%~1"
set "cfn=%~n1"
set "cfx=%~x1"
set "force=%~2"

set "build=1"
if "!force!"=="0" (
    if exist "%dir%\%cfn%.exe" (
        for %%a in ("%cf%") do for %%b in ("%dir%\%cfn%.exe") do (
            if "%%~ta" LEQ "%%~tb" set "build=0"
        )
    )
)

if "!build!"=="1" (
    if exist "%dir%\%cfn%.exe" del "%dir%\%cfn%.exe"
    echo   [BUILD] %cfn%%cfx%

    if exist "%dir%\.ktemp" rd /s /q "%dir%\.ktemp"
    mkdir "%dir%\.ktemp"

    cl /c %CXXFLAGS% "%cf%" /Fo:"%dir%\.ktemp\%cfn%.obj"
    if errorlevel 1 (
        echo   [FAIL] %cfn%%cfx% - compilation error
        rd /s /q "%dir%\.ktemp"
        exit /b 1
    )

    link %LINKFLAGS% "%KFLIB%" "%dir%\.ktemp\%cfn%.obj" /out:"%dir%\%cfn%.exe"
    if errorlevel 1 (
        echo   [RETRY] Link failed, recompiling KF.lib ...
        if exist "%KFLIB%" del "%KFLIB%"
        call :precompile_base
        if errorlevel 1 (
            echo   [FAIL] %cfn%%cfx% - KF.lib recompile error
            rd /s /q "%dir%\.ktemp"
            exit /b 1
        )
        link %LINKFLAGS% "%KFLIB%" "%dir%\.ktemp\%cfn%.obj" /out:"%dir%\%cfn%.exe"
        if errorlevel 1 (
            echo   [FAIL] %cfn%%cfx% - linking error
            rd /s /q "%dir%\.ktemp"
            exit /b 1
        )
    )

    rd /s /q "%dir%\.ktemp"
    echo   [OK] %cfn%%cfx%
) else (
    echo   [SKIP] %cfn%%cfx%
)
exit /b 0

REM ========== --setup: MSVC env only ==========
:setup_only
call :msvc_setup
exit /b %errorlevel%

REM ========== :msvc_setup ==========
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
call "%VSROOT%\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul 2>nul
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

REM ========== :precompile_base ==========
REM Compiles base/*.cpp to base/obj/*.obj (incremental)
REM Then packs all .obj into base/KF.lib (if any recompiled or lib missing)
:precompile_base
set "BASE_OBJ=%BASE%\obj"
set "KFLIB=%BASE%\KF.lib"
set "PCFLAGS=/O2 /std:c++17 /utf-8 /EHsc /MD /I%BASE%"
if not exist "%BASE_OBJ%" mkdir "%BASE_OBJ%"

set "recompiled=0"
for %%f in ("%BASE%\*.cpp") do (
    set "src=%%f"
    set "obj=%BASE_OBJ%\%%~nf.obj"
    set "need=1"
    if exist "!obj!" (
        for %%a in ("!src!") do for %%b in ("!obj!") do (
            if "%%~ta" LEQ "%%~tb" set "need=0"
        )
    )
    if "!need!"=="1" (
        echo   [COMPILE] %%~nxf
        cl /c !PCFLAGS! "%%f" /Fo:"!obj!"
        if errorlevel 1 (
            echo [ERROR] Failed to compile base: %%~nxf
            exit /b 1
        )
        set "recompiled=1"
    ) else (
        echo   [SKIP] %%~nxf
    )
)

REM (Re)create KF.lib if any .obj was recompiled or lib is missing
if "!recompiled!"=="1" goto :build_lib
if not exist "%KFLIB%" goto :build_lib
echo   [SKIP] KF.lib up-to-date
exit /b 0

:build_lib
echo   [LIB] Creating KF.lib ...
set "LIBOBJS="
for %%f in ("%BASE_OBJ%\*.obj") do set "LIBOBJS=!LIBOBJS! %%f"
lib /out:"%KFLIB%" !LIBOBJS! >nul 2>nul
if errorlevel 1 (
    echo [ERROR] Failed to create KF.lib
    exit /b 1
)
echo   [LIB] KF.lib created.
exit /b 0

REM ========== :gen_builds - recursive generate ==========
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
