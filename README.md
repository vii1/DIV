# ![DIV GAMES STUDIO 2.0](https://github.com/vii1/DIV/blob/master/docs/img/divtitle.png)
[![FOSSA Status](https://app.fossa.io/api/projects/git%2Bhttps%3A%2F%2Fgithub.com%2Fvii1%2FDIV.svg?type=shield)](https://app.fossa.io/projects/git%2Bhttps%3A%2F%2Fgithub.com%2Fvii1%2FDIV?ref=badge_shield)
Reconstrucción y posible fork de DIV Games Studio 2.0

Repositorio original: https://github.com/DIVGAMES/DIV-Games-Studio

## Cómo compilar
* Clona el repositorio con `git clone https://github.com/vii1/DIV.git`
* Descarga e instala [OpenWatcom](http://www.openwatcom.org/) (de momento los makefiles son sólo para DOS/Windows).
* Inicia una consola con el Build Environment de Watcom.
* Desde el directorio `build-dos` del proyecto, ejecuta `wmake`
* Observa los errores que salen y ponte a arreglar cosas :)

**¡Ojo!** Para compilar algunos archivos hace falta Turbo Assembler. Puedes obtenerlo gratis para Win32 si te descargas una [trial de C++Builder](https://www.embarcadero.com/es/products/cbuilder/starter/promotional-download). ¡Requiere registrarse! :(

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
* **div_stub**: El programa que hacía de "cabecera" para los EXE de DIV2. Lo único que hacía era ejecutar DIV32RUN.DLL, pasándose a sí mismo como parámetro.
* **dll**: El SDK y los ejemplos
* **jpeglib**: Librería de [JPEG](http://ijg.org/).
* **pmwlite**: Extensor de 32 bits para DOS alternativo a DOS/4GW, que se usaba para algunos ejecutables, o todos, no sé
* **visor**: Código del generador de sprites.


## License
[![FOSSA Status](https://app.fossa.io/api/projects/git%2Bhttps%3A%2F%2Fgithub.com%2Fvii1%2FDIV.svg?type=large)](https://app.fossa.io/projects/git%2Bhttps%3A%2F%2Fgithub.com%2Fvii1%2FDIV?ref=badge_large)