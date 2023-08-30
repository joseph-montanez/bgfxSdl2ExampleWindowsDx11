## How To Build

    mkdir build
    cd build

If you want to open this in Visual Studio:

    cmake.exe -G "Visual Studio 16 2019" -A x64 ..

If not then:

    cmake -A x64 ..
    cmake --build . --config Debug

Binary will be in `build\Debug\bgfxSdl2ExampleWindowsDx11.exe`

All DLLs, and assets should be in that debug folder so just run it from there.