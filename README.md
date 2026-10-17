# Neon Breaker · 霓虹破界

经典打砖块(Arkanoid)风格街机游戏 —— **C++20 + SDL3** 跨平台实现。

霓虹美学、120Hz 固定步长物理、程序化合成音效(零外部资源)、五种平台构建链。

> **历史**:项目曾包含 TypeScript(web)实现,已于 2026-08-06 移除(源码备份:`/tmp/breakout-web-backup-20260806.tar.gz`)。当前为 C++ 单实现。

---

## 特性

- **玩法**:5 个关卡、7 种砖块、11 种道具、连击倍率计分、120Hz 固定步长物理
- **粘球自由方向**:粘球吸附时拖动鼠标/手指,**鼠标与球的连线即为发射方向**(虚线瞄准线 + 最小仰角防穿板)
- **界面**:主菜单、关卡选择、设置对话框(语言/主题/拖尾/粒子特效/球色)、HUD、overlay、过关 toast
- **特效**:霓虹粒子、砖块碎片、得分飘字、扩散光环、全屏闪光、屏幕震动、动态背景光点、连击边缘辉光
- **音效**:12 种程序化合成音效(SDL 音频流,无外部音频文件)
- **设置持久化**:`SDL_GetPrefPath` 平台自动定位(`~/.local/share/NeonBreaker/settings/settings.kv`)
- **输入**:键盘 / 鼠标 / 触屏(SDL 触摸事件,移动端适配)
- **主题**:浅色 / 深色 / 跟随系统
- **自动暂停**:窗口失焦 / 最小化
- **自由调节窗口**:任意缩放,界面等比居中(letterbox),最小 640×420

---

## 快速开始(Linux)

```bash
# 依赖已随项目提供(vendor/),无需系统安装
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/neon-breaker
```

> 要求:C++20 编译器、CMake ≥ 3.20。SDL3 3.2.10 + SDL3_ttf 3.2.2 已内置 `vendor/`。

---

## 玩法指南

### 操作

| 操作 | 键盘 | 鼠标 / 触屏 |
| --- | --- | --- |
| 移动挡板 | `A` / `D`、`←` / `→` | 移动指针(挡板跟随) |
| 发球 / 发射粘球 | `空格`(或任意方向键) | 点击 / 触摸 |
| 暂停 / 继续 | `Esc`、`P` | — |
| 重开当前关 | `R`(游戏中) | — |
| 粘球瞄准(粘球增益下) | 发射方向 = 球心指向指针位置 | 鼠标/手指位置即瞄准点 |

### 砖块(7 种)

| 砖块 | 生命 | 说明 |
| --- | --- | --- |
| 普通 `·` | 1 | 基础砖 |
| 坚固 `= · =` | 2–4 | 多段生命,颜色随生命变化 |
| 不可摧毁 `◆` | ∞ | 只能被穿透球/激光绕过 |
| 奖励 `★` | 1 | 掉落道具 |
| 爆炸 `＊` | 1 | 击碎时连锁引爆周围砖 |
| 彩虹 `R` | 3 | 每次击中变色(青→紫→粉),击碎三色爆发 |
| 再生 `♻` | 1 | 击碎后 5 秒原位重生(倒计时幽灵框警示) |

### 道具(11 种)

| 道具 | 效果 |
| --- | --- |
| `W` 加宽 | 挡板宽度 +30%(可叠加至 2 倍,有上限) |
| `N` 缩窄 | 挡板宽度 -20% |
| `M` 多球 | 现有球分裂为 3 个(上限 9) |
| `S` 减速 | 球速 -15%(下限) |
| `F` 加速 | 球速 +10% |
| `L` 生命 | +1(上限 5) |
| `P` 穿透 | 球穿透砖块并摧毁路径砖 5 秒 |
| `SH` 护盾 | 底部缓冲,护盾耗尽前不掉球 |
| `LA` 激光 | 挡板双管发射激光,摧毁首个接触的砖 2.2 秒冷却 |
| `ST` 粘球 | 球吸附挡板,自由瞄准后发射 |
| `MG` 巨球 | 球半径 +50%,更易命中 |

### 计分

- 击砖得分 × 连击倍率(4 秒内连续命中递增,最高 ×5)
- 连击升级时全屏爆发 + 飘字;高连击屏幕边缘辉光
- 道具下落接住得分;失球清空连击

---

## 界面与设置

- **主菜单**:开始游戏 / 选择关卡 / 设置 / 退出
- **关卡选择**:5 关任意选择,显示各关球速与挡板宽度
- **设置对话框**:语言(中文/English)、主题(浅/深/跟随系统)、球拖尾、粒子特效、球色(8 色板)
- **HUD**:分数、连击、生命、关卡、当前生效道具标签
- **overlay**:准备(发球提示)、暂停、过关、失败,均支持键盘/点击交互

---

## 构建与运行

### 测试参数

