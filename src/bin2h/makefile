!include ../../os.mif

ROOT		= ..$(SEP)..
OUTDIR_BASE	= $(ROOT)$(SEP)build.dos
OUTDIR		= $(OUTDIR_BASE)$(SEP)bin2h
EXE			= $(OUTDIR)$(SEP)bin2h$(EXE_SUFFIX)
OPTIONS		= -bt -q

.BEFORE
	@if not exist $(OUTDIR_BASE) mkdir $(OUTDIR_BASE)
	@if not exist $(OUTDIR) mkdir $(OUTDIR)

$(EXE): bin2h.c
	*wcl386 $(OPTIONS) -fo=$(OUTDIR)$(SEP)$^& -fr=$(OUTDIR)$(SEP)$^& -fe=$^@ $<

clean: .SYMBOLIC
	@if exist $(EXE) $(DELETE) $(EXE)
	@if exist $(OUTDIR)$(SEP)bin2h.obj $(DELETE) $(OUTDIR)$(SEP)bin2h.obj
	@if exist $(OUTDIR)$(SEP)bin2h.err $(DELETE) $(OUTDIR)$(SEP)bin2h.err
