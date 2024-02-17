#!/usr/bin/sh

SOURCE_DIR=$(dirname $0)
BUILD_DIR="${SOURCE_DIR}/build"
WINDOWS_SOURCE_DIR="/mnt/c/Users/chenz/Documents/GitHub/CuRefreshRateTuner"
TOOLCHAIN_BIN="/home/android-ndk/toolchains/llvm/prebuilt/linux-x86_64/bin"
TARGET_PREFIX="aarch64-linux-android29"

rm -rf ${BUILD_DIR}
mkdir -p ${BUILD_DIR}

cmake \
    -DCMAKE_BUILD_TYPE="release" \
    -DCMAKE_C_COMPILER="${TOOLCHAIN_BIN}/${TARGET_PREFIX}-clang" \
    -DCMAKE_CXX_COMPILER="${TOOLCHAIN_BIN}/${TARGET_PREFIX}-clang++" \
    -H${SOURCE_DIR} \
    -B${BUILD_DIR} \
    -G "Unix Makefiles"
cmake --build ${BUILD_DIR} --config "release" --target "CuRefreshRateTuner" -j4

exit 0;
