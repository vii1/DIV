//----------------------------------------------------------------------
// Programa: Tutorial 7
// Autor:  Daniel Navarro
// Fecha:  20/09/97
//----------------------------------------------------------------------

PROGRAM Tutorial_7;

GLOBAL
    // Tablas con los c¢digos de los gr ficos en 16 posiciones
    // C¢digos de la silla
    silla[]=16,100,101,101,103,104,105,106,107,
            108,109,110,111,112,113,114,115;
    // C¢digos del baul o cofre
    cofre[]=16,200,201,202,203,204,205,206,207,
            208,209,210,211,212,213,214,215;
    // C¢digos de la mesa
    mesa[]=16,300,301,302,303,304,305,306,307,
            300,301,302,303,304,305,306,307;
    // C¢digos de la percha
    percha[]=16,400,401,402,403,400,401,402,403,
            400,401,402,403,400,401,402,403;
    // C¢digos del armario
    armario[]=16,500,501,502,503,504,505,506,507,
            508,509,510,511,512,513,514,515;
    // C¢digos del sill¢n
    sill¢n[]=16,600,601,602,603,604,605,606,607,
            608,609,610,611,612,613,614,615;

BEGIN
    // Se selecciona el modo de video
    set_mode(m640x480);

    // Carga el archivo de gr ficos necesarios en el juego
    load_fpg("tutorial\tutor7.fpg");

    // Enciende la pantalla
    fade_on();

    m7.camera=id;       // Hace que la c mara siga a este proceso (el principal)
    m7.height=512;      // Altura de la c mara
    m7.distance=640;    // Distancia de la c mara al punto de observaci¢n
    // Inicia el modo 7 o de plano abatido
    start_mode7(0,0,1,0,0,128);
    // Pone un texto explicativo
    write(0,320,0,1,"Utilice los cursores para mover la c mara");

    // Crea los objetos tipo mesa
    objeto(&mesa,128,128,0,600);
    objeto(&mesa,600,440,0,600);
    objeto(&mesa,600,340,0,600);

    // Crea los objetos tipo silla
    objeto(&silla,64,64,-pi/2,400);
    objeto(&silla,192,64,-pi/2,400);
    objeto(&silla,64,192,pi/2,400);
    objeto(&silla,192,192,pi/2,400);

    // Crea los objetos tipo armario
    objeto(&armario,32,384,0,600);
    objeto(&armario,384,32,-pi/2,600);

    // Crea los objetos tipo baul
    objeto(&cofre,64,440,pi/2,800);
    objeto(&cofre,128,440,pi/2,800);
    objeto(&cofre,384,440,pi/2,800);

    // Crea el objeto tipo percha
    objeto(&percha,192,440,0,600);

    // Crea los objetos tipo sill¢n
    objeto(&sill¢n,512,32,-pi/2,400);
    objeto(&sill¢n,600,128,-pi,400);

    // Pone las coordenadas de este proceso que es la camara
    x=320; y=240;
    LOOP
        // Si se pulsan los cursores se varia el angulo de la camara (se mueve)
        if (key(_right)) angle-=pi/8; END
        if (key(_left)) angle+=pi/8; END
        FRAME;
    END
END

//----------------------------------------------------------------------
// Proceso objeto
// Visualiza todos los objetos del tutorial
// Entradas: xgraph = Puntero a la tabla de gr ficos del objeto
//           x,y    = Coordenadas
//           angle  = Angulo del objeto
//           size   = Tama¤o de objeto en tanto por ciento
//----------------------------------------------------------------------

PROCESS objeto(xgraph,x,y,angle,size)

BEGIN
    z=-16;          // Elige la profundidad
    ctype=c_m7;     // Introduce al proceso dentro del modo 7
    LOOP
        FRAME;      // Muestra el gr fico
    END
END
