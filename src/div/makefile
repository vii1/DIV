# Makefile para el IDE
# CPU=586 genera D.EXE
# CPU=386 genera D.386

!ifndef SEP
!ifdef __UNIX__
SEP = /
!else
SEP = '\\'
!endif
!endif

!ifndef ROOT
ROOT=..$(SEP)..
!endif

!ifndef CONFIG
CONFIG=$(%CONFIG)
!endif

!ifndef %OUTDIR
%OUTDIR = $(ROOT)$(SEP)build.dos$(SEP)$(CONFIG)
!endif
OUTDIR = $(%OUTDIR).$(CPU)

!include $(ROOT)$(SEP)os.mif

MAKE=$+ $(MAKE) -h $-
WASM=$(%WASM_EXE)
TASM=$(%TASM_EXE)
WASM_OPTIONS=$(%WASM_OPTIONS)
TASM_OPTIONS=$(%TASM_OPTIONS)

!ifeq CPU 586

%OPTIONS += -5r -fp5
WASM_OPTIONS = -5r -fp5
EXE = d.exe
!ifndef %STUB
OP_STUB = option nostub
!else
OP_STUB = option stub=$(%STUB)
!endif

!else

%OPTIONS += -3r -fpc
WASM_OPTIONS = -3r -fpc
EXE = d.386
OP_STUB = option nostub

!endif

!ifeq CONFIG debug
%OPTIONS += -D_DEBUG
!endif

# No usar de momento!
#DIV_STUB_DIR=..$(SEP)div_stub
#DIV_STUB_H=$(DIV_STUB_DIR)$(SEP)$(CONFIG)$(SEP)div_stub.h
#INCLUDE += $(DIV_STUB_DIR)$(SEP)$(CONFIG)

INC_NOTARGETS=1
!include $(ROOT)$(SEP)3rdparty.mif
!undef INC_NOTARGETS

SOURCES = cdrom.cpp det_vesa.cpp div.cpp divbasic.cpp divbin.cpp &
	divbrow.cpp divc.cpp divcalc.cpp divcdrom.cpp divcolor.cpp divdsktp.cpp &
	divedit.cpp diveffec.cpp divfont.cpp divforma.cpp divfpg.cpp divgama.cpp &
	divhandl.cpp divhelp.cpp divinsta.cpp divkeybo.cpp divlengu.cpp &
	divmap3d.cpp divmix.cpp divmixer.cpp divmouse.cpp divpaint.cpp &
	divpalet.cpp divpcm.cpp divsb.cpp divsetup.cpp divsound.cpp divspr.cpp &
	divsprit.cpp divtimer.cpp divvideo.cpp divwindo.cpp fpgfile.cpp &
	grabador.cpp ifs.cpp divdll1.c divdll2.c a.asm
SOURCES += timer.asm
SOURCES += vesa.asm

#	divfrm.cpp mem.cpp
# VPE (modo-8) esto es para el int‚rprete!
#SOURCES += vpe$(SEP)draw_cw.cpp vpe$(SEP)draw_f.cpp vpe$(SEP)draw_o.cpp vpe$(SEP)draw_sw.cpp vpe$(SEP)fixed.cpp vpe$(SEP)gfx.cpp vpe$(SEP)globals.cpp &
#	vpe$(SEP)hard.cpp vpe$(SEP)load.cpp vpe$(SEP)mem.cpp vpe$(SEP)object.cpp vpe$(SEP)scan.cpp vpe$(SEP)update.cpp vpe$(SEP)view.cpp vpe$(SEP)vpe.cpp &
#	vpe$(SEP)vpedll.cpp vpe$(SEP)zone.cpp

!ifeq CPU 586
# VISOR (generador de sprites) s¢lo para pentium
SOURCES += visor$(SEP)animated.cpp visor$(SEP)complex.cpp visor$(SEP)fileanim.cpp &
	visor$(SEP)global.cpp visor$(SEP)hlrender.cpp visor$(SEP)llrender.cpp visor$(SEP)resource.cpp &
	visor$(SEP)sprite3d.cpp visor$(SEP)visor.cpp t.asm
#	visor$(SEP)main.cpp
!else
SOURCES += div386.c
!endif

OBJS = $(SOURCES:.cpp=.obj)
OBJS = $(OBJS:.c=.obj)
OBJS = $(OBJS:.asm=.obj)

LIBS = $(JUDAS_LIB) $(JPEG_LIB) $(ZLIB_LIB) $(SVGA_LIB) $(PMODE_LIB)
INCLUDE += -I..
%OPTIONS += $(INCLUDE)

.BEFORE
	@if not exist $(OUTDIR) mkdir $(OUTDIR)
!ifeq CPU 586
	@if not exist $(OUTDIR)$(SEP)visor mkdir $(OUTDIR)$(SEP)visor
