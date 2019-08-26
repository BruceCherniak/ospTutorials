# ospTutorials
## OSPRay Tutorials built & run against an OSPRay binary release package

*- Requires OSPRay v1.8.5 precompiled package from http://www.ospray.org/downloads.html*

- git clone this repo https://github.com/BruceCherniak/ospTutorials.git
- cd ospTutorials
- mkdir build
- cd build

## Linux/MacOS:
- export ospray_DIR=<path_to_OSPRay_binary_release>
- cmake ..
- make

## Windows
- set ospray_DIR=<path_to_OSPRay_binary_release>
- ..\winbuild.bat
