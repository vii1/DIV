CONFIG = release
%CONFIG=$(CONFIG)
%OUTDIR = build.dos\$(%CONFIG)

#INSTALL_DIR = C:\DIV2
INSTALL_DIR = $(%USERPROFILE)\dosbox\DIV
COPY = xcopy /Y

ASM = WASM
%ASM=$(ASM)
%TASM_EXE = tasm32.exe
%WASM_EXE = wasm.exe
%CC = wcc386.exe

%OPTIONS = -wx -mf -bt=dos -i=judas -i=netlib -i=vbe -zq
%TASM_OPTIONS = /w2 /z /ml

!ifeqi %CONFIG debug
%OPTIONS += -d2
%TASM_OPTIONS += /zi
!else
%OPTIONS += -oneatr -d0
%TASM_OPTIONS += /zn
!endif

%STUB = wstub\$(%CONFIG)\wstub.exe

all: d.exe d.386 .SYMBOLIC

d.exe: wstub d.mif .SYMBOLIC
	$(MAKE) -f d.mif CPU=586 d.exe

d.386: wstub d.mif .SYMBOLIC
	$(MAKE) -f d.mif CPU=386 d.386

wstub: $(%STUB) .SYMBOLIC
	! pushd wstub && $(MAKE) CONFIG=$(%CONFIG) && popd

clean: .SYMBOLIC
	$(MAKE) -f d.mif CPU=586 clean
	$(MAKE) -f d.mif CPU=386 clean

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
	$(COPY) /S bin\genspr\*.* $(INSTALL_DIR)\genspr
	if exist $(INSTALL_DIR)\system\setup.bin del $(INSTALL_DIR)\system\setup.bin
	if exist $(INSTALL_DIR)\system\session.dtf del $(INSTALL_DIR)\system\session.dtf
	if exist $(INSTALL_DIR)\system\user.nfo del $(INSTALL_DIR)\system\user.nfo
	if exist $(INSTALL_DIR)\system\exec.* del $(INSTALL_DIR)\system\exec.*

.SILENT
testinstall: install .SYMBOLIC
	for %i in (setup.bin session.dtf user.nfo) do $(COPY) system\%i $(INSTALL_DIR)\system
