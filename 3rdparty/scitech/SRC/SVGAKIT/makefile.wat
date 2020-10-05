SCITECH = $(%SCITECH)
CC = $(%CC)
TASM=$(%TASM_EXE)
ASM=$(%ASM)
%INCLUDE += INCLUDE;$(SCITECH)\INCLUDE
C_FLAGS = -zq-j-w2-s -fh -fhq -onaslmr-5r -DDOS4GW
TASM_FLAGS = /t /mx /m /D__FLAT__ /DSTDCALL_MANGLE /iINCLUDE &
	/i$(SCITECH)\INCLUDE /q -DDOS4GW -I$(SCITECH)\include\svgakit
SOURCES = vesavbe.c svgasdk.c vbeaf.c font8x16.c vgapal.c gtfcalc.c event.c &
	maskcode.c cpu.asm _svgasdk.asm _linsdk.asm _vbeaf.asm _event.asm
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
