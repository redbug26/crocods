CROCOVERSION="\"4.0 beta 3\""

#-Wl,-dead_strip
# -arch arm64

ifeq ($(OS), linux)
CFLAGS 		=-Wno-format-overflow -Wno-unused-result -Wno-restrict
LDFLAGS     =-lstdc++ -pthread -lm
LDFLAGS_LUA =-DLUA_USE_LINUX
DEFINES 	=-DTARGET_OS_LINUX
EXT         =_linux
endif

ifeq ($(OS), macosx-arm)
CFLAGS 		=-Wno-typedef-redefinition -fno-unique-section-names -flto
LDFLAGS     =-Wl,-dead_strip -lstdc++ -Wl,-map,crocods.map
LDFLAGS_GUI =
LDFLAGS_LUA =-DLUA_USE_MACOSX
DEFINES 	=-DTARGET_OS_MAC
EXT         =_macosx
endif

ifeq ($(OS), windows)
CFLAGS 		=-Wno-format-overflow
LDFLAGS     =-lstdc++ -mthreads -static
LDFLAGS_GUI =-mwindows
LDFLAGS_LUA =-DLUA_USE_WINDOWS
DEFINES 	=-DTARGET_OS_MAC
PREFIX      =i686-w64-mingw32-
EXT         =.exe
endif

ifeq ($(OS), windowsdebug)
CFLAGS 		=-Wno-format-overflow
LDFLAGS     =-lstdc++ -mthreads -static
# LDFLAGS_GUI =-mwindows
LDFLAGS_LUA =-DLUA_USE_WINDOWS
DEFINES 	=-DTARGET_OS_MAC
PREFIX      =i686-w64-mingw32-
EXT         =_debug.exe
endif

CC 		    =$(PREFIX)gcc
STRIP       =$(PREFIX)strip
CPP 	    =$(PREFIX)g++
AR  	    =$(PREFIX)ar

SRCROOT = ..

CFLAGS 	   += -Wall ${DEFINES} -DCROCOVERSION=${CROCOVERSION} -g -Os -Wfatal-errors


EMUL		= crocods$(EXT)
TOOLS       = crocodsCLI$(EXT)

include ${SRCROOT}/makefile.shared

# SDL Support
SDL_CFLAGS  = `sdl2-config --cflags`
SDL_LDFLAGS = `sdl2-config --static-libs`

ifeq ($(OS), windows)
# SDL_CFLAGS = `sdl2-config --cflags`
SDL_CFLAGS = -I/root/sdl2/include/SDL2 -Dmain=SDL_main

# SDL_LDFLAGS = `sdl2-config --static-libs`
# remove -mwindows for printf stdout
SDL_LDFLAGS = -L/root/sdl2/lib -lmingw32 -lSDL2main -lSDL2 -Wl,--no-undefined -Wl,--dynamicbase -Wl,--nxcompat -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lsetupapi -lversion -luuid -static-libgcc
endif

ifeq ($(OS), windowsdebug)
# SDL_CFLAGS = `sdl2-config --cflags`
SDL_CFLAGS = -I/root/sdl2/include/SDL2 -Dmain=SDL_main

# SDL_LDFLAGS = `sdl2-config --static-libs`
# remove -mwindows for printf stdout
SDL_LDFLAGS = -L/root/sdl2/lib -lmingw32 -lSDL2main -lSDL2 -Wl,--no-undefined -Wl,--dynamicbase -Wl,--nxcompat -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lsetupapi -lversion -luuid -static-libgcc
endif


ifeq ($(OS), linux)
SDL_LDFLAGS = -lSDL2
endif

all		: ${EMUL} ${TOOLS} 

EMULSOURCES = ${SDLMAIN} ${CROCODS} obj/lua.a obj/rasm.a obj/hxcmod.a obj/libstsound.a obj/lodepng.a 
TOOLSSOURCES = ${TOOLSMAIN} ${CROCODS} obj/lua.a obj/rasm.a obj/hxcmod.a obj/libstsound.a obj/lodepng.a 

${EMUL}	: ${EMULSOURCES}
	@echo "Making ${EMUL}"
	@echo "Cmd ${CC} -o ${EMUL} ${EMULSOURCES} ${CFLAGS} ${LDFLAGS} ${LDFLAGS_GUI} ${SDL_CFLAGS} ${SDL_LDFLAGS}"
	@${CC} -o ${EMUL} ${EMULSOURCES} ${CFLAGS} ${LDFLAGS} ${LDFLAGS_GUI} ${SDL_CFLAGS} ${SDL_LDFLAGS} 

