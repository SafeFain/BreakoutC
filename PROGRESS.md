# PROGRESS.md — 当前进度

更新时机:每个步骤完成/中断时更新;恢复会话时先读本文件与 `PLAN.md`。

## 当前任务

**无进行中任务**(上一任务:Windows 开发环境迁移修复,已于 2026-10-17 完成并归档)。

## 上一任务:Windows 开发环境迁移修复(已完成)

| # | 步骤 | 状态 | 说明 |
| --- | --- | --- | --- |
| 1 | 环境盘点 | ✅ | VS 18 / MSVC 19.51 / CMake 4.3.1 / Ninja 1.13.2;无 vcpkg、无 MinGW;Clash 代理 7897 可用 |
| 2 | MSVC 构建链 | ✅ | `/W4 /utf-8 /EHsc`;自动查找 `tools/win-deps/vc/`;构建后复制 SDL DLL + 字体 |
| 3 | 警告清零 | ✅ | `sscanf` → strtol/strtof;`TTF_SetFontSize` 传 float;0 warning / 0 error |
| 4 | 内置字体 | ✅ | 补空格与 `\xNN` 转义字符(578 字符/113KB);`✦`/`↻`/`⚫` → `＊`/`♻`/`×` |
| 5 | Git 卫生 | ✅ | `.gitattributes`;`core.filemode=false`;vendor 链接占位恢复,status 干净 |
| 6 | 工具/文档 | ✅ | `tools/fetch-win-deps.ps1`、`tools/copy-runtime-dlls.cmake`;README/AGENTS 同步 |
| 7 | 清理与验证 | ✅ | 删 `build-win/`、`build-android/`、`.act-tools/`(约 1GB);构建/自检/截图全绿 |

**本次修改文件**:`CMakeLists.txt`、`src/{main,renderer,ui}.cpp`、`tools/{make_font.py,fetch-win-deps.ps1,copy-runtime-dlls.cmake}`、`assets/fonts/NotoSansSC-Bold-subset.otf`、`.gitattributes`、`.gitignore`、`README.md`、`AGENTS.md`、`PLAN.md`、`TASKS.md`、`PROGRESS.md`、`docs/tasks/archive/2026-10-17-windows-migration.md`

**遗留**:Linux 回归待 CI(本机无 g++/WSL);Android 需 windows-x86_64 NDK;macOS/iOS 未实测。

## 上一任务:自动 CD 发布流水线(已完成)

| # | 步骤 | 状态 | 说明 |
| --- | --- | --- | --- |
| 1 | release.yml 编写 | ✅ | tag v* + workflow_dispatch 触发;linux/windows/macos/android/ios 五打包 job + release 发布 job |
| 2 | Linux 打包验证 | ✅ | act 端到端(打包 597ms)+ 本地模拟:RUNPATH 相对布局、aim-test、截图、tar.gz 3.7MB |
| 3 | 其余平台打包 | ✅ | CI 已验证编译链路复用(macOS dylib 闭包重定位、Android APK、iOS .app) |
| 4 | 文档同步 | ✅ | README CD 小节;AGENTS 目录;归档+TASKS |

**遗留**:首次 tag 推送触发真实 CD;iOS 真机需签名;act 限制 upload-artifact@v6(验证到打包为止)。

## 上一任务:CI 拓展至 Android 与 iOS(已完成)

| # | 步骤 | 状态 | 说明 |
| --- | --- | --- | --- |
| 1 | android-build job | ✅ | act 端到端全绿:NDK 交叉编译零警告、libmain.so 上传、613MB 依赖缓存 |
| 2 | android-apk job | ✅ | act 端到端全绿:JDK/SDK 自装兼容、Gradle assembleDebug、APK 6.0MB 上传 |
| 3 | ios-build job | ⚠️ 配置就绪 | macOS runner 模拟器交叉编译(CMakeLists iOS 分支 + Info.plist),待真实运行 |
| 4 | 配套改动 | ✅ | fetch-deps.sh --no-ndk;CMakeLists iOS 分支;assets/ios/Info.plist |
| 5 | 文档同步 | ✅ | README 5 job 矩阵;AGENTS 验证规则 8;归档+TASKS |

