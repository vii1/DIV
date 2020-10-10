# Makefile para la DIV32RUN.DLL en todas sus variantes.
# SESSION=1 genera el int‚rprete que incorpora el trazador. Es el int‚rprete
# que se ejecuta desde el IDE cuando arrancamos un programa.
# SESSION=0 genera la DIV32RUN.DLL que se incluir  con el instalador de nuestro
# juego. Pero a£n no es la versi¢n redistribuible, ya que al generar el
# instalador se le agrega la tabla de textos traducidos.

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

!include $(ROOT)$(SEP)os.mif

#%OPTIONS += -fti

!ifeq SESSION 1
%OPTIONS += -DDEBUG
SYSTEM=dos4g
OUTDIR = $(%OUTDIR).$(CPU)$(SEP)session
!ifeq CPU 586
EXE=session.div
!else
EXE=session.386
!endif
!else
SYSTEM=pmodew
OUTDIR = $(%OUTDIR).$(CPU)$(SEP)div32run
!ifeq CPU 586
EXE=div32run.ins
!else
EXE=div32run.386
!endif
!endif

MAKE=$(MAKE) -h
WASM=$(%WASM_EXE)
TASM=$(%TASM_EXE)
WASM_OPTIONS=$(%WASM_OPTIONS)
TASM_OPTIONS=$(%TASM_OPTIONS) /i..\vpe /jMASM

!ifeq CPU 586
%OPTIONS += -5r -fp5
WASM_OPTIONS = -5r -fp5 -mf
!else
%OPTIONS += -3r -fpc
WASM_OPTIONS = -3r -fpc -mf
!endif

INC_NOTARGETS=1
!include $(ROOT)$(SEP)3rdparty.mif
!undef INC_NOTARGETS

SOURCES = i.cpp f.cpp c.cpp s.cpp v.cpp a.asm divfli.cpp cdrom.cpp &
	divkeybo.cpp mouse.cpp divdll1.c divdll2.c ia.cpp divsound.cpp &
	divmixer.cpp divtimer.cpp timer.asm divlengu.cpp vesa.asm &
	det_vesa.cpp

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
%OPTIONS += $(INCLUDE)

.BEFORE
	@if not exist $(%OUTDIR).$(CPU) mkdir $(%OUTDIR).$(CPU)
	@if not exist $(OUTDIR) mkdir $(OUTDIR)
	@if not exist $(OUTDIR)$(SEP)netlib mkdir $(OUTDIR)$(SEP)netlib
	@if not exist $(OUTDIR)$(SEP)vpe mkdir $(OUTDIR)$(SEP)vpe

$(EXE): $(OUTDIR)$(SEP)$(EXE) .SYMBOLIC
	@%null

$(OUTDIR)$(SEP)$(EXE): $(OBJS) $(LIBS)
	*wlink &
		system $(SYSTEM) &
		name $(OUTDIR)$(SEP)$^. &
!ifeqi %CONFIG debug
		debug all &
!endif
		option stack=64k &
		option map=$(OUTDIR)$(SEP)$^& &
		path $(OUTDIR) &
		file { $(OBJS) } &
		libfile { $(LIBS) }

.asm: .;..;..$(SEP)vpe
.c: .;..$(SEP)netlib
.cpp: .;..$(SEP)vpe;..$(SEP)netlib
#.c: source;vbe
#.cpp: source;vbe;netlib
#.asm: source

#.obj: $(OUTDIR)
.obj: $(OUTDIR);$(OUTDIR)$(SEP)netlib;$(OUTDIR)$(SEP)vpe

.c.obj:
	*$(%CC) $(%OPTIONS) -fo=$(OUTDIR)$(SEP)$^:$^. -fr=$(OUTDIR)$(SEP)$^:$^& $<

.cpp.obj:
	*$(%CC) $(%OPTIONS) -fo=$(OUTDIR)$(SEP)$^:$^. -fr=$(OUTDIR)$(SEP)$^:$^& $<

.asm.obj:
!ifeqi %ASM WASM
	*$(WASM) $(WASM_OPTIONS) -fo=$(OUTDIR)$(SEP)$^:$^. -fr=$(OUTDIR)$(SEP)$^:$^& $<
!else
	$(TASM) $(TASM_OPTIONS) /m2 $<,$(OUTDIR)$(SEP)$^:$^.
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

clean: .SYMBOLIC
	@if exist $(OUTDIR)$(SEP)*.err $(DELETE) $(OUTDIR)$(SEP)*.err
	@if exist $(OUTDIR)$(SEP)*.obj $(DELETE) $(OUTDIR)$(SEP)*.obj
	@if exist $(OUTDIR)$(SEP)*.exe $(DELETE) $(OUTDIR)$(SEP)*.exe
	@if exist $(OUTDIR)$(SEP)vpe$(SEP)*.err $(DELETE) $(OUTDIR)$(SEP)vpe$(SEP)*.err
	@if exist $(OUTDIR)$(SEP)vpe$(SEP)*.obj $(DELETE) $(OUTDIR)$(SEP)vpe$(SEP)*.obj

INC_NOVARS=1
!include $(ROOT)$(SEP)3rdparty.mif
!undef INC_NOVARS
