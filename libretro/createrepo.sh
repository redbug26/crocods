#!/bin/bash

# Run to create the repository in /Documents
# Run to backup the modification 

DEST=${HOME}/Downloads/Libretro

if [ -d "$DEST" ]; then
	cp $DEST/libretro.c .
	cp $DEST/libretro.h .
	rm -rf $DEST
fi

mkdir $DEST
git clone git@github.com:libretro/libretro-crocods.git $DEST
rm -rf $DEST/crocods-core 
mkdir $DEST/crocods-core 
mkdir $DEST/crocods-core/iniparser
cp -R ../crocods-core/*.c $DEST/crocods-core
cp -R ../crocods-core/*.h $DEST/crocods-core
cp -R ../crocods-core/iniparser/*.c $DEST/crocods-core/iniparser
cp -R ../crocods-core/iniparser/*.h $DEST/crocods-core/iniparser

cp libretro.c $DEST
cp libretro.h $DEST

cd $DEST
