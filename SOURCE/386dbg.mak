#컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
#       Interprete DIV
#컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴

# i - Int굍prete
# f - Funciones internas
# c - Detecci줻 de colisiones
# s - Sprites, textos y scrolles
# v - Video, paleta
# m - Mouse
# d - Debugger

EXE=session.div
#EXE=div32run.ins

OPTIONS=/oneatx /wx /mf /zp4 /bt=dos /3r /fpc
OPTIONS2=/oneatx /wx /mf /zp4 /bt=dos /3r /fpc

$(EXE): i.obj f.obj c.obj s.obj v.obj a.obj
$(EXE): divfli.obj cdrom.obj divkeybo.obj mouse.obj
$(EXE): divdll1.obj divdll2.obj ia.obj
$(EXE): divsound.obj divmixer.obj divtimer.obj timer.obj

$(EXE): d.obj

$(EXE): divlengu.obj vesa.obj det_vesa.obj

$(EXE): draw_cw.obj draw_f.obj draw_o.obj draw_sw.obj
$(EXE): fixed.obj gfx.obj globals.obj hard.obj
$(EXE): load.obj mem.obj object.obj scan.obj
$(EXE): update.obj view.obj vpe.obj zone.obj
$(EXE): draw_fa.obj draw_oa.obj draw_wa.obj vpedll.obj

$(EXE): red.obj net.obj comlib.obj comm.obj ipxlib.obj dpmi_net.obj

  wlink @i.lnk

i.obj: i.cpp kernel.cpp
  wcc386 $(OPTIONS) i.cpp

f.obj: f.cpp
  wcc386 $(OPTIONS) f.cpp

c.obj: c.cpp
  wcc386 $(OPTIONS) c.cpp

s.obj: s.cpp
  wcc386 $(OPTIONS) s.cpp

v.obj: v.cpp
  wcc386 $(OPTIONS) v.cpp

divkeybo.obj: divkeybo.cpp
  wcc386 $(OPTIONS) divkeybo.cpp

mouse.obj: mouse.cpp
  wcc386 $(OPTIONS) mouse.cpp

divfli.obj: divfli.cpp
  wcc386 $(OPTIONS) divfli.cpp

cdrom.obj: cdrom.cpp
  wcc386 /oneatx /wx /mf /bt=dos /3r /fpc cdrom.cpp

divdll1.obj: divdll1.c
  wcc386 $(OPTIONS) divdll1.c

divdll2.obj: divdll2.c
  wcc386 $(OPTIONS) divdll2.c

ia.obj: ia.cpp
  wcc386 $(OPTIONS) ia.cpp

divsound.obj: divsound.cpp
  wcc386 $(OPTIONS) divsound.cpp

divmixer.obj: divmixer.cpp
  wcc386 $(OPTIONS) divmixer.cpp

divtimer.obj: divtimer.cpp
  wcc386 $(OPTIONS) divtimer.cpp

timer.obj: timer.asm
        a /m /ml timer.asm

a.obj: a.asm
  wasm a.asm

d.obj: d.cpp
  wcc386 $(OPTIONS) d.cpp

divlengu.obj: divlengu.cpp
  wcc386 $(OPTIONS) divlengu.cpp

vesa.obj: vesa.asm
  a vesa.asm /ml /m2 /w2 /z /zn

det_vesa.obj: det_vesa.cpp
  wcc386 $(OPTIONS) det_vesa.cpp

#컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
#   VPE
#컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴

vpedll.obj: ..\vpe\vpedll.cpp
  wcc386 $(OPTIONS2) ..\vpe\vpedll.cpp

draw_cw.obj: ..\vpe\draw_cw.cpp
  wcc386 $(OPTIONS2) ..\vpe\draw_cw.cpp

draw_f.obj: ..\vpe\draw_f.cpp
  wcc386 $(OPTIONS2) ..\vpe\draw_f.cpp

draw_o.obj: ..\vpe\draw_o.cpp
  wcc386 $(OPTIONS2) ..\vpe\draw_o.cpp

draw_sw.obj: ..\vpe\draw_sw.cpp
  wcc386 $(OPTIONS2) ..\vpe\draw_sw.cpp

fixed.obj: ..\vpe\fixed.cpp
  wcc386 $(OPTIONS2) ..\vpe\fixed.cpp

gfx.obj: ..\vpe\gfx.cpp
  wcc386 $(OPTIONS2) ..\vpe\gfx.cpp

globals.obj: ..\vpe\globals.cpp
  wcc386 $(OPTIONS2) ..\vpe\globals.cpp

hard.obj: ..\vpe\hard.cpp
  wcc386 $(OPTIONS2) ..\vpe\hard.cpp

load.obj: ..\vpe\load.cpp
  wcc386 $(OPTIONS2) ..\vpe\load.cpp

mem.obj: ..\vpe\mem.cpp
  wcc386 $(OPTIONS2) ..\vpe\mem.cpp

object.obj: ..\vpe\object.cpp
  wcc386 $(OPTIONS2) ..\vpe\object.cpp

scan.obj: ..\vpe\scan.cpp
  wcc386 $(OPTIONS2) ..\vpe\scan.cpp

update.obj: ..\vpe\update.cpp
  wcc386 $(OPTIONS2) ..\vpe\update.cpp

view.obj: ..\vpe\view.cpp
  wcc386 $(OPTIONS2) ..\vpe\view.cpp

vpe.obj: ..\vpe\vpe.cpp
  wcc386 $(OPTIONS2) ..\vpe\vpe.cpp

zone.obj: ..\vpe\zone.cpp
  wcc386 $(OPTIONS2) ..\vpe\zone.cpp

draw_fa.obj: ..\vpe\draw_fa.asm
  wasm /mf ..\vpe\draw_fa

draw_oa.obj: ..\vpe\draw_oa.asm
  wasm /mf ..\vpe\draw_oa

draw_wa.obj: ..\vpe\draw_wa.asm
  wasm /mf ..\vpe\draw_wa

#컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
#   RED
#컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴

red.obj: ..\netlib\red.cpp
  wcc386 $(OPTIONS2) ..\netlib\red.cpp

net.obj: ..\netlib\net.c
  wcc386 $(OPTIONS2) ..\netlib\net.c

comlib.obj: ..\netlib\comlib.c
  wcc386 $(OPTIONS2) ..\netlib\comlib.c

comm.obj: ..\netlib\comm.c
  wcc386 $(OPTIONS2) ..\netlib\comm.c

ipxlib.obj: ..\netlib\ipxlib.c
  wcc386 $(OPTIONS2) ..\netlib\ipxlib.c

dpmi_net.obj: ..\netlib\dpmi_net.c
  wcc386 $(OPTIONS2) ..\netlib\dpmi_net.c

