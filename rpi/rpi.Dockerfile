# To create the image:
#   $ docker build -t rpi -f rpi.Dockerfile .
# To run the container:
#   $ docker run -v ${PWD}:/src/ -it rpi

FROM ubuntu:latest

LABEL Version="1.0" \
      Date="2019-Dec-24" \
      Docker_Version="19.12.24 (1)" \
      Maintainer="RedBug (@kyuran)" \
      Description="A basic Docker container to cross-compile win32 exe (with SDL)"

ARG DEBIAN_FRONTEND=noninteractive

ENV TERM="xterm"
ENV TZ=Europe/Paris

RUN apt-get update \
    && apt-get install -y git ca-certificates wget make patch gcc bzip2 unzip g++ texinfo bison flex libboost-dev libsdl1.2-dev pkgconf libfreetype6-dev libncurses-dev cmake vim gcc-multilib g++-multilib 

RUN apt-get install -y gcc-arm-linux-gnueabi

#gcc-arm-linux-gnueabi

RUN   cd /root \
     && wget https://www.libsdl.org/release/SDL-1.2.15.zip \
     && unzip SDL-1.2.15.zip \
     && cd SDL-1.2.15  \
     && ./configure --build=arm-linux-gnueabihf --host=arm-linux-gnueabi --disable-shared --disable-video-opengl --disable-esd --disable-video-mir --disable-video-wayland --disable-stdio-redirect --disable-pulseaudio --disable-video-x11 --prefix=$HOME/sdl1 \
     && make \
     && make install

RUN  apt-get install libdrm-dev libgbm-dev \
     cd /root \
     && wget https://www.libsdl.org/release/SDL2-2.0.10.zip \
     && unzip SDL2-2.0.10.zip \
     && cd SDL2-2.0.10 \
     &&  ./configure --build=arm-linux-gnueabihf --host=arm-linux-gnueabi --disable-shared --disable-video-opengles1 --disable-esd --disable-video-mir --disable-video-wayland --disable-stdio-redirect --disable-pulseaudio --disable-video-x11 --prefix=$HOME/sdl2 \
     && make \
     && make install

ENV PATH="/src/bin:${PATH}"

WORKDIR /src/
