# TrafficAnalyzer - Video Analysis Application

An application written in C++ as project used for my Master's Thesis.\
Tested on **`Ubuntu 22.04`** with **`RTX 3060-Ti`**.

**Main features:**
* Vehicle detection via cuDNN and a YOLOv8 model
* Vehicle tracking using SORT algorithm
* Road heatmap generation
* Background estimation
* Time to collision and PET metric calculation
* Modular GUI

# Table of contents
* [Dependencies](#build-dependencies)
* [Build](#build)
* [NN Model](#neural-network-model)
* [Other tools](#other-used-tools)
* [Object tracker](#object-tracker)
* [Usage](#example-use)
    * [Input video](#input-video-file-selection)
    * [Object tracking](#object-tracking)
    * [Heatmaps](#heatmap-generation)
        * [Example heatmaps](#example-speed-heatmaps)
* [Reffereced and used 3rd party projects](#special-thanks-to-the-developers-of-these-projects)
#

<div align="center">
<img src="./readme-assets/tracker.png"
    alt="Tracking module"
    style="
    width: 45%;
    " />
&nbsp;
<img src="./readme-assets/tracking.png"
    alt="SORT object tracking"
    style="
    width: 29%;
    " />
<br/>
<img src="./readme-assets/app.png"
    alt="Speed heatmap"
    style="
    float: middle;
    width: 75%;
    " />

</div>
&nbsp;

## Build dependencies
***[Back to table of contents](#table-of-contents)***
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
***[Back to table of contents](#table-of-contents)***

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
***[Back to table of contents](#table-of-contents)***

Project comes with a pretrained neural network based on YOLOv8-s architecture distributed as ONNX model.\
Model file is located at **`./src/app/assets/NN-models/YOLOv8s-VSAI.onnx`**

## Other used tools
***[Back to table of contents](#table-of-contents)***

* [YOLOv8](https://github.com/ultralytics/ultralytics) - image recognition convolutional neural network architecture distributed with python scripts for training and inference as well as examples such as C++ use.

## Object tracker
***[Back to table of contents](#table-of-contents)***

Application uses SORT multiple object tracking algorithm:
[*Simple Online and Realtime Tracking* by A.Bewley, Z. Ge, L. Ott, F. Ramos, B. Upcroft](https://arxiv.org/abs/1602.00763)

It is based on ***Intersection over Union*** metric of object position predicted using ***Kalman Filter*** and incoming detections, matched using ***hungarian algorithm*** for optimal assignment.

System state matrices used in Kalman Filter:

$`
s_k=
\begin{bmatrix}
x\\
y\\
v_x\\
v_y
\end{bmatrix}_k=
\begin{bmatrix}
1 & 0 & T & 0\\
0 & 1 & 0 & T\\
0 & 0 & 1 & 0\\
0 & 0 & 0 & 1
\end{bmatrix}
\begin{bmatrix}
x\\
y\\
v_x\\
v_y
\end{bmatrix}_{k-1}+
\begin{bmatrix}
\frac{T^2}{2}\\
\frac{T^2}{2}\\
T\\
T
\end{bmatrix}a_{k-1}
`$

$`
y_k=
\begin{bmatrix}
x\\
y
\end{bmatrix}=
\begin{bmatrix}
1 & 0 & 0 & 0\\
0 & 1 & 0 & 0
\end{bmatrix}
\begin{bmatrix}
x\\
y\\
v_x\\
v_y
\end{bmatrix}_k
`$


# Example use

## Input video file selection
***[Back to table of contents](#table-of-contents)***
* On *`Image/Video Source`* tab click Select File.

<br/>
<div align="center">
<img src="./readme-assets/video_selection.png"
    alt="Video input control widget"
    style="
    width: 50%;
    " />
</div>
<br/>

* File browser will open with video file extension filter, select file and click *`ok`*.

<br/>
<div align="center">
<img src="./readme-assets/file_selection.png"
    alt="File browser"
    style="
    width: 50%;
    " />
</div>
<br/>

* First video frame should be possible to preview on *`Image/Video Source`* tab via *`Preview`* button.

<br/>
<div align="center">
<img src="./readme-assets/frame_preview.png"
    alt="Frame preview"
    style="
    float: middle;
    width: 45%;
    " />
</div>

## Object tracking
***[Back to table of contents](#table-of-contents)***
* After selecting input video head over to *`Object Detector`* tab.
* Select CUDA mode (0=CPU / 1=GPU).
* Select NN model.

<br/>
<div align="center">
<img src="./readme-assets/detector.png"
    alt="Video input control widget"
    style="
    width: 50%;
    " />
</div>
<br/>

* On *`Traffic Analyzer`* tab click *`Start traffic analysis`* button.

<br/>
<div align="center">
<img src="./readme-assets/traffic_analyzer.png"
    alt="Video input control widget"
    style="
    width: 25%;
    " />
</div>
<br/>

* On *`Object Tracker`* tab click *`Preview`* button.
* Detections and trackers should be visible on the preview.

<br/>

<div align="center">
<img src="./readme-assets/tracker.png"
    alt="File browser"
    style="
    width: 45%;
    " />
&nbsp;
<img src="./readme-assets/tracking.png"
    alt="Video input control widget"
    style="
    width: 29%;
    " />
</div>

## Heatmap generation
***[Back to table of contents](#table-of-contents)***
* If object tracking is running head to *`Traffic analyzer`* tab click *`Stop Processing`* button.
* On the same tab (*`Traffic Analyzer`*) click *`Start trajectory generator`* button.
* On *`Trajectory generator`* select colormap (TURBO is a good default)
* On the same tab (*`Trajectory Generator`*) click *`Toggle heatmap`* button.
* Then click on the corresponding heatmap button (e.g. *`Presence heatmap`*)

<br/>
<div align="center">
<img src="./readme-assets/trajectory_generator.png"
    alt="Video input control widget"
    style="
    width: 75%;
    " />
</div>
<br/>

## Example speed heatmaps
***[Back to table of contents](#table-of-contents)***
<div align="center">
<br/>
<br/>
<img src="./readme-assets/speed_30ms.gif"
    alt="Video input control widget"
    style="
    width: 35%;
    " />
<br/>
Speed magnitude
<br/>
<br/>
<img src="./readme-assets/x_30ms.gif"
    alt="Video input control widget"
    style="
    width: 35%;
    " />
<br/>
x direction speed
<br/>
<br/>
<img src="./readme-assets/y_30ms.gif"
    alt="Video input control widget"
    style="
    width: 35%;
    " />
<br/>
y direction speed
</div>

## Special thanks to the developers of these projects:
***[Back to table of contents](#table-of-contents)***
* Build system and GUI base [cpp-gui-template-sdl2](https://github.com/MartinHelmut/cpp-gui-template-sdl2)
* Imgui file browser component [imgui-filebrowser](https://github.com/AirGuanZ/imgui-filebrowser)
* Hungarian algorithm C++ implementation [hunharian-algorithm-cpp](https://github.com/mcximing/hungarian-algorithm-cpp)
