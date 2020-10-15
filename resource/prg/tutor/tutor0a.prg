
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
END

//-----------------------------------------------------------------------
// Maneja la nave protagonista
//-----------------------------------------------------------------------

PROCESS nave()
BEGIN
    graph=1; x=160; y=180;    // Selecciona el gr fico y las coordenadas
    LOOP                      // Entra en un bucle infinito
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
    REPEAT      // Bucle de repetici¢n
        y-=16;  // Se mueve hacia arriba 16 puntos
        FRAME;  // Muestra la siguiente imagen
    UNTIL (y<0) // Repite hasta que se salga por la parte superior
END

