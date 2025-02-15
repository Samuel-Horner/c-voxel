if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <build_mode>"
    exit 1
fi

if [ "$1" = "release" ]; then
    echo "Creating release build"
    cmake -S . -B build/ -D CMAKE_BUILD_TYPE=Release
    cmake --build build/
    echo "Created new release build"
else
    rm ./build/c_voxel
    echo "Creating debug build"
    cmake -S . -B build/ -D CMAKE_BUILD_TYPE=Debug
    cmake --build build/
    ./build/c_voxel
fi
