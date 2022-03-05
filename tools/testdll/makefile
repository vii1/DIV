!include ../../os.mif

ROOT			= ..$(SEP)..
CONFIG			= release
OUTDIR_BASE		= $(ROOT)$(SEP)build.$(SYS)
OUTDIR			= $(OUTDIR_BASE)$(SEP)testdll$(SEP)$(CONFIG)
SRCDIR			= $(ROOT)$(SEP)src

OPTIONS			= -bt -q -zp4 -I$(SRCDIR) -DPEDEBUG

EXE				= testdll$(EXE_SUFFIX)

!ifeqi CONFIG debug
OPTIONS			+= -d2
!endif

SOURCES = testdll.c divdll.c pe_load.c

OBJS = $(SOURCES:.c=.obj)

.BEFORE
	@if not exist $(OUTDIR_BASE) mkdir $(OUTDIR_BASE)
	@if not exist $(OUTDIR_BASE)$(SEP)testdll mkdir $(OUTDIR_BASE)$(SEP)testdll
	@if not exist $(OUTDIR) mkdir $(OUTDIR)

$(EXE): $(OUTDIR)/$(EXE) .SYMBOLIC
	@%null

$(OUTDIR)/$(EXE): $(OBJS)
	*wlink &
		option quiet &
		name $^@ &
!ifeqi CONFIG debug
		debug all &
!endif
		option map=$^* &
		path $(OUTDIR) &
		file { $(OBJS) }

.obj: $(OUTDIR)
.c: .;$(SRCDIR)

.c.obj: .AUTODEPEND
	*wcc386 $(OPTIONS) -fo=$(OUTDIR)$(SEP)$^. -fr=$(OUTDIR)$(SEP)$^& $<

clean: .SYMBOLIC
	@if exist $(OUTDIR)$(SEP)*.err $(DELETE) $(OUTDIR)$(SEP)*.err
	@if exist $(OUTDIR)$(SEP)*.obj $(DELETE) $(OUTDIR)$(SEP)*.obj
	@if exist $(OUTDIR)$(SEP)$(EXE) $(DELETE) $(OUTDIR)$(SEP)$(EXE)
