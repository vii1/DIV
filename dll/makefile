!include ../os.mif

OUTDIR = ..$(SEP)build.dos$(SEP)dll

OPTIONS = -l=div_dll -s -q

%DOS4G = QUIET
%WLINK_LNK = div_dll.lnk

.SUFFIXES:
.SUFFIXES: .dll .obj .cpp .c

.BEFORE
	@if not exist ..$(SEP)build.dos mkdir ..$(SEP)build.dos
	@if not exist $(OUTDIR) mkdir $(OUTDIR)

all: agua.dll hboy.dll ss1.dll .SYMBOLIC
	@%null

.dll: $(OUTDIR)
.obj: $(OUTDIR)

.cpp.dll: .AUTODEPEND
	*wcl386 $(OPTIONS) -fo=$(OUTDIR)$(SEP)$^& -fr=$(OUTDIR)$(SEP)$^& -fe=$(OUTDIR)$(SEP)$^. $<

clean: .SYMBOLIC
	@if exist $(OUTDIR)$(SEP)*.err $(DELETE) $(OUTDIR)$(SEP)*.err
	@if exist $(OUTDIR)$(SEP)*.obj $(DELETE) $(OUTDIR)$(SEP)*.obj
	@if exist $(OUTDIR)$(SEP)*.dll $(DELETE) $(OUTDIR)$(SEP)*.dll
