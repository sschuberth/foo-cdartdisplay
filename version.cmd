@echo off

setlocal enabledelayedexpansion

rem Read the Git for Windows installation path from the Registry.
set REG=64
:REG_QUERY
for /f "skip=2 delims=: tokens=1*" %%a in ('reg query HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Git_is1 /v InstallLocation /reg:%REG% 2^> nul') do (
    for /f "tokens=3" %%i in ("%%a") do (
        set GIT=%%i:%%b
    )
)
if "%GIT%"=="" (
    if "%REG%"=="64" (
        rem Assume we are on 64-bit Windows, so explicitly read the 32-bit Registry.
        set WOW=32
        goto REG_QUERY
    )
)

if "%GIT%"=="" (
    echo Error: No installation of Git for Windows found.
    exit /b 1
)

echo Using Git at %GIT%...

pushd "%~dp0"
for /f "delims=" %%l in ('"%GIT%\bin\git.exe" describe --tags') do (
    echo Description: %%l

    for /f "delims=- tokens=2" %%t in ("%%l") do (
        set VERSION=%%t
    )
    echo Version: !VERSION!

    for /f "delims=- tokens=4*" %%t in ("%%l") do (
        set REVISION=%%t
        set INFO=%%u
    )
    echo Revision: !REVISION!
    echo Info: !INFO!

    echo // The component's full name.>version.inl
    echo #define FOO_COMP_NAME    "CD Art Display Interface">>version.inl

    echo // Major and minor CAD version number to target at.>>version.inl
    echo #define FOO_COMP_VERSION "!VERSION!">>version.inl

    if not "!INFO!"=="" (
        set REVISION=!REVISION!.!INFO!
    )

    echo // Additional information that identifies the build.>>version.inl
    echo #define FOO_COMP_BUILD   "!REVISION!">>version.inl
)
popd
