!include ../../../os.mif

ROOT			= ..$(SEP)..$(SEP)..
CONFIG			= release
OUTDIR_BASE		= $(ROOT)$(SEP)build.$(SYS)
OUTDIR			= $(OUTDIR_BASE)$(SEP)pmwlite$(SEP)$(CONFIG)

OPTIONS			= -bt -q -3r -zp1

EXE				= pmwlite$(EXE_SUFFIX)

!ifeqi CONFIG debug
OPTIONS			+= -d2
!endif

!ifdef __MSDOS__
LINK_OPTIONS	= system pmodew
!else
LINK_OPTIONS	= 
!endif

OBJS			= pmwlite.obj encode.obj

.BEFORE
	@if not exist $(OUTDIR_BASE) mkdir $(OUTDIR_BASE)
	@if not exist $(OUTDIR_BASE)$(SEP)pmwlite mkdir $(OUTDIR_BASE)$(SEP)pmwlite
	@if not exist $(OUTDIR) mkdir $(OUTDIR)

$(EXE): $(OUTDIR)/$(EXE) .SYMBOLIC
	@%null

$(OUTDIR)/$(EXE): $(OBJS)
	*wlink $(LINK_OPTIONS) &
		option quiet &
		name $^@ &
!ifeqi CONFIG debug
		debug all &
!endif
		path $(OUTDIR) &
		file { $(OBJS) }

.obj: $(OUTDIR)

pmwlite.obj: pmwlite.c pmw1.h encode.h ../pmwver.h
	*wcc386 $(OPTIONS) -fo=$(OUTDIR)$(SEP)$^. -fr=$(OUTDIR)$(SEP)$^& $[

encode.obj: encode.c encode.h
	*wcc386 $(OPTIONS) -fo=$(OUTDIR)$(SEP)$^. -fr=$(OUTDIR)$(SEP)$^& $[

clean: .SYMBOLIC
	@if exist $(OUTDIR)$(SEP)*.err $(DELETE) $(OUTDIR)$(SEP)*.err
	@if exist $(OUTDIR)$(SEP)*.obj $(DELETE) $(OUTDIR)$(SEP)*.obj
	@if exist $(OUTDIR)$(SEP)$(EXE) $(DELETE) $(OUTDIR)$(SEP)$(EXE)
