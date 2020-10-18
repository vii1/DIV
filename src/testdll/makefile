!include ../../os.mif

EXE = testdll
!ifndef __UNIX__
EXE = $+ $(EXE).exe $-
!endif

!ifndef CONFIG
CONFIG = release
!endif

OUTDIR = $(CONFIG)

SOURCES = testdll.c divdll.c pe_loader.c

OBJS = $(SOURCES:.c=.obj)

OPTIONS = -bt= -zq -zp4 -I.. -DPEDEBUG

!ifeqi CONFIG debug
OPTIONS += -d2
!endif

.BEFORE
	@if not exist $(OUTDIR) mkdir $(OUTDIR)

$(EXE): $(OUTDIR)/$(EXE) .SYMBOLIC
	@%null

$(OUTDIR)/$(EXE): $(OBJS)
	*wlink &
		option quiet &
		name $(OUTDIR)$(SEP)$^. &
!ifeqi CONFIG debug
		debug all &
!endif
		option map=$(OUTDIR)$(SEP)$^& &
		path $(OUTDIR) &
		file { $(OBJS) }

.obj: $(OUTDIR)
.c: .;..

.c.obj: .AUTODEPEND
	*wcc386 $(OPTIONS) -fo=$(OUTDIR)$(SEP)$^:$^. -fr=$(OUTDIR)$(SEP)$^:$^& $<

clean: .SYMBOLIC
	@if exist $(OUTDIR)$(SEP)*.err $(DELETE) $(OUTDIR)$(SEP)*.err
	@if exist $(OUTDIR)$(SEP)*.obj $(DELETE) $(OUTDIR)$(SEP)*.obj
	@if exist $(OUTDIR)$(SEP)$(EXE) $(DELETE) $(OUTDIR)$(SEP)$(EXE)
