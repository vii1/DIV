@echo off
wcc386 -3r pmwbind
IF ERRORLEVEL 1 goto quit
IF ERRORLEVEL 0 goto link
:link
wlink system dos4g op stub=..\pmodew.exe file pmwbind
:quit