```bash
# 截取画面(0.5s 后保存 BMP 退出):菜单 / 第 N 关 ready / 发球后 playing
./build/neon-breaker --screenshot shots/menu.bmp
./build/neon-breaker --level 3 --screenshot shots/level3.bmp
./build/neon-breaker --level 5 --auto --screenshot shots/play5.bmp
# 模拟窗口缩放后截图
./build/neon-breaker --resize 1280x800 --screenshot shots/resize.bmp
# 粘球发射方向计算自检(无头,回归用)
./build/neon-breaker --aim-test
# 无头冒烟(SDL dummy 驱动)
SDL_VIDEODRIVER=dummy ./build/neon-breaker --level 3 --auto --screenshot shots/x.bmp
```

### 平台支持矩阵

| 平台 | 状态 | 构建方式 | 验证程度 |
| --- | --- | --- | --- |
| Linux(X11/Wayland) | ✅ 可用 | `cmake -B build && cmake --build build`(自带 vendor/) | 完整运行验证 |
| Windows 10/11 | ✅ 可用 | 原生 MSVC + CMake(VS 生成器;依赖用 `tools/fetch-win-deps.ps1` 下载官方 VC 开发包,或 vcpkg) | 本机零警告构建 + 自检 + 截图 |
| macOS 11+ | ⚠️ 配置就绪 | Homebrew `sdl3 sdl3_ttf` + CMake → `docs/platform/macos.md` | 未实测(需 macOS) |
| Android 10+(API 29) | ⚠️ 构建链就绪 | `tools/android/build-android.sh` → `android-project/` Gradle 组装 | libmain.so 交叉编译验证 |
| iOS 13+ | ⚠️ 配置就绪 | SDL3 Xcode 工程 → `docs/platform/ios.md` | 未实测(需 macOS/Xcode) |

⚠️ 项需对应平台实测;移动端输入/生命周期差异均已适配(见 `PROGRESS.md` 核对表)。

### 持续交付(CD)

`.github/workflows/release.yml` — 推送 `v*` tag(或手动 `workflow_dispatch`)时自动打包五平台产物并发布 GitHub Release:

| Job | 产物 | 内容 |
| --- | --- | --- |
| Linux | `neon-breaker-linux-x86_64.tar.gz` | 可执行 + SDL 运行时库(保持 vendor 布局匹配 RUNPATH)+ 字体,解压即跑 |
| Windows | `neon-breaker-windows-x64.zip` | exe + SDL3.dll/SDL3_ttf.dll(vcpkg applocal)+ 字体 |
| macOS | `neon-breaker-macos-arm64.zip` | 可执行 + Homebrew dylib 传递闭包(`@loader_path` 重定位)+ 字体 |
| Android | `neon-breaker-android-arm64.apk` | NDK 交叉编译 + Gradle 组装 Debug APK |
| iOS | `neon-breaker-ios-simulator.zip` / `neon-breaker-unsigned.ipa` | 模拟器 .app(arm64+x86_64)+ **真机未签名 IPA**(iphoneos arm64,`CODE_SIGNING_ALLOWED=NO`,Payload 结构,开发者可自行重签名) |

发布 job 汇总全部产物,`gh release create`(tag 已存在则 `--clobber` 覆盖上传),自动生成 release notes。手动触发时生成 `nightly-<时间戳>` release。

### 各平台构建

```bash
# Linux(见上方"快速开始")

# Windows(原生 MSVC;需 Visual Studio 2022/2026 + Windows SDK)
# 依赖二选一:vcpkg install sdl3 sdl3-ttf,或下载官方 VC 开发包到 tools/win-deps/vc/
powershell -ExecutionPolicy Bypass -File tools/fetch-win-deps.ps1
cmake -S . -B build -G "Visual Studio 18 2026" -A x64   # VS 2022 用 "Visual Studio 17 2022"
cmake --build build --config Release
# 产物 build/Release/neon-breaker.exe;SDL3.dll / SDL3_ttf.dll 与字体由构建后步骤自动复制
# 网络受限时:powershell -File tools/fetch-win-deps.ps1 -Proxy http://127.0.0.1:7897

# macOS:见 docs/platform/macos.md
# Android:tools/android/build-android.sh(依赖 tools/android/fetch-deps.sh 获取)→ android-project/(见 android-project/README.md)
# iOS:见 docs/platform/ios.md
```

### 跨平台要点

- **字体**:内置子集字体 `assets/fonts/NotoSansSC-Bold-subset.otf`(113KB,`tools/make_font.py` 生成,含空格与 `＊`/`♻` 等符号)优先;桌面回退系统字体(Windows 微软雅黑 / macOS 苹方 / Linux Noto CJK)
- **MSVC 选项**:`/utf-8`(源码 UTF-8 无 BOM,缺省按系统 ANSI 代码页解析会乱码/报错)、`/EHsc`(异常展开)由 `CMakeLists.txt` 统一添加
- **存档**:`SDL_GetPrefPath` 各平台自动定位
- **触屏 / 返回键 / 切后台**:SDL3 事件统一处理,已适配
- **窗口**:桌面等比缩放 letterbox,最小 640×420;移动端全屏 + 横屏锁(工程层配置)

