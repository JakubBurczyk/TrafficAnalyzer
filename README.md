# TrafficAnalyzer - Video Analysis Application

An application written in C++ as project used for my Master's Thesis.\
Tested on **`Ubuntu 22.04`** with **`RTX 3060-Ti`**.

Main features:
* Vehicle detection via cuDNN and a YOLOv8 model
* Vehicle tracking using SORT algorithm
* Road heatmap generation
* Background estimation
* Modular GUI

#

<div align="center">
<img src="./readme-assets/test.png"
    alt="Markdown Monster icon"
    style="
    width: 45%;
    " />
&nbsp;
<img src="./readme-assets/test.png"
    alt="Markdown Monster icon"
    style="
    width: 45%;
    " />
<br/>

<img src="./readme-assets/test.png"
    alt="Markdown Monster icon"
    style="
    float: middle;
    width: 45%;
    " />

</div>
&nbsp;

## Build dependencies
* **CUDA and cuDNN**\
`CUDA version: 11.6`\
It will most likely work with newer versions, mind you will have to modify **`./CMakeLists.txt`**.\
 \
Attaching some steps (as extracted from terminal history) for installation at **`./scripts/cuda_install_help.txt`** but they may not work, you should try to get the latests drivers anyways that are supported by your Nvidia driver and hardware.

* **ffmpeg**\
`version: 4.4.2-0ubuntu0.22.04.1`

* **OpenCV**\
`version: 4.7.0`\
Built from srouce with CUDA and FFMPEG support.


## Build
The project uses [CMake](https://cmake.org) and [Ninja](https://ninja-build.org) as build systems.\
Build scripts are located in **`./scripts`** directory which can be streamlined to use via VSCode tasks.

Built with **g++** `version: 11.4`

Manual build:
```shell
cmake -GNinja -DCMAKE_BUILD_TYPE=Release -B build/release
ninja -C build/release
```
### NOTICE: Due to problems with OpenCV, CUDA and tested hardware, debug build may be unstable and not support GPU acceleration!

Run the application:

```shell
cd ./build/release/src/app && ./TrafficAnalyzer
```

## Neural Network Model
Project comes with a pretrained neural network based on YOLOv8-s architecture distributed as ONNX model.\
Model file is located at **`./src/app/assets/NN-models/YOLOv8s-VSAI.onnx`**

## Other used tools

* [YOLOv8](https://github.com/ultralytics/ultralytics) - image recognition convolutional neural network architecture distributed with python scripts for training and inference as well as examples such as C++ use.


## Example use

### File selection

### Object tracking

### Heatmap generation

## Special thanks to the developers of these projects:

* Build system and GUI base [cpp-gui-template-sdl2](https://github.com/MartinHelmut/cpp-gui-template-sdl2)
* Imgui file browser component [imgui-filebrowser](https://github.com/AirGuanZ/imgui-filebrowser)
* Hungarian algorithm C++ implementation [hunharian-algorithm-cpp](https://github.com/mcximing/hungarian-algorithm-cpp)
