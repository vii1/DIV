// --------------------------------------------------
// Visualizador de mapas 3D - DIV Games Studio 2
//
// Instrucciones:
//
// 1 - Crear un mapa 3D en un archivo WLD con su FPG
//     con las texturas asociadas al mismo.
// 2 - Modificar los nombres de archivo WLD_VIEW.WLD
//     y WLD_VIEW.FPG de la secci¢n CONST.
// --------------------------------------------------

PROGRAM WLD_VIEW;

CONST
    ARCHIVO_WLD = "HELP\WLD_VIEW.WLD";
    ARCHIVO_FPG = "HELP\WLD_VIEW.FPG";

BEGIN

  set_mode(m640x480);
  set_fps(70,0);

  write_int(0,0,0,0,&x);
  write_int(0,0,8,0,&y);

  load_fpg(ARCHIVO_FPG);
  load_wld(ARCHIVO_WLD,0);

  start_mode8(id,0,0); // Define la regi¢n de modo 8

  ctype=c_m8;          // Es un proceso de modo 8
  height=64;           // Altura del personaje
  radius=64;           // Radio del personaje (ancho)
  m8.height=32;        // Altura de los ojos

  go_to_flag(0);       // Coordenadas de inicio

  loop

    if (key(_alt))
      // `Strafe' con ALT
      if (key(_right)) xadvance(angle-90000,16); end
      if (key(_left))  xadvance(angle+90000,16); end
    else
      // Giro derecha/izquierda
      if (key(_right)) angle-=5000; end
      if (key(_left))  angle+=5000; end
    end

    // Avance y retroceso
    if (key(_up))   advance(32); end
    if (key(_down)) advance(-16); end

    // Subir y bajar con Q/A
    if (key(_q)) z+=16; end
    if (key(_a)) z-=16; end

    // Mirar arriba y abajo con W/S
    if (key(_w)) m8.angle+=8; end
    if (key(_s)) m8.angle-=8; end

    frame; // Mostrar la siguiente imagen

  end
end

