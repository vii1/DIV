# ![DIV GAMES STUDIO 2.0](https://github.com/vii1/DIV/blob/master/docs/img/divtitle.png)
Reconstrucción y posible fork de DIV Games Studio 2.0

Repositorio original: https://github.com/DIVGAMES/DIV-Games-Studio

[¿Qué es esto?](#qué-es-esto)  
[¿Qué es DIV Games Studio?](#qué-es-div-games-studio)  
[Cómo compilar](#cómo-compilar)  
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

## Cómo compilar
* Clona el repositorio con `git clone https://github.com/vii1/DIV.git`
* Descarga e instala [OpenWatcom](http://www.openwatcom.org/) (de momento los makefiles son sólo para la versión DOS/Windows, aunque en teoría Watcom permite la compilación cruzada desde otros sistemas operativos. Si lo intentas, házmelo saber).
* Inicia una consola con el Build Environment de Watcom (lo encontrarás en la carpeta Open Watcom del menú de Windows).
* Desde el directorio raíz del proyecto, ejecuta `wmake`. De momento sólo se compila el IDE (ficheros `D.EXE` y `D.386`), aún estoy en el proceso de escribir los makefiles para crear el resto de archivos esenciales (`DIV32RUN.DLL`, `SESSION.*`, etc).

Para ejecutar tu DIV recién compilado, necesitarás crear el árbol de directorios y ficheros necesario. Puedes basarte en una instalación de DIV 2 que ya tengas (haz copias de seguridad), o bien puedes dejar que `wmake` haga el trabajo por ti:
* Desde el directorio raíz del proyecto, ejecuta `wmake INSTALL_DIR=<ruta> install`, donde _\<ruta\>_ es la ruta donde quieres instalar DIV 2. Puede ser un directorio ya existente o no, por ejemplo `C:\DIV2`. Te recomiendo que lo instales directamente en una carpeta que posteriormente puedas montar desde **DOSBox**.
* Arranca [DOSBox](http://www.dosbox.com/) y entra en la carpeta donde acabas de instalar DIV 2 (usa el comando `MOUNT` para mapear una letra de unidad a su carpeta superior).
* Ejecuta `D.EXE`.

## Problemas conocidos
Unos cuantos. Mira en la sección de [issues](https://github.com/vii1/DIV/issues) para verlos.

## Descripción de archivos y carpetas
* En el directorio raíz está el código fuente del IDE (D.EXE). Archivos destacables:
  * **div.cpp**: fichero principal del Sistema Operativo DIV™.
  * **global.h**: cabecera principal
  * **divc.cpp**: el compilador de lenguaje DIV. Con unas pocas modificaciones se puede convertir en un EXE independiente ;)
  * **diveffec.cpp**: el generador de explosiones :D
  * **divpaint.cpp**: ¡el programa de dibujo! :D :D
  * **divwindo.cpp**: todas las funciones de la GUI
* **SOURCE**: Código fuente del intérprete (DIV32RUN.DLL). Archivos destacables:
  * **inter.h**: cabecera principal
  * **i.cpp**: código principal del intérprete
  * **kernel.cpp**: se incluye dentro de i.cpp y es el cuerpo de un gigantesco `switch`, con un `case` por cada bytecode, que conforma la máquina virtual de DIV.
  * **f.cpp**: muchas de las funciones accesibles desde DIV.
  * **s.cpp**: gráficos, sprites, scroll, modo-7...
  * **v.cpp**: funciones de vídeo de bajo nivel
  * **d.cpp**: el debugger de DIV2, incluyendo toda la GUI (!)
* **INC**: Cabeceras varias de terceras partes
* **JUDAS**: Librería de sonido
* **NETLIB**: Rutinas de red (originalmente era una DLL)
* **VPE**: Librería de Modo-8 (una versión modificada de [Virtual Presence Engine](http://www.ii.uib.no/~alexey/vpe/index.html))
* **div_stub**: El programa que hacía de "cabecera" para los EXE de DIV2. A éste se le concatenaba el bytecode del programa compilado y algunos datos para el intérprete. Lo único que hacía el EXE en sí era ejecutar DIV32RUN.DLL, pasándose a sí mismo como parámetro.
* **dll**: El SDK y los ejemplos
* **jpeglib**: Librería de [JPEG](http://ijg.org/).
* **pmwlite**: Extensor de 32 bits para DOS alternativo a DOS/4GW, que se usaba para algunos ejecutables, o todos, no sé
* **visor**: Código del generador de sprites.
