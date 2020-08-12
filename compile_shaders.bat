echo off
mkdir build
for %%x in (simple.vert simple.frag) do tools\glslangValidator.exe -V res\shaders\%%x -o build\%%x.spv"