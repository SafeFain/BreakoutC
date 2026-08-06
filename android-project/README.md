# Android 工程(预编译 .so 模式)

基于 SDL3 官方 `android-project` 模板改造。**SDL3 / SDL3_ttf / freetype 全部静态链接进 `libmain.so`**,本工程只负责 APK 组装(Java 启动器 + 资源 + assets)。

## 目录

```
android-project/
├── app/
│   ├── build.gradle          预编译模式(无 externalNativeBuild)
│   └── src/main/
│       ├── AndroidManifest.xml   横屏锁(sensorLandscape)、SDLActivity
│       ├── assets/fonts/         内置子集字体(APK assets,运行时经 SDL_IOFromFile 读取)
│       ├── java/org/libsdl/app/  SDL 官方 Java 启动器(已改:仅加载 "main")
│       └── res/                  资源(应用名 Neon Breaker)
├── build.gradle / settings.gradle / gradle.properties / gradlew
└── README.md(本文件)
```

## 构建步骤

### 1. 交叉编译 libmain.so

```bash
# 前置:NDK 解压至 tools/android/android-ndk-r27c;依赖源码在 tools/android/(见下)
tools/android/build-android.sh            # 默认 arm64-v8a
tools/android/build-android.sh armeabi-v7a
tools/android/build-android.sh x86_64
```

产物:`build-android/libmain.so`(CMake ANDROID 分支:add_subdirectory 构建 SDL3/SDL3_ttf/vendored freetype,游戏源码编为 `libmain.so`)。

### 2. 放入工程

```bash
mkdir -p android-project/app/libs/arm64-v8a
cp build-android/libmain.so android-project/app/libs/arm64-v8a/
```

> 仓库已预置 `android-project/app/libs/arm64-v8a/libmain.so`(2026-08-06 交叉编译产物);仅当重新编译后才需执行上述复制。

### 3. 组装 APK(需 Android Studio 或 Android SDK)

用 Android Studio 打开 `android-project/`,或命令行:

```bash
cd android-project
./gradlew assembleDebug
# 产物:app/build/outputs/apk/debug/app-debug.apk
adb install -r app/build/outputs/apk/debug/app-debug.apk
```

## 依赖源码(构建 libmain.so 所需)

`tools/android/` 下(由 `tools/android/fetch-deps.sh` 下载,或手动):

- `android-ndk-r27c/` — NDK(toolchain)
- `SDL-release-3.2.10/` — SDL3 源码
- `SDL_ttf-release-3.2.2/` — SDL3_ttf 源码(其 `external/freetype` 已放入 freetype 源码)

## 内置字体说明

- 字体文件:`assets/fonts/NotoSansSC-Bold-subset.otf`(107KB 子集,由 `tools/make_font.py` 生成)
- 打包:进入 APK `assets/fonts/`(Gradle 自动打包 `app/src/main/assets/`)
- 加载:`renderer.cpp` 的 `__ANDROID__` 分支用相对路径 `fonts/NotoSansSC-Bold-subset.otf` — SDL3 的 `SDL_IOFromFile` 在 Android 直接映射 APK assets(`AAssetManager_open`),`SDL_GetBasePath()` 在 Android 返回 NULL 故不用

## 输入/生命周期

- 触屏:SDL `SDL_EVENT_FINGER_*`(InputManager 已支持,含粘球瞄准方向)
- UI 点击:SDL 默认把单指触摸模拟为鼠标事件(`SDL_EVENT_MOUSE_BUTTON_DOWN`),UI 按钮可直接点击
- 返回键:`SDL_SCANCODE_AC_BACK` 等效 Esc(暂停/关闭设置,代码已适配)
- 切后台:焦点丢失自动暂停(`SDL_EVENT_WINDOW_FOCUS_LOST`,代码已适配)
- 存档:`SDL_GetPrefPath` → 应用内部存储(`~/.local/share` 的 Android 等价物)

## 已知限制

- 仅适配 arm64-v8a(其余 ABI 需用脚本参数重建 libmain.so)
- 游戏逻辑为 960×700 横屏布局,竖屏设备需旋转(已锁 sensorLandscape)
- 本仓库开发环境为 Linux,APK 组装未在本环境执行(需 Android SDK/Gradle);`libmain.so` 交叉编译已验证
