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

$(OUTDIR)\$(EXE): $(OBJS)
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
	*$(CC) $(OPTIONS) -fo=$(OUTDIR)\$^. -fr=$(OUTDIR)\$^& $<

cpuid.obj: ..\cpuid.asm
	*$(ASM) -zcm=tasm $(WASM_OPTIONS) -fo=$(OUTDIR)\$^. -fr=$(OUTDIR)\$^& $<

clean: .SYMBOLIC
	@if exist $(OUTDIR)\*.err del $(OUTDIR)\*.err
	@if exist $(OUTDIR)\*.obj del $(OUTDIR)\*.obj
	@if exist $(OUTDIR)\*.exe del $(OUTDIR)\*.exe
	@if exist $(OUTDIR)\*.map del $(OUTDIR)\*.map
	@if exist $(OUTDIR)\*.sym del $(OUTDIR)\*.sym
