# As¡ se compilar¡a el div_stub pero de momento no podemos hacerlo ya que
# si cambia su tama¤o en un solo byte se rompe la compatibilidad con la
# DIV32RUN.DLL de DIV2.

!include ../../os.mif

ROOT		= ..$(SEP)..
CONFIG		= release
OUTDIR_BASE	= $(ROOT)$(SEP)build.dos
OUTDIR_SYS	= $(ROOT)$(SEP)build.$(SYS)
OUTDIR		= $(OUTDIR_BASE)$(SEP)div_stub$(SEP)$(CONFIG)
H			= $(OUTDIR)$(SEP)div_stub.h
EXE			= $(OUTDIR)$(SEP)div_stub.exe
BIN2H		= $(OUTDIR_SYS)$(SEP)bin2h$(SEP)bin2h$(EXE_SUFFIX)
MAKE		= $+$(MAKE) -h$-

OPTIONS		= -bt=dos -q

!ifeqi CONFIG debug
OPTIONS		+= -d1
!endif

.BEFORE
	@if not exist $(OUTDIR_BASE) mkdir $(OUTDIR_BASE)
	@if not exist $(OUTDIR_BASE)$(SEP)div_stub mkdir $(OUTDIR_BASE)$(SEP)div_stub
	@if not exist $(OUTDIR) mkdir $(OUTDIR)

$(H): bin2h $(EXE)
	$(BIN2H) $(EXE) div_stub > $@

bin2h: .SYMBOLIC
	cd ..$(SEP)..$(SEP)tools$(SEP)bin2h
	*$(MAKE)
	cd ..$(SEP)..$(SEP)src$(SEP)div_stub
	
$(EXE): $(OUTDIR)$(SEP)exec.obj
	*wlink &
		option quiet &
        system dos &
        name $(EXE) &
		option stack=512 &
		path $(OUTDIR) &
        file exec.obj

.obj: $(OUTDIR)

$(OUTDIR)$(SEP)exec.obj: asm$(SEP)exec.asm
#	wcc $(OPTIONS) -fo=$@ -fr=$^* div_stub.cpp
	*wasm asm$(SEP)exec.asm $(OPTIONS) -fo=$^@ -fr=$^*

#$(OUTDIR)/cstrt086.obj:
#	wasm asm$(SEP)cstrt086.asm $(OPTIONS) -fo=$@ -fr=$^*

clean: .SYMBOLIC
	@if exist $(EXE) $(DELETE) $(EXE)
	@if exist $(OUTDIR)$(SEP)*.obj $(DELETE) $(OUTDIR)$(SEP)*.obj
	@if exist $(OUTDIR)$(SEP)*.err $(DELETE) $(OUTDIR)$(SEP)*.err
	@if exist $(OUTDIR)$(SEP)*.sym $(DELETE) $(OUTDIR)$(SEP)*.sym
	@if exist $(OUTDIR)$(SEP)*.map $(DELETE) $(OUTDIR)$(SEP)*.map
	@if exist $(H) $(DELETE) $(H)
