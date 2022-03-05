
//-----------------------------------------------------------------------
// TITULO: Tutorial 0
// AUTOR:  Daniel Navarro
// FECHA:  15/09/97
//-----------------------------------------------------------------------

PROGRAM Tutorial_0;
BEGIN
    load_fpg("tutorial\tutor0.fpg"); // Se carga el fichero de gr ficos
    put_screen(0,2);  // Se pone el gr fico n£mero 2 como fondo de pantalla
    nave();           // Se crea el proceso tipo "nave"

    LOOP
        // Si un n£mero al azar entre 0 y 100 es menor que 30, entonces crea
        // un proceso tipo "enemigo" en la parte superior de la pantalla
        IF (rand(0,100)<30)
            enemigo(rand(0,320),rand(-4,4),rand(6,12));
        END
        FRAME;        // Pasa a la siguiente imagen del juego
    END
END

//-----------------------------------------------------------------------
// Maneja la nave protagonista
//-----------------------------------------------------------------------

PROCESS nave()
BEGIN
    graph=1; x=160; y=180;    // Selecciona el gr fico y las coordenadas

    LOOP
        x=mouse.x;            // Pone la nave en la coordenada x del rat¢n
        IF (mouse.left)       // Al pulsar el bot¢n izquierdo del rat¢n se
            disparo(x,y-20);  // crea un proceso de tipo "disparo"
        END
        FRAME;                // Muestra la siguiente imagen de la nave
    END
END

//-----------------------------------------------------------------------
// Maneja los disparos de la nave protagonista
// Entradas: Coordenadas del gr fico
//-----------------------------------------------------------------------

PROCESS disparo(x,y)
BEGIN
    graph=3;    // Se selecciona el gr fico

    REPEAT
        y-=16;  // Se mueve hacia arriba 16 puntos
        FRAME;  // Muestra la siguiente imagen
    UNTIL (y<0) // Repite hasta que se salga por la parte superior
END

//-----------------------------------------------------------------------
// Enemigos que caen de la parte superior de la pantalla
// Entradas: Coordenada x, incremento x e incremento y (por cada imagen)
//-----------------------------------------------------------------------

PROCESS enemigo(x,inc_x,inc_y)
BEGIN
    graph=4;            // Define el gr fico del proceso (el 4§ del fichero)
    y=-20;              // Sit£a el gr fico en la parte superior de pantalla
    size=rand(25,100);  // Se escoge al azar su tama¤o (entre 25% y 100%)

    REPEAT
      x+=inc_x;         // Le suma el incremento x a su coordenada x
      y+=inc_y;         // Igual con la coordenada y
      FRAME;
    // Repite hasta que se salga de la pantalla o choque con un disparo
    UNTIL (y>220 OR collision(TYPE disparo));

    FROM graph=5 TO 10; // Muestra una explosi¢n en la posici¢n del enemigo
        FRAME;          // (los gr ficos de la explosi¢n son del 5 al 10)
    END
END
