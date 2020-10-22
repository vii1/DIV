# Makefile para la DIV32RUN.DLL en todas sus variantes.
# SESSION=1 genera el int‚rprete que incorpora el trazador. Es el int‚rprete
# que se ejecuta desde el IDE cuando arrancamos un programa.
# SESSION=0 genera la DIV32RUN.DLL que se incluir  con el instalador de nuestro
# juego. Pero a£n no es la versi¢n redistribuible, ya que al generar el
# instalador se le agrega la tabla de textos traducidos.

!include ../../os.mif

ROOT			= ..$(SEP)..
CONFIG			= release
CPU				= 586
SESSION			= 0
OUTDIR_BASE		= $(ROOT)$(SEP)build.dos

MAKE			= $+ $(MAKE) -h $-
ASM				= WASM
TASM_EXE		= tasm32.exe

OPTIONS			= -bt=dos -wx -mf -i=judas -i=netlib -i=vbe -q -DDIV32RUN
WASM_OPTIONS	= -bt=dos -q
TASM_OPTIONS	= /w2 /z /ml /t /m2 /i..\vpe /jMASM

PMW_DEPS		=

!ifeq SESSION 1
OPTIONS 		+= -DDEBUG
SYSTEM			= dos4g
OUTDIR			= $(OUTDIR_BASE)$(SEP)session$(SEP)$(CONFIG).$(CPU)
!ifeq CPU 586
EXE				= session.div
!else
EXE				= session.386
!endif # CPU
!else # SESSION = 0
SYSTEM			= pmodew
OUTDIR			= $(OUTDIR_BASE)$(SEP)div32run$(SEP)$(CONFIG).$(CPU)
PMW_DEPS		= pmwlite
!ifeq CPU 586
EXE				= div32run.ins
!else
EXE				= div32run.386
!endif # CPU
!endif # SESSION

!ifeqi CONFIG debug
OPTIONS			+= -d2 -D_DEBUG
WASM_OPTIONS	+= -d1
TASM_OPTIONS	+= /zi
!else
OPTIONS			+= -oneatr -d0
TASM_OPTIONS	+= /zn
!endif

!ifeq CPU 586
OPTIONS 		+= -5r -fp5
WASM_OPTIONS 	+= -5r -fp5 -mf
!else
OPTIONS			+= -3r -fpc
WASM_OPTIONS	+= -3r -fpc -mf
!endif

INC_NOTARGETS=1
!include $(ROOT)$(SEP)3rdparty.mif
!undef INC_NOTARGETS

SOURCES = i.cpp f.cpp c.cpp s.cpp v.cpp a.asm divfli.cpp cdrom.cpp &
	divkeybo.cpp mouse.cpp ia.cpp divsound.cpp divmixer.cpp divtimer.cpp &
	divlengu.cpp det_vesa.cpp dll.c
SOURCES += timer.asm vesa.asm divdll.c pe_load.c
!ifeq SESSION 1
SOURCES += d.cpp
!endif

# VPE (modo-8)
SOURCES += vpe$(SEP)draw_cw.cpp vpe$(SEP)draw_f.cpp vpe$(SEP)draw_o.cpp vpe$(SEP)draw_sw.cpp &
	vpe$(SEP)fixed.cpp vpe$(SEP)gfx.cpp vpe$(SEP)globals.cpp vpe$(SEP)hard.cpp vpe$(SEP)load.cpp &
	vpe$(SEP)mem.cpp vpe$(SEP)object.cpp vpe$(SEP)scan.cpp vpe$(SEP)update.cpp vpe$(SEP)view.cpp &
	vpe$(SEP)vpe.cpp vpe$(SEP)zone.cpp vpe$(SEP)draw_fa.asm vpe$(SEP)draw_oa.asm vpe$(SEP)draw_wa.asm &
	vpe$(SEP)vpedll.cpp

# NETLIB (red)
SOURCES += netlib$(SEP)red.cpp netlib$(SEP)net.c netlib$(SEP)comlib.c netlib$(SEP)comm.c &
	netlib$(SEP)ipxlib.c netlib$(SEP)dpmi_net.c

OBJS = $(SOURCES:.cpp=.obj)
OBJS = $(OBJS:.c=.obj)
OBJS = $(OBJS:.asm=.obj)

LIBS = $(JUDAS_LIB) $(JPEG_LIB) $(ZLIB_LIB) $(SVGA_LIB) $(PMODE_LIB) $(TFLC_LIB)
INCLUDE += -I..
OPTIONS += $(INCLUDE)

.BEFORE
	@if not exist $(OUTDIR_BASE) mkdir $(OUTDIR_BASE)
!ifeq SESSION 1
	@if not exist $(OUTDIR_BASE)$(SEP)session mkdir $(OUTDIR_BASE)$(SEP)session
!else
	@if not exist $(OUTDIR_BASE)$(SEP)div32run mkdir $(OUTDIR_BASE)$(SEP)div32run
!endif
	@if not exist $(OUTDIR) mkdir $(OUTDIR)
	@if not exist $(OUTDIR)$(SEP)netlib mkdir $(OUTDIR)$(SEP)netlib
	@if not exist $(OUTDIR)$(SEP)vpe mkdir $(OUTDIR)$(SEP)vpe

$(EXE): $(OUTDIR)$(SEP)$(EXE) .SYMBOLIC
	@%null

$(OUTDIR)$(SEP)$(EXE): $(OBJS) $(LIBS) $(PMW_DEPS)
	*wlink &
		option quiet &
		system $(SYSTEM) &
		name $^@ &
!ifeqi %CONFIG debug
		debug all &
!endif
		option stack=64k &
		option map=$(OUTDIR)$(SEP)$^& &
		path $(OUTDIR) &
		file { $(OBJS) } &
		libfile { $(LIBS) }
!ifeqi CONFIG release
	-*wstrip $^@
!ifeq SESSION 0
	$(OUTDIR_BASE)$(SEP)pmwlite$(SEP)$(CONFIG)$(SEP)pmwlite /C4 $^@
!endif
!endif

pmwlite: .SYMBOLIC
	cd $(ROOT)$(SEP)pmwlite$(SEP)src$(SEP)pmwlite
	*$(MAKE)
	cd $+$(%cwd)$+

.asm: .;..;..$(SEP)vpe
.c: .;..;..$(SEP)netlib
.cpp: .;..$(SEP)vpe;..$(SEP)netlib

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

clean: .SYMBOLIC
	@if exist $(OUTDIR)$(SEP)*.err $(DELETE) $(OUTDIR)$(SEP)*.err
	@if exist $(OUTDIR)$(SEP)*.obj $(DELETE) $(OUTDIR)$(SEP)*.obj
	@if exist $(OUTDIR)$(SEP)*.exe $(DELETE) $(OUTDIR)$(SEP)*.exe
	@if exist $(OUTDIR)$(SEP)vpe$(SEP)*.err $(DELETE) $(OUTDIR)$(SEP)vpe$(SEP)*.err
	@if exist $(OUTDIR)$(SEP)vpe$(SEP)*.obj $(DELETE) $(OUTDIR)$(SEP)vpe$(SEP)*.obj
	@if exist $(OUTDIR)$(SEP)netlib$(SEP)*.err $(DELETE) $(OUTDIR)$(SEP)netlib$(SEP)*.err
	@if exist $(OUTDIR)$(SEP)netlib$(SEP)*.obj $(DELETE) $(OUTDIR)$(SEP)netlib$(SEP)*.obj

INC_NOVARS=1
!include $(ROOT)$(SEP)3rdparty.mif
!undef INC_NOVARS
