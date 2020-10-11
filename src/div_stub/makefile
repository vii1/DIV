# As¡ se compilar¡a el div_stub pero de momento no podemos hacerlo ya que
# si cambia su tama¤o en un solo byte se rompe la compatibilidad con la
# DIV32RUN.DLL de DIV2.

!ifndef SEP
!ifdef __UNIX__
SEP = /
!else
SEP = \
!endif
!endif

!ifdef __UNIX__
DELETE = rm
!else
DELETE = del
!endif

OUTDIR=$(CONFIG)

!ifeq CONFIG debug
OPTIONS=-bt=dos -d2
!else # CONFIG = release
OPTIONS=-bt=dos
!endif

.BEFORE
	@if not exist $(OUTDIR) mkdir $(OUTDIR)

H=$(OUTDIR)$(SEP)div_stub.h
EXE=$(OUTDIR)$(SEP)div_stub.exe
BIN2H = ..$(SEP)bin2h$(SEP)bin2h
!ifdef __MSDOS__
BIN2H = $(BIN2H).exe
!endif
!ifdef __NT__
BIN2H = $(BIN2H).exe
!endif

$(H): $(BIN2H) $(EXE)
	$(BIN2H) $(EXE) div_stub > $@

$(BIN2H): ../bin2h/makefile ../bin2h/bin2h.c
	cd ../bin2h
	$(MAKE)
	cd ../div_stub
	
$(EXE): $(OUTDIR)/exec.obj
	wlink &
        system dos &
        name $(EXE) &
		option stack=512 &
        file $(OUTDIR)/exec.obj

$(OUTDIR)/exec.obj: asm/exec.asm
#	wcc $(OPTIONS) -fo=$@ -fr=$^* div_stub.cpp
	wasm asm$(SEP)exec.asm $(OPTIONS) -fo=$@ -fr=$^*

#$(OUTDIR)/cstrt086.obj:
#	wasm asm$(SEP)cstrt086.asm $(OPTIONS) -fo=$@ -fr=$^*

.SILENT
clean: .SYMBOLIC
	if exist $(EXE) $(DELETE) $(EXE)
	if exist $(OUTDIR)$(SEP)*.obj $(DELETE) $(OUTDIR)$(SEP)*.obj
	if exist $(OUTDIR)$(SEP)*.err $(DELETE) $(OUTDIR)$(SEP)*.err
	if exist $(OUTDIR)$(SEP)*.sym $(DELETE) $(OUTDIR)$(SEP)*.sym
	if exist $(OUTDIR)$(SEP)*.map $(DELETE) $(OUTDIR)$(SEP)*.map
	if exist $(H) $(DELETE) $(H)
