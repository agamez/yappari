#!/bin/sh
OLD_BUILD_NUMBER=$(sed -rn 's/#define BUILD_NUMBER "([0-9]+)"/\1/gp' yappari-application/version.h)
NEW_BUILD_NUMBER=$(expr $OLD_BUILD_NUMBER + 1)
echo \#define BUILD_NUMBER \"$NEW_BUILD_NUMBER\" > yappari-application/version.h
echo \#define VERSION \"$(head -1 ../debian/changelog | sed "s/.*(\(.*\)).*/\1/")\" >> yappari-application/version.h
echo \#define FULL_VERSION \"$(head -1 ../debian/changelog | sed "s/.*(\(.*\)).*/\1/") Build \" BUILD_NUMBER >> yappari-application/version.h

mkdir build-scratchbox
tar cpf - . --exclude=build-scratchbox | tar xpf - -C build-scratchbox

echo DEFINES += Q_WS_SCRATCHBOX=1 >> build-scratchbox/yappari-application/yappari-application.pro
cd build-scratchbox
qmake yappari.pro
make
cd ..
cp yappari-application/share/whatsnew.txt /usr/share/yappari
if [ -z $1 ]; then
    run-standalone.sh build-scratchbox/yappari-application/Yappari
fi
