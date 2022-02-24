!include ../../os.mif

ROOT			= ..$(SEP)..
CONFIG			= release
OUTDIR_BASE		= $(ROOT)$(SEP)build.$(SYS)
OUTDIR			= $(OUTDIR_BASE)$(SEP)unpak$(SEP)$(CONFIG)

OPTIONS			= -bt -q -zp4

EXE				= unpak$(EXE_SUFFIX)

!ifeqi CONFIG debug
OPTIONS			+= -d2
!endif

INC_NOTARGETS=1
!include $(ROOT)$(SEP)3rdparty.mif
!undef INC_NOTARGETS

LIBS = $(ZLIB_LIB)

SOURCES = unpak.c

OBJS = $(SOURCES:.c=.obj)

.BEFORE
	@if not exist $(OUTDIR_BASE) mkdir $(OUTDIR_BASE)
	@if not exist $(OUTDIR_BASE)$(SEP)unpak mkdir $(OUTDIR_BASE)$(SEP)unpak
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
.c: .

.c.obj: .AUTODEPEND
	*wcc386 $(OPTIONS) -fo=$(OUTDIR)$(SEP)$^. -fr=$(OUTDIR)$(SEP)$^& $<

clean: .SYMBOLIC
	@if exist $(OUTDIR)$(SEP)*.err $(DELETE) $(OUTDIR)$(SEP)*.err
	@if exist $(OUTDIR)$(SEP)*.obj $(DELETE) $(OUTDIR)$(SEP)*.obj
	@if exist $(OUTDIR)$(SEP)$(EXE) $(DELETE) $(OUTDIR)$(SEP)$(EXE)

INC_NOVARS=1
!include $(ROOT)$(SEP)3rdparty.mif
!undef INC_NOVARS