---

## 项目结构

```
CMakeLists.txt            平台分支构建(Linux vendor / Windows find_package / Android 子构建)
vendor/                   SDL3 + SDL3_ttf 本地依赖(Linux,来自 Debian 官方包)
assets/fonts/             内置子集字体
android-project/          Android Gradle 工程(预编译 so 模式)
src/
  main.cpp                入口:装配、事件循环、每帧 step+渲染、测试参数
  game.h/cpp              状态机 / 物理 / 计分 / 道具 / 粘球瞄准
  renderer.h/cpp          SDL 渲染:渐变 / 圆角 / 文本缓存 / 霓虹特效 / 窗口缩放
  input.h/cpp             键盘 / 鼠标 / 触屏 → InputActions
  audio.h/cpp             音效 PCM 合成 + SDL 音频流
  fx.h/cpp                粒子 / 碎片 / 飘字 / 圆环 / 闪光 / 屏震
  ui.h/cpp                按钮系统 / HUD / overlay / 菜单 / 选关 / 设置对话框
  storage.h/cpp           存档 KV 文件读写
  types.h / constants.h / i18n.h / levels.h
tools/
  make_font.py            内置字体子集化脚本(--src 指定源字体;需 fontTools)
  fetch-win-deps.ps1      Windows:下载 SDL3/SDL3_ttf 官方 MSVC 开发包到 tools/win-deps/vc/
  copy-runtime-dlls.cmake Windows:构建后把 SDL 运行库复制到 exe 同目录
  mingw-toolchain.cmake   mingw-w64 交叉编译工具链(非主路径,保留备用)
  android/                fetch-deps.sh(依赖获取)+ build-android.sh(NDK 交叉编译)
docs/                     平台文档(docs/platform/)、任务归档(docs/tasks/archive/)
AGENTS.md                 工作规则(任务/状态文件/归档/跨平台验证)
```

---

## 开发与验证

```bash
# 完整验证序列(提交前)
cmake --build build -j                # 零警告
./build/neon-breaker --aim-test       # 粘球瞄准自检: ALL PASS
SDL_VIDEODRIVER=dummy ./build/neon-breaker --level 5 --auto --screenshot shots/x.bmp  # 无头截图回归
```

- 构建要求零警告(`-Wall -Wextra`;MSVC 下为 `/W4`)
- 截图回归:任意改动后跑 `--screenshot` 三个状态(菜单 / ready / playing)人工比对
- 字体子集如需扩展字符集:改 `tools/make_font.py` 后重新生成(需系统 Noto CJK 字体)

### 持续集成(CI)

`.github/workflows/ci.yml` — 推送到 `main` 或开 PR 时自动触发,五个 job 并行:

| Job | Runner | 依赖 | 验证 |
| --- | --- | --- | --- |
| Linux | `ubuntu-latest` | `vendor/` 自带 + apt 运行时库 | 构建零警告 → `--aim-test` → dummy 截图 |
| macOS | `macos-latest` | Homebrew `sdl3 sdl3_ttf` | 同上 |
| Windows | `windows-latest` | vcpkg(`vcpkg.json`: `sdl3` `sdl3-ttf`) | 同上(`--config Release`) |
| Android (libmain.so) | `ubuntu-latest` | `fetch-deps.sh`(NDK + SDL 源码,actions/cache 缓存) | NDK 交叉编译零警告 → 上传 libmain.so |
| Android (APK) | `ubuntu-latest` | 依赖上一 job 的 libmain.so + Android SDK(platform 35 / build-tools) | Gradle `assembleDebug` → 上传 APK |
| iOS | `macos-latest` | `fetch-deps.sh --no-ndk`(SDL 源码) | CMake Xcode 模拟器交叉编译(`CMAKE_SYSTEM_NAME=iOS`)→ .app 产物验证 |

桌面三平台冒烟截图上传为 artifact(`smoke-<平台>.bmp`);Android 产物上传 `libmain.so` 与 `app-debug.apk`。本地预览可用 act:`act -j build-test --matrix "os:ubuntu-latest"`(Android job 亦可:`act -j android-build` / `act -j android-apk`)。

---

## 已知限制

- 非整数倍窗口缩放有轻微插值模糊(桌面)
- 特效默认全开(系统 reduced-motion 设置不可读),可在设置中关闭粒子特效
- 球色为 8 色预设色板
- Android APK 组装与 iOS 构建需对应平台工具链(Android Studio / Xcode)
- macOS / iOS 构建文档未在本环境实测,字体路径与工程配置需真机确认

---

## 许可

本项目未指定开源许可证(保留所有权利)。依赖:SDL3 / SDL3_ttf(zlib 许可)、内置字体子集源自 Noto Sans CJK(SIL OFL 1.1)。
