# ¿Qué hay en esta carpeta?

Entre las dependencias de DIV se encuentran varias que, deduzco, salieron de un conjunto de librerías de SciTech llamado MGL, un kit pensado para el desarrollo de aplicaciones gráficas profesionales.

Buscando en Internet pude descargar la versión 4.05 (aunque probablemente Hammer Technologies usó la 3.0). Me decidí por esta versión porque parece ser que en la versión 5 quitaron cosas que requiere DIV, como el SVGAKIT. Como MGL es un kit muy grande y complejo, en esta carpeta pretendo incluir únicamente lo esencial para compilar y mejorar DIV.

La forma oficial de compilar MGL es mediante un programa llamado DMAKE, que es un "meta-make" pensado para funcionar con diversos compiladores. DMAKE lee el makefile original y lo traduce al make del compilador destino. Sin embargo no he encontrado por ninguna parte el código fuente de DMAKE, por lo que prefiero prescindir de él y traducir los makefiles necesarios al formato de Watcom.

## Acerca de SciTech Software

**SciTech Software, Inc.** era una empresa de software basada en Chico, California. Desarrollaron diversas tecnologías entre las que se encuentran **UniVESA**/**UniVBE**, **SciTech Display Doctor**, **SciTech SNAP** o **SciTech MGL**. También, casualmente, fue la empresa que negoció con SyBase la apertura del compilador WATCOM, lo que dio origen al proyecto [OpenWatcom](http://openwatcom.org). Actualmente SciTech es propiedad de [Alt Richmond Inc.](http://www.altrichmond.ca/), una empresa canadiense que si bien sigue desarrollando los proyectos de SciTech para empresas, ya no provee SDKs al público.

Su página web, _www.scitechsoft.com_, cerró en junio de 2009 pero se puede encontrar [archivada en archive.org](https://web.archive.org/web/*/http://www.scitechsoft.com).
