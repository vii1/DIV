//---------------------------------------------------------------------------
// Programa: Tutorial 3
// Autor: Daniel Navarro Medrano
// Fecha: 27/2/97
//---------------------------------------------------------------------------

PROGRAM Tutorial_3;

GLOBAL
    raqueta1,raqueta2; // Identificadores de las raquetas

BEGIN
    // Carga el fichero con los gr ficos del juego
    load_fpg("tutorial\tutor3.fpg");

    put_screen(0,1);    // Pone el fondo de pantalla
    fade_on();          // Enciende la pantalla
    // Crea las dos raquetas y coge sus identificadores
    raqueta1=raqueta(6,24,_q,_a);
    raqueta2=raqueta(314,24,_p,_l);
    // Crea el proceso de la bola
    bola(160,100,1,1);
END

//---------------------------------------------------------------------------
// Proceso bola
// Maneja la bola del juego
// Entradas: x,y   = Coordenadas del gr fico
//           ix,iy = Incrementos en cada una de las coordenadas
//---------------------------------------------------------------------------

PROCESS bola(x,y,ix,iy);

BEGIN
    graph=3;    // Selecciona el gr fico
    REPEAT
        FRAME(25);  // Lo muestra en pantalla m s veces que los demas
        // Comprueba si rebota con los laterales superior e inferior
        IF (y==14 or y==186)
            iy=-iy; // Cambia la direccion vertical
        END
        // Comprueba si rebota con las raquetas
        IF ((x==10 and abs(y-raqueta1.y)<22) or
           (x==310 and abs(y-raqueta2.y)<22))
            ix=-ix; // Cambia la direccion horizontal
        END
        // Mueve la pelota
        x=x+ix;
        y=y+iy;
    UNTIL (out_region(id,0))    // Repite hasta que se salga de pantalla
    bola(160,100,ix,iy);        // Crea una nueva pelota
END

//---------------------------------------------------------------------------
// Proceso raqueta
// Maneja las raquetas de los jugadores
// Entradas: x,y    = Coordenadas de los gr ficos
//           arriba = Tecla para moverse hacia arriba
//           abajo  = Tecla para moverse hacia abajo
//---------------------------------------------------------------------------

PROCESS raqueta(x,y,arriba,abajo)

BEGIN
    graph=2;        // Selecciona el gr fico
    LOOP
        FRAME;      // Muestra la imagen
        // Si se pulsa la tecla para arriba y no ha llegado al limite
        IF (key(arriba) and y>24)
            y=y-4;  // Mueve la raqueta
        END
        // Si se pulsa la tecla para abajo y no ha llegado al limite
        IF (key(abajo) and y<176)
            y=y+4; // Mueve la raqueta
        END
    END
END
