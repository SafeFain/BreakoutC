# AGENTS.md — Neon Breaker(霓虹破界)

本文件是项目长期事实与工作规则的权威来源,供任何代码代理(Agent)读取。内容经与 `README.md`、源码逐项核实,非推断。

## 项目概况

经典打砖块(breakout)游戏,C++20 + SDL3 桌面版本。历史上有过 TypeScript(web)实现,已于 2026-08-06 移除(备份 `/tmp/breakout-web-backup-20260806.tar.gz`),当前为单实现项目。

## 主要特性

- 5 个关卡、7 种砖块:普通 / 坚固(3 血) / 不可摧毁 / 奖励 / 爆炸 / 彩虹(3 血变色) / 再生(5 秒重生)
- 11 种道具:加宽 / 缩窄 / 多球 / 减速 / 加速 / 生命 / 穿透 / 护盾 / 激光 / 粘球 / 巨球
- 连击倍率系统(combo,每 5 连击 +0.5 倍,上限 ×5,3.5 秒超时)
- 粘球自由方向发射:球→鼠标连线为发射方向(最小仰角防穿板,`--aim-test` 可自检)
- 特效系统:粒子 / 砖块碎片 / 得分飘字 / 扩散圆环 / 全屏闪光 / 屏幕震动 / 动态背景光点 / 连击边缘辉光
- 12 种程序化合成音效(无外部资源)
- 主菜单(开始 / 关卡选择 / 设置)、设置对话框(语言 / 主题 / 拖尾 / 粒子特效 / 球色)
- 存档持久化(KV 文本文件)、中英双语、浅色 / 深色 / 跟随系统主题
- 输入:键盘 / 鼠标 / 触屏;窗口自由缩放(等比 letterbox,最小 640×420)
- 窗口失焦 / 最小化自动暂停

## 技术栈

| 版本 | 技术 |
| --- | --- |
| 桌面/移动 | C++20,SDL3(3.2.10,Linux 本地 vendor/,其他平台系统依赖或交叉源码),SDL3_ttf(3.2.2) |
| 平台 | Linux(X11/Wayland)✅ / Windows 10/11 ✅(原生 MSVC 验证) / macOS 11+ ⚠️ / Android 10+ ⚠️构建链 / iOS 13+ ⚠️ |
| 已移除 | TypeScript + Vite + Canvas(2026-08-06,备份于 /tmp,勿恢复为维护对象) |

- Linux 依赖本地化在 `vendor/`(Debian 官方包解包),**不修改、不升级**
- 交叉编译依赖源码在 `tools/android/`(SDL3/SDL3_ttf/freetype 源码 + NDK,由 `tools/android/build-android.sh` 使用);Windows 依赖为 `tools/win-deps/vc/`(官方 MSVC 开发包,`tools/fetch-win-deps.ps1` 下载;`tools/win-deps/*mingw*` 为旧交叉编译包)

## 目录结构

```
BreakOut/
├── AGENTS.md            长期项目事实与工作规则(本文件)
├── PLAN.md              当前任务计划
├── PROGRESS.md          当前任务进度(含整理/迁移记录)
├── TASKS.md             任务索引(Active/Completed)
├── docs/tasks/archive/  已完成任务归档
├── docs/platform/       平台构建指南(macos.md / ios.md)
├── CMakeLists.txt       构建配置(平台分支:Linux vendor/、Win/mac find_package、Android 专用)
├── vcpkg.json           Windows(vcpkg)依赖清单:sdl3 / sdl3-ttf
├── .github/workflows/ci.yml      五平台 CI(构建+自检+截图冒烟)
├── .github/workflows/release.yml CD(tag v* 触发,五平台产物打包 + GitHub Release)
├── README.md            用户文档:构建/运行/平台矩阵
├── src/                 C++ 源码(见模块职责)
├── assets/fonts/        内置子集字体(tools/make_font.py 生成,勿手改)
├── tools/               工具与交叉依赖(Windows 依赖获取/字体生成/Android NDK+源码)
├── android-project/     Android Gradle 工程(预编译 so 模式)
├── vendor/              SDL3 + SDL3_ttf 本地依赖(Linux 专用,只读)
├── build/               构建产物(可删除重建;Windows 为 build/msvc-vs,Android 输出 build-android/)
└── shots/               截图测试产物(可删除)
```

## 模块职责(src/)

