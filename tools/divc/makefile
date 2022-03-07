!include ../../os.mif

ROOT			= ..$(SEP)..
CONFIG			= release
OUTDIR_BASE		= $(ROOT)$(SEP)build.$(TARGET_SYS)
OUTDIR			= $(OUTDIR_BASE)$(SEP)divc$(SEP)$(CONFIG)
CPU				= 386

OPTIONS			= -bt=$(WATCOM_BT) -q -zp4

EXE				= divc$(EXE_SUFFIX)

!ifeqi CONFIG debug
OPTIONS			+= -d2
!endif

!inject 1 NO_JPEG NO_SCITECH NO_TOPFLC
INC_NOTARGETS=1
!include $(ROOT)$(SEP)3rdparty.mif
!undef INC_NOTARGETS

LIBS = $(ZLIB_LIB)

INCLUDE += -I$(ROOT)$(SEP)src -I$(ROOT)$(SEP)src$(SEP)div
OPTIONS += $(INCLUDE)

OBJS = main.obj divc.obj divdll.obj pe_load.obj divlengu.obj

.BEFORE
	@if not exist $(OUTDIR_BASE) mkdir $(OUTDIR_BASE)
	@if not exist $(OUTDIR_BASE)$(SEP)divc mkdir $(OUTDIR_BASE)$(SEP)divc
	@if not exist $(OUTDIR) mkdir $(OUTDIR)

$(EXE): $(OUTDIR)$(SEP)$(EXE) .SYMBOLIC
	@%null

$(OUTDIR)$(SEP)$(EXE): $(OBJS)
	*wlink &
		$(WLINK_TARGET_SYS) &
		option quiet &
		name $^@ &
!ifeqi CONFIG debug
		debug all &
!endif
		option map=$^* &
		path $(OUTDIR) &
		file { $(OBJS) } &
		libfile { $(LIBS) }

.obj: $(OUTDIR)
.c: .;$(ROOT)$(SEP)src
.cpp: $(ROOT)$(SEP)src$(SEP)div

.c.obj: .AUTODEPEND
	*wcc386 $(OPTIONS) -fo=$(OUTDIR)$(SEP)$^. -fr=$(OUTDIR)$(SEP)$^& $<

.cpp.obj: .AUTODEPEND
	*wcc386 $(OPTIONS) -fo=$(OUTDIR)$(SEP)$^. -fr=$(OUTDIR)$(SEP)$^& $<

clean: .SYMBOLIC
	@if exist $(OUTDIR)$(SEP)*.err $(DELETE) $(OUTDIR)$(SEP)*.err
	@if exist $(OUTDIR)$(SEP)*.obj $(DELETE) $(OUTDIR)$(SEP)*.obj
	@if exist $(OUTDIR)$(SEP)$(EXE) $(DELETE) $(OUTDIR)$(SEP)$(EXE)

install: $(OUTDIR)$(SEP)$(EXE) .SYMBOLIC
!ifndef INSTALL_DIR
	@echo Debe indicar INSTALL_DIR
	@%abort
!endif
	$(COPY) $(OUTDIR)$(SEP)$(EXE) $(INSTALL_DIR)

INC_NOVARS=1
!include $(ROOT)$(SEP)3rdparty.mif
!undef INC_NOVARS
