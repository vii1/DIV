//---------------------------------------------------------------------------
// PROGRAMA: SETUP DE SONIDO
// AUTOR:    DANIEL NAVARRO
// FECHA:    23/03/97
//---------------------------------------------------------------------------

SETUP_PROGRAM div_sound_setup;

CONST
    max_botones=18;

GLOBAL
    STRUCT botones[max_botones] // Estructura general para cada bot¢n
        x,y,                // Coordenadas en pantalla
        graph,              // Gr fico que utiliza al pulsarlo
        inc,                // Incremento que realiza el bot¢n
        max,                // Valor m ximo permitido con su incremento
        valor;              // Valor actual del bot¢n
    END=302,2,12,0,0,0,                  // Bot¢n de escape
        159,58,10,-1,0,&setup.card,      // Bot¢n + de selecci¢n de tarjeta
        177,58,11,1,7,&setup.card,       // Bot¢n - de selecci¢n de tarjeta
        60,98,10,-1,0,&setup.port,       // Bot¢n + de selecci¢n de puerto
        78,98,11,1,5,&setup.port,        // Bot¢n - de selecci¢n de puerto
        159,98,10,-1,0,&setup.irq,       // Bot¢n + de selecci¢n de irq
        177,98,11,1,9,&setup.irq,        // Bot¢n - de selecci¢n de irq
        60,138,10,-1,0,&setup.dma,       // Bot¢n + de selecci¢n de dma
        78,138,11,1,10,&setup.dma,       // Bot¢n - de selecci¢n de dma
        159,138,10,-1,0,&setup.dma2,     // Bot¢n + de selecci¢n de dma2
        177,138,11,1,10,&setup.dma2,     // Bot¢n - de selecci¢n de dma2
        279,58,10,-1,0,&setup.master,    // Bot¢n + de volumen maestro
        297,58,11,1,15,&setup.master,    // Bot¢n - de volumen maestro
        279,98,10,-1,0,&setup.sound_fx,  // Bot¢n + de volumen canal de efectos
        297,98,11,1,15,&setup.sound_fx,  // Bot¢n - de volumen canal de efectos
        279,138,10,-1,0,&setup.cd_audio, // Bot¢n + de volumen CD audio
        297,138,11,1,15,&setup.cd_audio, // Bot¢n - de volumen CD audio
        2,221,13,0,0,0,
        100,221,14,0,0,0;

    // Textos que denominan los tipos de tarjeta de sonido
    tarjeta_sonido[]="No Sound","Sound Blaster 1.5","Sound Blaster 2.0","Sound Blaster Pro",
           "Sound Blaster 16","Sound Blaster AWE","Gravis Ultra Sound","Gravis Ultra Sound MAX";

    // Textos que denominan los puertos posibles
    puerto[]="0x210","0x220","0x230","0x240","0x250","0x260";

    // Textos que denominan las irq posibles
    irq[]="2","3","5","7","10","11","12","13","14","15";

    // Texto que denomiman los canales de dma posibles
    dma[]="0","1","2","3","4","5","6","7","8","9","10";

    id_sonido;  // Identificador del sonido usado

BEGIN
    // Selecciona el modo de video y la velocidad de impresi¢n
    set_mode(m320x240);
    set_fps(100,0);

    fade_on();                      // Enciende la pantalla
    load_fpg("setup\setup_o.fpg");  // Carga el fichero de gr ficos
    put_screen(0,1);                // Pone la pantalla de fondo

    FRAME;                          // Imprime todo

    dump_type=partial_dump;         // Hace que no se restaure la pantalla
    restore_type=partial_restore;   // para asi ganar velocidad

    mouse.graph=100;                // Selecciona el gr fico del rat¢n
    sombra_rat¢n(101,4);            // Y crea una sombra del mismo debajo

    // Crea todos los botones necesarios
    FROM x=0 TO max_botones;
        bot¢n(x);
    END

    // Crea los tres volumenes necesarios
    volumen(1,210,60);
    volumen(2,210,100);
    volumen(3,210,140);

    // Muestra los valores de todos los parametros
    muestra_valores();

    // Repite indefinidamente
    LOOP

        // Si se pulsa la tecla [ESCAPE] sale del programa
        IF (key(_esc)) exit("",0); END
        FRAME;
    END
END

//---------------------------------------------------------------------------
// Proceso sombra_rat¢n
// Pone el gr fico de la sombra del rat¢n
// Entradas: graph = n£mero de grafico
//           flags = bandera de espejado y transparencia
//---------------------------------------------------------------------------

