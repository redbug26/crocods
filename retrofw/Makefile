CC 		= mipsel-linux-gcc

DEFINES 	= -DGCW -DSDL1 
DEFINES        += -DREDBUG
ZIP_SUPPORT     = 1

SRCROOT = ..

CFLAGS 		= -mips32
CFLAGS	   += -O3 -fsigned-char -ffast-math -fomit-frame-pointer  ${DEFINES} 

LDFLAGS 	= -Wl,--as-needed -Wl,--gc-sections -flto -lSDL -s
OUT	 	= crocods.dge

include ${SRCROOT}/makefile.shared

SOURCES 	= ${SDL} ${CROCODS}
SOURCES	   += ${THIRD_PARTY}

OBJS 		= ${SOURCES:.c=.o}

all		: ${OUT} pack

pack	:
		mipsel-linux-strip crocods.dge
		./pack.sh

${OUT}	: ${OBJS}
		${CC} -o ${OUT} ${SOURCES} ${CFLAGS} ${LDFLAGS}
	
clean	:
		rm ${OBJS}
		rm -R ipkg/home/retrofw/emus/crocods/
		rm ipkg/control.tar.gz
		rm ipkg/data.tar.gz
		
