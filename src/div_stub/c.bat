@echo off
wcc -bt=DOS -ms div_stub.cpp
wasm cstrt086 -bt=DOS -ms -0r_
wlink @div_stub.lnk
rem lzexe div_stub.exe
rem bin2h div_stub.exe
rem echo Se ha creado el nuevo div_stub.h, qu¡tale la coma y c¢pialo donde est‚ DIV
rem echo ­OJO!, la longitud del div_stub.exe se debe indicar en i.cpp y divc.cpp (x2)
