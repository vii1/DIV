!include ../../os.mif

ROOT			= ..$(SEP)..
CONFIG			= release
OUTDIR_BASE		= $(ROOT)$(SEP)build.dos
OUTDIR_BASE_SYS = $(ROOT)$(SEP)build.$(SYS)
OUTDIR			= $(OUTDIR_BASE)$(SEP)install$(SEP)$(CONFIG)

MAKE			= $+ $(MAKE) -h $-
ASM				= WASM
TASM_EXE		= tasm32.exe

OPTIONS			= -bt=dos -wx -mf -3r -q
WASM_OPTIONS	= -bt=dos -q -3r
TASM_OPTIONS	= /w2 /z /ml /t /m2 /jMASM

PMW_DEPS		=
PACK			= 1

EXE				= install.ovl

!ifeqi CONFIG debug
OPTIONS			+= -d2 -D_DEBUG
WASM_OPTIONS	+= -d1
TASM_OPTIONS	+= /zi
!else
OPTIONS			+= -oneatr -d0
TASM_OPTIONS	+= /zn
!ifeq PACK 1
PMW_DEPS		= pmwlite
!endif
!endif

INC_NOTARGETS=1
!include $(ROOT)$(SEP)3rdparty.mif
!undef INC_NOTARGETS

SOURCES = main.c video.c vesa.asm fnt.c fpg.c mouse.c time.c keyboard.c &
		  fases.c timer.asm

OBJS = $(SOURCES:.c=.obj)
OBJS = $(OBJS:.asm=.obj)

LIBS = $(ZLIB_LIB)
INCLUDE += -I..
OPTIONS += $(INCLUDE)

.BEFORE
	@if not exist $(OUTDIR_BASE) mkdir $(OUTDIR_BASE)
	@if not exist $(OUTDIR_BASE)$(SEP)install mkdir $(OUTDIR_BASE)$(SEP)install
	@if not exist $(OUTDIR) mkdir $(OUTDIR)

$(EXE): $(OUTDIR)$(SEP)$(EXE) .SYMBOLIC
	@%null

$(OUTDIR)$(SEP)$(EXE): $(OBJS) $(LIBS) $(PMW_DEPS)
	*wlink &
		option quiet &
		system pmodew &
		name $^@ &
!ifeqi CONFIG debug
		debug all &
!endif
		option map=$(OUTDIR)$(SEP)$^& &
		path $(OUTDIR) &
		file { $(OBJS) } &
		libfile { $(LIBS) }
!ifeqi CONFIG release
!ifeq PACK 1
	$(OUTDIR_BASE_SYS)$(SEP)pmwlite$(SEP)$(CONFIG)$(SEP)pmwlite -C4 -S$(ROOT)$(SEP)pmwlite$(SEP)pmodew.exe $^@
!endif
!endif

pmwlite: .SYMBOLIC
	cd $(ROOT)$(SEP)pmwlite$(SEP)src$(SEP)pmwlite
	*$(MAKE)
	cd $+$(%cwd)$+

.asm: .;..
.c: .

.obj: $(OUTDIR)

.c.obj: .AUTODEPEND
	@if exist $^@ &
		*wcc386 $(OPTIONS) -fo=$^@ -fr=$^* $<
	@if not exist $^@ &
		*wcc386 $(OPTIONS) -fo=$(OUTDIR)$(SEP)$^@ -fr=$(OUTDIR)$(SEP)$^* $<

.asm.obj: .AUTODEPEND
!ifeqi ASM WASM
	@if exist $^@ &
		*wasm $(WASM_OPTIONS) -fo=$^@ -fr=$^* $<
	@if not exist $^@ &
		*wasm $(WASM_OPTIONS) -fo=$(OUTDIR)$(SEP)$^@ -fr=$(OUTDIR)$(SEP)$^* $<
!else
	@if exist $^@ &
		$(TASM_EXE) $(TASM_OPTIONS) $<,$^@
	@if not exist $^@ &
		$(TASM_EXE) $(TASM_OPTIONS) $<,$(OUTDIR)$(SEP)$^@
!endif

clean: .SYMBOLIC
	@if exist $(OUTDIR)$(SEP)*.err $(DELETE) $(OUTDIR)$(SEP)*.err
	@if exist $(OUTDIR)$(SEP)*.obj $(DELETE) $(OUTDIR)$(SEP)*.obj
	@if exist $(OUTDIR)$(SEP)$(EXE) $(DELETE) $(OUTDIR)$(SEP)$(EXE)

INC_NOVARS=1
!include $(ROOT)$(SEP)3rdparty.mif
!undef INC_NOVARS

test: $(EXE) .SYMBOLIC
	@if not exist $(OUTDIR)$(SEP)test mkdir $(OUTDIR)$(SEP)test
	$(COPY) test$(SEP)heliobal.001 $(OUTDIR)$(SEP)test
!ifdef __UNIX__
	cat $(OUTDIR)/$(EXE) test/install.dat > $(OUTDIR)/test/install.exe
!else
	copy /b $(OUTDIR)$(SEP)$(EXE) + test$(SEP)install.dat $(OUTDIR)$(SEP)test$(SEP)install.exe
!endif
!ifdef __MSDOS__
	$(OUTDIR)$(SEP)test$(SEP)install.exe
!else
	@echo Copy these files to a DOS box:
	@echo - $(OUTDIR)$(SEP)test$(SEP)install.exe
	@echo - $(OUTDIR)$(SEP)test$(SEP)heliobal.001
!endif
	