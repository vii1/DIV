
//--------------------------------------------------------------------
// Programa: Tutorial 2
// Autor:    Daniel Navarro Medrano
// Fecha:    20/10/97
//--------------------------------------------------------------------

PROGRAM Tutorial_2;

BEGIN
    // Carga el archivo que contiene todos los gr ficos
    load_fpg("tutorial\tutor2.fpg");
    // Selecciona el n£mero de imagenes por segundo
    set_fps(24,0);
    // Pone la pantalla de fondo
    put_screen(0,41);
    // Escribe un texto explicativo
    write(0,160,2,1,"Pulse espacio para crear planetas");
    // Crea un bucle infinito, ya que como no hay ningun
    // proceso creado por ahora, el programa acabaria si no
    // hubiera dicho bucle
    LOOP
        // Cada vez que se pulsa la tecla espacio se crea un
        // proceso del tipo tierra
        IF (key(_space)) tierra(); END
        FRAME;
    END
END

//--------------------------------------------------------------------
// Proceso tierra
// Maneja las animaciones de la tierra
//--------------------------------------------------------------------

PROCESS tierra();

PRIVATE
    velocidad_x;         // Incremento en la coordenada horizontal
    velocidad_y;         // Incremento en la coordenada vertical
    velocidad_y_inicial; // Longitud del bote

BEGIN
    // Crea el proceso en la mitad del lateral izquierdo
    x=0; y=1800;
    resolution=10;  // Hace que las coordenadas usen un decimal
    // Inicia el incremento horizontal de 1 a 8 puntos
    velocidad_x=rand(10,80);
    // Reinicia la longitud del bote inicial entre 8 y 25 puntos
    velocidad_y_inicial=rand(-80,-250);
    // El incremento vertical es igual a la longitud del bote
    velocidad_y=velocidad_y_inicial;
    // Crea un bucle infinito
    LOOP
        // Crea un bucle que va pasando por todas las imagenes
        // que componen la animacion que va de los codigos 0 a 40
        FROM graph=1 TO 40;
            // Mueve el proceso horizontalmente
            x=x+velocidad_x;
            // Si llega a algun extremo de la pantalla
            IF (x<0 OR x>3200)
                // Cambia el signo del incremento y con ello
                // la direccion del movimiento
                velocidad_x=-velocidad_x;
            END
            // Mueve el proceso verticalmente
            y=y+velocidad_y;
            // Si la longitud del bote a llegado a su limite
            IF (-velocidad_y<=velocidad_y_inicial)
                // Reinicia la longitud a la inicial
                velocidad_y=-velocidad_y;
            ELSE
                // Va bajando el incremento en la longitud del bote
                velocidad_y=velocidad_y+20;
            END
            FRAME;  // Hace que el proceso aparezca en pantalla
        END
    END
END