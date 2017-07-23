//---------------------------------------------------------------------------
// PROGRAMA: SETUP DE SONIDO
// AUTOR:    DANIEL NAVARRO
// FECHA:    23/03/97
//---------------------------------------------------------------------------

SETUP_PROGRAM div_sound_setup;

CONST
    max_botones=24;

GLOBAL
    STRUCT botones[max_botones] // Estructura general para cada bot¢n
        x,y,                // Coordenadas en pantalla
        graph,              // Gr fico que utiliza al pulsarlo
        inc,                // Incremento que realiza el bot¢n
        max,                // Valor m ximo permitido con su incremento
        valor;              // Valor actual del bot¢n
    END =
        262,136,10,-1,0,&setup.card,         // Bot¢n + de selecci¢n de tarjeta
        285,136,11,1,7,&setup.card,          // Bot¢n - de selecci¢n de tarjeta

        135,195,10,-1,0,&setup.port,         // Bot¢n + de selecci¢n de puerto
        158,195,11,1,5,&setup.port,          // Bot¢n - de selecci¢n de puerto
        261,195,10,-1,0,&setup.irq,          // Bot¢n + de selecci¢n de irq
        284,195,11,1,9,&setup.irq,           // Bot¢n - de selecci¢n de irq
        135,255,10,-1,0,&setup.dma,          // Bot¢n + de selecci¢n de dma
        158,255,11,1,10,&setup.dma,          // Bot¢n - de selecci¢n de dma
        261,255,10,-1,0,&setup.dma2,         // Bot¢n + de selecci¢n de dma2
        284,255,11,1,10,&setup.dma2,         // Bot¢n - de selecci¢n de dma2
        135,315,10,-8,8,&setup.bits,         // Bot¢n + de selecci¢n de bits
        158,315,11,8,16,&setup.bits,         // Bot¢n - de selecci¢n de bits
        261,315,10,-11025,11025,&setup.rate, // Bot¢n + de selecci¢n de frecuencia
        284,315,11,11025,44100,&setup.rate,  // Bot¢n - de selecci¢n de frecuencia

        539,136,10,-1,0,&setup.master,       // Bot¢n + de volumen maestro
        562,136,11,1,15,&setup.master,       // Bot¢n - de volumen maestro
        539,195,10,-1,0,&setup.sound_fx,     // Bot¢n + de volumen canal de efectos
        562,195,11,1,15,&setup.sound_fx,     // Bot¢n - de volumen canal de efectos
        539,255,10,-1,0,&setup.cd_audio,     // Bot¢n + de volumen CD audio
        562,255,11,1,15,&setup.cd_audio,     // Bot¢n - de volumen CD audio
        539,315,10,-1,1,&setup.mixer,        // Bot¢n + de selecci¢n de mezclador
        562,315,11,1,2,&setup.mixer,         // Bot¢n - de selecci¢n de mezclador

        423,38,13,0,0,0,                     // Bot¢n de guardar
        510,38,14,0,0,0,                     // Bot¢n de prueba
        597,38,12,0,0,0;                     // Bot¢n de salir

    // Textos que denominan los tipos de tarjeta de sonido
    tarjeta_sonido[]="No Sound","Sound Blaster 1.5","Sound Blaster 2.0","Sound Blaster Pro",
           "Sound Blaster 16","Sound Blaster AWE","Gravis Ultra Sound","Gravis Ultra Sound MAX";

    // Textos que denominan los puertos posibles
    puerto[]="0x210","0x220","0x230","0x240","0x250","0x260";

    // Textos que denominan las irq posibles
    irq[]="2","3","5","7","10","11","12","13","14","15";

    // Texto que denomiman los canales de dma posibles
    dma[]="0","1","2","3","4","5","6","7","8","9","10";

    // Texto que denomiman el tipo de mezclador
    mixer[]="","M s r pido","Mayor calidad";

    id_sonido;  // Identificador del sonido usado

BEGIN
    // Selecciona el modo de video y la velocidad de impresi¢n
    set_mode(m640x480);
    set_fps(100,0);

    load_fpg("setup\setup.fpg");    // Carga el fichero de gr ficos
    load_fnt("setup\setup.fnt");    // Y la fuente de letras
    put_screen(0,1);                // Pone la pantalla de fondo

    dump_type=partial_dump;         // Hace que no se restaure la pantalla
    restore_type=partial_restore;   // para asi ganar velocidad

    mouse.graph=100;                // Selecciona el gr fico del rat¢n

    // Crea todos los botones necesarios
    FROM x=0 TO max_botones;
        bot¢n(x);
    END

    // Crea los tres volumenes necesarios
    volumen(1,458,164);
    volumen(2,458,222);
    volumen(3,458,282);

    // Muestra los valores de todos los parametros
    muestra_valores();

    // Repite indefinidamente
    LOOP
        // Si se pulsa la tecla [ESC] sale del programa
        IF (key(_esc)) exit("",0); END
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
    bot¢n_pulsado; // Indica si est  pulsado el bot¢n izquierdo del rat¢n

BEGIN
    // Coge las coordenadas de la tabla de botones
    x=botones[n£mero].x;
    y=botones[n£mero].y;
    // Repite indefinidamente
    LOOP
        // Comprueba si se hace click con el rat¢n encima del bot¢n
        graph=botones[n£mero].graph;
        IF (NOT collision(TYPE mouse) OR NOT mouse.left)
            graph=0;
            // Si la £ltima vez se puls¢ el bot¢n
            IF (bot¢n_pulsado)
                // Quita el gr fico del bot¢n pulsado
                bot¢n_pulsado=FALSE;
                FRAME;
                // Comprueba que bot¢n es el pulsado
                switch(n£mero)
                    // Graba la configuraci¢n
                    case 22:
                        reset_sound();
                        save("sound.cfg",&setup,sizeof(setup));
                    END
                    // Hace un test de sonido
                    case 23:
                        unload_pcm(id_sonido);  // Descarga el sonido si lo hubiera
                        reset_sound();          // Actualiza los valores de configuraci¢n
                        // Carga el sonido de prueba
                        id_sonido=load_pcm("setup\setup.pcm",0);
                        // Ejecuta el sonido
                        sound(id_sonido,256,256);
                    END
                    // Sale del programa
                    case 24:
                        exit("",0);
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
        ELSE
            IF (mouse.left) bot¢n_pulsado=TRUE; END
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
    write(1,178,162,4,tarjeta_sonido[setup.card]);
    write(1,120,220,4,puerto[setup.port]);
    write(1,244,220,4,irq[setup.irq]);
    write(1,120,280,4,dma[setup.dma]);
    write(1,244,280,4,dma[setup.dma2]);
    write_int(1,120,340,4,&setup.bits);
    write_int(1,244,340,4,&setup.rate);
    write(1,450,340,4,mixer[setup.mixer]);

    // Define tres regiones para los tres volumenes configurables
    // con un tama¤o proporcional a sus valores
    define_region(1,346,157,14+(14*setup.master),15);
    define_region(2,346,215,14+(14*setup.sound_fx),15);
    define_region(3,346,275,14+(14*setup.cd_audio),15);
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
