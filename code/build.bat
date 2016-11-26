@echo off

REM HUSK!! set "visual studio solution working dir" til data dir for a kjore
REM hvis du ikke bruker visual studio, kjor exe fila fra data dir (../build/win32_opengl.exe)
REM eller bruk run.bat script i misc til a kjore
REM TODO ha alt i relative paths

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build

set includePath=-I ..\lib\win32\include
set libPath=/LIBPATH:..\lib\win32
set linkerFlags=User32.lib Opengl32.lib %libPath%  glew32.lib glfw3dll.lib
set compilerFlags=-Zi -Od -nologo -EHsc %includePath%

cl  %compilerFlags% ..\code\win32_opengl.cpp /link %linkerFlags%

popd
