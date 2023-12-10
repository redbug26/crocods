#!/bin/bash

echo 
echo "# Run these commands to make the executabe"
echo "cd /src/bittboy"
echo "make"

docker run --rm -v ${PWD}/..:/src/ -it benbeshara/buildroot-bittboy:0.1
