!include ../os.mif

ROOT		= ..
OUTDIR_BASE	= $(ROOT)$(SEP)build.dos
OUTDIR		= $(OUTDIR_BASE)$(SEP)dll$(SEP)$(CONFIG)
CONFIG		= release

OPTIONS		= -l=div_dll -q

TESTDLL		= $(OUTDIR_BASE)$(SEP)testdll$(SEP)$(CONFIG)$(SEP)testdll$(EXE_SUFFIX)

MAKE		= $+$(MAKE) -h$-
%DOS4G		= QUIET
%WLINK_LNK	= div_dll.lnk

!ifeqi CONFIG debug
OPTIONS		+= -d2
!else
OPTIONS		+= -s
!endif

.SUFFIXES:
.SUFFIXES: .dll .obj .cpp .c

.BEFORE
	@if not exist $(OUTDIR_BASE) mkdir $(OUTDIR_BASE)
	@if not exist $(OUTDIR_BASE)$(SEP)dll mkdir $(OUTDIR_BASE)$(SEP)dll
	@if not exist $(OUTDIR) mkdir $(OUTDIR)

all: agua.dll hboy.dll ss1.dll .SYMBOLIC
	@%null

.dll: $(OUTDIR)
.obj: $(OUTDIR)

.cpp.dll: .AUTODEPEND
	*wcl386 $(OPTIONS) -fo=$(OUTDIR)$(SEP)$^& -fr=$(OUTDIR)$(SEP)$^& -fe=$(OUTDIR)$(SEP)$^. $<

test: all testdll .SYMBOLIC
	$(TESTDLL) -xa $(OUTDIR)$(SEP)agua.dll
	$(TESTDLL) -xa $(OUTDIR)$(SEP)hboy.dll
	$(TESTDLL) -xa $(OUTDIR)$(SEP)ss1.dll

testdll: .SYMBOLIC
	cd ../src/testdll
	*$(MAKE) CONFIG=$(CONFIG)
	cd ../../dll

clean: .SYMBOLIC
	@if exist $(OUTDIR)$(SEP)*.err $(DELETE) $(OUTDIR)$(SEP)*.err
	@if exist $(OUTDIR)$(SEP)*.obj $(DELETE) $(OUTDIR)$(SEP)*.obj
	@if exist $(OUTDIR)$(SEP)*.dll $(DELETE) $(OUTDIR)$(SEP)*.dll
