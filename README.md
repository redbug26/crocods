

# Keyboard shortcut
cmd+a call the autoload menu

cmd+r restart the CPC


# Compile from sources

## MGW (rg350 & other opendingux device)
```
docker run --rm -ti -v ${PWD}:/code soarqin/rg350_toolchain:latest
cd /code
make 2>&1 | sed -E -e "s/error/ $(echo -e "\\033[31m" ERROR "\\033[0m"/g)"   -e "s/warning/ $(echo -e "\\033[0;33m" WARNING "\\033[0m"/g)"
```

## Web (emscripten)
```
docker run --rm -v ${PWD}:/src/ -it trzeci/emscripten-ubuntu bash
emmake make -f Makefile.emscripten 
emcc crocods.bc -o hello.html -s USE_SDL=2
```

# Development

## Destination (DEFINE)

### OpenDingux (RG350)
**GCW** (with **SDL1**)

### Windows (with MinGW)
**_WIN32** (with **SDL1**)
or
**_WIN32** (with **SDL2**)

### Windows (with Visual Studio)
**_WIN32** (with **SDL2**)

### MacOS 
**TARGET_OS_MAC** (with **SDL2**)
or
**TARGET_OS_MAC** (with **METAL**)

### Web
**EMSCRIPTEN** (with **SDL2**)

# Keyboard

## on **SDL2**

``#define PAD_UP     SDL_SCANCODE_UP
#define PAD_LEFT   SDL_SCANCODE_LEFT;
#define PAD_RIGHT  SDL_SCANCODE_RIGHT
#define PAD_DOWN   SDL_SCANCODE_DOWN
#define PAD_A      SDL_SCANCODE_SPACE
#define PAD_B      SDL_SCANCODE_ESCAPE
#define PAD_X      SDL_SCANCODE_LSHIFT
#define PAD_Y      SDL_SCANCODE_X
#define PAD_L      SDL_SCANCODE_TAB
#define PAD_R      SDL_SCANCODE_BACKSPACE
#define PAD_START  SDL_SCANCODE_RETURN
#define PAD_SELECT SDL_SCANCODE_F12
#define PAD_QUIT   SDL_SCANCODE_C
#define PAD_L2     SDL_SCANCODE_PAGEUP
#define PAD_R2     SDL_SCANCODE_PAGEDOWN``

GUI+R: reset
GUI+A: autorun
GUI+Q: exit
GUI:F: fullscreen

(GUI: CMD key on macOS, shift+ctrl key on Windows)

# C Tips
Display all define from gcc:
`gcc -dM -E -x c /dev/null`

Debug
`lldb ./crocods_macosx`
// DEBUG
// lldb --file ./crocods_tools /Users/miguelvanhove/Dropbox/Sources/cpc/karaoke/karaoke.dsk
// bt (pour backtrace)
// b malloc_error_break

# Documentation
http://www.cantrell.org.uk/david/tech/cpc/cpc-firmware/firmware.pdf
http://www.cpctech.org.uk/
https://www.cpcwiki.eu/index.php?title=Technical_information_about_Locomotive_BASIC&mobileaction=toggle_view_desktop
https://cpcrulez.fr/coding_ali_gator-09_carte_memoire_CPC__MMAG.htm
https://www.cpcwiki.eu/forum/programming/binary-data-in-a-basic-program-loaded-in-one-go/msg187670/#msg187670
https://github.com/Bread80/Amstrad-CPC-BASIC-Source

# optimisateur ASM
CALL XX ; RET   => JMP XX
LD A,0   => XOR A


// DEBUG BASIC
TODO
- Activer le TRON lorsqu'on lance un debug
- Ajouter la modif ROM lorsqu'on lance un debug
- Verifier que le fichier editer est bien le fichier qui run
- Ajout des points d'arrets
- Afficher la valeurs des variables
- Continuer/stopper après un arrêt
- Faire du ligne par ligne
- Tester si le fichier est bien du basic :)

# TODO