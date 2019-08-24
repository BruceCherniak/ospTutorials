## ======================================================================== ##
## Copyright 2018-2019 Intel Corporation                                    ##
##                                                                          ##
## Licensed under the Apache License, Version 2.0 (the "License");          ##
## you may not use this file except in compliance with the License.         ##
## You may obtain a copy of the License at                                  ##
##                                                                          ##
##     http://www.apache.org/licenses/LICENSE-2.0                           ##
##                                                                          ##
## Unless required by applicable law or agreed to in writing, software      ##
## distributed under the License is distributed on an "AS IS" BASIS,        ##
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. ##
## See the License for the specific language governing permissions and      ##
## limitations under the License.                                           ##
## ======================================================================== ##

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

cmake_minimum_required(VERSION 3.12)

project(ospTutorials VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD   99)
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_C_STANDARD_REQUIRED   ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

find_package(ospray 1.8.5 REQUIRED)

set(OSPRAY_DEFAULT_COMPONENT tutorials)

# prefer libGL over libOpenGl for better compatibility with SWR
set(OpenGL_GL_PREFERENCE "LEGACY")
find_package(OpenGL 2 REQUIRED)

add_subdirectory(imgui)
add_subdirectory(glfw)

## build simple ospTutorial, for demonstration and API testing ##

add_executable(ospTutorial ospTutorial.c)
target_link_libraries(ospTutorial PRIVATE ospray::ospray)
# C++ version
add_executable(ospTutorialCpp ospTutorial.cpp)
target_link_libraries(ospTutorialCpp PRIVATE ospray::ospray)

## create a static library of common tutorial bits ##

add_library(tutorial_common STATIC
  ArcballCamera.cpp
  GLFWOSPRayWindow.cpp
  imgui/imgui_impl_glfw_gl3.cpp
)

if (WIN32)
  # avoid problematic min/max defines of windows.h
  target_compile_definitions(tutorial_common PUBLIC -DNOMINMAX)
endif()

target_link_libraries(tutorial_common PUBLIC
  ospray::ospray_testing
  ospray_imgui
  glfw
  ${OPENGL_LIBRARIES}
)

## build actual tutorials ##

add_executable(ospTutorialSpheres ospTutorialSpheres.cpp)
target_link_libraries(ospTutorialSpheres PRIVATE tutorial_common)

add_executable(ospTutorialBouncingSpheres ospTutorialBouncingSpheres.cpp)
target_link_libraries(ospTutorialBouncingSpheres PRIVATE tutorial_common)

add_executable(ospTutorialSubdivision ospTutorialSubdivision.cpp)
target_link_libraries(ospTutorialSubdivision PRIVATE tutorial_common)

add_executable(ospTutorialStructuredVolume ospTutorialStructuredVolume.cpp)
target_link_libraries(ospTutorialStructuredVolume PRIVATE tutorial_common)

add_executable(ospTutorialUnstructuredVolume ospTutorialUnstructuredVolume.cpp)
target_link_libraries(ospTutorialUnstructuredVolume PRIVATE tutorial_common)

add_executable(ospTutorialQuadMesh ospTutorialQuadMesh.cpp)
target_link_libraries(ospTutorialQuadMesh PRIVATE tutorial_common)
