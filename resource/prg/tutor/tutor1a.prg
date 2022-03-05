
//-------------------------------------------------------------------
// Programa: Tutorial 1
// Autor:    Antonio Marchal
// Fecha:    10/10/97
//-------------------------------------------------------------------

PROGRAM Tutorial_1;
BEGIN
    // Se carga el archivo de gr ficos necesario
    load_fpg("tutorial\tutor1.fpg");
    set_mode(m640x480); // Selecciona el modo de video 640 x 480
    put_screen(0,63);   // Pone la pantalla de fondo
    nave(320,240,41);   // Crea el proceso nave
    // Crea con un bucle 4 asteroides grandes
    FROM x=1 TO 8; asteroide(); END
END

//-------------------------------------------------------------------
// Proceso nave
// Maneja la nave protagonista
// Entradas: Coordenadas y c¢digo del gr fico
//-------------------------------------------------------------------

PROCESS nave(x,y,graph)
BEGIN
    LOOP
        // Comprueba si se pulsan las teclas de izquierda o derecha
        // Y modifica el angulo si estan pulsadas
        IF (key(_left)) angle=angle+10000; END
        IF (key(_right)) angle=angle-10000; END
        // Si se pulsa la tecla de arriba, la nave avanza
        IF (key(_up)) advance(8); END
        FRAME;  // Hace aparecer el gr fico de la nave en pantalla
    END
END

//-------------------------------------------------------------------
// Proceso asteroide
// Maneja todos los asteroides del juegos tanto grandes como peque¤os
//-------------------------------------------------------------------

PROCESS asteroide()
BEGIN
    LOOP
        // Crea el asteroide en la esquina superior izquierda
        // (Coordenadas: 0,0) y asigna el c¢digo de gr fico
        x=0; y=0; graph=1;
        // Elige un angulo al azar
        angle=rand(-180000,180000);
        // Repite mientras este en pantalla
        WHILE (x>=0 AND x<=640 AND
               y>=0 AND y<=480)
            // Anima el gr fico, sumando uno a su c¢digo
            graph=graph+1;
            // Si sobrepasa el limite de la animaci¢n, la reinicia
            IF (graph==21) graph=1; END
            // Hace avanzar el gr fico en la direccion determinada
            advance(4);
            FRAME;      // Hace que el gr fico salga en pantalla
        END
    END
END

