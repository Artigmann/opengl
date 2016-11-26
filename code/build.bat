@echo off

REM HUSK!! set "visual studio solution working dir" til data dir ellers fungerer ikke shaders
REM hvis du ikke bruker visual studio til a kjore, last shaders ved bruk av relative paths,
REM eller legg dem i code dir

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build

set includePath=-I ..\lib\win32\include
set libPath=/LIBPATH:..\lib\win32
set linkerFlags=User32.lib Opengl32.lib %libPath%  glew32.lib glfw3dll.lib
set compilerFlags=-Zi -Od -nologo -EHsc %includePath%

cl  %compilerFlags% ..\code\win32_opengl.cpp /link %linkerFlags%

popd
