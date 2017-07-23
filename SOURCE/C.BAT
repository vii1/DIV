
copy 586ins.inc include.div /y
copy 386ins.mak makefile /y
copy 386ins.lnk i.lnk /y

wmake /a

wstrip div32run.ins
\source\div\pmode\pmwlite /c4 div32run.ins
copy div32run.ins ..\..\install\div32run.386 /y

copy 586dbg.inc include.div /y
copy 386dbg.mak makefile /y
copy 386dbg.lnk i.lnk /y

wmake /a

wstrip session.div
copy session.div ..\..\system\session.386 /y

copy 586ins.inc include.div /y
copy 586ins.mak makefile /y
copy 586ins.lnk i.lnk /y

wmake /a

wstrip div32run.ins
\source\div\pmode\pmwlite /c4 div32run.ins
copy div32run.ins ..\..\install /y

copy 586dbg.inc include.div /y
copy 586dbg.mak makefile /y
copy 586dbg.lnk i.lnk /y

wmake /a

wstrip session.div
copy session.div ..\..\system /y

@echo Creados SYSTEM\SESSION.DIV/386 y INSTALL\DIV32RUN.INS/386 ...
