//--------------------------------------------------------------------
// Titulo: Tutorial 4
// Autor:  Daniel Navarro Medrano
// Fecha:  28/04/97
//--------------------------------------------------------------------

PROGRAM Tutorial_4;

GLOBAL
    longitud_gusano=8;  // Longitud de la cola del gusano
    manzanas=0;         // N£mero de manzanas en pantalla
    puntos=0,record=0;  // Variables para puntuaci¢n y record

BEGIN

    load_fpg("tutorial\tutor4.fpg");    // Carga el fichero de gr ficos
    put_screen(0,1);                    // Pone el fondo de pantalla

    // Pone los textos de la puntuaci¢n y de los records
    write_int(0,64,8,0,&puntos);
    write_int(0,256,8,0,&record);

    // Crea la cabeza del gusano que maneja todo el cuerpo
    cabeza_gusano(8,96,8,0);

    LOOP                                // Entra en un bucle infinito
        FRAME;                          // Muestra todo en pantalla
        // Aleatoriamente se elige si se pone una manzana
        // o no y siempre si hay menos de 3 manzanas
        IF (rand(0,32)==0 and manzanas<3)
            // Pone una manzana e incrementa el contador de las mismas
            manzana(rand(1,38)*8,rand(3,23)*8);
            manzanas++;
        END
    END
END

//--------------------------------------------------------------------
// Proceso cabeza_gusano
// Maneja la cabeza del gusano
// Entradas: x,y   = Coordenadas del gr fico
//           xi,yi = Incrementos horizontal y vertical
//--------------------------------------------------------------------

PROCESS cabeza_gusano(x,y,ix,iy)

PRIVATE
    identificador_manzana;  // Identificador al proceso de la manzana

BEGIN
    graph=2;                // Se pone el gr fico

    // Y se crea un cuerpo del gusano que crea a los otros
    segmento_gusano(128,priority+1);

    REPEAT
        FRAME;              // Visualiza todo

        // Comprueba las teclas de los cursores y cambia los incrementos
        IF (key(_right))
            ix=8; iy=0;
        END
        IF (key(_left))
            ix=-8; iy=0;
        END
        IF (key(_down))
            ix=0; iy=8;
        END
        IF (key(_up))
            ix=0; iy=-8;
        END

        // Comprueba si el gusano ha colisionado con la manzana
        identificador_manzana=collision(TYPE manzana);
        IF (identificador_manzana)
            // Elimina esa manzana
            signal(identificador_manzana,s_kill);
            manzanas--;         // Decrementa el contador de manzanas
            longitud_gusano+=4; // Incrementa la cola del gusano
            puntos+=10;         // Suma 10 puntos a la puntuaci¢n
        END
        // Mueve al gusano en la direccion deseada
        x=x+ix;
        y=y+iy;
    // Repite has que choque con la pared de fondo (get_pixel) o
    // colisione con su propio cuerpo
    UNTIL (get_pixel(x,y)!=0 or collision(type segmento_gusano));

    // Apaga y enciende la pantalla para el proximo paso
    fade_off();
    fade_on();

    // Comprueba si se ha superado el record y lo actualiza
    IF (puntos>record)
        record=puntos;
    END

    // Reinicia las variable de puntos y longitud de cola
    puntos=0;
    longitud_gusano=8;
    // Elimina todos los procesos del tipo segmento_gusano
    signal(son,s_kill_tree);
    // Crea un nuevo gusano, creando su cabeza
    cabeza_gusano(8,96,8,0);
END

//--------------------------------------------------------------------
// Proceso segmento_gusano
// Maneja los segmentos del cuerpo del gusano
// Entradas: n        = N£mero de cuerpo (total=128)
//           priority = prioridad del proceso en cuanto a ejecuci¢n
//--------------------------------------------------------------------

PROCESS segmento_gusano(n,priority)

BEGIN
    // Si no es el ultimo cuerpo crea otro con mayor prioridad
    // y menor n£mero de cuerpo
    IF (n>0) segmento_gusano(n-1,priority+1); END

    LOOP        // Entra en un bucle infinito

        // Comprueba la prioridad que indica el orden en la cola
        // si esta dentro de la longitud de la cola
        IF (priority<longitud_gusano)
            // Si esta dentro de la longitud lo imprime
            graph=2;
        ELSE
            // Si no, no pone gr fico
            graph=0;
        END
        // Coge las variables de padre, el cuerpo de delante,
        // sin actualizar, es decir,donde estaba antes
        x=father.x;
        y=father.y;
        FRAME;
    END
END

//--------------------------------------------------------------------
// Proceso manzana
// Maneja los gr ficos de las manzanas
// Entradas: x,y = Coordenadas del gr fico
//--------------------------------------------------------------------

PROCESS manzana(x,y)

BEGIN
    graph=3;    // Se elige el gr fico y se entra en un bucle infinito
    LOOP FRAME; END
END
