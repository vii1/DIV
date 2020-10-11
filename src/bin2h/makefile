EXE = bin2h
!ifdef __MSDOS__
EXE = $(EXE).exe
!endif
!ifdef __NT__
EXE = $(EXE).exe
!endif
!ifdef __UNIX__
DELETE = rm
!else
DELETE = del
!endif

CL = wcl386 -bt

$(EXE): bin2h.c
	$(CL) $<

.SILENT
clean: .SYMBOLIC
	-$(DELETE) $(EXE)
	-$(DELETE) bin2h.obj
	-$(DELETE) bin2h.err
