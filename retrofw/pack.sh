#!/bin/bash

find . -name .DS_Store -exec rm {} \;

mkdir -p ipkg/home/retrofw/emus/crocods
mv crocods.dge ipkg/home/retrofw/emus/crocods
cp -R dingux/* ipkg/home/retrofw/emus/crocods/

cd ipkg
tar -czvf control.tar.gz control
tar -czvf data.tar.gz home
ar rv ../release/crocods.ipk control.tar.gz data.tar.gz debian-binary