PROCESS sombra_rat¢n(graph,flags)

BEGIN
    z=mouse.z+1;    // Hace que se imprima un plano por debajo del rat¢n
    LOOP
        // Sus coordenadas estan m s a la derecha y m s abajo que el rat¢n
        x=mouse.x+4;
        y=mouse.y+2;
        FRAME;
    END
END

//---------------------------------------------------------------------------
// Proceso bot¢n
// Maneja los todos los botones del programa
// Entradas: n£mero = n£mero de bot¢n que tiene que manejar
//---------------------------------------------------------------------------

PROCESS bot¢n(n£mero)

PRIVATE
    ancho; // Ancho en pixels del bot¢n

BEGIN
    // Coge las coordenadas de la tabla de botones
    x=botones[n£mero].x;
    y=botones[n£mero].y;
    // Si el bot¢n es de los dos £ltimos es m s ancho
    IF (n£mero>16) ancho=96; ELSE ancho=16; END
    // Repite indefinidamente
    LOOP
        // Comprueba si se hace click con el rat¢n encima del bot¢n
        IF (mouse.x>x and mouse.x<x+ancho and
            mouse.y>y and mouse.y<y+16 and mouse.left)
            // Cambia el gr fico por el del bot¢n pulsado
            graph=botones[n£mero].graph;
        ELSE
            // Si la £ltima vez se pulso el bot¢n
            IF (graph<>0)
                // Quita el gr fico del bot¢n pulsado
                graph=0;
                FRAME;
                // Comprueba que bot¢n es el pulsado
                switch(n£mero)
                    // Sale del programa
                    case 0:
                        exit("",0);
                    END
                    // Graba la configuraci¢n y sale del programa
                    case 17:
                        save("sound.cfg",&setup,sizeof(setup));
                        exit("",0);
                    END
                    // Hace un test de sonido
                    case 18:
                        unload_pcm(id_sonido);  // Descarga el sonido si lo hubiera
                        reset_sound();          // Actualiza los valores de configuraci¢n
                        // Carga el sonido de prueba
                        id_sonido=load_pcm("setup\setup_o.pcm",0);
                        // Si no esta tocando una canci¢n el CD
                        IF (not is_playing_cd())
                            // Hace que suene el CD
                            play_cd(2,0);
                        END
                        // Ejecuta el sonido
                        sound(id_sonido,256,256);
                    END
                    // Para los dem s botones
                    default:
                        // Si el incremento a realizar es positivo
                        IF (botones[n£mero].inc>0)
                            // Y si no se sobrepasa el limite
                            IF ([botones[n£mero].valor]<botones[n£mero].max)
                                // Realiza el incremento
                                [botones[n£mero].valor]+=botones[n£mero].inc;
                            END
                        ELSE    // Si no, es negativo
                            // Y si tampoco se sobrepasa el limite
                            IF ([botones[n£mero].valor]>botones[n£mero].max)
                                // Realiza el decremento
                                [botones[n£mero].valor]+=botones[n£mero].inc;
                            END
                        END
                    END
                END
                // Muestra los valores actualizados
                muestra_valores();
            END
        END
        FRAME;
    END
END

//---------------------------------------------------------------------------
// Proceso muestra_valores
// Muestra los valores actualizados
//---------------------------------------------------------------------------

PROCESS muestra_valores()

BEGIN
    // Borra cualquier texto que hubiera impreso
    delete_text(all_text);
    // Escribe los nuevos textos actualizados
    write(0,12,61,0,tarjeta_sonido[setup.card]);
    write(0,12,101,0,puerto[setup.port]);
    write(0,111,101,0,irq[setup.irq]);
    write(0,12,141,0,dma[setup.dma]);
    write(0,111,141,0,dma[setup.dma2]);
    // Define tres regiones para los tres volumenes configurables
    // con un tama¤o proporcional a sus valores
    define_region(1,210,60,4+(4*setup.master),11);
    define_region(2,210,100,4+(4*setup.sound_fx),11);
    define_region(3,210,140,4+(4*setup.cd_audio),11);
END

//---------------------------------------------------------------------------
// Proceso volumen
// Maneja los gr ficos de los volumenes
// Entradas: region = N£mero de regi¢n del volum‚n
//           x,y    = Coordenadas en pantalla
//---------------------------------------------------------------------------

PROCESS volumen(region,x,y)

BEGIN
    // Selecciona el gr fico
    graph=50;
    // Repite indefinidamente
    LOOP
        FRAME;
    END
END
