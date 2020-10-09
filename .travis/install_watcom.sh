#!/bin/bash
case $TRAVIS_OS_NAME in
    linux)
        wget https://nav.dl.sourceforge.net/project/openwatcom/open-watcom-1.9/open-watcom-c-linux-1.9 -O owatcom.zip || wget ftp://ftp.openwatcom.org/install/open-watcom-c-linux-1.9 -O owatcom.zip
        mkdir ~/watcom
        unzip owatcom.zip -d ~/watcom/
        pushd ~/watcom/binl
        chmod a+x ctags dmpobj edbind exe2bin fcenable ide2make ms2wlink owcc parserv par.trp std.trp tcpserv tcp.trp vi wasaxp wasm wasppc wbind wcc wcc386 wcl wcl386 wd wdis wdump whelp wlib wlink wmake wpp wpp386 wrc wstrip wtouch
        popd
        wcl386 -v
        ;;
    windows)
        wget https://nav.dl.sourceforge.net/project/openwatcom/open-watcom-1.9/open-watcom-c-win32-1.9.exe -O owatcom.zip || wget ftp://ftp.openwatcom.org/install/open-watcom-c-win32-1.9.exe -O owatcom.zip
        mkdir ~/watcom
        unzip owatcom.zip -d ~/watcom/
        wcl386 -v
        ;;
esac
