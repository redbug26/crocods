== build the docker (from this folder)
docker build -t rpi -f rpi.Dockerfile .

== from your local path
docker run --rm -v ${PWD}:/src/ -it rpi
