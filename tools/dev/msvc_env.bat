@echo off
rem ---------------------------------------------------------------------------
rem Enters the MSVC x64 developer environment and prepends the CMake/Ninja that
rem ship with Visual Studio to PATH, then runs the given command.
rem
rem Usage (from any shell):
rem   tools\dev\msvc_env.bat cmake --preset windows-debug
rem
rem This script only changes the environment of the process it starts. It never
rem edits system settings and never installs anything.
rem
rem Note: variables are set outside parenthesized blocks on purpose. Inside a
rem block, %VAR% expands when the block is parsed, so a file name set in the
rem same block would be empty. Keeping the flow flat avoids delayed expansion.
rem ---------------------------------------------------------------------------
setlocal

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" set "VSWHERE=%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"

set "VSPATH="
if not exist "%VSWHERE%" goto :fallback

set "VSPATHFILE=%TEMP%\peri_vs_path_%RANDOM%%RANDOM%.txt"
"%VSWHERE%" -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath > "%VSPATHFILE%" 2>nul
for /f "usebackq delims=" %%i in ("%VSPATHFILE%") do set "VSPATH=%%i"
del "%VSPATHFILE%" >nul 2>nul

:fallback
if defined VSPATH goto :found

for %%e in (Community Professional Enterprise BuildTools) do (
    if not defined VSPATH if exist "%ProgramFiles%\Microsoft Visual Studio\2022\%%e\VC\Auxiliary\Build\vcvars64.bat" set "VSPATH=%ProgramFiles%\Microsoft Visual Studio\2022\%%e"
    if not defined VSPATH if exist "%ProgramFiles%\Microsoft Visual Studio\18\%%e\VC\Auxiliary\Build\vcvars64.bat" set "VSPATH=%ProgramFiles%\Microsoft Visual Studio\18\%%e"
    if not defined VSPATH if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\%%e\VC\Auxiliary\Build\vcvars64.bat" set "VSPATH=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\%%e"
)

:found
if not defined VSPATH (
    echo [msvc_env] No Visual Studio installation with the C++ toolset was found.
    echo [msvc_env] Install the "Desktop development with C++" workload.
    exit /b 1
)

rem vcvars prints an internal vswhere warning on some installs; discard both
rem streams, then rely on the exit code for real failures.
call "%VSPATH%\VC\Auxiliary\Build\vcvars64.bat" >nul 2>nul
if errorlevel 1 (
    echo [msvc_env] Failed to initialize the MSVC environment.
    exit /b 1
)

set "VSCMAKE=%VSPATH%\Common7\IDE\CommonExtensions\Microsoft\CMake"
if exist "%VSCMAKE%\CMake\bin" set "PATH=%VSCMAKE%\CMake\bin;%PATH%"
if exist "%VSCMAKE%\Ninja" set "PATH=%VSCMAKE%\Ninja;%PATH%"
if exist "%VSPATH%\VC\Tools\Llvm\x64\bin" set "PATH=%VSPATH%\VC\Tools\Llvm\x64\bin;%PATH%"

%*
exit /b %ERRORLEVEL%
