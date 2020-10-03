SCITECH = $(%SCITECH)
CC = $(%CC)
TASM=$(%TASM_EXE)
ASM=$(%ASM)
%INCLUDE += INCLUDE;$(SCITECH)\INCLUDE
C_FLAGS = -zq-j-w2-s -fh -fhq -onaslmr-5r -DDOS4GW
TASM_FLAGS = /t /mx /m /D__FLAT__ /DSTDCALL_MANGLE /iINCLUDE &
	/i$(SCITECH)\INCLUDE /q -DDOS4GW -I$(SCITECH)\include\pmode
SOURCES = pmlite.c pmpro.c vflat.c _pmlite.asm _pmpro.asm _vflat.asm
OBJS = $(SOURCES:.c=.obj)
OBJS = $(OBJS:.asm=.obj)
WAS = $(SOURCES_ASM:.asm=.was)

LIBCMD = -+$(OBJS: = -+)

all: pmode.lib .SYMBOLIC

pmode.lib: $(OBJS)
	*wlib -q $@ $(LIBCMD)

.c.obj:
	*$(CC) $(C_FLAGS) $<

.asm.obj:
!ifeq ASM TASM
	$(TASM) $(TASM_FLAGS) $<
!else
	@echo ERROR: Es necesario Turbo Assembler para compilar las librer�as de SciTech
	@%abort
!endif

clean: .SYMBOLIC
	-del *.obj
	-del *.lib
	-del *.err
	-del *.pch
