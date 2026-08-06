# 任务归档:跨平台兼容性拓展(Windows/macOS/Linux/iOS/Android)

- **日期**:2026-08-06
- **状态**:已完成(归档)
- **目标**:拓展至 Windows 10/11、macOS 11+、Linux(X11/Wayland)、iOS 13+、Android 10+(用户确认全量交叉编译验证)

## 实施内容

### Phase 1 代码层
- 字体候选平台条件编译(Windows 微软雅黑/黑体;macOS 苹方/STHeiti;Linux 原有;移动端内置字体)
- 内置子集字体:`tools/make_font.py` 从系统 Noto CJK SC Bold 提取 552 字符 → 107KB `assets/fonts/NotoSansSC-Bold-subset.otf`;加载路径:桌面 `SDL_GetBasePath()+assets/fonts/`、iOS bundle `fonts/`、Android APK assets 相对路径(`SDL_IOFromFile`→`AAssetManager`,因 Android 的 `SDL_GetBasePath` 返回 NULL)
- 移动端:全屏标志分支、Android 返回键(`SDL_SCANCODE_AC_BACK`)等效 Esc

### Phase 2 桌面
- CMake 平台分支:Linux vendor/;Windows/macOS `find_package`(vcpkg/Homebrew);Android 专用子构建;rpath 仅 Linux;POST_BUILD 复制字体
- Windows:SDL3 官方 mingw 开发包 + mingw-w64 交叉编译 `neon-breaker.exe`(706KB,零警告);wine 冒烟因容器 `/run/user` 只读不可用(环境限制)
- macOS:构建文档 `docs/platform/macos.md`(Homebrew + CMake + .app 打包,未实测标注)

### Phase 3 移动
- Android:NDK r27c + SDL3 3.2.10 + SDL3_ttf 3.2.2(vendored freetype)+ 游戏源码静态链接 → `libmain.so` 18.4MB(arm64-v8a,仅依赖 Android 系统库,零警告);`android-project/` 模板改造(预编译 so 模式、横屏 `sensorLandscape`、仅加载 main、应用名、字体 assets)
- iOS:构建文档 `docs/platform/ios.md`(SDL3 官方 Xcode 工程 `Xcode/SDL/SDL.xcodeproj`、横屏 Info.plist、字体 bundle、未实测标注)
- 移动端输入/生命周期八项核对全部就绪(触屏瞄准/UI 点击/自动暂停/返回键/存档/全屏/字体/缩放)

## 关键修复

- SDL_ttf 的 `find_package(SDL3)` 依赖:先 `add_subdirectory(SDL3)` 使 target 存在即可跳过
- SDL_ttf vendored freetype:`external/freetype` 需源码内容(占位目录嵌套坑)
- ANDROID 分支与公共 `add_executable` target 冲突:`return()` 分离
- Android `SDL_GetBasePath()` 返回 NULL → 字体走 APK assets 相对路径
- `SDL_RenderReadPixels` SDL3 新签名(截图)与 `SDL_GetBasePath` 返回 `const char*`
- 清理遗留 `lastTime_` 未使用字段

## 验证

- Linux:构建零警告 + aim-test ALL PASS + 截图回归(子集字体字形检视正常)
- Windows:mingw 交叉编译零警告产出 exe(706KB)
- Android:交叉编译零警告产出 libmain.so(18.4MB,readelf 确认仅系统库依赖)
- macOS/iOS:配置与文档交付,需对应平台实测(README 平台矩阵标注 ⚠️)

## 涉及文件

`src/{renderer,input,ui,main,game}.{h,cpp}`、`CMakeLists.txt`、`tools/{make_font.py,mingw-toolchain.cmake,win-deps/,android/}`、`assets/fonts/`、`android-project/`、`docs/platform/{macos,ios}.md`、`README.md`、`AGENTS.md` 及状态文件

## 遗留事项

- macOS/iOS 真实构建与运行验证(需 macOS/Xcode)
- Android APK 组装(Gradle)与真机运行(需 Android SDK/设备)
- wine 运行冒烟(容器 /run/user 只读限制)
- 更多 ABI(armeabi-v7a/x86_64)可用 `build-android.sh` 参数扩展
