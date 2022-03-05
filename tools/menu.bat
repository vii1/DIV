@echo off
set RUTA=C:\DIV
set DEBUGGER=wd -tr=rsi -ini=colour

set CPU=586
set DIV=D.EXE
set SESSION=SYSTEM\SESSION.DIV

set LAUNCHER=SYSTEM\DOS4GW.EXE
set DEPURAR=NO

:menu
mode co80
cls
echo ==================
echo  DIV Games Studio
echo ==================
echo.
echo   CPU actual: %CPU%
echo   Depurar:    %DEPURAR%
echo.
echo 1. Iniciar DIV
echo 2. Volver de la ejecuci¢n
echo 3. Volver de la ejecuci¢n con error
echo 4. Probar modo de v¡deo
echo 5. Ejecutar juego compilado
echo.
echo 6. Cambiar CPU
echo 7. Activar/desactivar depurador
echo.
echo 8. Salir
echo.
choice /C:12345678 /N Opci¢n: 
echo.
if errorlevel 8 goto salir
if errorlevel 7 goto cambiardepurar
if errorlevel 6 goto cambiarcpu
if errorlevel 5 goto exec
if errorlevel 4 goto test
if errorlevel 3 goto error
if errorlevel 2 goto next
if errorlevel 1 goto init
goto menu

:init
cd %RUTA%
%LAUNCHER% %DIV% INIT
if errorlevel 128 pause
goto menu

:next
cd %RUTA%
%LAUNCHER% %DIV% NEXT
if errorlevel 128 pause
goto menu

:error
cd %RUTA%
%LAUNCHER% %DIV% ERROR
if errorlevel 128 pause
goto menu

:test
cd %RUTA%
%LAUNCHER% %DIV% TEST
if errorlevel 128 pause
goto menu

:exec
cd %RUTA%
if not exist SYSTEM\EXEC.EXE goto exec_noexiste
%LAUNCHER% %SESSION% SYSTEM\EXEC.EXE
goto menu
:exec_noexiste
echo No se encuentra EXEC.EXE. Compila un juego primero.
pause
goto menu

:cambiarcpu
if "%CPU%"=="386" goto set_586
set CPU=386
set DIV=SYSTEM\D.386
set SESSION=SYSTEM\SESSION.386
goto menu
:set_586
set CPU=586
set DIV=D.EXE
set SESSION=SYSTEM\SESSION.DIV
goto menu

:cambiardepurar
if "%DEPURAR%"=="SI" goto depurar_no
set LAUNCHER=%DEBUGGER%
set DEPURAR=SI
goto menu
:depurar_no
set LAUNCHER=SYSTEM\DOS4GW.EXE
set DEPURAR=NO
goto menu

:salir
set RUTA=
set CPU=
set DIV=
set SESSION=
set LAUNCHER=
set DEPURAR=
set DEBUGGER=
