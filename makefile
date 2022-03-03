# Makefile principal de DIV.
# ``wmake install`` compilar  todo lo necesario y crear  el  rbol de ficheros
# en la ruta indicada por INSTALL_DIR.

# Se requiere Watcom 10.0 o superior, o bien OpenWatcom 1.0 o superior.

!include os.mif

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

# Para ejecutar algunos tests se requiere DOSBOX-X (excepto en DOS, claro).
DOSBOX = dosbox-x$(EXE_SUFFIX)

############################
# FIN DE COSAS CONFIGURABLES
############################

ROOT=$+$(%cwd)$-
MAKE=$+$(MAKE) -h$-
OUTDIR_BASE = $(ROOT)$(SEP)build.dos
OUTDIR_BASE_SYS = $(ROOT)$(SEP)build.$(SYS)

MAKE_OPTIONS = CONFIG=$(CONFIG) ASM=$(ASM) ROOT=$(ROOT) #OUTDIR_BASE=$(OUTDIR_BASE)

!ifeq ASM TASM
MAKE_OPTIONS += TASM_EXE=$(TASM_EXE)
!endif
!ifdef PACK
MAKE_OPTIONS += PACK=$(PACK)
!endif

SRC_DIV= src$(SEP)div
SRC_DIV32RUN= src$(SEP)div32run

.BEFORE
	@echo Host OS: $(%OS)
	@echo Building on: $(OUTDIR_BASE)
	@echo Building host system tools on: $(OUTDIR_BASE_SYS)
	@if not exist $(OUTDIR_BASE) mkdir $(OUTDIR_BASE)

all: d.exe d.386 session.div session.386 div32run.ins div32run.386 dlls install.ovl .SYMBOLIC
	@%null

d.exe: .SYMBOLIC
	cd $(SRC_DIV)
	$(MAKE) $(MAKE_OPTIONS) CPU=586 d.exe
	cd ..$(SEP)..

d.386: .SYMBOLIC
	cd $(SRC_DIV)
	$(MAKE) $(MAKE_OPTIONS) CPU=386 d.386
	cd ..$(SEP)..

session.div: .SYMBOLIC
	cd $(SRC_DIV32RUN)
	$(MAKE) $(MAKE_OPTIONS) CPU=586 SESSION=1 session.div
	cd ..$(SEP)..

session.386: .SYMBOLIC
	cd $(SRC_DIV32RUN)
	$(MAKE) $(MAKE_OPTIONS) CPU=386 SESSION=1 session.386
	cd ..$(SEP)..

div32run.ins: .SYMBOLIC
	cd $(SRC_DIV32RUN)
	$(MAKE) $(MAKE_OPTIONS) CPU=586 SESSION=0 div32run.ins
	cd ..$(SEP)..

div32run.386: .SYMBOLIC
	cd $(SRC_DIV32RUN)
	$(MAKE) $(MAKE_OPTIONS) CPU=386 SESSION=0 div32run.386
	cd ..$(SEP)..

dlls: .SYMBOLIC
	cd dll
	$(MAKE) $(MAKE_OPTIONS)
	cd ..

install.ovl: .SYMBOLIC
	cd src$(SEP)install
	$(MAKE) $(MAKE_OPTIONS) install.ovl
	cd ..$(SEP)..

bin2h: .SYMBOLIC
	cd tools$(SEP)bin2h
	$(MAKE) $(MAKE_OPTIONS)
	cd ..$(SEP)..

testdll: .SYMBOLIC
	cd tools$(SEP)testdll
	$(MAKE) $(MAKE_OPTIONS)
	cd ..$(SEP)..

unpak: .SYMBOLIC
	cd tools$(SEP)unpak
	$(MAKE) $(MAKE_OPTIONS)
	cd ..$(SEP)..

tools: bin2h testdll unpak .SYMBOLIC
	@%null

clean: clean_lib3p clean_tools .SYMBOLIC
	cd $(SRC_DIV)
	@for %i in (586 386) do $(MAKE) $(MAKE_OPTIONS) CPU=%i clean
	cd ..$(SEP)..
	cd $(SRC_DIV32RUN)
	@for %i in (586 386) do $(MAKE) $(MAKE_OPTIONS) CPU=%i SESSION=1 clean
	@for %i in (586 386) do $(MAKE) $(MAKE_OPTIONS) CPU=%i SESSION=0 clean
	cd ..$(SEP)..
	cd src$(SEP)install
	$(MAKE) $(MAKE_OPTIONS) clean
	cd ..$(SEP)..
	cd tools$(SEP)bin2h
	$(MAKE) $(MAKE_OPTIONS) clean
	cd ..$(SEP)..
	cd tools$(SEP)testdll
	$(MAKE) $(MAKE_OPTIONS) clean
	cd ..$(SEP)..
	cd dll
	$(MAKE) $(MAKE_OPTIONS) clean
	cd ..

clean_tools: .SYMBOLIC
	cd tools$(SEP)bin2h
	$(MAKE) $(MAKE_OPTIONS) clean
	cd ..$(SEP)testdll
	$(MAKE) $(MAKE_OPTIONS) clean
	cd ..$(SEP)unpak
	$(MAKE) $(MAKE_OPTIONS) clean
	cd ..$(SEP)..

DIRS_RAIZ = genspr help install setup system
DIRS_GENSPR = enano enano_a hombre hombre_a mujer mujer_a
DIRS_VACIOS = dat fli mod pcm wav fnt fpg map prg dll dll$(SEP)source
DIRS_RESOURCE = fnt$(SEP)tutorial fpg$(SEP)tutorial ifs map$(SEP)tapices &
	pal pal$(SEP)libreria prg$(SEP)tutor wld
