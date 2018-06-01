setlocal

SET BUILD=maxbuild
SET COMPILER=Visual Studio 11 2012 Win64

SET PFX=%~dp0
cd %PFX%
rmdir %BUILD% /s /q
mkdir %BUILD%
cd %BUILD%

cmake ^
    -DTARGET_DCC=Max ^
    -DMAX_VERSION=2016 ^
    -G "%COMPILER%" ..\

REM cmake --build . --config Release --target INSTALL

pause
