# Makefile principal de DIV. WORK IN PROGRESS.
# ``wmake install`` compilar  todo lo necesario y crear  el  rbol de ficheros
# en la ruta indicada por INSTALL_DIR.

# Se requiere Watcom 10.0 o superior, o bien OpenWatcom 1.0 o superior.

############################################
# COSAS DEPENDENDIENTES DEL SITEMA OPERATIVO
############################################
!include os.mif
!ifdef __UNIX__
ROOT=$+ $(%cwd) $-
!else
ROOT=$+ $(%cdrive):$(%cwd) $-
!ifdef %USERPROFILE
%HOME=$(%USERPROFILE)
!else
%HOME=$(SEP)
!endif
!endif

#####################
# COSAS CONFIGURABLES (pueden indicarse en la l¡nea de comando de wmake)
#####################

# Usa CONFIG=debug si quieres compilar una versi¢n depurable de DIV.
CONFIG = release

# INSTALL_DIR indica la ruta donde se instalar  DIV con ``wmake install``.
# Recomiendo usarlo para instalar DIV dentro de una carpeta de DOSBOX.
# Muy c¢modo para agilizar tu ciclo de modificar-compilar-depurar.
# Se puede configurar con una variable de entorno.
!ifdef %INSTALL_DIR
INSTALL_DIR = $(%INSTALL_DIR)
!else
INSTALL_DIR = $(%HOME)$(SEP)dosbox$(SEP)DIV
!endif

# Originalmente DIV usaba Turbo Assembler (TASM) para compilar su c¢digo en
# ensamblador. En este fork preferimos usar WASM, ya que TASM es privativo y
# no es f cil de conseguir. Ten en cuenta que si usas WASM podr¡as encontrarte
# potencialmente con alg£n bug debido a diferencias entre ensambladores.
# Para compilar las librer¡as de terceros s¡ necesitas TASM. Pero te las damos
# ya compiladas, por lo que no te har  falta a menos que quieras modificarlas.
ASM = WASM

# Si usas TASM y tu versi¢n no dispone de TASM32.EXE, quiz  quieras cambiar
# esta opci¢n.
# TASM32.EXE deber¡a valer tanto para compilar en DOS como para Windows.
#TASM_EXE = tasm.exe
TASM_EXE = tasm32.exe

############################
# FIN DE COSAS CONFIGURABLES
############################

# Nota: definimos variables de entorno (con prefijo %) para datos que
# necesitamos pasar a otros makefiles

MAKE=$+ $(MAKE) -h $-
%CONFIG=$(CONFIG)

%ASM=$(ASM)

%CC = wcc386
%TASM_EXE = $(TASM_EXE)
%WASM_EXE = wasm

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

OUTDIR_BASE = build.dos
%OUTDIR = $+ $(ROOT)$(SEP)$(OUTDIR_BASE)$(SEP)$(CONFIG) $-
%STUB = $(ROOT)$(SEP)src$(SEP)wstub$(SEP)$(%CONFIG)$(SEP)wstub.exe

SRC_DIV= src$(SEP)div
SRC_DIV32RUN= src$(SEP)div32run

.BEFORE
	@echo Host OS: $(%OS)
	@echo Building on : $(OUTDIR_BASE)
	@if not exist $(OUTDIR_BASE) mkdir $(OUTDIR_BASE)

all: d.exe d.386 session.div session.386 div32run.ins div32run.386 .SYMBOLIC


d.exe: .SYMBOLIC
	cd $(SRC_DIV)
	$(MAKE) ROOT=$(ROOT) SEP=$(SEP) CPU=586 d.exe
	cd ../..

d.386: .SYMBOLIC
	cd $(SRC_DIV)
	$(MAKE) ROOT=$(ROOT) SEP=$(SEP) CPU=386 d.386
	cd ../..

session.div: .SYMBOLIC
	cd $(SRC_DIV32RUN)
	$(MAKE) ROOT=$(ROOT) SEP=$(SEP) CPU=586 SESSION=1 session.div
	cd ../..

session.386: .SYMBOLIC
	cd $(SRC_DIV32RUN)
	$(MAKE) ROOT=$(ROOT) SEP=$(SEP) CPU=386 SESSION=1 session.386
	cd ../..

div32run.ins: .SYMBOLIC
	cd $(SRC_DIV32RUN)
	$(MAKE) ROOT=$(ROOT) SEP=$(SEP) CPU=586 SESSION=0 div32run.ins
	cd ../..

