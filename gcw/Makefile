CC 		= mipsel-linux-gcc

DEFINES 	= -DGCW -DSDL1
DEFINES	       += -DREDBUG
ZIP_SUPPORT     = 1


SRCROOT = ..

CFLAGS 		= -O2 -ffast-math -fomit-frame-pointer -fdata-sections -ffunction-sections -mips32r2 -std=gnu89 -Wall ${DEFINES} 
CFLAGS	   += -mno-relax-pic-calls -mno-abicalls -mlong-calls -mlong32 -mlocal-sdata -mno-check-zero-division -mfp32 
CFLAGS     += -mgp32 -mno-embedded-data -fno-pic -mno-interlink-compressed -mno-mt -mno-micromips -mno-interlink-mips16
LDFLAGS 	= -Wl,--as-needed -Wl,--gc-sections -flto -lSDL -s
OUT	 	= crocods

include ${SRCROOT}/makefile.shared

SOURCES 	= ${SDL} ${CROCODS}
SOURCES	   += ${THIRD_PARTY}

OBJS 		= ${SOURCES:.c=.o}

all		: ${OUT} pack

pack	:
		mipsel-linux-strip crocods
		mv crocods ./opk/crocods
		mksquashfs ./opk crocods.opk -all-root -noappend -no-exports -no-xattrs
		mv crocods.opk ./release/crocods.opk
		scp release/crocods.opk root@10.1.1.2:/media/sdcard/APPS/
		scp opk/crocods root@10.1.1.2:/media/sdcard/APPS/

${OUT}	: ${OBJS}
		${CC} -o ${OUT} ${SOURCES} ${CFLAGS} ${LDFLAGS}
	
clean	:
		rm ${OBJS}
		rm ./opk/${OUT}
		rm ./opk/${OUT}.opk
