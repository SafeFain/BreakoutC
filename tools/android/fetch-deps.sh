#!/usr/bin/env bash
# 下载并准备 Android/iOS 交叉编译依赖(NDK + SDL3 + SDL3_ttf + freetype)
# 用法: tools/android/fetch-deps.sh [--no-ndk]   # --no-ndk: 跳过 NDK(iOS 构建不需要)
set -euo pipefail

DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$DIR"

NO_NDK=false
if [[ "${1:-}" == "--no-ndk" ]]; then NO_NDK=true; fi

NDK_VERSION="r27c"
SDL3_VERSION="3.2.10"
SDL3_TTF_VERSION="3.2.2"
FREETYPE_VERSION="VER-2-13-3"

if $NO_NDK; then
  echo "==> --no-ndk: 跳过 NDK 下载"
else
  echo "==> 下载 NDK $NDK_VERSION(~660MB)..."
  [ -d "android-ndk-$NDK_VERSION" ] || {
    curl -sL -o ndk.zip "https://dl.google.com/android/repository/android-ndk-$NDK_VERSION-linux.zip"
    unzip -q ndk.zip && rm -f ndk.zip
  }
fi

echo "==> 下载 SDL3 $SDL3_VERSION / SDL3_ttf $SDL3_TTF_VERSION / freetype..."
[ -d "SDL-release-$SDL3_VERSION" ] || {
  curl -sL -o SDL3.tar.gz "https://codeload.github.com/libsdl-org/SDL/tar.gz/refs/tags/release-$SDL3_VERSION"
  tar xzf SDL3.tar.gz && rm -f SDL3.tar.gz
}
[ -d "SDL_ttf-release-$SDL3_TTF_VERSION" ] || {
  curl -sL -o SDL3_ttf.tar.gz "https://codeload.github.com/libsdl-org/SDL_ttf/tar.gz/refs/tags/release-$SDL3_TTF_VERSION"
  tar xzf SDL3_ttf.tar.gz && rm -f SDL3_ttf.tar.gz
}
[ -d "freetype-$FREETYPE_VERSION" ] || {
  curl -sL -o freetype.tar.gz "https://codeload.github.com/freetype/freetype/tar.gz/refs/tags/$FREETYPE_VERSION"
  tar xzf freetype.tar.gz && rm -f freetype.tar.gz
}

echo "==> 准备 SDL_ttf 的 vendored freetype..."
rm -rf "SDL_ttf-release-$SDL3_TTF_VERSION/external/freetype"
cp -r "freetype-$FREETYPE_VERSION" "SDL_ttf-release-$SDL3_TTF_VERSION/external/freetype"

echo "依赖就绪:$(ls -d android-ndk-* SDL-release-* SDL_ttf-release-* freetype-*)"
