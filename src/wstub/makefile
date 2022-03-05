!include ../../os.mif

ROOT			= ..$(SEP)..
CONFIG			= release
OUTDIR_BASE		= $(ROOT)$(SEP)build.dos
OUTDIR			= $(OUTDIR_BASE)$(SEP)wstub$(SEP)$(CONFIG)

ASM				= WASM
TASM_EXE		= tasm32.exe

EXE				= wstub.exe

OPTIONS			= -bt=dos -wx -ms -q
WASM_OPTIONS	= -wx -ms -q
TASM_OPTIONS	= -ml -t

!ifeqi CONFIG debug
OPTIONS			+= -d2
WASM_OPTIONS	+= -d1
TASM_OPTIONS	+= /zi
!else
OPTIONS			+= -oneatx -d0
!endif

OBJS = wstub.obj cpuid.obj

.BEFORE
	@if not exist $(OUTDIR_BASE) mkdir $(OUTDIR_BASE)
	@if not exist $(OUTDIR_BASE)$(SEP)wstub mkdir $(OUTDIR_BASE)$(SEP)wstub
	@if not exist $(OUTDIR) mkdir $(OUTDIR)

WSTUB_EXE = $(OUTDIR)$(SEP)$(EXE)

$(WSTUB_EXE): $(OBJS)
	*wlink &
		option quiet &
		system dos &
		name $^@ &
!ifeqi CONFIG debug
		debug all &
		option symfile=$^* &
!endif
		option map=$^* &
		path $(OUTDIR) &
		file { $(OBJS) }

.obj: $(OUTDIR)

wstub.obj: wstub.c
	*wcc $(OPTIONS) -fo=$(OUTDIR)$(SEP)$^. -fr=$(OUTDIR)$(SEP)$^& $<

cpuid.obj: ../cpuid.asm
!ifeqi ASM WASM
	*wasm -zcm=tasm $(WASM_OPTIONS) -fo=$(OUTDIR)$(SEP)$^. -fr=$(OUTDIR)$(SEP)$^& $<
!else
	$(TASM_EXE) $(TASM_OPTIONS) $<,$(OUTDIR)$(SEP)$^.
!endif

clean: .SYMBOLIC
	@if exist $(OUTDIR)$(SEP)*.err $(DELETE) $(OUTDIR)$(SEP)*.err
	@if exist $(OUTDIR)$(SEP)*.obj $(DELETE) $(OUTDIR)$(SEP)*.obj
	@if exist $(OUTDIR)$(SEP)*.exe $(DELETE) $(OUTDIR)$(SEP)*.exe
	@if exist $(OUTDIR)$(SEP)*.map $(DELETE) $(OUTDIR)$(SEP)*.map
	@if exist $(OUTDIR)$(SEP)*.sym $(DELETE) $(OUTDIR)$(SEP)*.sym
