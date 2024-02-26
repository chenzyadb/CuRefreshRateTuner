#!/usr/bin/sh

ANDROID_NDK="/home/android-ndk"
SOURCE_DIR=$(dirname $0)
BUILD_DIR="${SOURCE_DIR}/build"
TOOLCHAIN_BIN="${ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/bin"
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
cmake --build ${BUILD_DIR} --config "release" --target "CuRefreshRateTuner" -j16

exit 0;
