@echo off
if not exist build mkdir build
cd build
::g++ ../pgmviewer.cpp ../lib/glad.c -lglfw3dll -lglfw3 -o pgmviewer.exe
windres ..\resource.rc resource.o
g++ ../pgmviewer.cpp ../lib/glad.c resource.o -o pgmviewer.exe -lglfw3dll -lglfw3 -mwindows
cd ..