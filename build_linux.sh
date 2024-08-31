conan install . --build=missing --profile=debian_release
conan install . --build=missing --profile=debian_release --settings=build_type=Debug

cd build/Release

source ./generators/conanbuild.sh
cmake ../.. -DCMAKE_TOOLCHAIN_FILE=generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release

cd ../Debug

source ./generators/conanbuild.sh
cmake ../.. -DCMAKE_TOOLCHAIN_FILE=generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug

cd ../../