# MatOS

MatOS is an operating system that I'm developing in my free time.

# Compilation

MatOS uses `.exe` file as a kernel so it has to be compiled under windows. However you can use `i686-w64-mingw32-gcc` command to compile it under linux but it will require a little bit modification of `build.py`.

To compile MatOS type: 

`mkdir build`

`python build.py` (must be python 3)

Note that build.py script uses programs such as `nasm`, `strip` or `gcc`. They need to be in `PATH`. It also reference to files using relative pathes so you have to run it from prioject main folder.

`build.py` will create `floppy.raw` file in `build` folder which you can use as floppy image on emulators or you can write it to pendrive low levely using program such as `ImageWriter`.