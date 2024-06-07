setlocal

SET MAYA_VERSION=2024
REM debug debugoptimized release
SET BUILDTYPE=debug
SET BUILDDIR=mayabuild_%BUILDTYPE%_%MAYA_VERSION%

if not exist %BUILDDIR%\ (
    meson setup -Dmaya:maya_version=%MAYA_VERSION% --buildtype %BUILDTYPE% --vsenv %BUILDDIR%
)

if exist %BUILDDIR%\ (
    meson compile -C %BUILDDIR%
    meson install -C %BUILDDIR%
)

pause
