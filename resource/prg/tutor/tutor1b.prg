
//-------------------------------------------------------------------
// TITULO: Tutorial 1
// AUTOR:  Antonio Marchal
// FECHA:  10/10/97
//-------------------------------------------------------------------

PROGRAM Tutorial_1;

GLOBAL
    puntuaci¢n=0;     // Variable para guardar la puntuaci¢n

BEGIN
    load_fpg("tutorial\tutor1.fpg");  // Se carga el fichero de gr ficos
    load_fnt("tutorial\tutor1.fnt");  // Se carga el tipo de letras

    set_mode(m640x480);               // Pone el modo de video 640 x 480
    set_fps(24,0);                    // N£mero de im genes por segundo
    put_screen(0,63);                 // Pone la pantalla de fondo
    nave(320,240,41);                 // Crea el proceso nave

    write_int(1,320,0,1,OFFSET puntuaci¢n); // Muestra la puntuacion
    FROM x=1 TO 4; asteroide(1); END        // Crea 4 asteroides grandes
    FROM x=1 TO 4; asteroide(21); END       // Crea 4 asteroides peque¤os
END

//-------------------------------------------------------------------
// Maneja la nave protagonista
// Entradas: Coordenadas y c¢digo del gr fico
//-------------------------------------------------------------------

PROCESS nave(x,y,graph)
BEGIN
    LOOP
        // Comprueba si se pulsan las teclas de izquierda o derecha
        // Y modifica el  ngulo si est n pulsadas
        IF (key(_left)) angle=angle+10000; END
        IF (key(_right)) angle=angle-10000; END
        // Si se pulsa la tecla de arriba, la nave avanza
        IF (key(_up)) advance(8); END
        // Si se pulsa la tecla control crea un proceso del tipo disparo
        IF (key(_control)) disparo(x,y,angle); END
        // Corrige las coordenadas, para que la nave no desaparezca
        // de pantalla
        corrige_coordenadas();
        FRAME;  // Hace aparecer el gr fico de la nave en pantalla
    END
END

//-------------------------------------------------------------------
// Proceso disparo
// Crea los disparos de la nave protagonista
// Entradas: Coordenadas y  ngulo del gr fico
//-------------------------------------------------------------------

PROCESS disparo(x,y,angle)
BEGIN
    // Hace un avance inicial y define el gr fico a usar
    advance(24); graph=42;
    // Mientras el gr fico este dentro de pantalla
    WHILE (NOT out_region(id,0))
        // Hace que avance en el angulo que tenga predeterminado
        advance(16);
        FRAME;  // Hace que el gr fico aparezca en pantalla
    END
END

//-------------------------------------------------------------------
// Proceso asteroide
// Maneja todos los asteroides del juegos tanto grandes como peque¤os
// Entradas: Codigo del tipo de asteroide que es su gr fico inicial
//-------------------------------------------------------------------

PROCESS asteroide(c¢digo)
BEGIN
    LOOP
        // Crea el asteroide en la esquina superior izquierda
        // (Coordenadas: 0,0) y asigna el c¢digo de gr fico
        x=0; y=0; graph=c¢digo;
        // Elige un angulo al azar
        angle=rand(-180000,180000);
        LOOP
            // Anima el gr fico, sumando uno a su c¢digo
            graph=graph+1;
            // Si sobrepasa el limite de la animaci¢n, la reinicia
            IF (graph==c¢digo+20) graph=c¢digo; END
            // Hace avanzar el gr fico en la direccion determinada
            advance(4);
            // Si colisiona con el gr fico del disparo sale
            // del bucle, que de otra manera no tiene final
            IF (collision (TYPE disparo)) BREAK; END
            // Corrige las coordenadas para que no desaparezca de
            // pantalla, haciendole aparecer por el otro lado
            corrige_coordenadas();
            FRAME;      // Hace que el gr fico salga en pantalla
        END
        puntuaci¢n=puntuaci¢n+5;    // Suma 5 puntos a la puntuaci¢n
        // Pone la animaci¢n de la explosion mediante un bucle
        FROM graph=43 TO 62; FRAME; END
    END
END

//-------------------------------------------------------------------
// Proceso corrige_coordenadas
// Corrige las coordenadas del proceso que los llama haciendo
// que dichas coordenadas sean ciclicas en pantalla
//-------------------------------------------------------------------

PROCESS corrige_coordenadas()
BEGIN
    // Si se sale por la izquierda hace que aparezca por la derecha
    // restando para ello el ancho de pantalla
    IF (father.x<-20) father.x=father.x+680; END
    // Si se sale por la derecha hace que aparezca por la izquierda
    IF (father.x>660) father.x=father.x-680; END
    // Si se sale por la arriba hace que aparezca por la abajo
    IF (father.y<-20) father.y=father.y+520; END
    // Si se sale por la abajo hace que aparezca por la arriba
    IF (father.y>500) father.y=father.y-520; END
END