DLL_SOURCE = *.cpp div_dll.lnk div.h LEEME.txt make.bat

update: all .SYMBOLIC
	@echo Instalando en: $(INSTALL_DIR)
	@if not exist $(INSTALL_DIR) mkdir $(INSTALL_DIR)
	@for %i in ($(DIRS_RAIZ) $(DIRS_VACIOS) $(DIRS_RESOURCE)) do &
		@if not exist $(INSTALL_DIR)$(SEP)%i mkdir $(INSTALL_DIR)$(SEP)%i
	@for %i in ($(DIRS_GENSPR)) do &
		@if not exist $(INSTALL_DIR)$(SEP)genspr$(SEP)%i mkdir $(INSTALL_DIR)$(SEP)genspr$(SEP)%i
	@for %i in ($(DIRS_RAIZ)) do &
		$(COPY) %i$(SEP)*.* $(INSTALL_DIR)$(SEP)%i
	@for %i in ($(DIRS_GENSPR)) do &
		$(COPY) genspr$(SEP)%i$(SEP)*.* $(INSTALL_DIR)$(SEP)genspr$(SEP)%i
	@for %i in ($(DIRS_RESOURCE)) do &
		@if exist resource$(SEP)%i$(SEP)*.* $(COPY) resource$(SEP)%i$(SEP)*.* $(INSTALL_DIR)$(SEP)%i
	
	@for %i in ($(DLL_SOURCE)) do &
		$(COPY) dll$(SEP)%i $(INSTALL_DIR)$(SEP)dll$(SEP)source
	$(COPY) $(OUTDIR_BASE)$(SEP)dll$(SEP)$(CONFIG)$(SEP)*.dll $(INSTALL_DIR)$(SEP)dll

	$(COPY) $(OUTDIR_BASE)$(SEP)div$(SEP)$(CONFIG).586$(SEP)d.exe $(INSTALL_DIR)
	$(COPY) $(OUTDIR_BASE)$(SEP)div$(SEP)$(CONFIG).386$(SEP)d.386 $(INSTALL_DIR)$(SEP)system
	$(COPY) $(OUTDIR_BASE)$(SEP)session$(SEP)$(CONFIG).586$(SEP)session.div $(INSTALL_DIR)$(SEP)system
	$(COPY) $(OUTDIR_BASE)$(SEP)session$(SEP)$(CONFIG).386$(SEP)session.386 $(INSTALL_DIR)$(SEP)system
	$(COPY) $(OUTDIR_BASE)$(SEP)div32run$(SEP)$(CONFIG).586$(SEP)div32run.ins $(INSTALL_DIR)$(SEP)install
	$(COPY) $(OUTDIR_BASE)$(SEP)div32run$(SEP)$(CONFIG).386$(SEP)div32run.386 $(INSTALL_DIR)$(SEP)install
	$(COPY) $(OUTDIR_BASE)$(SEP)install$(SEP)$(CONFIG)$(SEP)install.ovl $(INSTALL_DIR)$(SEP)install
	@for %i in (LICENSE README.md) do $(COPY) %i $(INSTALL_DIR)

install: update .SYMBOLIC
	@if exist $(INSTALL_DIR)$(SEP)system$(SEP)setup.bin $(DELETE) $(INSTALL_DIR)$(SEP)system$(SEP)setup.bin
	@if exist $(INSTALL_DIR)$(SEP)system$(SEP)session.dtf $(DELETE) $(INSTALL_DIR)$(SEP)system$(SEP)session.dtf
	@if exist $(INSTALL_DIR)$(SEP)system$(SEP)user.nfo $(DELETE) $(INSTALL_DIR)$(SEP)system$(SEP)user.nfo
	@if exist $(INSTALL_DIR)$(SEP)system$(SEP)exec.* $(DELETE) $(INSTALL_DIR)$(SEP)system$(SEP)exec.*
	@if exist $(INSTALL_DIR)$(SEP)sound.cfg $(DELETE) $(INSTALL_DIR)$(SEP)sound.cfg

#testinstall: install .SYMBOLIC
#	for %i in (setup.bin session.dtf user.nfo) do $(COPY) system$(SEP)%i $(INSTALL_DIR)$(SEP)system

test_dll: .SYMBOLIC
	cd dll
	$(MAKE) $(MAKE_OPTIONS) test
	cd ..

test_div32run_386: .SYMBOLIC
	cd src$(SEP)div32run
	$(MAKE) $(MAKE_OPTIONS) CPU=386 DOSBOX=$(DOSBOX) test
	cd ..$(SEP)..

test_div32run_586: .SYMBOLIC
	cd src$(SEP)div32run
	$(MAKE) $(MAKE_OPTIONS) CPU=586 DOSBOX=$(DOSBOX) test
	cd ..$(SEP)..

test: test_dll test_div32run_386 test_div32run_586 .SYMBOLIC
	@%null

!include 3rdparty.mif

libclean: .SYMBOLIC
	@echo --------------------------------------------------------------
	@echo Esto eliminara el contenido de 3rdparty$(SEP)lib.
	@echo Tendras que recompilar las librerias (necesitaras TASM)
	@echo o bien volver a descargarlas.
	@echo No se recomienda!! LEE EL README PRIMERO!!
	@%stop
	-$(DELETE) 3rdparty$(SEP)lib$(SEP)*.lib
