@echo off
if "%1"=="win7" (
    set "LIB_DIR=lib_win7"
) else (
    set "LIB_DIR=lib_win10"
)

call "G:\vs\2022_community\VC\Auxiliary\Build\vcvarsall.bat" x64
cl.exe /EHsc /Fe:bin\VideoPlayer.exe src\VideoPlayer.cpp /I"include" /link lib\%LIB_DIR%\libmpv.dll.a Shlwapi.lib user32.lib delayimp.lib /DELAYLOAD:libmpv-2.dll /SUBSYSTEM:WINDOWS
echo Build completed!

echo.
echo Usage: compile.bat [win7^|win10]
echo Default is win10