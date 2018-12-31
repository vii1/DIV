CPU = 586
CONFIG = release
OUTDIR = $(CONFIG)$(CPU)

USE_NASM = 0
TASM = tasm32
NASM = "C:\Program Files (x86)\NASM\nasm.exe"
CC = wcc386
AS = wasm

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
!else
OPTIONS += -3r -fpc
EXE = d.386
!endif

SOURCES = cdrom.cpp det_vesa.cpp div.cpp divasm.cpp divbasic.cpp divbin.cpp divbrow.cpp divc.cpp &
	divcalc.cpp divcdrom.cpp divcolor.cpp divdsktp.cpp divedit.cpp diveffec.cpp divfont.cpp divforma.cpp &
	divfpg.cpp divgama.cpp divhandl.cpp divhelp.cpp divinsta.cpp divkeybo.cpp divlengu.cpp &
	divmap3d.cpp divmix.cpp divmixer.cpp divmouse.cpp divpaint.cpp divpalet.cpp divpcm.cpp divsb.cpp &
	divsetup.cpp divsound.cpp divspr.cpp divsprit.cpp divtimer.cpp divvideo.cpp divwindo.cpp fpgfile.cpp &
	grabador.cpp ifs.cpp divdll1.c divdll2.c a.asm
!ifeq USE_NASM 0
SOURCES += vesa.asm timer.asm
!else
SOURCES += vesa_n.asm timer_n.asm
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

.BEFORE
	@if not exist $(OUTDIR) mkdir $(OUTDIR)
	@if not exist $(OUTDIR)\visor mkdir $(OUTDIR)\visor
#	@if not exist $(OUTDIR)\vpe mkdir $(OUTDIR)\vpe
	
$(OUTDIR)\$(EXE): $(OBJS) $(LIBS)
	*wlink &
		system dos4g &
		name $^@ &
!ifeq CONFIG debug
		debug all &
!endif
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
!ifeq USE_NASM 0
vesa.obj: vesa.asm
	$(TASM) $(TASM_OPTIONS) /m2 $<,$(OUTDIR)\$^@
!else
vesa_n.obj: vesa_n.asm
	$(NASM) $(NASM_OPTIONS) -Z$(OUTDIR)\$^*.err -o $(OUTDIR)\$^@ $<
!endif

!ifeq USE_NASM 0
timer.obj: timer.asm
	$(TASM) $(TASM_OPTIONS) /m $<,$(OUTDIR)\$^@
!else
timer_n.obj: timer_n.asm
	$(NASM) $(NASM_OPTIONS) -Z$(OUTDIR)\$^*.err -o $(OUTDIR)\$^@ $<
!endif

a.obj: a.asm
	*$(AS) -fo=$(OUTDIR)\$^@ -fr=$(OUTDIR)\$^* $<

$(JUDAS_LIB):
	! pushd $^: && wmake $^. && popd

$(JPEG_LIB):
	! pushd $^: && wmake -f makefile.wat $^. && popd

clean: .SYMBOLIC
	-del $(OUTDIR)\*.err $(OUTDIR)\*.obj $(OUTDIR)\*.exe
#	-del $(OUTDIR)\vpe\*.err $(OUTDIR)\vpe\*.obj	
!ifeq CPU 586
	-del $(OUTDIR)\visor\*.err $(OUTDIR)\visor\*.obj
!endif
