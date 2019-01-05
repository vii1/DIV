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
!endif
		option map=$^* &
		path $(OUTDIR) &
		file { $(OBJS) }

.obj: $(OUTDIR)

wstub.obj: wstub.c
	*$(CC) $(OPTIONS) -fo=$(OUTDIR)\$^@ -fr=$(OUTDIR)\$^* $<

cpuid.obj: ..\source\cpuid.asm
	*$(ASM) -zcm=tasm $(WASM_OPTIONS) -fo=$(OUTDIR)\$^@ -fr=$(OUTDIR)\$^* $<

clean: .SYMBOLIC
	-del $(OUTDIR)\*.err
	-del $(OUTDIR)\*.obj
	-del $(OUTDIR)\*.exe
