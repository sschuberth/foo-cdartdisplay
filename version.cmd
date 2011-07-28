@echo off

setlocal

rem Read the msysGit installation path from the Registry.
:REG_QUERY
for /f "skip=2 delims=: tokens=1*" %%a in ('reg query HKLM\SOFTWARE%WOW%\Microsoft\Windows\CurrentVersion\Uninstall\Git_is1 /v InstallLocation 2^> nul') do (
    for /f "tokens=3" %%i in ("%%a") do (
        set GIT=%%i:%%b
    )
)
if "%GIT%"=="" (
    if "%WOW%"=="" (
        rem Assume we are on 64-bit Windows, so explicitly read the 32-bit Registry.
        set WOW=\Wow6432Node
        goto REG_QUERY
    )
)

pushd "%~dp0"
for /f "delims=" %%l in ('"%GIT%\bin\git.exe" describe --dirty --tags') do (
    echo // The component's full name.>version.inl
    echo #define FOO_COMP_NAME    "CD Art Display Interface">>version.inl
    echo // Major and minor CAD version number to target at.>>version.inl
    echo #define FOO_COMP_VERSION "3.0">>version.inl
    echo // Additional information that identifies the build.>>version.inl
    for /f "delims=- tokens=5" %%t in ("%%l") do (
        echo #define FOO_COMP_BUILD   "%%t">>version.inl
    )
)
popd
