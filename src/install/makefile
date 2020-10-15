!include ../../os.mif

!ifndef ROOT
ROOT=..$(SEP)..
!endif

!ifdef %CONFIG
CONFIG=$(%CONFIG)
!else
CONFIG=release
!endif

!ifndef %OUTDIR
%OUTDIR = $(ROOT)$(SEP)build.dos$(SEP)$(CONFIG)
!endif
OUTDIR = $(%OUTDIR)$(SEP)install

!ifndef %CC
%CC = wcc386
!endif

%OPTIONS += -3r -fpc
WASM_OPTIONS = -3r -fpc
EXE = install.ovl

INC_NOTARGETS=1
!include $(ROOT)$(SEP)3rdparty.mif
!undef INC_NOTARGETS

SOURCES = main.c

OBJS = $(SOURCES:.c=.obj)

LIBS = $(ZLIB_LIB) $(SVGA_LIB) $(PMODE_LIB)
INCLUDE += -I..
%OPTIONS += $(INCLUDE)

.BEFORE
	@if not exist $(%OUTDIR) mkdir $(%OUTDIR)
	@if not exist $(OUTDIR) mkdir $(OUTDIR)

$(EXE): $(OUTDIR)$(SEP)$(EXE) .SYMBOLIC
	@%null

$(OUTDIR)$(SEP)$(EXE): $(OBJS) $(LIBS)
	*wlink &
		system dos4g &
		name $(OUTDIR)$(SEP)$^. &
!ifeqi %CONFIG debug
		debug all &
!endif
		option nostub &
		option map=$(OUTDIR)$(SEP)$^& &
		path $(OUTDIR) &
		file { $(OBJS) } &
		libfile { $(LIBS) }

.obj: $(OUTDIR)

.c.obj:
	*$(%CC) $(%OPTIONS) -fo=$(OUTDIR)$(SEP)$[:$^. -fr=$(OUTDIR)$(SEP)$[:$^& $<

clean: .SYMBOLIC
	@if exist $(OUTDIR)$(SEP)*.err $(DELETE) $(OUTDIR)$(SEP)*.err
	@if exist $(OUTDIR)$(SEP)*.obj $(DELETE) $(OUTDIR)$(SEP)*.obj
	@if exist $(OUTDIR)$(SEP)$(EXE) $(DELETE) $(OUTDIR)$(SEP)$(EXE)
