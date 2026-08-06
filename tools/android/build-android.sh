#!/usr/bin/env bash
# 交叉编译 Android libmain.so(静态链接 SDL3/SDL3_ttf/freetype)
# 用法: tools/android/build-android.sh [arm64-v8a|armeabi-v7a|x86_64]
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
NDK="$ROOT/tools/android/android-ndk-r27c"
ABI="${1:-arm64-v8a}"
API="${API:-29}"

[ -d "$NDK" ] || { echo "NDK 未找到: $NDK(下载 android-ndk-r27c-linux.zip 并解压到 tools/android/)"; exit 1; }

cmake -S "$ROOT" -B "$ROOT/build-android" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE="$NDK/build/cmake/android.toolchain.cmake" \
  -DANDROID_ABI="$ABI" \
  -DANDROID_PLATFORM="android-$API" \
  -DANDROID_STL=c++_static \
  -DSDLTTF_VENDORED=ON \
  -DSDLTTF_HARFBUZZ=OFF \
  -DSDLTTF_PLUTOSVG=OFF

cmake --build "$ROOT/build-android" -j"$(nproc)"

echo "=== 产物 ==="
ls -la "$ROOT/build-android/libmain.so"
