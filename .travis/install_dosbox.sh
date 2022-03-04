#!/bin/bash
if [ ! -f $DOSBOX ]; then
    echo Installing Dosbox-X...
    case $TRAVIS_OS_NAME in
        linux)
            sudo apt-get -y update || exit 1
            sudo apt-get -y install build-essential automake autoconf libsdl2-dev libsdl2-net-dev || exit 1
            wget https://github.com/joncampbell123/dosbox-x/archive/refs/tags/dosbox-x-v0.83.23.tar.gz || exit 1
            tar -xzf dosbox-x-v0.83.23.tar.gz || exit 1
            cd dosbox-x-dosbox-x-v0.83.23 || exit 1
            ./autogen.sh || exit 1
            chmod +x vs/sdl/build-scripts/strip_fPIC.sh
            chmod +x configure
            ./configure --enable-core-inline --disable-debug --disable-x11 --disable-sdl2test \
                --disable-sdltest --disable-alsatest --disable-freetype --disable-printer --disable-mt32 \
                --disable-screenshots --disable-libslirp --disable-libfluidsynth --disable-opengl --disable-sdl \
                --enable-sdl2 --prefix=$HOME/dosbox-x || exit 1
            make -j`nproc` || exit 1
            make install || exit 1
            ;;
        windows)
            wget https://github.com/joncampbell123/dosbox-x/releases/download/dosbox-x-v0.83.23/dosbox-x-vsbuild-win64-20220228210811.zip -O dosbox-x.zip
            unzip dosbox-x.zip -d dosbox_tmp
            mv dosbox_tmp/bin/x64/Release\ SDL2 ./dosbox-x
            rm -fR dosbox_tmp
            ;;
    esac
else
    echo Dosbox-X already installed!
fi