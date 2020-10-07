SCITECH = $(%SCITECH)
CC = $(%CC)
TASM=$(%TASM_EXE)
ASM=$(%ASM)
%INCLUDE += INCLUDE;$(SCITECH)\INCLUDE
C_FLAGS = -zq-w2-s-fh -fhq -DDOS4GW -D_SVGAKIT
TASM_FLAGS = /t /mx /m /D__FLAT__ /i$(SCITECH)\INCLUDE /q -DDOS4GW
SOURCES = vesavbe.c svgasdk.c _svgasdk.asm _linsdk.asm font8x16.asm cpu.asm &
		  vgapal.c vbeaf.c _vbeaf.asm
OBJS = $(SOURCES:.c=.obj)
OBJS = $(OBJS:.asm=.obj)

LIBCMD = -+$(OBJS: = -+)

all: svga.lib .SYMBOLIC

svga.lib: $(OBJS)
	-@if exist $^ del $^
	*wlib -q $^ $(LIBCMD)

.c.obj:
	*$(CC) $(C_FLAGS) $<

.asm.obj:
!ifeq ASM TASM
	$(TASM) $(TASM_FLAGS) $<
!else
	@echo ERROR: Es necesario Turbo Assembler para compilar las librer¡as de SciTech
	@%abort
!endif

clean: .SYMBOLIC
	-@if exist *.obj del *.obj
	-@if exist *.lib del *.lib
	-@if exist *.err del *.err
