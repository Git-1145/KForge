@echo off
setlocal

set BASE=..\base
set SOURCES=%BASE%\KSON.cpp %BASE%\KLOGGER.cpp %BASE%\KFIO.cpp %BASE%\KCLI.cpp %BASE%\KTIMER.cpp
set FLAGS=/EHsc /std:c++17 /utf-8 /I%BASE%

echo ==========================================
echo  KForge Test Build Script
echo ==========================================
echo.

echo [1/5] Compiling dbgKSON...
cl %FLAGS% %SOURCES% dbgKSON.cpp /Fe:dbgKSON.exe 2>dbgKSON.err
if errorlevel 1 (echo   FAILED & type dbgKSON.err) else (echo   OK)

echo [2/5] Compiling dbgKFIO...
cl %FLAGS% %SOURCES% dbgKFIO.cpp /Fe:dbgKFIO.exe 2>dbgKFIO.err
if errorlevel 1 (echo   FAILED & type dbgKFIO.err) else (echo   OK)

echo [3/5] Compiling dbgKLOGGER...
cl %FLAGS% %SOURCES% dbgKLOGGER.cpp /Fe:dbgKLOGGER.exe 2>dbgKLOGGER.err
if errorlevel 1 (echo   FAILED & type dbgKLOGGER.err) else (echo   OK)

echo [4/5] Compiling dbgKCLI...
cl %FLAGS% %SOURCES% dbgKCLI.cpp /Fe:dbgKCLI.exe 2>dbgKCLI.err
if errorlevel 1 (echo   FAILED & type dbgKCLI.err) else (echo   OK)

echo [5/5] Compiling dbgKTIMER...
cl %FLAGS% %SOURCES% dbgKTIMER.cpp /Fe:dbgKTIMER.exe 2>dbgKTIMER.err
if errorlevel 1 (echo   FAILED & type dbgKTIMER.err) else (echo   OK)

echo.
echo ==========================================
echo  Build complete.
echo ==========================================
