@echo off
echo.
echo Building MGL Delphi 32-bit files...
echo.

set PATH=%DEFPATH%;"C:\Program Files\Borland\Delphi 2.0\BIN"
set UNITS=%SCITECH_LIB%\lib\win32\delphi

dcc32 -q -N%UNITS% mgltypes.pas
if errorlevel 1 goto err

dcc32 -q -N%UNITS% mglfx.pas
if errorlevel 1 goto err

dcc32 -q -N%UNITS% mgl3d.pas
if errorlevel 1 goto err

dcc32 -q -N%UNITS% -B mglglu.pas
if errorlevel 1 goto err

dcc32 -q -N%UNITS% -B mglglut.pas
if errorlevel 1 goto err

dcc32 -q -N%UNITS% -B mglgm.pas
if errorlevel 1 goto err

dcc32 -q -N%UNITS% -B mglspr.pas
if errorlevel 1 goto err

dcc32 -q -N%UNITS% -B ztimer.pas
if errorlevel 1 goto err

brcc32 -fo%UNITS%\glutdlg.res glutdlg.rc
if errorlevel 1 goto err

brcc32 -fo%UNITS%\gmdlg.res gmdlg.rc
if errorlevel 1 goto err

echo DONE
goto end
:err
echo Error occured while building files
:end

