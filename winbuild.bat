echo off
rem Windows Build Steps
rem Note: Might need to change "Visual Studio..." depending on user's install
rem Create and cd into a build directory and then run this file with ..\winbuild.bat

if not exist "%ospray_DIR%" (
  echo "Must 'set ospray_DIR=<OSPRay Binary Package>'"
) else (
  cmake -G "Visual Studio 15 2017 Win64" ..  
  cmake --build . --config Release

  rem To run, we need *.dll from the OSPRay binary install
  copy %ospray_DIR%\bin\*.dll Release\
)