!endif
#	@if not exist $(OUTDIR)$(SEP)vpe mkdir $(OUTDIR)$(SEP)vpe

$(EXE): $(OUTDIR)$(SEP)$(EXE) .SYMBOLIC

$(OUTDIR)$(SEP)$(EXE): $(OBJS) $(LIBS) wstub
	*wlink &
		system dos4g &
		name $(OUTDIR)$(SEP)$^. &
!ifeqi %CONFIG debug
		debug all &
!endif
		$(OP_STUB) &
		option map=$(OUTDIR)$(SEP)$^& &
		path $(OUTDIR) &
		file { $(OBJS) } &
		libfile { $(LIBS) }

.asm: .;..;visor
.cpp: .;visor
#.c: .;vbe
#.cpp: .;visor;vpe;source
#.asm: .;source;visor

.obj: $(OUTDIR);$(OUTDIR)$(SEP)visor

.c.obj:
	*$(%CC) $(%OPTIONS) -fo=$(OUTDIR)$(SEP)$[:$^. -fr=$(OUTDIR)$(SEP)$[:$^& $<

.cpp.obj:
	*$(%CC) $(%OPTIONS) -fo=$(OUTDIR)$(SEP)$[:$^. -fr=$(OUTDIR)$(SEP)$[:$^& $<

.asm.obj:
!ifeqi %ASM WASM
	* $(WASM) $(WASM_OPTIONS) -fo=$(OUTDIR)$(SEP)$^. -fr=$(OUTDIR)$(SEP)$^& $<
!else
	$(TASM) $(TASM_OPTIONS) /m2 $<,$(OUTDIR)$(SEP)$^.
!endif
#vesa.obj: vesa.asm
#!ifeqi %ASM WASM
#	* $(WASM) -zcm=tasm $(WASM_OPTIONS) -fo=$(OUTDIR)$(SEP)$^. -fr=$(OUTDIR)$(SEP)$^& $<
#!else
#	$(TASM) $(TASM_OPTIONS) /m2 $<,$(OUTDIR)$(SEP)$^.
#!endif
#
#timer.obj: timer.asm
#!ifeqi %ASM WASM
#	* $(WASM) -zcm=tasm $(WASM_OPTIONS) -fo=$(OUTDIR)$(SEP)$^. -fr=$(OUTDIR)$(SEP)$^& $<
#!else
#	$(TASM) $(TASM_OPTIONS) /m $<,$(OUTDIR)$(SEP)$^.
#!endif
#
#a.obj: a.asm
#	* $(WASM) $(WASM_OPTIONS) -fo=$(OUTDIR)$(SEP)$^. -fr=$(OUTDIR)$(SEP)$^& $<
#
#visor$(SEP)t.obj: visor$(SEP)t.asm
#	* $(WASM) $(WASM_OPTIONS) -fo=$(OUTDIR)$(SEP)$^. -fr=$(OUTDIR)$(SEP)$^& $<

wstub: .SYMBOLIC
!ifdef %STUB
	@%make savedir
	cd $(ROOT)$(SEP)src$(SEP)wstub
	! $(MAKE) ROOT=$(ROOT) CONFIG=$(%CONFIG) SEP=$(SEP)
	@%make loaddir
!else
	@%null
!endif

clean: .SYMBOLIC
	@if exist $(OUTDIR)$(SEP)*.err $(DELETE) $(OUTDIR)$(SEP)*.err
	@if exist $(OUTDIR)$(SEP)*.obj $(DELETE) $(OUTDIR)$(SEP)*.obj
	@if exist $(OUTDIR)$(SEP)*.exe $(DELETE) $(OUTDIR)$(SEP)*.exe
#	@if exist $(OUTDIR)$(SEP)vpe$(SEP)*.err $(DELETE) $(OUTDIR)$(SEP)vpe$(SEP)*.err
#	@if exist $(OUTDIR)$(SEP)vpe$(SEP)*.obj $(DELETE) $(OUTDIR)$(SEP)vpe$(SEP)*.obj
!ifeq CPU 586
	@if exist $(OUTDIR)$(SEP)visor$(SEP)*.err $(DELETE) $(OUTDIR)$(SEP)visor$(SEP)*.err
	@if exist $(OUTDIR)$(SEP)visor$(SEP)*.obj $(DELETE) $(OUTDIR)$(SEP)visor$(SEP)*.obj
!endif
 	@%make savedir
 	cd $(ROOT)$(SEP)src$(SEP)wstub
 	! $(MAKE) CONFIG=$(%CONFIG) ROOT=$(ROOT) SEP=$(SEP) clean
 	@%make loaddir

INC_NOVARS=1
!include $(ROOT)$(SEP)3rdparty.mif
!undef INC_NOVARS

!include $(ROOT)$(SEP)common.mif
