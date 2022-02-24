# ¿Qué hay en esta carpeta?

Entre las dependencias de DIV se encuentran varias que al principio pensé que salieron de un conjunto de librerías de SciTech llamado MGL, un enorme kit para crear aplicaciones gráficas. Ahora creo que usaron otro más pequeño, el SVGA Kit, de la misma compañía, que fue incorporado a MGL posteriormente.

DIV originalmente usaba una versión de SVGA Kit de 1995, pero la versión más antigua que pude encontrar en Internet fue la 6.0, de 1996. Parece ser que esta versión introducía cambios importantes que la hacían incompatible a nivel de código, pero con unos pocos cambios pude adaptar DIV para que funcionara con ella. SVGA Kit incluía diversos programas, utilidades y demos, pero creo que no son necesarios para desarrollar DIV así que no los he incluido en este repositorio.

La forma oficial de compilar SVGA Kit es mediante un programa llamado DMAKE, una versión mejorada de MAKE. SVGA Kit incorporaba una serie de scripts que permitían compilar las librerías con diversos compiladores. Para simplificar el proceso, he prescindido de esos scripts y directamente he incluido makefiles para Watcom adaptados al proceso de compilación de DIV, con el nombre `makefile.wat`.

**Nota:** Al igual que con las demás librerías de terceros, SVGA Kit se proporciona ya compilada en este repositorio. Para recompilarla se requiere [Turbo Assembler](https://github.com/vii1/DIV/wiki/Acerca-de-Turbo-Assembler).

## Acerca de SciTech Software

**SciTech Software, Inc.** era una empresa de software basada en Chico, California. Desarrollaron diversas tecnologías entre las que se encuentran **UniVESA**/**UniVBE**, **SciTech Display Doctor**, **SciTech SNAP** o **SciTech MGL**. También, casualmente, fue la empresa que negoció con SyBase la apertura del compilador WATCOM, lo que dio origen al proyecto [OpenWatcom](http://openwatcom.org). Actualmente SciTech es propiedad de [Alt Richmond Inc.](http://www.altrichmond.ca/), una empresa canadiense que, si bien sigue desarrollando los proyectos de SciTech para empresas, ya no provee SDKs al público.

Su página web, _www.scitechsoft.com_, cerró en junio de 2009 pero se puede encontrar [archivada en archive.org](https://web.archive.org/web/*/http://www.scitechsoft.com).
