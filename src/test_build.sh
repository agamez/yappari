#!/bin/sh
OLD_BUILD_NUMBER=$(sed -rn 's/#define BUILD_NUMBER ([0-9]+)/\1/gp' yappari-application/version.h)
NEW_BUILD_NUMBER=$(expr $OLD_BUILD_NUMBER + 1)
VERSION=$(head -1 ../debian/changelog | sed 's/.*(\(.*\)).*/\1/')

cat > yappari-application/version.h << __EOF__
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define BUILD_NUMBER ${NEW_BUILD_NUMBER}
#define VERSION "${VERSION}"
#define FULL_VERSION VERSION "${VERSION_NUMBER} Build " STR(BUILD_NUMBER)
__EOF__

mkdir build-scratchbox
tar cpf - . --exclude=build-scratchbox | tar xpf - -C build-scratchbox

if [ -z $1 ]; then
	echo DEFINES += Q_WS_SCRATCHBOX=1 >> build-scratchbox/yappari-application/yappari-application.pro
fi
cd build-scratchbox

qmake yappari.pro && make
RET=$?

cd ..

if [ $RET -ne 0 ]; then
	exit 1
fi

cp yappari-application/share/whatsnew.txt /usr/share/yappari
if [ -z $1 ]; then
    run-standalone.sh build-scratchbox/yappari-application/Yappari
fi
