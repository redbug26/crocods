#!/bin/bash

VERSION="4b3"

docker run --rm -v "${PWD}:/src/" -it mingw make -C /src/build OS=windows dist
# docker run --rm -v ${PWD}:/src/ -it solarus/mingw-build-env make -C /src/mingw/cli PREFIX=i686-w64-mingw32- dist 

make -C build OS=macosx-arm dist

cd dist
rm crocods.zip

zip crocods_x64_${VERSION} crocods.exe README.md
zip crocods_macm1_${VERSION} crocods_macosx README.md

