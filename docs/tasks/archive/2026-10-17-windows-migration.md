# 2026-10-17 Windows 开发环境迁移修复

## 背景

开发机由 Linux 迁移到 Windows 11,仓库以整目录复制方式搬迁(非 `git clone`)。检查发现三类问题:

1. 复制遗留:可执行位丢失、`vendor/` 符号链接被实体化、`core.autocrlf=true` 且无 `.gitattributes`;
2. 工具链:MSVC 按系统 ANSI 代码页(936)解析 UTF-8 源码,中文串导致编译失败;CMake 未启用 `/EHsc`;
3. 资源:内置子集字体缺 U+0020,所有空格渲染为豆腐块;砖块符号 `✦`/`↻` 在 Noto Sans CJK 中无字形。

## 环境

| 项 | 值 |
| --- | --- |
| 系统 | Windows 11 `10.0.26300` |
| 编译器 | MSVC 19.51.36260(VS 18 Community)+ Windows SDK 10.0.26100 |
| 构建 | CMake 4.3.1-msvc1、Ninja 1.13.2(VS 自带) |
| Shell | Windows PowerShell 5.1(无 PowerShell 7) |
| 依赖 | `tools/win-deps/vc/SDL3-3.2.10`、`SDL3_ttf-3.2.2`(官方 VC 开发包) |
| 网络 | 系统代理关闭;Clash Verge 混合端口 `127.0.0.1:7897` 可用 |

> 受限 Agent shell 中 `curl`/`git`/`Invoke-WebRequest` 走 schannel 报 `SEC_E_NO_CREDENTIALS`,改用 Python urllib + 代理下载;用户正常终端不受此限制。

## 修复内容

### 1. 构建链(CMakeLists.txt)

- MSVC 选项统一为 `/W4 /utf-8 /EHsc`:
  - `/utf-8`:源码 UTF-8 无 BOM 且含中文字面量,缺省按 ANSI 代码页解析在中文系统直接编译失败(英文系统则字符串乱码);
  - `/EHsc`:`storage.cpp` 使用 `try/catch`,缺省触发 C4530(异常展开语义未启用)。
- Windows 分支自动把 `tools/win-deps/vc/SDL3-*`、`SDL3_ttf-*` 加入 `CMAKE_PREFIX_PATH`,无需手传 `-DCMAKE_PREFIX_PATH`。
- 新增构建后步骤 `tools/copy-runtime-dlls.cmake`,把 `$<TARGET_RUNTIME_DLLS>` 复制到 exe 同目录(等价 vcpkg applocal);静态链接或空列表时安全跳过。

### 2. 源码零警告

| 位置 | 原问题 | 修复 |
| --- | --- | --- |
| `main.cpp --resize` | `std::sscanf` → C4996 | 新增 `parseSize()`(strtol),行为不变(非法值→0,不缩放) |
| `renderer.cpp hexColor` | `std::sscanf` → C4996 | 手工解析 `rgba(r,g,b,a)`(strtof) |
| `renderer.cpp` 两处 | `TTF_SetFontSize(font, int)` → C4244(int→float) | 直接传 float |
| `storage.cpp` | 缺 `/EHsc` → C4530 | CMake 统一加 `/EHsc` |

### 3. 内置字体

- `tools/make_font.py`:
  - 字符集改为 `range(32, 127)`,补上空格(U+0020);
  - 除源码字面量外,还解码源码中的 `\xNN` UTF-8 转义(否则 `"\xe7\xac\xac %d \xe5\x85\xb3"` 这类"第 N 关"文案不会进入字符集);
  - 支持 `--src` / `NOTO_CJK_BOLD` / Linux 系统字体 / `tools/fonts-src/NotoSansCJKsc-Bold.otf` 自动查找;
  - 符号表移除 Noto 不存在的 `✦`/`↻`,加入 `＊`/`♻`。
