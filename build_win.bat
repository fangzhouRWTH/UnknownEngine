conan install . --build=missing --profile=windows_release
conan install . --build=missing --profile=windows_debug
conan install . --build=missing --profile=windows_release -s "&:build_type=RelWithDebInfo"

cd build

call ./generators/conanbuild.bat
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .. -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE=generators/conan_toolchain.cmake