**遗留**:ios-build 首次真实运行待 GitHub runner(macOS);iOS 仅编译期验证。

## 上一任务:配置 GitHub Actions CI(已完成)

| # | 步骤 | 状态 | 说明 |
| --- | --- | --- | --- |
| 1 | workflow 设计 | ✅ | push(main)+PR 触发,三平台矩阵,每平台 6 步(依赖→配置→构建→aim-test→截图→artifact) |
| 2 | 平台依赖 | ✅ | Linux apt(20 包+cmake/g++,act 实测修 `libsndio7`→`libsndio7.0`);macOS brew;Windows vcpkg |
| 3 | MSVC 兼容 | ✅ | CMakeLists `/W4`;源码检查无 M_PI/strcasecmp 等不兼容项 |
| 4 | act 端到端验证 | ✅ | Linux job 6 步全 Success,截图 state=2,artifact 上传成功 |
| 5 | 文档同步 | ✅ | README CI 小节;AGENTS 目录/验证规则 8/修改边界;归档+TASKS |

**遗留**:macOS/Windows job 首次真实运行需推送 GitHub(本地 act 仅支持 Linux runner);仓库尚无远端。

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

- Linux:`cmake --build build -j8` 零警告;`--aim-test` ALL PASS;dummy 截图回归(state=2)(2026-08-06 记录)
- Windows(2026-10-17,原生 MSVC):配置 26s、构建 54s、零警告零错误;`--aim-test` ALL PASS;dummy/真实窗口截图正常;字体/空格/符号渲染正常;存档 7 字段写入/读回一致并经真实启动应用(经 junction 验证)
- Windows(2026-10-17,mingw 交叉,历史):零警告,`build-win/neon-breaker.exe` 706KB(产物已清理)
- Android:交叉编译零警告,`build-android/libmain.so` 18.4MB(arm64-v8a,ELF for Android 29,readelf 仅系统库)(产物已清理)
- wine 冒烟:容器 `/run/user/1000` 只读,wineserver 无法启动(环境限制)

## 修改过的文件

`src/{renderer,input,ui,main,game}.{h,cpp}`、`CMakeLists.txt`、`tools/{make_font.py,mingw-toolchain.cmake,android/build-android.sh}`、`assets/fonts/NotoSansSC-Bold-subset.otf`、`android-project/`(模板改造+README)、`docs/platform/{macos,ios}.md`、`docs/tasks/archive/2026-08-06-cross-platform.md`、`README.md`、`AGENTS.md`、`TASKS.md`、`PLAN.md`

## 当前错误/阻塞

无阻塞。遗留人工项:Linux 回归(待 CI/原 Linux 机器)、Android 需 windows-x86_64 NDK、macOS/iOS 实测、wine 冒烟(环境限制)。

## 整理/迁移记录(2026-08-06)

- 状态文件体系按 1.txt 规范建立(CLAUDE.md 不存在,事实来源为代码+README);此后每个任务完成后同步归档与索引
- 跨平台任务归档见 `docs/tasks/archive/2026-08-06-cross-platform.md`

## 迁移记录(2026-10-17,Linux → Windows)

- 仓库以整目录复制搬迁(非 clone):`vendor/` 符号链接实体化、可执行位丢失、`core.autocrlf` 未配置;
  已按 Windows 正常检出状态恢复(链接占位 + `core.filemode=false` + `.gitattributes`)
- 开发构建改为原生 MSVC + `tools/win-deps/vc/` 官方开发包;mingw 交叉路径降级为备用
- 迁移垃圾已清理:`build-win/`、`build-android/`、`.act-tools/`、`build/` 下 Linux 产物(约 1GB)
- 归档见 `docs/tasks/archive/2026-10-17-windows-migration.md`
