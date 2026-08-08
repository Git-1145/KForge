@echo off
call "I:\msc\pkg\VC\Auxiliary\Build\vcvarsall.bat" x64
set "SDK_VER=10.0.26100.0"
set "INCLUDE=%INCLUDE%;C:\Program Files (x86)\Windows Kits\10\Include\%SDK_VER%\ucrt;C:\Program Files (x86)\Windows Kits\10\Include\%SDK_VER%\shared;C:\Program Files (x86)\Windows Kits\10\Include\%SDK_VER%\um"
set "LIB=%LIB%;C:\Program Files (x86)\Windows Kits\10\Lib\%SDK_VER%\ucrt\x64;C:\Program Files (x86)\Windows Kits\10\Lib\%SDK_VER%\um\x64"
set "BASE=..\base"
set "SOURCES=%BASE%\KSON.cpp %BASE%\KLOGGER.cpp %BASE%\KFIO.cpp %BASE%\KCLI.cpp %BASE%\KTIMER.cpp %BASE%\KBIGNUM.cpp"
set "FLAGS=/EHsc /std:c++17 /utf-8 /I%BASE%"
cl %FLAGS% %SOURCES% dbgKBIGNUM.cpp /Fe:dbgKBIGNUM.exe
