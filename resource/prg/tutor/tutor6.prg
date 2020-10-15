//-------------------------------------------------------------------------
// Programa: Tutorial 6
// Autor:    Antonio Marchal
// Fecha:    3/10/97
//-------------------------------------------------------------------------

PROGRAM Tutorial_6;

GLOBAL
    tablero[99]= 100 dup (1);   // Guarda las 100 casillas del tablero

LOCAL
    contador;                   // Contadores de uso general
    contador2;

BEGIN
    load_fpg("tutorial\tutor6.fpg");     // Carga el archivo de graficos
    load_fnt("tutorial\tutor6.fnt");     // Carga el archivo de fuentes de letras
    set_mode(m640x480);         // Selecciona el modo de video
    put_screen(0,1);            // Pone la pantalla de fondo
    mouse.graph=999;            // Selecciona el gr fico del cursor del raton
    mouse.file=0;
    tablero[44]=0;              // Pone la ficha del centro cambiada
    dibuja_tablero();           // Proceso que crea el tablero

    // Repite hasta que se pulse escape o se complete el juego
    WHILE (NOT key (_esc) AND NOT tablero_completado())
        IF(key(_space))         // Si se pulsa la tecla espacio
            // Vacia el tablero y lo pone como inicialmente
            FROM contador=0 TO 99; tablero[contador]=1; END
            tablero[44]=0;
        END
        FRAME;
    END
    // Si se ha salido del bucle porque el tablero esta completado
    IF (tablero_completado())
        // Elimina todos los procesos que hubiera
        let_me_alone();
        // Pone un mensaje de enhorabuena
        write(1,320,240,4,"ENHORABUENA !!!");
        // Espera has que se pulse espacio
        WHILE (NOT key (_space))
            FRAME;
        END
    END
    // Sale totalmente del juego
    exit("Gracias por jugar !!!",0);
END

//-------------------------------------------------------------------------
// Proceso dibuja_tablero
// Crea los procesos de todas las losetas del tablero
//-------------------------------------------------------------------------

PROCESS dibuja_tablero()

BEGIN
    // Mediante un bucle va creando todas los procesos de las losetas
    FROM contador=0 to 99;
        loseta(140+((contador MOD 10)*40),60+((contador/10)*40),100+tablero[contador],0,contador);
    END
END

//-------------------------------------------------------------------------
// Proceso loseta
// Maneja cada una de las losetas
// Entradas: x,y        = coordenadas
//           graph,file = grafico
//           numero     = n£mero de la loseta en la tabla
//-------------------------------------------------------------------------

PROCESS loseta(x,y,graph,file,numero)

BEGIN
    LOOP
        // Comprueba si la loseta esta colisionando con el rat¢n
        IF (collision(TYPE mouse))
            // Si colisiona y se pulsa el boton cambia las losetas
            // que se ven implicadas mediante un proceso cambia_loseta
            IF (mouse.left) cambia_loseta(numero MOD 10,numero/10); END
            // Actualiza el gr fico a la nueva posicion
            graph=tablero[numero]+100;
            // Repite hasta que se suelte el raton
            WHILE (mouse.left) FRAME; END
        END
        // Actualiza el gr fico a la nueva posicion
        graph=tablero[numero]+100;
        FRAME;
    END
END

//-------------------------------------------------------------------------
// Proceso cambia_loseta
// Realiza el cambio de la loseta indicada y la de alrededor
// Entradas: losetax, losetay = Coordenadas de la loseta en la tabla
//-------------------------------------------------------------------------

PROCESS cambia_loseta(losetax,losetay)

PRIVATE
    realx;  // Coordenadas reales de cada loseta
    realy;

BEGIN
    // Realiza dos bucles, uno horizontal y otro vertical
    FOR (contador=losetax-1;contador<losetax+2;contador++)
        FOR (contador2=losetay-1;contador2<losetay+2;contador2++)
            // Guarda las coordenadas de dicha loseta en las coordenadas reales
            realx=contador; realy=contador2;

            // Comprueba si la loseta esta fuera del tablero
            // y hace que dicha loseta sea la del otro extremo
            IF (contador<0) realx=contador+10; END
            IF (contador>9) realx=contador-10; END
            IF (contador2<0) realy=contador2+10; END
            IF (contador2>9) realy=contador2-10; END

            // Cambia la loseta de estado
            IF (tablero[realx+(realy*10)]==1)
                tablero[realx+(realy*10)]=0;
            ELSE
                tablero[realx+(realy*10)]=1;
            END
        END
    END
END

//-------------------------------------------------------------------------
// Proceso tablero_completado
// Comprueba si se ha completado todo el tablero
// Salidas: Devuelve 'Verdadero' si esta completado
//-------------------------------------------------------------------------

PROCESS tablero_completado()

BEGIN
    // Pone el contador de conseguidos a 0
    contador2=0;

    // Va mirando mediante un bucle todas las casillas e incrementa
    // el contador de hechas cada vez que encuentra una
    FROM contador=0 TO 99;
        IF (tablero[contador]==0) contador2++; END
    END

    // Si se han contado 100 hechas es que esta el tablero completado
    // sino retorna un valor 'Falso'
    IF (contador2==100) return(true);ELSE return(false); END

END