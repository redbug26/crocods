#!/bin/bash

echo 
echo "# Run these commands to make the executabe"
echo "cd /src/rpi"
echo "make"

docker run --rm -v ${PWD}/..:/src/ -it rpi
