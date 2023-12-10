# CrocoDS
 
Amstrad emulator for everyone.

First version of CrocoDS (based on Win-CPC by Ludovic Deplanque) was created for... Nintendo DS. 
Over the years, emulation has become better.

* * *

# Why another Amstrad emulator ?

CrocoDS is made to give the best playing experience to all users!

All we want is to give you a lot of fun! 

Amstrad have a lot of great games (my preferences for Boulderdash, Bomb Jack, ...).

* * *

# crocodsCLI #

## crocodsCLI Samples ##
`crocodsCLI -B fichier.bas`

Convert a .bas file to a .sna file

`crocodsCLI fichier.sna --saveImage --stopAfter 1`

Convertir a .sna file to .gif file

`crocodsCLI fichier.dsk --saveSnaphsot --stopAfter 500`

Convertir a .dsk file to .sna file

* * *

# CrocoDS #

## Shortcuts ##
In the emulator the shortcuts are called:
- On Windows, with key **ctrl left** + **shift left**
- On macOSX, with key **command**

### shortcut+r
Reset the emulator. Launch the argument file if it exists.

### shortcut+a
Launch the autorun mode (et restart if needed) 

### shortcut+q
Exit to a better world

### shortcut+f
Switch to fullscreen

### shortcut+e
Switch to warp mode (it's fast)

### shortcut+c
Switch to LUA console

## Console LUA ##

// to be added

### Module cpc ####

#### load ####

Usage:  

cpc.load (**filename**)

Load a .bas, .s or .lua file directly in memory

```cpc.load("fichier.bas")```

#### run ####

Usage:  

cpc.run (**filename**)

Run a .bas, .s or .lua file directly in memory

```cpc.run("fichier.bas")```

#### peek ####

Usage:  

cpc.peek (**adress**)

Reads the contents of the Z80 memory.

Associated key words: [cpc.poke](#cpc.poke)

Eg:

```cpc.peek(0)```

#### poke ####

Usage:  

cpc.poke (**adress**, **value**)

Write the value in the Z80 memory.

Associated key words: [cpc.peek](#cpc.peek)

Eg:

```cpc.poke(0,3)```

#### setreg ####

#### getreg ####

#### text ####

#### insert ####

#### capture ####

#### call ####

Example:

Réinitilisation:
```cpc.call(0)```

CLS:
```cpc.call(0xBB6C)```

#### dump ####

Example:

Dump Basic memory:
```cpc.dump(0x170)```

### Module fs ###

#### cd ####

Usage:  

cd (**folder**)

Change current directory to folder. If used without parameters, it sync the current folder with the media folder from the Amstrad.

* * *

# Behind the scenes #

`CrocoDS` will not exist without them:

- [rasm](https://github.com/EdouardBERGE/rasm): Z80 assembler by Edouard Bergé (aka Roudoudou)
- [miniz](https://github.com/richgel999/miniz/ "miniz"): ZIP reader/writer library by Rich GELDREICH
- [lua](https://www.lua.org "lua"): the extensible embedded language by Lua.org, PUC-Rio
- [SDL2](https://www.libsdl.org "SDL2"): cross-platform development library designed to provide low level access to audio, keyboard, mouse, joystick and graphics hardware by libsdl.org
- [StSoundLibrairy 2.0](https://github.com/arnaud-carre/StSound): Library to replay YM files (Atari ST music file format) by Arnaud Carré (aka Leonard/Oxygene)
- [LodePNG](https://github.com/lvandeve/lodepng): by Lode Vandevenne
- [TextEd](https://github.com/gauri-singh/textEd): Ansi text editor by Gauri Singh
- [Win-CPC](http://demoniak-contrib.forumactif.com "Win-CPC"): Amstrad CPC Emulator - Copyright 2012 Ludovic Deplanque (aka Demoniak).
- [Caprice32](https://sourceforge.net/projects/caprice32/ "Caprice32"): Amstrad CPC Emulator - Copyright 1997-2004 Ulrich Doewich.

* * *

# Greets go to #

- Roudoudou for his wonderful rasm
- Rétro Poke for helping with the basic injection
- Kukulcan for the keyboard GFX

* * *

The MIT License (MIT)  

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

* * *