CPU = 586
CONFIG = release
OUTDIR = $(CONFIG)$(CPU)

ASM = TASM
TASM = tasm32
NASM = "C:\Program Files (x86)\NASM\nasm.exe"
WASM = wasm
CC = wcc386

OPTIONS = -wx -mf -zp4 -bt=dos -i=judas -i=netlib -i=vbe
TASM_OPTIONS = /w2 /z /ml
NASM_OPTIONS = -f obj -Wall

!ifeq CONFIG debug
OPTIONS += -d2
TASM_OPTIONS += /zi
NASM_OPTIONS += -g -O0
!else
OPTIONS += -oneatx -d0
TASM_OPTIONS += /zn
NASM_OPTIONS += -Ox
!endif

!ifeq CPU 586
OPTIONS += -5r -fp5
EXE = d.exe
WASM_OPTIONS += -5r -fp5
!else
OPTIONS += -3r -fpc
EXE = d.386
WASM_OPTIONS += -3r -fpc
!endif

SOURCES = cdrom.cpp det_vesa.cpp div.cpp divasm.cpp divbasic.cpp divbin.cpp divbrow.cpp divc.cpp &
	divcalc.cpp divcdrom.cpp divcolor.cpp divdsktp.cpp divedit.cpp diveffec.cpp divfont.cpp divforma.cpp &
	divfpg.cpp divgama.cpp divhandl.cpp divhelp.cpp divinsta.cpp divkeybo.cpp divlengu.cpp &
	divmap3d.cpp divmix.cpp divmixer.cpp divmouse.cpp divpaint.cpp divpalet.cpp divpcm.cpp divsb.cpp &
	divsetup.cpp divsound.cpp divspr.cpp divsprit.cpp divtimer.cpp divvideo.cpp divwindo.cpp fpgfile.cpp &
	grabador.cpp ifs.cpp divdll1.c divdll2.c a.asm
!ifeq ASM NASM
SOURCES += vesa_n.asm timer_n.asm
!else
SOURCES += vesa.asm timer.asm
!endif

#	divfrm.cpp mem.cpp
# VPE (modo-8) esto es para el int‚rprete!
#SOURCES += vpe\draw_cw.cpp vpe\draw_f.cpp vpe\draw_o.cpp vpe\draw_sw.cpp vpe\fixed.cpp vpe\gfx.cpp vpe\globals.cpp &
#	vpe\hard.cpp vpe\load.cpp vpe\mem.cpp vpe\object.cpp vpe\scan.cpp vpe\update.cpp vpe\view.cpp vpe\vpe.cpp &
#	vpe\vpedll.cpp vpe\zone.cpp

# VISOR (generador de sprites) s¢lo para pentium
!ifeq CPU 586
SOURCES += visor\animated.cpp visor\complex.cpp visor\fileanim.cpp visor\global.cpp visor\hlrender.cpp &
	visor\llrender.cpp visor\resource.cpp visor\sprite3d.cpp visor\visor.cpp
#	visor\main.cpp
!endif

OBJS = $(SOURCES:.cpp=.obj)
OBJS = $(OBJS:.c=.obj)
OBJS = $(OBJS:.asm=.obj)

JUDAS_LIB = ..\judas\judas.lib
JPEG_LIB = ..\jpeglib\libjpeg.lib
LIBS = $(JUDAS_LIB) $(JPEG_LIB) ..\source\zlib.lib ..\source\svga.lib ..\source\pmode.lib

STUB = ..\wstub\$(CONFIG)\wstub.exe

.BEFORE
	@if not exist $(OUTDIR) mkdir $(OUTDIR)
	@if not exist $(OUTDIR)\visor mkdir $(OUTDIR)\visor
#	@if not exist $(OUTDIR)\vpe mkdir $(OUTDIR)\vpe
	
$(OUTDIR)\$(EXE): $(OBJS) $(LIBS) $(STUB)
	*wlink &
		system dos4g &
		name $^@ &
!ifeq CONFIG debug
		debug all &
!endif
		option stub=$(STUB) &
		option map=$^* &
		path $(OUTDIR) &
		file { $(OBJS) } &
		libfile { $(LIBS) }

.c: ..;..\vbe

.cpp: ..;..\visor;..\vpe;..\source

.asm: ..\source

.obj: $(OUTDIR)

.c.obj:
	*$(CC) $(OPTIONS) -fo=$(OUTDIR)\$^@ -fr=$(OUTDIR)\$^* $<

.cpp.obj:
	*$(CC) $(OPTIONS) -fo=$(OUTDIR)\$^@ -fr=$(OUTDIR)\$^* $<

#.asm.obj:
	#wasm -zcm=masm -fo=$(OUTDIR)\$^ -fr=$(OUTDIR)\$^* $<
!ifeqi ASM NASM
vesa_n.obj: vesa_n.asm
	$(NASM) $(NASM_OPTIONS) -Z$(OUTDIR)\$^*.err -o $(OUTDIR)\$^@ $<
!else
vesa.obj: vesa.asm
!ifeqi ASM WASM
	*$(WASM) -zcm=tasm $(WASM_OPTIONS) -fo=$(OUTDIR)\$^@ -fr=$(OUTDIR)\$^* $<
!else
	$(TASM) $(TASM_OPTIONS) /m2 $<,$(OUTDIR)\$^@
!endif
!endif

!ifeqi ASM NASM
timer_n.obj: timer_n.asm
	$(NASM) $(NASM_OPTIONS) -Z$(OUTDIR)\$^*.err -o $(OUTDIR)\$^@ $<
!else
timer.obj: timer.asm
!ifeqi ASM WASM
	*$(WASM) -zcm=tasm $(WASM_OPTIONS) -fo=$(OUTDIR)\$^@ -fr=$(OUTDIR)\$^* $<
!else
	$(TASM) $(TASM_OPTIONS) /m $<,$(OUTDIR)\$^@
!endif
!endif

a.obj: a.asm
	*$(WASM) $(WASM_OPTIONS) -fo=$(OUTDIR)\$^@ -fr=$(OUTDIR)\$^* $<

$(JUDAS_LIB):
	! pushd $^: && wmake $^. && popd

$(JPEG_LIB):
	! pushd $^: && wmake -f makefile.wat $^. && popd

$(STUB):
	! pushd ..\wstub && wmake CONFIG=$(CONFIG) && popd

clean: .SYMBOLIC
	-del $(OUTDIR)\*.err
	-del $(OUTDIR)\*.obj
	-del $(OUTDIR)\*.exe
#	-del $(OUTDIR)\vpe\*.err $(OUTDIR)\vpe\*.obj	
!ifeq CPU 586
	-del $(OUTDIR)\visor\*.err
	-del $(OUTDIR)\visor\*.obj
!endif
