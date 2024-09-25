conan install . --build=missing --profile=windows_release
conan install . --build=missing --profile=windows_debug

cd build

call ./generators/conanbuild.bat
cmake .. -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE=generators/conan_toolchain.cmake