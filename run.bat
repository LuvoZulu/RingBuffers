@echo off
echo  Configuring project 
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=C:/Users/Kaos/vcpkg/scripts/buildsystems/vcpkg.cmake

echo  Building project 
cmake --build build --config Debug

echo  Running tests 
if exist build\Debug\tests.exe (
    echo Running tests...
    build\Debug\tests.exe
) else (
    echo ERROR: tests.exe not found!
)

pause