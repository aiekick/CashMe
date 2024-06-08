[![Win](https://github.com/aiekick/CashMe/actions/workflows/Win.yml/badge.svg)](https://github.com/aiekick/CashMe/actions/workflows/Win.yml)
[![Linux](https://github.com/aiekick/CashMe/actions/workflows/Linux.yml/badge.svg)](https://github.com/aiekick/CashMe/actions/workflows/Linux.yml)
[![Osx](https://github.com/aiekick/CashMe/actions/workflows/Osx.yml/badge.svg)](https://github.com/aiekick/CashMe/actions/workflows/Osx.yml)

# CashMe

CashMe is a Money Manager.

The features are :
 - Bank statement imports based on plugins
 - Can show Tranactions, with filtering
 - Can Update Once/All Transactions
 - Can Delete Once/All Transactions
 - Can display categories / operations per transactions
 
THe available plugins are :
 - LCL : Can import ofc and pdf files

## How to Build : 

You need to use cMake. 
You can use the gui (my prefered way).
but, if you run cmake by command line, For the 3 Os (Win, Linux, MacOs), the cMake usage is exactly the same,

    Choose a build directory. (called here my_build_directory for instance) and
    Choose a Build Mode : "Release" / "MinSizeRel" / "RelWithDebInfo" / "Debug" (called here BuildMode for instance)
    Run cMake in console : (the first for generate cmake build files, the second for build the binary)

cmake -B my_build_directory -DCMAKE_BUILD_TYPE=BuildMode
cmake --build my_build_directory --config BuildMode

Some cMake version need Build mode define via the directive CMAKE_BUILD_TYPE or via --Config when we launch the build. 
This is why i put the boths possibilities

## Dependencies per Os :

### On Windows :

You need to have the opengl library installed

### On Linux :

You need many lib : (X11, xrandr, xinerama, xcursor, mesa)

If you are on debian you can run :  

```cpp
sudo apt-get install libgl1-mesa-dev libx11-dev libxi-dev libxrandr-dev libxinerama-dev libxcursor-dev
```

### On MacOs :

you need many lib : opengl and cocoa framework



