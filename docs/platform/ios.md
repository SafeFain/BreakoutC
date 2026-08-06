# iOS 13+ 构建说明(Neon Breaker)

> ⚠️ 本文件由 Linux 交叉开发环境编写,**未在真实 macOS/Xcode 上验证**。SDL3 的 iOS 支持成熟,但本工程尚未执行过 iOS 构建;以下步骤基于 SDL3 官方文档与源码模板。

## 前置条件

- macOS + Xcode(iOS 13+ 部署目标)
- 依赖源码:`tools/android/SDL-release-3.2.10/`(SDL3 源码含 Xcode 模板)

## 构建步骤(参考)

SDL3 官方 iOS 集成有两种方式,任选:

### 方式 A:Xcode 工程(推荐)

1. SDL3 源码自带 Xcode 工程:打开 `tools/android/SDL-release-3.2.10/Xcode/SDL/SDL.xcodeproj`(官方文档 `docs/README-ios.md`)
2. 按 SDL3 官方步骤(SDL.xcodeproj 含 SDL3.xcframework target):把 SDL 工程加入你的 app 工程,或直接用 SDL3.xcframework 拖入
2. 把本项目 `src/*.cpp|h` 加入 app target 编译源
3. 链接:`libSDL3.a`(静态)+ `libSDL3_ttf.a` + `libfreetype.a` + SDL3 要求的系统框架(CoreGraphics、QuartzCore、UIKit、AVFoundation 等,按 SDL3 模板配置)
4. 字体资源:把 `assets/fonts/NotoSansSC-Bold-subset.otf` 加入 bundle 的 `fonts/` 子目录(代码用 `SDL_GetBasePath() + "fonts/"` 加载;iOS 的 `SDL_GetBasePath` 返回 bundle 目录)
5. 部署目标:iOS 13.0(`IPHONEOS_DEPLOYMENT_TARGET`)

### 方式 B:CMake 交叉编译

```bash
# 在 macOS 上,用 Xcode 的 iOS toolchain
cmake -B build-ios -DCMAKE_SYSTEM_NAME=iOS \
  -DCMAKE_OSX_SYSROOT=iphoneos \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0 \
  -DCMAKE_OSX_ARCHITECTURES=arm64
# 然后按主 CMakeLists 的 IOS 分支接入 SDL3/SDL3_ttf 源码(与 ANDROID 分支同构)
```

## Info.plist 要点

```xml
<key>UISupportedInterfaceOrientations</key>
<array>
    <string>UIInterfaceOrientationLandscapeLeft</string>
    <string>UIInterfaceOrientationLandscapeRight</string>
</array>
<key>UIRequiresFullScreen</key><true/>
```

## 输入/生命周期

- 触屏、UI 点击、切后台暂停、存档路径:与 Android 相同(SDL3 统一处理,代码已适配)
- iOS 无实体返回键:UI 顶栏"暂停"按钮可用(代码已有)

## 已知限制(未验证项)

- 本环境无 macOS/Xcode,上述步骤与 `__IPHONEOS__` 代码分支(全屏标志、字体路径)未实测
- 上架 App Store 需签名/公证与隐私声明(未配置)
- 静态链接 SDL3_ttf 需要同时链接 freetype(建议复用 `tools/android/SDL_ttf-release-3.2.2` 的 vendored freetype 构建)
