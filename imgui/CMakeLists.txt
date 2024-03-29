## ======================================================================== ##
## Copyright 2009-2019 Intel Corporation                                    ##
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

add_library(ospray_imgui SHARED
  imgui.cpp
  imgui_draw.cpp
  imgui_demo.cpp
  imguifilesystem/imguifilesystem.cpp
)
target_link_libraries(ospray_imgui
  ospray::ospray
  ${OPENGL_LIBRARIES}
)
if (WIN32)
  # avoid problematic min/max defines of windows.h
  target_compile_definitions(ospray_imgui PUBLIC -DNOMINMAX)
endif()

target_include_directories(
  ospray_imgui
INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/imguifilesystem>
PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/imguifilesystem
)