div32run.386: .SYMBOLIC
	cd $(SRC_DIV32RUN)
	$(MAKE) ROOT=$(ROOT) SEP=$(SEP) CPU=386 SESSION=0 div32run.386
	cd ../..

.SILENT
clean: .SYMBOLIC
	cd $(SRC_DIV)
	@for %i in (586 386) do $(MAKE) ROOT=$(ROOT) SEP=$(SEP) CPU=%i clean
	cd ../..
	cd $(SRC_DIV32RUN)
	@for %i in (586 386) do $(MAKE) ROOT=$(ROOT) SEP=$(SEP) CPU=%i SESSION=1 clean
	@for %i in (586 386) do $(MAKE) ROOT=$(ROOT) SEP=$(SEP) CPU=%i SESSION=0 clean
	cd ../..
	cd src/bin2h
	$(MAKE) clean
	cd ../..

DIRS_RAIZ = dll genspr help install setup system
DIRS_GENSPR = enano enano_a hombre hombre_a mujer mujer_a
DIRS_VACIOS = dat fli mod pcm wav fnt fpg map prg
DIRS_RESOURCE = fnt$(SEP)tutorial fpg$(SEP)tutorial ifs map$(SEP)tapices &
	pal pal$(SEP)libreria prg$(SEP)tutor wld

.SILENT
update: all .SYMBOLIC
	echo Instalando en: $(INSTALL_DIR)
	if not exist $(INSTALL_DIR) mkdir $(INSTALL_DIR)
	for %i in ($(DIRS_RAIZ) $(DIRS_VACIOS) $(DIRS_RESOURCE)) do &
		if not exist $(INSTALL_DIR)$(SEP)%i mkdir $(INSTALL_DIR)$(SEP)%i
	for %i in ($(DIRS_GENSPR)) do &
		if not exist $(INSTALL_DIR)$(SEP)genspr$(SEP)%i mkdir $(INSTALL_DIR)$(SEP)genspr$(SEP)%i
	for %i in ($(DIRS_RAIZ)) do &
		$(COPY) %i$(SEP)*.* $(INSTALL_DIR)$(SEP)%i
	for %i in ($(DIRS_GENSPR)) do &
		$(COPY) genspr$(SEP)%i$(SEP)*.* $(INSTALL_DIR)$(SEP)genspr$(SEP)%i
	for %i in ($(DIRS_RESOURCE)) do &
		if exist resource$(SEP)%i$(SEP)*.* $(COPY) resource$(SEP)%i$(SEP)*.* $(INSTALL_DIR)$(SEP)%i

	$(COPY) $(%OUTDIR).586$(SEP)d.exe $(INSTALL_DIR)
	$(COPY) $(%OUTDIR).386$(SEP)d.386 $(INSTALL_DIR)$(SEP)system
	$(COPY) $(%OUTDIR).586$(SEP)session$(SEP)session.div $(INSTALL_DIR)$(SEP)system
	$(COPY) $(%OUTDIR).386$(SEP)session$(SEP)session.386 $(INSTALL_DIR)$(SEP)system
	$(COPY) $(%OUTDIR).586$(SEP)div32run$(SEP)div32run.ins $(INSTALL_DIR)$(SEP)install
	$(COPY) $(%OUTDIR).386$(SEP)div32run$(SEP)div32run.386 $(INSTALL_DIR)$(SEP)install
	for %i in (LICENSE README.md) do $(COPY) %i $(INSTALL_DIR)

.SILENT
install: update .SYMBOLIC
	if exist $(INSTALL_DIR)$(SEP)system$(SEP)setup.bin $(DELETE) $(INSTALL_DIR)$(SEP)system$(SEP)setup.bin
	if exist $(INSTALL_DIR)$(SEP)system$(SEP)session.dtf $(DELETE) $(INSTALL_DIR)$(SEP)system$(SEP)session.dtf
	if exist $(INSTALL_DIR)$(SEP)system$(SEP)user.nfo $(DELETE) $(INSTALL_DIR)$(SEP)system$(SEP)user.nfo
	if exist $(INSTALL_DIR)$(SEP)system$(SEP)exec.* $(DELETE) $(INSTALL_DIR)$(SEP)system$(SEP)exec.*

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
	-del 3rdparty/lib/*.lib
