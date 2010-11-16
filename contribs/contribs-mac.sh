#!/bin/sh

VLC_VERSION_DATE="20101110-0104"
VLC_VERSION_PREFIX="1.1-branch-intel-${VLC_VERSION_DATE}"
VLC_FILE="${VLC_VERSION_PREFIX}.zip"
VLC_URL="http://nightlies.videolan.org/build/macosx-intel/${VLC_FILE}"
FREI0R_FILE="frei0r-latest.tar.gz"
FREI0R_URL="http://www.piksel.no/frei0r/snapshot/frei0r-latest.tar.gz"

ROOT_FOLDER=`pwd`

# Get the dependencies in this directory
mkdir -p src-dl/
cd src-dl/

if [ ! -f $VLC_FILE ]; then
    wget $VLC_URL ;
else
    echo "VLC OK";
fi

if [ ! -f $FREI0R_FILE ]; then
    wget $FREI0R_URL ;
else
    echo "FREI0R OK";
fi

cd $ROOT_FOLDER
mkdir temp

# libvlc
unzip src-dl/$VLC_FILE "vlc*/VLC.app/Contents/MacOS/*" -d temp

cd temp/vlc*/VLC.app/Contents/MacOS/
mv -fv include/ $ROOT_FOLDER
mv -fv lib/ $ROOT_FOLDER
mv -fv plugins/ $ROOT_FOLDER

cd $ROOT_FOLDER

# frei0r
tar xvf src-dl/$FREI0R_FILE -C temp --strip-components=2
cp temp/frei0r.h include
rm -rf temp