${TOOLS} : ${TOOLSSOURCES}
	@echo "Making ${TOOLS}"
	@${CC} -o ${TOOLS} ${TOOLSSOURCES} ${CFLAGS} ${LDFLAGS} -DCLI

# libstsound

st_sources := $(wildcard ${SRCROOT}/crocods-core/3rdparty/libstsound/*.cpp)
st_objects := $(addprefix obj/libstsound/,$(patsubst %.cpp,%.o,$(notdir $(st_sources)))) 

${st_objects} : obj/libstsound/%.o : ${SRCROOT}/crocods-core/3rdparty/libstsound/%.cpp
# $(info Compiling $@ with $<)
	@mkdir -p $(dir $@)
	@${CPP} ${CFLAGS} -Wno-typedef-redefinition -c -w -o  $@ $<

obj/libstsound.a : ${st_objects}
# 	$(info Compiling $@ with $?)
	@${AR} rcs $@ $^

# png

# -Xanalyzer -analyzer-disable-all-checks

png_sources := $(wildcard ${SRCROOT}/crocods-core/3rdparty/lodepng/*.c)
png_objects := $(addprefix obj/lodepng/,$(patsubst %.c,%.o,$(notdir $(png_sources))))

${png_objects} : obj/lodepng/%.o : ${SRCROOT}/crocods-core/3rdparty/lodepng/%.c
# $(info Compiling $@ with $<)
	@mkdir -p $(dir $@)
	@${CC} ${CFLAGS} -Wno-typedef-redefinition -c -w -o  $@ $<

obj/lodepng.a : ${png_objects}
# 	$(info Compiling $@ with $?)
	@${AR} rcs $@ $^

# hxc

hxc_sources := $(wildcard ${SRCROOT}/crocods-core/3rdparty/hxcmod/*.c)
hxc_objects := $(addprefix obj/hxcmod/,$(patsubst %.c,%.o,$(notdir $(hxc_sources))))

${hxc_objects} : obj/hxcmod/%.o : ${SRCROOT}/crocods-core/3rdparty/hxcmod/%.c
# $(info Compiling $@ with $<)
	@mkdir -p $(dir $@)
	@${CC} ${CFLAGS} -Wno-typedef-redefinition -c -w -o  $@ $<

obj/hxcmod.a : ${hxc_objects}
# 	$(info Compiling $@ with $?)
	@${AR} rcs $@ $^

# lua

lua_sources := $(wildcard ${SRCROOT}/crocods-core/3rdparty/lua/*.c)
lua_objects := $(addprefix obj/lua/,$(patsubst %.c,%.o,$(notdir $(lua_sources))))

${lua_objects} : obj/lua/%.o : ${SRCROOT}/crocods-core/3rdparty/lua/%.c
# $(info Compiling $@ with $<)
	@mkdir -p $(dir $@)
	@${CC} ${CFLAGS} ${LDFLAGS_LUA} -Wno-typedef-redefinition -c -w -o  $@ $<

obj/lua.a : ${lua_objects}
# 	$(info Compiling $@ with $?)
	@${AR} rcs $@ $^


# rasm

obj/rasm.a : obj/rasm/rasm.o
	@${AR} rcs $@ $^

obj/rasm/rasm.o : ${SRCROOT}/crocods-core/3rdparty/rasm/rasm.c
	@mkdir -p $(dir $@)
	@${CC} ${CFLAGS} -c -w -O2 -pthread -DINTEGRATED_ASSEMBLY -DNO_3RD_PARTIES -o obj/rasm/rasm.o ${SRCROOT}/crocods-core/3rdparty/rasm/rasm.c

dist-tools: clean ${TOOLS}
	@${STRIP} ${TOOLS}
	@cp ${TOOLS} ${SRCROOT}/dist/${TOOLS}

dist: clean ${EMUL} 
	@${STRIP} ${EMUL}
	@cp ${EMUL} ${SRCROOT}/dist/${EMUL}

clean	:
	@echo "Cleaning"
	@rm -rf ${EMUL}.dSYM ${TOOLS}.dSYM
	@rm -f ${OBJS}
	@rm -f ${EMUL} ${TOOLS} 
	@rm -rf obj
