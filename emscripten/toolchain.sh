#!/bin/bash

echo 
echo "# Run these commands to make the executabe"
echo "cd /src/emscripten"
echo "make"

#docker run --rm -v ${PWD}/..:/src/ -it trzeci/emscripten-ubuntu bash
docker run --rm -v ${PWD}/..:/src/ -it emscripten/emsdk bash
