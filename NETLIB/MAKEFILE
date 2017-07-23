net.exe:        net.obj comlib.obj comm.obj ipxlib.obj dpmi_net.obj

net.obj:        net.c net.h ipxlib.h comm.h comlib.h
                wcc386 -zp1 -5r -mf net.c

comlib.obj:     comlib.c net.h comlib.h
                wcc386 -zp1 -5r -mf comlib.c

comm.obj:       comm.c net.h comm.h
                wcc386 -zp1 -5r -mf comm.c

ipxlib.obj:     ipxlib.c net.h ipxlib.h
                wcc386 -zp1 -5r -mf ipxlib.c

dpmi_net.obj:   dpmi_net.c net.h dpmi_net.h
                wcc386 -zp1 -5r -mf dpmi_net.c

