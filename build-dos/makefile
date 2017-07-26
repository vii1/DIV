CPU=586
CONFIG=debug
OUTDIR=$(CONFIG)$(CPU)

OPTIONS=-oneatx -wx -mf -zp4 -bt=dos -i=judas -i=netlib -i=vbe

!ifeq CPU 586
OPTIONS += -5r -fp5
!else
OPTIONS += -3r -fpc
!endif

SOURCES = cdrom.cpp det_vesa.cpp div.cpp divasm.cpp divbasic.cpp divbin.cpp divbrow.cpp divc.cpp &
	divcalc.cpp divcdrom.cpp divcolor.cpp divdsktp.cpp divedit.cpp diveffec.cpp divfont.cpp divforma.cpp &
	divfpg.cpp divgama.cpp divhandl.cpp divhelp.cpp divinsta.cpp divkeybo.cpp divlengu.cpp &
	divmap3d.cpp divmix.cpp divmixer.cpp divmouse.cpp divpaint.cpp divpalet.cpp divpcm.cpp divsb.cpp &
	divsetup.cpp divsound.cpp divspr.cpp divsprit.cpp divtimer.cpp divvideo.cpp divwindo.cpp fpgfile.cpp &
	grabador.cpp ifs.cpp divdll1.c divdll2.c &
	vesa.asm a.asm timer.asm

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
	
d.exe: $(OBJS) $(LIBS)
	wlink &
		system dos4g &
		name $^@ &
		debug all &
		path $(OUTDIR) &
		file { $(OBJS) } &
		libfile { $(LIBS) }

.c: ..;..\vbe

.cpp: ..;..\visor;..\vpe;..\source

.asm: ..\source

.obj: $(OUTDIR)

.c.obj:
	wcc386 $(OPTIONS) -fo=$(OUTDIR)\$^ -fr=$(OUTDIR)\$^* $<

.cpp.obj:
	wcc386 $(OPTIONS) -fo=$(OUTDIR)\$^ -fr=$(OUTDIR)\$^* $<

#.asm.obj:
	#wasm -zcm=masm -fo=$(OUTDIR)\$^ -fr=$(OUTDIR)\$^* $<
vesa.obj: vesa.asm
	tasm32 /ml /m2 /w2 /z /zn $<,$(OUTDIR)\$^@

timer.obj: timer.asm
	tasm32 /m /ml $<,$(OUTDIR)\$^@

a.obj: a.asm
	wasm -fo=$(OUTDIR)\$^ -fr=$(OUTDIR)\$^* $<

$(JUDAS_LIB):
	! pushd $^: && wmake $^. && popd

$(JPEG_LIB):
	! pushd $^: && wmake -f makefile.wat $^. && popd

clean: .SYMBOLIC
	-del $(OUTDIR)\*.err $(OUTDIR)\*.obj
#	-del $(OUTDIR)\vpe\*.err $(OUTDIR)\vpe\*.obj	
!ifeq CPU 586
	-del $(OUTDIR)\visor\*.err $(OUTDIR)\visor\*.obj
!endif
