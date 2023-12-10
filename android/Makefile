CC = /opt/android-sdk-linux/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64/bin/clang

SYSROOT     := $(CHAINPREFIX)/arm-miyoo-linux-musleabi/sysroot
SDL_CFLAGS  := $(shell $(SYSROOT)/usr/bin/sdl-config --cflags)
SDL_LIBS    := $(shell $(SYSROOT)/usr/bin/sdl-config --libs)


DEFINES 	= -DGCW -DSDL1
DEFINES	       += -DREDBUG
ZIP_SUPPORT     = 1


SRCROOT = ..

CFLAGS 		= -O2 -ffast-math -fomit-frame-pointer -fdata-sections -ffunction-sections -std=gnu89 -Wall ${DEFINES} 

#CFLAGS	= -O2 -ffast-math -fomit-frame-pointer -fdata-sections -ffunction-sections -mips32r2 -std=gnu89 -Wall ${DEFINES} 
#CFLAGS	   += -mno-relax-pic-calls -mno-abicalls -mlong-calls -mlong32 -mlocal-sdata -mno-check-zero-division -mfp32 
#CFLAGS     += -mgp32 -mno-embedded-data -fno-pic -mno-interlink-compressed -mno-mt -mno-micromips -mno-interlink-mips16
CFLAGS         += -fno-PIC -march=armv5te -mtune=arm926ej-s $(SDL_CFLAGS)

LDFLAGS 	= $(SDL_LIBS) -Wl,--as-needed -Wl,--gc-sections -flto -lSDL -s
OUT	 	= crocods

include ${SRCROOT}/makefile.shared

SOURCES 	= ${SDL} ${CROCODS}
SOURCES	   += ${THIRD_PARTY}

OBJS 		= ${SOURCES:.c=.o}

all		: ${OUT} pack

pack	:
	apt-get install zip
		/buildroot/output/host/bin/arm-miyoo-linux-musleabi-strip crocods
		mv crocods ./archive/bittboy-crocods/bittboy-crocods
		cd archive ; zip -r ../bittboy-crocods.zip *

${OUT}	: ${OBJS}
		${CC} -o ${OUT} ${SOURCES} ${CFLAGS} ${LDFLAGS}
	
clean	:
		rm ${OBJS}
		rm ./opk/${OUT}
		rm ./opk/${OUT}.opk
