#!/bin/bash
# Instalamos los paquetes que necesitamos.
sudo apt update
sudo apt upgrade -y
sudo apt install dosbox unzip -y

# Creamos la carpeta de Watcom.
[ ! -d /usr/bin/watcom ] && sudo mkdir /usr/bin/watcom
sudo chown vagrant:vagrant /usr/bin/watcom

#---------------------------------------------------------------------------------------------------------------------------------
# Esto es para Open Watcom 1.9 
#---------------------------------------------------------------------------------------------------------------------------------
# Instalamos
wget --show-progress=off 'https://downloads.sourceforge.net/project/openwatcom/open-watcom-1.9/open-watcom-c-linux-1.9' -O 'open-watcom-1.9'
unzip -q -o 'open-watcom-1.9' -d /usr/bin/watcom
rm 'open-watcom-1.9'

#---------------------------------------------------------------------------------------------------------------------------------
# Esto es para Open Watcom 2.0
#---------------------------------------------------------------------------------------------------------------------------------
#wget 'https://github.com/open-watcom/open-watcom-v2/releases/download/Current-build/open-watcom-2_0-c-linux-x64' -O 'open-watcom-2.0'
#unzip open-watcom-2.0 -d /usr/bin/watcom

echo '#!/bin/sh
echo Open Watcom Build Environment
export PATH=/usr/bin/watcom/binl:/usr/bin/watcom/binw:$PATH
#export INCLUDE=/usr/bin/watcom/lh:$INCLUDE
export INCLUDE=/usr/bin/watcom/h:$INCLUDE
export WATCOM=/usr/bin/watcom
export EDPATH=/usr/bin/watcom/eddat
export WIPFC=/usr/bin/watcom/wipfc' > /usr/bin/watcom/owsetenv.sh

# Le damos permisos de ejecución.
# Por lo visto esto sólo está en OW2.0
#chmod +x /usr/bin/watcom/binl64/*
chmod +x /usr/bin/watcom/binl/*
chmod +x /usr/bin/watcom/owsetenv.sh

# Register Open Watcom Environment
fgrep -q owsetenv .bashrc || ( echo 'source /usr/bin/watcom/owsetenv.sh' >> .bashrc )
source /usr/bin/watcom/owsetenv.sh

# Creamos la carpeta de DOSBOX que se comenta en el makefile.
mkdir dosbox
