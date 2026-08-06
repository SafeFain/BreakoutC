# CI 拓展至 Android 与 iOS(2026-08-06)

## 目标
在既有三平台(桌面)CI 基础上,新增 Android 与 iOS 的自动构建验证。

## 完成内容

### 新增 CI jobs(`.github/workflows/ci.yml`)
| Job | Runner | 内容 |
| --- | --- | --- |
| `android-build` | ubuntu-latest | 安装构建工具 → actions/cache 缓存依赖 → `fetch-deps.sh`(NDK+SDL 源码)→ `build-android.sh` 交叉编译零警告 → 上传 `libmain.so` |
| `android-apk` | ubuntu-latest | `needs: android-build` → download libmain.so → 准备 JDK/SDK(无预置时自装 cmdline-tools,兼容 act)→ sdkmanager 装 platform 35 + build-tools → Gradle `assembleDebug` → 上传 APK |
| `ios-build` | macos-latest | `fetch-deps.sh --no-ndk` → CMake Xcode 交叉编译(`CMAKE_SYSTEM_NAME=iOS` / iphonesimulator / arm64+x86_64)→ .app 产物验证(Info.plist + 可执行) |

### 配套代码/文件改动
- `tools/android/fetch-deps.sh`:新增 `--no-ndk` 参数(iOS 不需要 NDK)
- `CMakeLists.txt`:iOS 分支从 `FATAL_ERROR` 改为实际交叉编译支持(与 Android 分支同构:add_subdirectory SDL3_ttf→SDL3,`SDL3::SDL3-static` + `SDL3_ttf::SDL3_ttf-static`,`MACOSX_BUNDLE` + 自定义 Info.plist);补回被编辑吞掉的 `else()` 分支
- 新增 `assets/ios/Info.plist`(横屏锁定、bundle 标识等)
- README CI 矩阵扩为 5 job;AGENTS 验证规则 8 更新为全平台

## 修复记录(2026-08-06,macOS 真实 runner 反馈)

**问题**:ios-build job 在真实 macOS runner 上报 `CMake Error at CMakeLists.txt:36 (find_package): Could not find a package configuration file provided by "SDL3"`。

**原因**:平台分支顺序错误——`elseif(APPLE)` 排在 `elseif(IOS)` 之前;iOS 交叉编译时 `APPLE` 为真,误走 macOS 分支的 Homebrew `find_package(SDL3)`,而 iOS 需要源码 `add_subdirectory`。

**修复**:分支顺序调整为 `Linux → WIN32 → ANDROID → IOS → APPLE → else`(iOS 优先于 APPLE 匹配);同时移除 ios-build 配置命令中的 `-DCMAKE_BUILD_TYPE=Release`(Xcode 多配置生成器忽略并警告)。Linux 构建回归通过。

## 修复记录(2026-08-06,Android APK 真实 runner 反馈)

**问题**:android-apk job 在真实 runner 上报 `sdkmanager: command not found`(exit 127)。

**原因**:runner 预置 `ANDROID_SDK_ROOT`(/usr/local/lib/android/sdk),但 `sdkmanager` 不在默认 PATH;此前仅"自装 cmdline-tools"分支追加了 PATH,预置分支缺失。

**修复**:PATH 追加从自装分支内移出,统一在步骤末尾执行——只要 `$ANDROID_SDK_ROOT/cmdline-tools/latest/bin` 存在即追加 `$GITHUB_PATH`(两种布局一致)。本地模拟验证两布局逻辑;act 全流程回归通过(APK 6.0MB 上传成功)。

## 验证
- `android-build` job:act 0.2.89(docker)端到端全绿——依赖下载 53s、交叉编译零警告、libmain.so 上传成功、依赖缓存 613MB 保存成功
- `android-apk` job:act 端到端全绿——JDK+SDK 自装分支、sdkmanager 组件安装、Gradle assembleDebug、APK(6.0MB)上传成功
- act 实测发现并修复 3 个问题:act 0.2.72 不支持 Node 24 action(升级 0.2.89);android-build 缺 cmake/ninja(补装);自装 SDK 后 sdkmanager 不在 PATH(追加 $GITHUB_PATH)
- `ios-build`:无法本地验证(需 macOS/Xcode),标准 CMake iOS 交叉编译流程,首次真实运行在 GitHub runner 上

## 遗留
- `ios-build` 首次真实运行待 GitHub runner 验证(macOS 模拟器交叉编译)
- iOS 为编译期验证(模拟器 SDK 交叉编译 .app),未运行模拟器
