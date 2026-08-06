# PROGRESS.md — 当前进度

更新时机:每个步骤完成/中断时更新;恢复会话时先读本文件与 `PLAN.md`。

## 当前任务

**无进行中任务**(上一任务:跨平台兼容性拓展,已于 2026-08-06 完成并归档)。

## 上一任务:跨平台兼容性拓展(已完成)

| # | 步骤 | 状态 | 说明 |
| --- | --- | --- | --- |
| 1 | 字体候选平台分支 | ✅ | Windows(微软雅黑/黑体)/macOS(苹方/STHeiti)/Linux(原有)/移动端(内置) |
| 2 | 内置子集字体 | ✅ | 552 字符 107KB(`tools/make_font.py` 生成),字形检视正常 |
| 3 | 移动端 main 修正 | ✅ | 全屏标志分支 + Android 返回键(AC_BACK) |
| 4 | Linux 验证 | ✅ | 构建零警告 + aim-test + 截图回归 |
| 5 | CMake 平台分支 | ✅ | Linux vendor/;Win/mac find_package;Android 专用;rpath 仅 Linux |
| 6 | Windows mingw 验证 | ✅ | exe 706KB 零警告;wine 受容器限制未跑 |
| 7 | macOS 文档 | ✅ | `docs/platform/macos.md`(未实测标注) |
| 8 | Android 交叉编译 | ✅ | libmain.so 18.4MB 零警告(仅系统库依赖)+ android-project 模板 |
| 9 | iOS 配置 | ✅ | `docs/platform/ios.md`(Xcode 工程路径核实) |
| 10 | 移动端输入/生命周期核对 | ✅ | 八项全部代码就绪(下表) |
| 11 | README/AGENTS 平台更新 | ✅ | 平台矩阵/构建指南/平台分支规范 |
| 12 | 归档与状态文件 | ✅ | 归档 + TASKS/PLAN/PROGRESS 同步 |

## 移动端输入/生命周期核对表(代码依据)

| 项 | 状态 | 依据 |
| --- | --- | --- |
| 触屏方向瞄准(粘球) | ✅ | `input.cpp` FINGER 事件记录指针 X/Y;`game.cpp` computeLaunchDirection |
| UI 点击(触摸模拟鼠标) | ✅ | `ui.cpp` 监听 `SDL_EVENT_MOUSE_BUTTON_DOWN` |
| 切后台自动暂停 | ✅ | `main.cpp` `SDL_EVENT_WINDOW_FOCUS_LOST` → autoPause |
| 返回键(Android) | ✅ | `input.cpp`/`ui.cpp` `SDL_SCANCODE_AC_BACK` 等效 Esc |
| 存档路径 | ✅ | `storage.cpp` `SDL_GetPrefPath`(平台自动) |
| 全屏窗口 | ✅ | `main.cpp` `__ANDROID__`/`__IPHONEOS__` → FULLSCREEN |
| 内置字体 | ✅ | `renderer.cpp`:Android=APK assets 相对路径;iOS=bundle `fonts/`;桌面=basePath+assets/fonts/ |
| 窗口缩放(移动端) | ✅ | `renderer.cpp` onWindowResized 等比 letterbox |

## 验证结果存档

- Linux:`cmake --build build -j8` 零警告;`--aim-test` ALL PASS;dummy 截图回归(state=2)
- Windows:交叉编译零警告,`build-win/neon-breaker.exe` 706KB
- Android:交叉编译零警告,`build-android/libmain.so` 18.4MB(arm64-v8a,ELF for Android 29,readelf 仅系统库)
- wine 冒烟:容器 `/run/user/1000` 只读,wineserver 无法启动(环境限制)

## 修改过的文件

`src/{renderer,input,ui,main,game}.{h,cpp}`、`CMakeLists.txt`、`tools/{make_font.py,mingw-toolchain.cmake,android/build-android.sh}`、`assets/fonts/NotoSansSC-Bold-subset.otf`、`android-project/`(模板改造+README)、`docs/platform/{macos,ios}.md`、`docs/tasks/archive/2026-08-06-cross-platform.md`、`README.md`、`AGENTS.md`、`TASKS.md`、`PLAN.md`

## 当前错误/阻塞

无。遗留人工项:macOS/iOS 实测、Android APK 组装与真机、wine 冒烟(环境限制)。

## 整理/迁移记录(2026-08-06)

- 状态文件体系按 1.txt 规范建立(CLAUDE.md 不存在,事实来源为代码+README);此后每个任务完成后同步归档与索引
- 跨平台任务归档见 `docs/tasks/archive/2026-08-06-cross-platform.md`
