# ![DIV GAMES STUDIO 2.0](https://github.com/vii1/DIV/blob/master/docs/img/divtitle.png)
Reconstrucción y posible fork de DIV Games Studio 2.0

[![Build Status](https://travis-ci.org/vii1/DIV.svg?branch=master)](https://travis-ci.org/vii1/DIV)

Repositorio original: https://github.com/DIVGAMES/DIV-Games-Studio

[¿Qué es esto?](#qué-es-esto)  
[¿Qué es DIV Games Studio?](#qué-es-div-games-studio)  
[Cómo compilar DIV](#cómo-compilar-div)  
[Problemas conocidos](#problemas-conocidos)  
[Descripción de archivos y carpetas](#descripción-de-archivos-y-carpetas)  

## ¿Qué es esto?
Este proyecto pretende recuperar el código fuente del DIV Games Studio 2.0 original para MS-DOS, quizá aplicarle algún parche y mejora, y limpiarlo y ordenarlo un poco para que sea más fácil su estudio y aprender de él.

El primer objetivo ([versión 2.01](https://github.com/vii1/DIV/milestone/1)) es conseguir reproducir lo más fielmente posible el DIV 2 en su versión 2.01 tal como salió a la venta en 1999. Si conoces proyectos como [Chocolate Doom](https://www.chocolate-doom.org) o [Chocolate Wolfenstein 3D](https://github.com/fabiensanglard/Chocolate-Wolfenstein-3D), este proyecto es similar (quizá debería haberlo llamado "Chocolate DIV", pero bueno).

Lo siguiente sería arreglar bugs conocidos ([versión 2.02](https://github.com/vii1/DIV/milestone/2)) y reordenar, limpiar, comentar y documentar el código para que todo el mundo pueda estudiarlo y aprender cómo funcionaba DIV2 por dentro.

Y luego… ya veremos ¯\\\_(ツ)\_/¯

DIV 2 funciona correctamente en el emulador [DOSBOX](https://www.dosbox.com/) y probablemente aún mejor en una máquina DOS real siempre que tenga al menos un procesador 486 (recomendado Pentium), 16 MB de RAM, ratón y tarjeta gráfica SVGA. De momento no tengo intención de portarlo a otras plataformas más allá de DOS, pero si un día me pongo a ello, seguramente mi primera elección sería [AMIGA](https://es.wikipedia.org/wiki/Commodore_Amiga). Si quieres DIV2 portado a plataformas modernas, te recomiendo [la versión de MikeDX](https://github.com/DIVGAMES/DIV-Games-Studio), que no sólo funciona nativamente en Windows/Mac/Linux sino que te permite compilar tus juegos para consolas, móviles e incluso HTML5.

## ¿Qué es DIV Games Studio?
En 1998, el estudio español Hammer Technologies publicó DIV Games Studio, un programa muy completo para desarrollar videojuegos. Integraba casi todas las herramientas necesarias: editor gráfico, editor de código, compilador, depurador, etc. Disponía de un entorno gráfico muy amigable y su propio lenguaje de programación, el lenguaje DIV. En 1999 salió la versión 2, que incluía muchas mejoras, tanto en las herramientas como en el lenguaje. La empresa británica FastTrak licenció DIV para su distribución en varios países de Europa y Latinoamérica.

Poco después de lanzar DIV 2 al mercado, Hammer Technologies cerró, por lo que el desarrollo de DIV nunca continuó, aunque la comunidad llegó a desarrollar diversas alternativas. El interés por DIV fue decreciendo con la aparición de nuevos sistemas operativos, tecnologías y lenguajes.

En 2015, MikeDX, antiguo miembro de FastTrak, anunció que había retomado el código original de DIV Games Studio para modernizarlo y portarlo a diversas plataformas actuales, junto con varias demos portadas a HTML5. Unos meses después, ya en 2016, publicó la primera beta de su proyecto, bautizado como DIV DX, y más tarde ese mismo año hizo público el [código fuente](https://github.com/DIVGAMES/DIV-Games-Studio) bajo una licencia [GPL v3](https://github.com/vii1/DIV/blob/master/LICENSE). Se puede seguir el progreso y descargar la última beta en su web, [div-arena.co.uk](http://div-arena.co.uk/).

[Saber más (Wikipedia)](https://es.wikipedia.org/wiki/DIV_Games_Studio)

## Cómo compilar DIV

### Requisitos previos
* Sistema operativo **Linux**, **MS-DOS** o **Windows** (cualquier versión, nueva o antigua).
* [OpenWatcom 1.9](http://www.openwatcom.org/) instalado y funcionando. Posiblemente también funcione Watcom 10 o superior, pero no lo hemos probado. OJO: necesitas instalar los compiladores para *DOS 16 bits* y *DOS 32 bits*. Actualmente hay incompatibilidades en el código con OpenWatcom 2.
* Si usas Linux, Windows NT o cualquier versión moderna de Windows, para ejecutar DIV necesitarás un emulador de DOS como [DOSBox](https://dosbox.com) o [DOSBox-X](https://dosbox-x.com/).
* **OPCIONAL**: Instala **Turbo Assembler**. Sólo es necesario si quieres recompilar las librerías de terceros (se encuentran ya compiladas en este repositorio). Consulta [la wiki](https://github.com/vii1/DIV/wiki/Acerca-de-Turbo-Assembler) para más información.

### Compilación
* Clona el repositorio con `git clone https://github.com/vii1/DIV.git`
* En DOS:
  * Asegúrate de haber inicializado el entorno de Watcom ejecutando `OWSETENV.BAT`.
* En Windows:
  * Abre una consola usando el icono **Build Environment** que encontrarás en el grupo de programas de Open Watcom en el menú de Windows.
* En Linux:
  * Asegúrate de lanzar `source owsetenv.sh` desde una consola ejecutando bash
      para inicializar el entorno.
* Desde el directorio raíz del proyecto, ejecuta `wmake`. El proceso puede durar varios minutos, especialmente en DOS. Verás muchísimos warnings. No tengas miedo.

### Instalación
* Para instalar tu DIV recién compilado, ejecuta `wmake install INSTALL_DIR=<ruta>`, donde _\<ruta\>_ es la ruta donde quieres instalar DIV 2. Si vas a usar DOSBox, puedes instalarlo directamente en una subcarpeta que vayas a montar desde el emulador.
* Para arrancar DIV, desde DOS (o DOSBox) ve a la carpeta donde lo has instalado y ejecuta `D.EXE`.

## Problemas conocidos
Aún existen algunos bugs y diferencias respecto a la versión comercial de DIV 2. Mira en la sección de [issues](https://github.com/vii1/DIV/issues) para verlos.

## Descripción de archivos y carpetas
* **3rdparty**: Librerías de terceros y su código fuente
  * **lib**: En esta subcarpeta encontrarás las librerías de terceros ya compiladas.
  * **jpeglib**: La librería [JPEG](https://ijg.org/).
  * **JUDAS**: La librería de sonido: [JUDAS Apocalyptic Softwaremixing Sound System](https://github.com/volkertb/JUDAS).
  * **SCITECH**: [SuperVGA Kit](https://web.archive.org/web/19961114153004/http://www.scitechsoft.com/devprod.html).
  * **topflc**: TopFLC, librería para reproducir archivos FLI/FLC.
  * **zlib**: Librería de compresión [zlib](https://zlib.net).
* **dll**: El SDK y los ejemplos
* **formats**: Descripción de los formatos de archivo propios de DIV, en formato [Kaitai Struct](http://kaitai.io/). Más información en la [wiki](https://github.com/vii1/DIV/wiki/Formatos-de-archivo).
* **pmwlite**: Extensor de 32 bits para DOS alternativo a DOS/4GW, que se usaba para la DIV32RUN.DLL.
* **src**: Carpeta principal de código fuente. Aquí hay algunos archivos compartidos entre módulos.
  * **div**: Código fuente del IDE (D.EXE). Algunos archivos destacables:
    * **div.cpp**: fichero principal del Sistema Operativo DIV™.
    * **global.h**: cabecera principal
    * **divc.cpp**: el compilador de lenguaje DIV. Con unas pocas modificaciones se puede convertir en un EXE independiente ;)
    * **diveffec.cpp**: el generador de explosiones :D
    * **divpaint.cpp**: ¡el programa de dibujo! :D :D
    * **divwindo.cpp**: todas las funciones de la GUI
    * **visor**: Código del generador de sprites.
  * **div32run**: Código fuente del intérprete (DIV32RUN.DLL). Algunos archivos destacables:
    * **inter.h**: cabecera principal
    * **i.cpp**: código principal del intérprete
    * **kernel.cpp**: se incluye dentro de i.cpp y es el cuerpo de un gigantesco `switch`, con un `case` por cada bytecode, que conforma la máquina virtual de DIV.
    * **f.cpp**: muchas de las funciones accesibles desde DIV.
    * **s.cpp**: gráficos, sprites, scroll, modo-7...
    * **v.cpp**: funciones de vídeo de bajo nivel
    * **d.cpp**: el debugger de DIV2, incluyendo toda la GUI (!)
  * **div_stub**: El programa que hacía de "cabecera" para los EXE de DIV2. A éste se le concatenaba el bytecode del programa compilado y algunos datos para el intérprete. Lo único que hacía el EXE en sí era ejecutar DIV32RUN.DLL, pasándose a sí mismo como parámetro.
  * **netlib**: Rutinas de red (originalmente era una DLL)
  * **vpe**: Librería de Modo-8 (una versión modificada de [Virtual Presence Engine](http://www.ii.uib.no/~alexey/vpe/index.html))
