//--------------------------------------------------------------------
// Titulo: Tutorial 5
// Autor:  Daniel Navarro Medrano
// Fecha:  26/01/97
//--------------------------------------------------------------------

PROGRAM Tutorial_5;

GLOBAL
    contador_cuerpo=0;  // Cuenta el n£mero de cuerpos

BEGIN
    // Carga el fichero de gr ficos del juego
    load_fpg("tutorial\tutor5.fpg");
    // imprime el texto necesario y borra la pantalla
    write(0,160,192,1,"Utilice el rat¢n para mover la serpiente.");
    fade_on();

    // Pone la resolucion de las coordenadas con dos decimales
    resolution=100;
    // Selecciona el gr fico de la cabeza
    graph=1;
    // Inicia el scroll de fondo
    start_scroll(0,0,4,5,0,0);
    // Crea el segmento de cuerpo inicial
    segmento();
    LOOP
        // Hace que la cabeza siga al cursor del raton
        x=mouse.x*100;
        y=mouse.y*100;
        // Mueve los dos planos de scroll
        scroll.x0=scroll.x0+1; scroll.y0=scroll.y0+1;
        scroll.x1=scroll.x1-1; scroll.y1=scroll.y1-1;
        FRAME;  // Hace que todo aparezca en pantalla
    END
END

PROCESS segmento()

PRIVATE
    x0,y0;  // Coordenadas temporales para hacer calculos

BEGIN
    // Hace que las coordenadas usen dos decimales
    resolution=100;
    // Incrementa el contador de segmentos del cuerpo
    contador_cuerpo=contador_cuerpo+1;
    // Pone la profundidad dependiento del numero de cuerpo
    z=-contador_cuerpo;

    // Si es el final de la cola, lo hace mas peque¤o
    IF (contador_cuerpo>156) size=256-contador_cuerpo; END

    // Va creando otros segmentos del cuerpo hasta 256
    IF (contador_cuerpo<256) segmento(); END

    // Selecciona el grafico, pero con transparencia
    graph=2; flags=4;
    LOOP
          // Calculas las coordenadas haciendo una media
          // entre la posicion actual y la posicion del
          // cuerpo que lo llamo, que es el que esta delante
          x=x0; x0=father.x; x=(x+x0*3)/4; x0=x;
          y=y0; y0=father.y; y=(y+y0*3)/4; y0=y;
        FRAME; // Pone el gr fico en pantalla
    END
END