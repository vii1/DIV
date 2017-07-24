CONFIG=debug386
OUTDIR=$(CONFIG)
OPTIONS=/oneatx /wx /zp4 /bt=dos /3r /fpc

SOURCES = cdrom.cpp &
	det_vesa.cpp &
	div.cpp &
	divasm.cpp &
	divbasic.cpp &
	divbin.cpp &
	divbrow.cpp &
	divc.cpp &
	divcalc.cpp &
	divcdrom.cpp &
	divcolor.cpp &
	divdsktp.cpp &
	divedit.cpp &
	diveffec.cpp &
	divfont.cpp &
	divforma.cpp &
	divfpg.cpp &
#	divfrm.cpp &
	divgama.cpp &
	divhandl.cpp &
	divhelp.cpp &
	divinsta.cpp &
	divkeybo.cpp &
	divlengu.cpp &
	divmap3d.cpp &
	divmix.cpp &
	divmixer.cpp &
	divmouse.cpp &
	divpaint.cpp &
	divpalet.cpp &
	divpcm.cpp &
	divsb.cpp &
	divsetup.cpp &
	divsound.cpp &
	divspr.cpp &
	divsprit.cpp &
	divtimer.cpp &
	divvideo.cpp &
	divwindo.cpp &
	fpgfile.cpp &
	grabador.cpp &
	ifs.cpp &
	mem.cpp &
	divdll1.c &
	divdll2.c

OBJS = $(SOURCES:.cpp=.obj)
OBJS = $(OBJS:.c=.obj)

.BEFORE
	if not exist $(OUTDIR) mkdir $(OUTDIR)
	
d.exe: $(OBJS)
	wlink &
		system dos4g &
		name $^ &
		debug all &
		file { $(OBJS) }

.c: ..

.cpp: ..

.obj: $(OUTDIR)

.c.obj:
	wcc386 $(OPTIONS) -fo=$(OUTDIR)\$^ -fr=$(OUTDIR)\$^* $<

.cpp.obj:
	wcc386 $(OPTIONS) -fo=$(OUTDIR)\$^ -fr=$(OUTDIR)\$^* $<

clean: .SYMBOLIC
	-del $(OUTDIR)\*.err $(OUTDIR)*.obj
