# Neon Breaker · 霓虹破界

经典打砖块游戏 —— C++20 + SDL3 桌面版。

> 历史:项目曾包含 TypeScript(web)实现,已于 2026-08-06 移除(源码备份:`/tmp/breakout-web-backup-20260806.tar.gz`)。

## 功能

- **玩法**:5 个关卡、7 种砖块(普通/坚固/不可摧毁/奖励/爆炸/**彩虹**/**再生**)、11 种道具(加宽/缩窄/多球/减速/加速/生命/穿透/护盾/**激光**/**粘球**/**巨球**)、连击倍率、120Hz 固定步长物理
- **粘球自由方向**:粘球吸附时,鼠标位置与球连线即为发射方向(虚线瞄准线提示,含最小仰角防穿板)
- **界面**:主菜单(开始/关卡选择/设置)、选关面板(5 关任意选)、设置对话框(语言/主题/拖尾/粒子特效/球色)、HUD、overlay 状态遮罩、过关 toast
- **特效**:霓虹粒子、砖块碎片、得分飘字、扩散光环、全屏闪光、屏幕震动、动态背景光点、连击边缘辉光
- **音效**:12 种程序化合成音效(SDL 音频流,无外部资源)
- **设置持久化**:`~/.local/share/NeonBreaker/settings/settings.kv`
- **输入**:键盘(A/D/方向键/空格/Esc/R)、鼠标、触屏(SDL 触摸事件)
- **主题**:浅色/深色/跟随系统(`SDL_GetSystemTheme`)
- **自动暂停**:窗口失焦/最小化
- **自由调节窗口大小**:任意缩放,界面等比缩放并居中(letterbox 黑边),最小 640×420

## 构建与运行

依赖已随项目提供(`vendor/`,SDL3 3.2.10 + SDL3_ttf 3.2.2,来自 Debian 官方包),无需系统安装。

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/neon-breaker
```

### 测试参数

```bash
# 截取画面(0.5s 后保存 BMP 退出):菜单 / 第 N 关 ready / 发球后 playing
./build/neon-breaker --screenshot shots/menu.bmp
./build/neon-breaker --level 3 --screenshot shots/level3.bmp
./build/neon-breaker --level 5 --auto --screenshot shots/play5.bmp
# 模拟窗口缩放后截图
./build/neon-breaker --resize 1280x800 --screenshot shots/resize.bmp
# 粘球发射方向计算自检(无头)
./build/neon-breaker --aim-test
# 无头冒烟(SDL dummy 驱动)
SDL_VIDEODRIVER=dummy ./build/neon-breaker --level 3 --auto --screenshot shots/x.bmp
```

## 源码结构

```
CMakeLists.txt
vendor/            SDL3 + SDL3_ttf 本地依赖(头文件/库/cmake 配置)
src/
  main.cpp         入口:装配、事件循环、每帧 step+渲染、测试参数(截图/aim-test/resize)
  game.h/cpp       状态机/物理/计分/道具/粘球瞄准逻辑
  renderer.h/cpp   SDL 渲染:渐变/圆角/文本缓存/霓虹特效/窗口缩放
  input.h/cpp      键盘/鼠标/触屏 → InputActions
  audio.h/cpp      音效 PCM 合成 + SDL 音频流
  fx.h/cpp         粒子/碎片/飘字/圆环/闪光/屏震
  ui.h/cpp         按钮系统/HUD/overlay/菜单/选关/设置对话框
  storage.h/cpp    存档 KV 文件读写
  types.h          类型定义
  constants.h      常量
  i18n.h           中英文案
  levels.h         关卡数据
docs/              任务归档与索引(TASKS.md / PLAN.md / PROGRESS.md / AGENTS.md 见工作规则)
```

## 平台支持矩阵

| 平台 | 状态 | 构建方式 | 验证程度 |
| --- | --- | --- | --- |
| Linux(X11/Wayland) | ✅ 可用 | `cmake -B build && cmake --build build`(自带 vendor/) | 本环境完整运行验证 |
| Windows 10/11 | ✅ 可构建 | mingw-w64 交叉编译(见 `tools/mingw-toolchain.cmake`)或 vcpkg(`sdl3` `sdl3-ttf`)+ MSVC | 交叉编译验证(零警告产出 exe) |
| macOS 11+ | ⚠️ 配置就绪 | Homebrew `sdl3 sdl3_ttf` + CMake(见 `docs/platform/macos.md`) | 未实测(需 macOS) |
| Android 10+(API 29) | ⚠️ 构建链就绪 | NDK 交叉编译 `libmain.so` + `android-project/` Gradle 组装 | libmain.so 交叉编译验证 |
| iOS 13+ | ⚠️ 配置就绪 | SDL3 Xcode 工程(见 `docs/platform/ios.md`) | 未实测(需 macOS/Xcode) |

⚠️ 项需对应平台实测;移动端输入/生命周期差异均已适配(见 `PROGRESS.md` 核对表)。

## 跨平台要点

- **字体**:内置子集字体 `assets/fonts/NotoSansSC-Bold-subset.otf`(107KB,`tools/make_font.py` 生成)优先;桌面回退系统字体(Windows 微软雅黑/macOS 苹方/Linux Noto CJK)
- **存档**:`SDL_GetPrefPath` 各平台自动定位
- **触屏/返回键/切后台**:SDL3 事件统一处理,已适配
- **桌面窗口**:等比缩放 letterbox,最小 640×420;移动端全屏 + 横屏锁(工程层配置)

## 各平台构建(详见对应文档)

```bash
# Linux
cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j && ./build/neon-breaker

# Windows(交叉编译,需 mingw-w64 + SDL3 mingw 开发包于 tools/win-deps/)
cmake -B build-win -DCMAKE_TOOLCHAIN_FILE=tools/mingw-toolchain.cmake \
  -DSDL3_DIR=tools/win-deps/SDL3-3.2.10/x86_64-w64-mingw32/lib/cmake/SDL3 \
  -DSDL3_ttf_DIR=tools/win-deps/SDL3_ttf-3.2.2/x86_64-w64-mingw32/lib/cmake/SDL3_ttf
cmake --build build-win -j   # 产物 build-win/neon-breaker.exe(运行时需 SDL3.dll/SDL3_ttf.dll 与 exe 同目录)

# macOS:见 docs/platform/macos.md
# Android:tools/android/build-android.sh → android-project/(见 android-project/README.md)
# iOS:见 docs/platform/ios.md
```

## 已知限制

- 非整数倍窗口缩放有轻微插值模糊(桌面)
- 特效默认全开(系统 reduced-motion 设置不可读),可在设置中关闭粒子特效
- 球色为 8 色预设色板
- Android APK 组装与 iOS 构建需对应平台工具链(Android Studio / Xcode)