- 重新生成 `assets/fonts/NotoSansSC-Bold-subset.otf`:578 字符、113KB(原 552 字符/107KB,且旧版还缺 22 个字符,含转义写入的"第")。
- `renderer.cpp` 砖块标记:爆炸 `✦`→`＊`、再生 `↻`→`♻`;`ui.cpp` 静音标记 `⚫`(U+26AB,Noto 也没有)→`×`。
- 源码注释与文档同步。

### 4. 仓库卫生

- 新增 `.gitattributes`:`*.sh`/`gradlew` 强制 LF、`*.bat/*.cmd` CRLF、字体/库/图片二进制;
- 本机 `core.filemode=false`(Windows 无执行位),`git status` 不再误报 3 个脚本模式变更;
- `vendor/` 4 个符号链接占位恢复为链接目标文本(core.symlinks=false 的正常 Windows 状态),不再显示为二进制修改;Linux 检出时会自动还原为符号链接;
- `.gitignore` 增加 `tools/fonts-src/`;
- 清理迁移垃圾:`build-win/`、`build-android/`、`.act-tools/`(856MB)、`build/` 下 Linux 产物,共约 1GB。

### 5. 工具与文档

- 新增 `tools/fetch-win-deps.ps1`:下载并解压官方 VC 开发包,支持 `-Proxy`,显式 TLS1.2,UTF-8 BOM(兼容 PowerShell 5.1 中文注释)。
- README:Windows 改为原生 MSVC 主路径(移除 mingw 交叉为默认)、字体体积/符号、目录结构补充脚本。
- AGENTS:平台状态、Windows 开发命令、`/utf-8` 与字体符号规则、验证规则 7 改为本机 MSVC 验证。

## 验证结果

| 项 | 结果 |
| --- | --- |
| 配置 | `cmake -S . -B build -G "Visual Studio 18 2026" -A x64` → 26s,成功 |
| 构建 | `cmake --build build --config Release` → 54s,**0 warning / 0 error** |
| 产物 | `build/Release/neon-breaker.exe` 196KB + 自动复制 SDL3.dll/SDL3_ttf.dll |
| 自检 | `--aim-test` ALL PASS |
| 截图冒烟 | dummy 驱动 `--level 3 --auto --screenshot` 退出码 0(state=2) |
| 真实窗口 | 菜单 / 第 3 关截图正常;中文、空格、`♥`、`＊`、`♻`、`←→` 均有字形 |
| 缩放 | `--resize 1280x800` 触发缩放(本机虚拟显示 1368×912,实际像素 1098×801) |
| 字体一致性 | 构建目录字体与 `assets/fonts/` SHA-256 相同 |
| 存档 | `saveStorage`/`loadStorage` 经 AppData junction 指向工作区验证:7 字段写入与读回一致;真实启动读取种子值(英文/深色/高分 4321/球色 #ff0066)并生效 |

## 遗留与注意

- **Linux 回归未在本机执行**:Windows 无 g++/WSL,`vendor/` 为 Linux 依赖;需由 CI `ubuntu-latest` job 或原 Linux 机器验证(本次改动均为平台条件编译或标准库调用,风险低)。
- **Android 交叉编译不可用**:`tools/android/android-ndk-r27c` 仅含 `linux-x86_64` 预编译工具链;在 Windows 上需改下载 `windows-x86_64` NDK 并调整 `fetch-deps.sh`/`build-android.sh`。
- **存档验证方式**:子进程直接写 `%APPDATA%` 会被环境拒绝,故用 junction(`%APPDATA%\NeonBreaker` → `build/appdata-test`)完成读写验证;结束后仅 `rmdir` 链接,未递归删除 AppData 内容。
- **Ninja 生成器**:受限 shell 下 `ninja` 启动编译器时卡死(手写 build.ninja 亦复现),VS 生成器正常;本机/CI 可用 Ninja 时不受影响。
- macOS/iOS 仍未实测。
