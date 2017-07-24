CPU=586
CONFIG=debug
OUTDIR=$(CONFIG)$(CPU)

OPTIONS=-oneatx -wx -zp4 -bt=dos -i=judas -i=vpe -i=netlib
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
	grabador.cpp ifs.cpp divdll1.c divdll2.c
#	divfrm.cpp mem.cpp
# VPE (modo-8)
SOURCES += VPE\DRAW_CW.CPP VPE\DRAW_F.CPP VPE\DRAW_O.CPP VPE\DRAW_SW.CPP VPE\FIXED.CPP VPE\GFX.CPP VPE\GLOBALS.CPP &
	VPE\HARD.CPP VPE\LOAD.CPP VPE\MEM.CPP VPE\OBJECT.CPP VPE\SCAN.CPP VPE\UPDATE.CPP VPE\VIEW.CPP VPE\VPE.CPP &
	VPE\VPEDLL.CPP VPE\ZONE.CPP

# VISOR (generador de sprites) s¢lo para pentium
!ifeq CPU 586
SOURCES += visor\animated.cpp visor\complex.cpp visor\fileanim.cpp visor\global.cpp visor\hlrender.cpp &
	visor\llrender.cpp visor\resource.cpp visor\sprite3d.cpp visor\visor.cpp
#	visor\main.cpp
!endif

OBJS = $(SOURCES:.cpp=.obj)
OBJS = $(OBJS:.c=.obj)

.BEFORE
	if not exist $(OUTDIR) mkdir $(OUTDIR)
	if not exist $(OUTDIR)\visor mkdir $(OUTDIR)\visor
	if not exist $(OUTDIR)\vpe mkdir $(OUTDIR)\vpe
	
d.exe: $(OBJS)
	wlink &
		system dos4g &
		name $^ &
		debug all &
		path $(OUTDIR) &
		file { $(OBJS) } &
		libfile { judas\judas.lib }

.c: ..

.cpp: ..;..\visor;..\vpe

.obj: $(OUTDIR)

.c.obj:
	wcc386 $(OPTIONS) -fo=$(OUTDIR)\$^ -fr=$(OUTDIR)\$^* $<

.cpp.obj:
	wcc386 $(OPTIONS) -fo=$(OUTDIR)\$^ -fr=$(OUTDIR)\$^* $<

clean: .SYMBOLIC
	-del $(OUTDIR)\*.err $(OUTDIR)\*.obj
	-del $(OUTDIR)\vpe\*.err $(OUTDIR)\vpe\*.obj	
!ifeq CPU 586
	-del $(OUTDIR)\visor\*.err $(OUTDIR)\visor\*.obj
!endif
