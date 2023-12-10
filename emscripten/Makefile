CC 		= emcc

DEFINES 	= -DSDL2 
ZIP_SUPPORT     = 1

SRCROOT = ..


CFLAGS 		= -fno-omit-frame-pointer -fdata-sections -ffunction-sections -Wno-typedef-redefinition
CFLAGS     += -std=gnu89 -Wall ${DEFINES}
CFLAGS     += -r
CFLAGS     += -Os
# -I/emsdk_portable/.data/cache/asmjs/ports-builds/sdl2/include
CFLAGS     += -static
OUT	 	= crocods.bc

include ${SRCROOT}/makefile.shared

# SDL Support
CFLAGS += `/emsdk/upstream/emscripten/system/bin/sdl2-config --cflags`
LDFLAGS = `/emsdk/upstream/emscripten/system/bin/sdl2-config --libs`

# CFLAGS += `/emsdk_portable/emscripten/tag-1.39.4/system/bin/sdl2-config --cflags`
# LDFLAGS = `/emsdk_portable/emscripten/tag-1.39.4/system/bin/sdl2-config --libs`

# remove -mwindows for printf stdout
# LDFLAGS = -L/root/sdl2/lib -lmingw32 -lSDL2main -lSDL2 -Wl,--no-undefined -Wl,--dynamicbase -Wl,--nxcompat -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lsetupapi -lversion -luuid -static-libgcc

#debug
# LDFLAGS += -O0 -g1

# Enable this to support zip files
# Here, Support for zips is enabled
#CFLAGS 	   +=-DZIP_SUPPORT -I./minizip
#LDFLAGS	   +=-lz
#THIRD_PARTY = minizip/unzip.o minizip/ioapi.o

SOURCES 	= ${SDL} ${CROCODS}
SOURCES	   += ${THIRD_PARTY}

OBJS 		= ${SOURCES:.c=.o}

all		: ${OUT} pack

# --embed-file brixen_v12.dsk@/home/web_user/disk.dsk

# em++ --pre-js download.js crocods.bc -g -o hello.html -s ALLOW_MEMORY_GROWTH=1 -s USE_SDL=2 -Os  --embed-file crocods.ini@/home/web_user/.crocods/crocods.ini


pack	:
		em++ --pre-js download.js ${OUT} -o hello.html -s ALLOW_MEMORY_GROWTH=1 -s USE_SDL=2 -Os --embed-file crocods.ini@/home/web_user/.crocods/crocods.ini
		mv hello.wasm hello.js web


${OUT}	: ${OBJS}
		${CC} -o ${OUT} ${SOURCES} ${CFLAGS} ${LDFLAGS}
	
clean	:
		rm ${OBJS} ${OUT} hello.*
