@echo off
setlocal enabledelayedexpansion

rem Define arrays for platforms, profiles, and output directories
set PLATFORMS[1]=windows
set PLATFORMS[2]=osx
set PLATFORMS[3]=ios

set PROFILES[1]=s_5_0
set PROFILES[2]=metal
set PROFILES[3]=metal

set OUTPUT_DIRS[1]=directx11
set OUTPUT_DIRS[2]=metal-osx
set OUTPUT_DIRS[3]=metal-ios

for /L %%I in (1,1,3) do (
    echo Running: shadercRelease.exe -f vertex.glsl -o ./!OUTPUT_DIRS[%%I]!/vs_texture.bin --platform !PLATFORMS[%%I]! --profile !PROFILES[%%I]! --type vertex
    shadercRelease.exe -f vertex.glsl -o ./!OUTPUT_DIRS[%%I]!/vs_texture.bin --platform !PLATFORMS[%%I]! --profile !PROFILES[%%I]! --type vertex

    echo Running: shadercRelease.exe -f fragment.glsl -o ./!OUTPUT_DIRS[%%I]!/fs_texture.bin --platform !PLATFORMS[%%I]! --profile !PROFILES[%%I]! --type fragment
    shadercRelease.exe -f fragment.glsl -o ./!OUTPUT_DIRS[%%I]!/fs_texture.bin --platform !PLATFORMS[%%I]! --profile !PROFILES[%%I]! --type fragment
)

endlocal