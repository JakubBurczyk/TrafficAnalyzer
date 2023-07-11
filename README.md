# Base GUI project setup with SDL2 and OpenGL


## Setup

The project uses [CMake](https://cmake.org) and [Ninja](https://ninja-build.org).

Build the application in debug mode:

```shell
cmake -GNinja -DCMAKE_BUILD_TYPE=Debug -B build/debug
ninja -C build/debug
```

Run the application:

```shell
cd ./build/debug/src/app && ./App
```


## Disclaimer

Special acknowledgment to the amazing open source font [Manrope](https://manropefont.com).
