# Makefile principal de DIV. WORK IN PROGRESS.
# ``wmake install`` compilar  todo lo necesario y crear  el  rbol de ficheros
# en la ruta indicada por INSTALL_DIR.

# Se requiere Watcom 10.0 o superior, o bien OpenWatcom 1.0 o superior.

#####################
# COSAS CONFIGURABLES (pueden indicarse en la l¡nea de comando de wmake)
#####################

# Usa CONFIG=debug si quieres compilar una versi¢n depurable de DIV.
CONFIG = release

# INSTALL_DIR indica la ruta donde se instalar  DIV con ``wmake install``.
# Recomiendo usarlo para instalar DIV dentro de una carpeta de DOSBOX.
# Muy c¢modo para agilizar tu ciclo de modificar-compilar-depurar.
#INSTALL_DIR = C:\DIV2
INSTALL_DIR = $(%USERPROFILE)\dosbox\DIV

# Siempre que se pueda, se usar  WASM, pero originalmente DIV se compilaba
# usando Turbo Assembler (TASM). Esto podr¡a, potencialmente, originar bugs
# debido a diferencias entre ambos ensambladores. El problema es que TASM es
# privativo, por lo que damos preferencia a WASM. Pero si esto diera problemas,
# quiz  prefieras conseguir una copia de TASM y usarlo en su lugar.
ASM = TASM

# Si usas TASM y tu versi¢n no dispone de TASM32.EXE, quiz  quieras cambiar
# esta opci¢n.
# TASM32.EXE deber¡a valer tanto para compilar en DOS como para Windows.
#TASM_EXE = tasm.exe
TASM_EXE = tasm32.exe

# El comando que se usar  para copiar cosas con ``wmake install``.
# Quiz  no te guste xcopy o quieras cambiar las opciones
# (/Y sirve para sobreescribir archivos sin pedir confirmaci¢n).
COPY = xcopy /Y

############################
# FIN DE COSAS CONFIGURABLES
############################

MAKE=$(MAKE) -h
%CONFIG=$(CONFIG)
%OUTDIR = build.dos\$(%CONFIG)

%ASM=$(ASM)
%TASM_EXE = $(TASM_EXE)
%WASM_EXE = wasm.exe
%CC = wcc386.exe

%OPTIONS = -wx -mf -i=judas -i=netlib -i=vbe -zq
%TASM_OPTIONS = /w2 /z /ml

%WCC386 = -bt=dos
%WASM = -bt=dos

!ifeqi %CONFIG debug
%OPTIONS += -d2
%TASM_OPTIONS += /zi
!else
%OPTIONS += -oneatr -d0
%TASM_OPTIONS += /zn
!endif

%STUB = wstub\$(%CONFIG)\wstub.exe

all: d.exe d.386 session.div session.386 div32run.ins div32run.386 .SYMBOLIC

d.exe: wstub d.mif .SYMBOLIC
	$(MAKE) -f d.mif CPU=586 d.exe

d.386: wstub d.mif .SYMBOLIC
	$(MAKE) -f d.mif CPU=386 d.386

session.div: div32run.mif .SYMBOLIC
	$(MAKE) -f div32run.mif CPU=586 SESSION=1 session.div

session.386: div32run.mif .SYMBOLIC
	$(MAKE) -f div32run.mif CPU=386 SESSION=1 session.386

div32run.ins: div32run.mif .SYMBOLIC
	$(MAKE) -f div32run.mif CPU=586 SESSION=0 div32run.ins

div32run.386: div32run.mif .SYMBOLIC
	$(MAKE) -f div32run.mif CPU=586 SESSION=0 div32run.386

wstub: $(%STUB) .SYMBOLIC

$(%STUB): wstub\makefile wstub\wstub.c source\cpuid.asm
	! pushd wstub && $(MAKE) CONFIG=$(%CONFIG) && popd

clean: clean_judas clean_jpeg clean_tflc clean_zlib clean_svga clean_pmode .SYMBOLIC
	@for %i in (586 386) do $(MAKE) -f d.mif CPU=%i clean
	@for %i in (586 386) do $(MAKE) -f div32run.mif CPU=%i SESSION=1 clean
	@for %i in (586 386) do $(MAKE) -f div32run.mif CPU=%i SESSION=0 clean

.SILENT
install: all .SYMBOLIC
	if not exist $(INSTALL_DIR) mkdir $(INSTALL_DIR)
	if not exist $(INSTALL_DIR)\system mkdir $(INSTALL_DIR)\system
	if not exist $(INSTALL_DIR)\dat mkdir $(INSTALL_DIR)\dat
	if not exist $(INSTALL_DIR)\dll mkdir $(INSTALL_DIR)\dll
	if not exist $(INSTALL_DIR)\fli mkdir $(INSTALL_DIR)\fli
	if not exist $(INSTALL_DIR)\fnt mkdir $(INSTALL_DIR)\fnt
	if not exist $(INSTALL_DIR)\fpg mkdir $(INSTALL_DIR)\fpg
	if not exist $(INSTALL_DIR)\genspr mkdir $(INSTALL_DIR)\genspr
	if not exist $(INSTALL_DIR)\help mkdir $(INSTALL_DIR)\help
	if not exist $(INSTALL_DIR)\ifs mkdir $(INSTALL_DIR)\ifs
	if not exist $(INSTALL_DIR)\install mkdir $(INSTALL_DIR)\install
	if not exist $(INSTALL_DIR)\map mkdir $(INSTALL_DIR)\map
	if not exist $(INSTALL_DIR)\mod mkdir $(INSTALL_DIR)\mod
	if not exist $(INSTALL_DIR)\pal mkdir $(INSTALL_DIR)\pal
	if not exist $(INSTALL_DIR)\pcm mkdir $(INSTALL_DIR)\pcm
	if not exist $(INSTALL_DIR)\prg mkdir $(INSTALL_DIR)\prg
	if not exist $(INSTALL_DIR)\setup mkdir $(INSTALL_DIR)\setup
	if not exist $(INSTALL_DIR)\wav mkdir $(INSTALL_DIR)\wav
	if not exist $(INSTALL_DIR)\wld mkdir $(INSTALL_DIR)\wld
	$(COPY) $(%OUTDIR).586\d.exe $(INSTALL_DIR)
	$(COPY) system\*.* $(INSTALL_DIR)\system
	$(COPY) $(%OUTDIR).386\d.386 $(INSTALL_DIR)\system
	$(COPY) help\*.* $(INSTALL_DIR)\help
	$(COPY) /S genspr\*.* $(INSTALL_DIR)\genspr
	if exist $(INSTALL_DIR)\system\setup.bin del $(INSTALL_DIR)\system\setup.bin
	if exist $(INSTALL_DIR)\system\session.dtf del $(INSTALL_DIR)\system\session.dtf
	if exist $(INSTALL_DIR)\system\user.nfo del $(INSTALL_DIR)\system\user.nfo
	if exist $(INSTALL_DIR)\system\exec.* del $(INSTALL_DIR)\system\exec.*

.SILENT
testinstall: install .SYMBOLIC
	for %i in (setup.bin session.dtf user.nfo) do $(COPY) system\%i $(INSTALL_DIR)\system

!include 3rdparty.mif

libclean: .SYMBOLIC
	@echo --------------------------------------------------------------
	@echo Esto eliminara el contenido de 3rdparty\lib.
	@echo Tendras que recompilar las librerias (necesitaras TASM)
	@echo o bien volver a descargarlas.
	@echo No se recomienda!! LEE EL README PRIMERO!!
	@%stop
	-del 3rdparty\lib\*.lib
