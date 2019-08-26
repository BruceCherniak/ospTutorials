echo off
rem Windows Build Steps
rem Note: Might need to change "Visual Studio..." depending on user's install
cmake -G "Visual Studio 15 2017 Win64" ..  
cmake --build . --config Release


rem To run, we need *.dll from the OSPRay binary install
rem copy <path to OSPRay-1.8.5.windows>\bin\*.dll Release
