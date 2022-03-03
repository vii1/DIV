# Makefile para el IDE
# CPU=586 genera D.EXE
# CPU=386 genera D.386

!include ../../os.mif

ROOT			= ..$(SEP)..
CONFIG			= release
CPU				= 586
OUTDIR_BASE		= $(ROOT)$(SEP)build.dos
OUTDIR			= $(OUTDIR_BASE)$(SEP)div$(SEP)$(CONFIG).$(CPU)

MAKE			= $+ $(MAKE) -h $-
ASM				= WASM
TASM_EXE		= tasm32.exe

OPTIONS			= -bt=dos -wx -mf -i=judas -i=netlib -i=vbe -q
WASM_OPTIONS	= -bt=dos -q
TASM_OPTIONS	= /w2 /z /ml /t /m2

STUB			= $(OUTDIR_BASE)$(SEP)wstub$(SEP)$(CONFIG)$(SEP)wstub.exe

!ifeqi CONFIG debug
OPTIONS			+= -d2 -D_DEBUG
WASM_OPTIONS	+= -d1
TASM_OPTIONS	+= /zi
!else
OPTIONS			+= -oneatr -d0
TASM_OPTIONS	+= /zn
!endif

!ifeq CPU 586	# Pentium

OPTIONS 		+= -5r -fp5
WASM_OPTIONS	+= -5r -fp5
EXE				= d.exe
!ifndef STUB
OP_STUB			= option nostub
!else
OP_STUB			= option stub=$(STUB)
!endif

!else			# 386

OPTIONS			+= -3r -fpc
WASM_OPTIONS	+= -3r -fpc
EXE				= d.386
OP_STUB			= option nostub

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
	grabador.cpp ifs.cpp a.asm
SOURCES += timer.asm vesa.asm divdll.c pe_load.c

!ifeq CPU 586
# VISOR (generador de sprites) s¢lo para pentium
SOURCES += visor$(SEP)animated.cpp visor$(SEP)complex.cpp &
	visor$(SEP)fileanim.cpp visor$(SEP)global.cpp visor$(SEP)hlrender.cpp &
	visor$(SEP)llrender.cpp visor$(SEP)resource.cpp visor$(SEP)sprite3d.cpp &
	visor$(SEP)visor.cpp visor$(SEP)t.asm
!else
SOURCES += div386.c
!endif

OBJS = $(SOURCES:.cpp=.obj)
OBJS = $(OBJS:.c=.obj)
OBJS = $(OBJS:.asm=.obj)

LIBS = $(JUDAS_LIB) $(JPEG_LIB) $(ZLIB_LIB) $(SVGA_LIB) $(PMODE_LIB)
INCLUDE += -I..
OPTIONS += $(INCLUDE)

.BEFORE
	@if not exist $(OUTDIR_BASE) mkdir $(OUTDIR_BASE)
	@if not exist $(OUTDIR_BASE)$(SEP)div mkdir $(OUTDIR_BASE)$(SEP)div
	@if not exist $(OUTDIR) mkdir $(OUTDIR)
!ifeq CPU 586
	@if not exist $(OUTDIR)$(SEP)visor mkdir $(OUTDIR)$(SEP)visor
!endif

$(EXE): $(OUTDIR)$(SEP)$(EXE) .SYMBOLIC
	@%null
	
$(OUTDIR)$(SEP)$(EXE): $(OBJS) $(LIBS) wstub
	*wlink &
		option quiet &
		system dos4g &
		name $^@ &
!ifeqi CONFIG debug
		debug all &
!endif
		$(OP_STUB) &
		option map=$^* &
		path $(OUTDIR) &
		file { $(OBJS) } &
		libfile { $(LIBS) }

.asm: .;..;visor
.cpp: .;visor
.c: .;..

.obj: $(OUTDIR)

.c.obj: .AUTODEPEND
	@if exist $^@ &
		*wcc386 $(OPTIONS) -fo=$^@ -fr=$^* $<
	@if not exist $^@ &
		*wcc386 $(OPTIONS) -fo=$(OUTDIR)$(SEP)$^@ -fr=$(OUTDIR)$(SEP)$^* $<

.cpp.obj: .AUTODEPEND
	@if exist $^@ &
		*wcc386 $(OPTIONS) -fo=$^@ -fr=$^* $<
	@if not exist $^@ &
		*wcc386 $(OPTIONS) -fo=$(OUTDIR)$(SEP)$^@ -fr=$(OUTDIR)$(SEP)$^* $<

.asm.obj: .AUTODEPEND
!ifeqi ASM WASM
	@if exist $^@ &
		*wasm $(WASM_OPTIONS) -fo=$^@ -fr=$^* $<
	@if not exist $^@ &
		*wasm $(WASM_OPTIONS) -fo=$(OUTDIR)$(SEP)$^@ -fr=$(OUTDIR)$(SEP)$^* $<
!else
	@if exist $^@ &
		$(TASM_EXE) $(TASM_OPTIONS) $<,$^@
	@if not exist $^@ &
		$(TASM_EXE) $(TASM_OPTIONS) $<,$(OUTDIR)$(SEP)$^@
!endif

wstub: .SYMBOLIC
!ifdef STUB
	cd ..$(SEP)wstub
	$(MAKE) ROOT=$(ROOT) CONFIG=$(CONFIG) ASM=$(ASM) TASM_EXE=$(TASM_EXE)
	cd ..$(SEP)div
!else
	@%null
!endif

clean: .SYMBOLIC
	@if exist $(OUTDIR)$(SEP)*.err $(DELETE) $(OUTDIR)$(SEP)*.err
	@if exist $(OUTDIR)$(SEP)*.obj $(DELETE) $(OUTDIR)$(SEP)*.obj
	@if exist $(OUTDIR)$(SEP)*.exe $(DELETE) $(OUTDIR)$(SEP)*.exe
!ifeq CPU 586
	@if exist $(OUTDIR)$(SEP)visor$(SEP)*.err $(DELETE) $(OUTDIR)$(SEP)visor$(SEP)*.err
	@if exist $(OUTDIR)$(SEP)visor$(SEP)*.obj $(DELETE) $(OUTDIR)$(SEP)visor$(SEP)*.obj
!endif
 	cd ..$(SEP)wstub
 	$(MAKE) CONFIG=$(CONFIG) ROOT=$(ROOT) clean
 	cd ..$(SEP)div

INC_NOVARS=1
!include $(ROOT)$(SEP)3rdparty.mif
!undef INC_NOVARS
