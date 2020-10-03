# JUDAS library makefile
#
# NOTE: modules which are or might be called from within an interrupt have the
# -zu option, to tell that SS != DS. It'll crash otherwise.

ASM=$(%ASM)
TASM_EXE=$(%TASM_EXE)
WASM_EXE=$(%WASM_EXE)

#CFLAGS1= -oneatx -w3 -zp4 -fpc -3r
CFLAGS1= -oneatx -w3 -zp4 -fp5 -5r -q
#CFLAGS1= /d2 /wx /zp4 /fp5 /mf /bt=dos /5r

#CFLAGS2= -oneatx -zu -w3 -zp4 -fpc -3r
CFLAGS2= -oneatx -zu -w3 -zp4 -fp5 -5r -q
#CFLAGS2= /d2 /zu /wx /zp4 /fp5 /mf /bt=dos /5r

judas.lib: judas.obj judassmp.obj judasraw.obj judaswav.obj judastbl.obj judasxm.obj judasmod.obj judass3m.obj judasio.obj judasasm.obj judasdma.obj judasmem.obj
        wlib -n @judas.cmd

judas.obj: judas.c
        wcc386 $(CFLAGS1) judas.c
judassmp.obj: judassmp.c
        wcc386 $(CFLAGS1) judassmp.c
judasraw.obj: judasraw.c
        wcc386 $(CFLAGS1) judasraw.c
judaswav.obj: judaswav.c
        wcc386 $(CFLAGS1) judaswav.c
judasio.obj: judasio.c
        wcc386 $(CFLAGS1) judasio.c
judasmem.obj: judasmem.c
        wcc386 $(CFLAGS1) judasmem.c
judastbl.obj: judastbl.c
        wcc386 $(CFLAGS1) judastbl.c
judasxm.obj: judasxm.c
        wcc386 $(CFLAGS2) judasxm.c
judasmod.obj: judasmod.c
        wcc386 $(CFLAGS2) judasmod.c
judass3m.obj: judass3m.c
        wcc386 $(CFLAGS2) judass3m.c
judasdma.obj: judasdma.c
        wcc386 $(CFLAGS2) judasdma.c

judasasm.obj: judasasm.asm
!ifeq ASM TASM
        $(TASM_EXE) /m /ml judasasm.asm
!else
        @echo ERROR: Es necesario Turbo Assembler para compilar JUDAS
        @%abort
!endif

clean: .SYMBOLIC
        @if exist *.obj del *.obj
        @if exist *.err del *.err
        @if exist judas.lib del judas.lib
