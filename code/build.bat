@echo off
IF NOT EXIST ..\build mkdir ..\build
pushd ..\build

cl -Zi -Od -nologo /EHsc ..\code\win32_opengl.cpp User32.lib Opengl32.lib ../lib/glew32.lib ../lib/glfw3dll.lib

REM microsoft assembler 
REM ML64 ..\code\hello.asm /link /subsystem:console /defaultlib:kernel32.lib /entry:main 
popd
