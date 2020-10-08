!include $(ROOT)$(SEP)os.mif

CONFIG = release
OUTDIR = $(CONFIG)
CC = wcc
ASM = wasm
EXE = wstub.exe

OPTIONS = -wx -ms
WASM_OPTIONS = -wx -ms
!ifeq CONFIG debug
OPTIONS += -d2
WASM_OPTIONS += -d1
!else
OPTIONS += -oneatx -d0
!endif

OBJS = wstub.obj cpuid.obj

.BEFORE
	@if not exist $(OUTDIR) mkdir $(OUTDIR)

WSTUB_EXE = $(OUTDIR)$(SEP)$(EXE)

$(WSTUB_EXE): $(OBJS)
	*wlink &
		system dos &
		name $^@ &
!ifeq CONFIG debug
		debug all &
		option symfile=$^* &
!endif
		option map=$^* &
		path $(OUTDIR) &
		file { $(OBJS) }

.obj: $(OUTDIR)

wstub.obj: wstub.c
	*$(CC) $(OPTIONS) -fo=$(OUTDIR)$(SEP)$^. -fr=$(OUTDIR)$(SEP)$^& $<

cpuid.obj: ../cpuid.asm
	*$(ASM) -zcm=tasm $(WASM_OPTIONS) -fo=$(OUTDIR)$(SEP)$^. -fr=$(OUTDIR)$(SEP)$^& $<

clean: .SYMBOLIC
	@if exist $(OUTDIR)$(SEP)*.err $(DELETE) $(OUTDIR)$(SEP)*.err
	@if exist $(OUTDIR)$(SEP)*.obj $(DELETE) $(OUTDIR)$(SEP)*.obj
	@if exist $(OUTDIR)$(SEP)*.exe $(DELETE) $(OUTDIR)$(SEP)*.exe
	@if exist $(OUTDIR)$(SEP)*.map $(DELETE) $(OUTDIR)$(SEP)*.map
	@if exist $(OUTDIR)$(SEP)*.sym $(DELETE) $(OUTDIR)$(SEP)*.sym
