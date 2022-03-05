!ifdef %SCITECH
SCITECH		= $(%SCITECH)
!else
SCITECH		= ..\..
!endif
CC			= wcc386
ASM			= TASM
TASM_EXE	= tasm32.exe
INCLUDE		= /I$(SCITECH)\INCLUDE
C_FLAGS		= -bt=dos -zq-w2-s-fh -fhq -DDOS4GW -D_SVGAKIT $(INCLUDE)
TASM_FLAGS	= /t /mx /m /D__FLAT__ /q -DDOS4GW $(INCLUDE)

!ifeq CPU 586
C_FLAGS += -5 -fp5
!else
C_FLAGS += -3 -fpc
!endif

SOURCES = vesavbe.c svgasdk.c _svgasdk.asm _linsdk.asm font8x16.asm cpu.asm &
		  vgapal.c vbeaf.c _vbeaf.asm

OBJS = $(SOURCES:.c=.obj)
OBJS = $(OBJS:.asm=.obj)

LIBCMD = -+$(OBJS: = -+)

all: svga.lib .SYMBOLIC
	@%null

svga.lib: $(OBJS)
	-@if exist $@ del $@
	*wlib -q $@ $(LIBCMD)

.c.obj:
	*$(CC) $(C_FLAGS) $<

.asm.obj:
!ifeqi ASM TASM
	$(TASM_EXE) $(TASM_FLAGS) $<
!else
	@echo ERROR: Es necesario Turbo Assembler para compilar las librer¡as de SciTech
	@%abort
!endif

clean: .SYMBOLIC
	-@if exist *.obj del *.obj
	-@if exist *.lib del *.lib
	-@if exist *.err del *.err
