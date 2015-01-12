#!/bin/sh
expr `cat yappari-build` + 1 >yappari-build
echo \#define BUILD_NUMBER \"`cat yappari-build`\" >src/yappari-application/version.h
echo \#define VERSION \"`head -1 src/qtc_packaging/debian_fremantle/changelog.original | sed "s/.*(\(.*\)).*/\1/"`\" >>src/yappari-application/version.h
echo \#define FULL_VERSION \"`head -1 src/qtc_packaging/debian_fremantle/changelog.original | sed "s/.*(\(.*\)).*/\1/"` Build `cat yappari-build`\" >>src/yappari-application/version.h
clone src build-scratchbox
echo DEFINES += Q_WS_SCRATCHBOX=1 >>build-scratchbox/yappari-application/yappari-application.pro
cd build-scratchbox
qmake yappari.pro
make
cd ..
cp src/yappari-application/share/whatsnew.txt /usr/share/yappari
if [ -z $1 ]; then
    run-standalone.sh build-scratchbox/yappari-application/Yappari
fi