| 文件 | 职责 |
| --- | --- |
| `main.cpp` | 装配、事件循环、120Hz step、测试参数(`--screenshot`/`--level`/`--auto`/`--resize`/`--aim-test`) |
| `game.h/cpp` | 状态机、物理、砖块、道具、计分、特效触发、粘球瞄准 |
| `renderer.h/cpp` | SDL 渲染、文本缓存、窗口缩放变换、霓虹特效 |
| `input.h/cpp` | 键盘/鼠标/触屏 → `InputActions`(含指针 X/Y 世界坐标) |
| `audio.h/cpp` | 12 音效 PCM 合成 + SDL 音频流 |
| `fx.h/cpp` | 粒子/碎片/飘字/圆环/闪光/屏震/开关 |
| `ui.h/cpp` | 按钮系统/HUD/overlay/菜单/选关/设置对话框 |
| `storage.h/cpp` | 存档 KV 读写(字段级校验) |
| `types.h` / `constants.h` / `i18n.h` / `levels.h` | 类型 / 常量 / 中英文案 / 关卡数据 |

## 开发命令

Linux(当前以 Windows 为开发机,Linux 回归在 CI/容器执行):

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release   # 配置(自动找 vendor/)
cmake --build build -j                      # 构建(要求零错误零警告)
./build/neon-breaker                        # 运行
./build/neon-breaker --aim-test             # 粘球方向自检(无头)
./build/neon-breaker --screenshot shots/m.bmp            # 截菜单图后退出
./build/neon-breaker --level 3 --auto --screenshot s.bmp # 选第 3 关并发球后截图
./build/neon-breaker --resize 1280x800 --screenshot s.bmp # 模拟窗口缩放后截图
SDL_VIDEODRIVER=dummy ./build/neon-breaker ...           # 无头冒烟测试
```

Windows(原生 MSVC;依赖在 `tools/win-deps/vc/`,由脚本下载,CMake 自动加入搜索路径):

```powershell
powershell -ExecutionPolicy Bypass -File tools/fetch-win-deps.ps1 [-Proxy http://127.0.0.1:7897]
cmake -S . -B build -G "Visual Studio 18 2026" -A x64   # 或 -G Ninja(见下方注意事项)
cmake --build build --config Release                      # 要求零错误零警告
build\Release\neon-breaker.exe --aim-test
build\Release\neon-breaker.exe --level 3 --auto --screenshot shot.bmp
$env:SDL_VIDEODRIVER='dummy'; build\Release\neon-breaker.exe --screenshot shot.bmp
```

- MSVC 的 `/W4 /utf-8 /EHsc` 由 `CMakeLists.txt` 统一添加(源码 UTF-8 无 BOM,缺 `/utf-8` 在中文系统会编译失败)
- 构建后自动复制 `SDL3.dll` / `SDL3_ttf.dll` 与字体到产物目录
- 本机 VS 生成器验证通过;Ninja 生成器在受限 shell 下曾出现卡死,若本机可用可自行选用

## 架构原则(不得破坏)

1. **单向数据流**:输入 → `Game::step`(固定步长 1/120s 累加器)→ `emit()` 快照 → UI 层消费快照渲染;UI 不反向修改游戏状态
2. **显式状态机**:`Menu / Ready / Playing / Paused / LevelComplete / GameOver / Victory`
3. **逻辑与渲染解耦**:`Game` 不接触 SDL 渲染细节,通过 getter 暴露渲染数据;`WorldRenderArgs` 一次传入
4. **快照驱动 UI**:`ui.setSnapshot()` 每帧重建按钮/文本
5. **窗口缩放**:逻辑坐标恒为 960×700,`Renderer::onWindowResized` 设置 scale/viewport;输入坐标经同一 transform 反向换算
6. **SDL 资源生命周期**:纹理/字体/音频流必须在 `SDL_DestroyRenderer`/`TTF_Quit`/`SDL_Quit` 之前释放(通过 `Renderer::shutdown`/`AudioManager::shutdown` 显式调用)
7. **平台分支规范**:平台差异用条件编译集中处理——字体路径(`renderer.cpp`)、窗口标志/返回键(`main.cpp`/`input.cpp`/`ui.cpp`)、CMake 平台分支(`CMakeLists.txt`);新增平台行为一律进对应平台分支,禁止散落 `#ifdef`
8. **移动端适配基准**:触屏(方向瞄准)、UI 点击(触摸模拟鼠标)、切后台暂停、返回键(AC_BACK)、全屏、内置字体为移动端最低要求(见 `PROGRESS.md` 核对表)

## 修改边界

- **`vendor/` 只读**:不修改、不删除、不升级(Linux 专用依赖)
- **`src/` 模块职责不可混淆**:游戏逻辑进 `game.cpp`,绘制进 `renderer.cpp`,UI 进 `ui.cpp`
- **`main.cpp` 的测试参数保留**:`--screenshot` / `--level` / `--auto` / `--resize` / `--aim-test` 是冒烟测试基础设施,新增参数不得破坏既有行为
- **`assets/fonts/` 勿手改**:由 `tools/make_font.py` 重新生成(字符集变化时重跑)
- **`android-project/` 为预编译 so 模式**:`libmain.so` 由 `tools/android/build-android.sh` 产出后放入 `app/libs/<abi>/`;不启用 Gradle externalNativeBuild
- **交叉依赖源码**(`tools/android/`、`tools/win-deps/`):仅构建用途,版本固定(SDL3 3.2.10 / SDL3_ttf 3.2.2 / NDK r27c),不修改;`tools/win-deps/vc/` 由 `tools/fetch-win-deps.ps1` 下载,`tools/fonts-src/` 为字体生成源,二者均被 `.gitignore` 忽略
- **CI 基础设施**(`.github/workflows/ci.yml`、`vcpkg.json`):依赖版本与安装步骤变更须保持三平台可构建(见验证规则 8);`vcpkg.json` 仅 Windows 生效,不影响 Linux/macOS 构建
- `build/`、`build-win/`、`build-android/`、`shots/` 为可再生物,可删除
- `.reasonix/` 为工具配置目录,不修改
- 已移除的 web 版不得恢复为维护对象(备份仅存档用途)

## 编码规范

- C++20;`-Wall -Wextra` 零警告;MSVC 下为 `/W4 /utf-8 /EHsc`(由 CMake 统一添加),要求同样零警告
- **`std::vector` 引用失效**:元素被 `erase` 后引用/迭代器失效;遍历中删除须按 id 重查或先拷贝(见 `game.cpp` 的 `hitBrick`/`explode`)
- 源文件 UTF-8;非 ASCII 文本字面量可用转义(`\xe2\x86\xbb` 等),避免依赖编译器默认编码
- 新增 UI 符号须确认内置字体有对应字形(Noto Sans CJK 不含 `✦`/`↻`/`⚫` 等),否则会渲染成豆腐块;改字符集后重跑 `tools/make_font.py`(会同时扫描源码字面量与 `\xNN` 转义字符)
- 颜色值以 `#rrggbb` / `rgba(r,g,b,a)` 字符串存储与解析
- 纯逻辑(如方向计算)优先做成静态可测函数,并配 `--*-test` 无头自检

## 验证规则(改动后必须)

1. `cmake --build build -j`(Linux)/ `cmake --build build --config Release`(Windows)零错误零警告
2. 涉及纯逻辑:对应自检(`--aim-test` 等)通过
3. `SDL_VIDEODRIVER=dummy ./build/neon-breaker --screenshot shots/x.bmp` 退出码 0 且生成 BMP(Windows 用 `$env:SDL_VIDEODRIVER='dummy'; build\Release\neon-breaker.exe ...`)
4. 涉及玩法/UI 的改动:真实显示(DISPLAY 可用时)运行或截图验证
5. 涉及存档:检查 `~/.local/share/NeonBreaker/settings/settings.kv` 读写(Windows:`%APPDATA%\NeonBreaker\settings\settings.kv`)
6. 涉及输入:验证键盘(空格/Esc/R/A/D/方向键)与鼠标路径
7. **跨平台改动**:平台分支代码必须保持其余平台可编译——桌面改动至少跑 Windows 原生 MSVC 构建(本机)与 Linux 回归(CI/容器);触碰移动端分支时用 `tools/android/build-android.sh` 交叉编译验证;旧 mingw 交叉编译路径(`tools/mingw-toolchain.cmake`)保留但非主验证路径
8. **CI 一致性**:`.github/workflows/ci.yml` 自动覆盖全平台——桌面三平台(Linux/macOS/Windows)构建+自检+截图冒烟、Android(NDK 交叉编译 + Gradle APK)、iOS(模拟器交叉编译);涉及依赖/构建链的改动须保证 CI 通过(本地可用 act 预览:`act -j build-test --matrix "os:ubuntu-latest"`、`act -j android-build`、`act -j android-apk`);`vcpkg.json` 依赖变更须经全平台验证

## 工作规则

- **禁止自动提交**:任何 git 写入操作(`commit` / `push` / `merge` / `reset` 等)必须经用户明确要求或批准,禁止在任务完成时自动提交;用户未指示提交时,变更只保留在工作区,并在回复中说明"变更未提交";用户要求提交时,先展示 `git status` 变更摘要与拟用提交信息,提交信息按用户原话或惯例撰写
- **状态文件**:`AGENTS.md`(长期事实)、`PLAN.md`(当前任务)、`PROGRESS.md`(当前进度)、`TASKS.md`(任务索引)在任务开始/完成/中断时更新,保持一致
- **归档**:任务完成后写入 `docs/tasks/archive/YYYY-MM-DD-名称.md`,`TASKS.md` 维护索引;不要默认加载全部归档
- **历史信息**:已废弃说明不得重新提升为现行规则;确需保留时标注"已废弃"
- **未记录事项**:本项目已建立本地 Git 仓库(初始提交 2026-08-06,`git log` 可查);状态文件与归档不引用具体提交哈希(提交会随仓库演进),但可引用提交时间;删除性操作前先确认已提交或备份
- **中断恢复**:会话重启后先读 `PLAN.md` + `PROGRESS.md`,再继续未完成步骤
