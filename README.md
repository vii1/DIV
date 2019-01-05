# ![DIV GAMES STUDIO 2.0](https://github.com/vii1/DIV/blob/master/docs/img/divtitle.png)
Reconstrucción y posible fork de DIV Games Studio 2.0

Repositorio original: https://github.com/DIVGAMES/DIV-Games-Studio

[¿Qué es esto?](#qué-es-esto)  
[¿Qué es DIV Games Studio?](#qué-es-div-games-studio)  
[Cómo compilar](#cómo-compilar)  
[Problemas conocidos](#problemas-conocidos)  
[Descripción de archivos y carpetas](#descripción-de-archivos-y-carpetas)

## ¿Qué es esto?
Este proyecto pretende recuperar el código fuente del DIV Games Studio 2.0 original para MS-DOS, y quizá aplicarle algún parche y mejora, y limpiarlo y ordenarlo un poco para que sea más fácil su estudio y aprender de él. Y luego… ya veremos ¯\\\_(ツ)\_/¯

## ¿Qué es DIV Games Studio?
En 1998, el estudio español Hammer Technologies publicó DIV Games Studio, un programa muy completo para desarrollar videojuegos. Integraba casi todas las herramientas necesarias: editor gráfico, editor de código, compilador, depurador, etc. Disponía de un entorno gráfico muy amigable y su propio lenguaje de programación, el lenguaje DIV. En 1999 salió la versión 2, que incluía muchas mejoras, tanto en las herramientas como en el lenguaje. La empresa británica FastTrak licenció DIV para su distribución en varios países de Europa y Latinoamérica.

Poco después de lanzar DIV 2 al mercado, Hammer Technologies cerró, por lo que el desarrollo de DIV nunca continuó, aunque la comunidad llegó a desarrollar diversas alternativas. El interés por DIV fue decreciendo con la aparición de nuevos sistemas operativos, tecnologías y lenguajes.

En 2015, MikeDX, antiguo miembro de FastTrak, anunció que había retomado el código original de DIV Games Studio para modernizarlo y portarlo a diversas plataformas actuales, junto con varias demos portadas a HTML5. Unos meses después, ya en 2016, publicó la primera beta de su proyecto, bautizado como DIV DX, y más tarde ese mismo año hizo público el [código fuente](https://github.com/DIVGAMES/DIV-Games-Studio) bajo una licencia [GPL v3](https://github.com/vii1/DIV/blob/master/LICENSE). Se puede seguir el progreso y descargar la última beta en su web, [div-arena.co.uk](http://div-arena.co.uk/).

[Saber más (Wikipedia)](https://es.wikipedia.org/wiki/DIV_Games_Studio)

## Cómo compilar
* Clona el repositorio con `git clone https://github.com/vii1/DIV.git`
* Descarga e instala [OpenWatcom](http://www.openwatcom.org/) (de momento los makefiles son sólo para la versión DOS/Windows, aunque en teoría Watcom permite la compilación cruzada desde otros sistemas operativos. Si lo intentas, házmelo saber).
* Instala Turbo Assembler. Puedes obtenerlo gratis para Win32 si te descargas una [trial de C++Builder](https://www.embarcadero.com/es/products/cbuilder/starter/promotional-download). Por desgracia, requiere registrarse :( En próximas versiones incluiré los OBJ ya compilados para mayor comodidad. TASM32.EXE debe estar en tu PATH cuando compiles.
* Inicia una consola con el Build Environment de Watcom.
* Desde el directorio `build-dos` del proyecto, ejecuta `wmake`
* Copia el fichero `d.exe` generado al directorio raíz del proyecto (¡o a una instalación de DIV 2 ya existente!).
* Copia `dos4gw.exe` al mismo directorio si no lo tienes ya. Puedes encontrarlo en el directorio `binw` del Watcom.
* Ejecuta `d.exe` desde un [emulador de DOS](http://www.dosbox.com/) y… voilà :)

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
