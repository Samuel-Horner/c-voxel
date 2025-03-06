if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <build_mode>"
    exit 1
fi

if [ "$1" = "release" ]; then
    rm ./build/c_voxel
    echo "Creating release build"
    cmake -S . -B build -G Ninja -D CMAKE_BUILD_TYPE=Release
    cmake --build build
    echo "Created new release build"
elif [ "$1" = "debug" ]; then
    rm ./build/c_voxel
    echo "Creating debug build"
    cmake -S . -B build -G Ninja -D CMAKE_BUILD_TYPE=Debug
    cmake --build build
    ./build/c_voxel
elif [ "$1" = "test" ]; then
    rm ./build/c_voxel
    echo "Creating testing build"
    cmake -S . -B build -G Ninja -D CMAKE_BUILD_TYPE=Debug
    cmake --build build
    # ctest --test-dir build --output-on-failure -T memcheck -V --build-run-dir .
    valgrind --tool=memcheck --leak-check=full --error-exitcode=1 --errors-for-leak-kinds=definite --show-leak-kinds=definite ./build/c_voxel 
else
    echo "Invalid argument. Valid args: [debug, release, test]"
fi
