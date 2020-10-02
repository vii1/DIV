SCITECH = $(%SCITECH)
CC = wcc386
ASM = tasm32
%INCLUDE += INCLUDE;$(SCITECH)\INCLUDE
C_FLAGS = -zq-j-w2-s -fh -fhq -onaslmr-5r -DDOS4GW
TASM_FLAGS = /t /mx /m /D__FLAT__ /DSTDCALL_MANGLE /iINCLUDE &
	/i$(SCITECH)\INCLUDE /q -DDOS4GW -I$(SCITECH)\include\svgakit

SOURCES = vesavbe.c svgasdk.c vbeaf.c font8x16.c vgapal.c gtfcalc.c cpu.asm &
	_svgasdk.asm _linsdk.asm _vbeaf.asm event.c maskcode.c _event.asm

OBJS = $(SOURCES:.c=.obj)
OBJS = $(OBJS:.asm=.obj)

LIBCMD = -+$(OBJS: = -+)

all: svga.lib .SYMBOLIC

svga.lib: $(OBJS)
	*wlib -q $^ $(LIBCMD)

.c.obj:
	*$(CC) $(C_FLAGS) $<

.asm.obj:
	$(ASM) $(TASM_FLAGS) $<

clean: .SYMBOLIC
	-del *.obj
	-del *.lib
	-del *.err
