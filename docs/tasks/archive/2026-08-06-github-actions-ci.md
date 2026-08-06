# 配置 GitHub Actions CI(2026-08-06)

## 目标
提交后自动触发 CI,测试平台:macOS / Windows / Linux。

## 完成内容
- `.github/workflows/ci.yml`:push(main)+ pull_request 触发,三平台矩阵(ubuntu-latest / macos-latest / windows-latest),每平台:依赖安装 → CMake 配置 → 构建(零警告)→ `--aim-test` → dummy 无头截图冒烟 → 上传截图 artifact(`smoke-<平台>.bmp`)
- `vcpkg.json`:Windows 依赖清单(sdl3 / sdl3-ttf,vcpkg manifest 模式)
- `CMakeLists.txt`:MSVC 用 `/W4`(原 `-Wall -Wextra` 在 MSVC 报 D9002)
- 依赖细节:
  - Linux:vendor/ 自带 SDL3,CI 需 apt 安装其 DT_NEEDED 运行时库(20 包,含 `libsndio7.0` — 24.04 改名,act 实测抓出)+ `cmake g++`(显式安装,兼容 act 镜像)
  - macOS:Homebrew `sdl3 sdl3_ttf` + `-DCMAKE_PREFIX_PATH=$(brew --prefix)`
  - Windows:vcpkg toolchain + `--config Release`(VS 多配置生成器,exe 在 `build/Release/`;vcpkg applocal 自动部署 DLL)
- README 新增 CI 小节;AGENTS 新增目录条目、验证规则 8、CI 基础设施修改边界

## 验证
- act 0.2.72(docker)端到端跑通 Linux job:6 步骤全 Success,截图 state=2,artifact 上传成功
- act 实测发现并修复 2 个问题:`libsndio7`→`libsndio7.0`(noble 不存在)、显式安装 cmake/g++
- Linux job 命令序列本机模拟通过
- macOS/Windows job 无法本地执行(act 不支持非 Linux runner),标准做法 + 源码级兼容性检查(无 M_PI/strcasecmp 等 MSVC 不兼容项)

## 修复记录(2026-08-06,Windows 实机反馈)

**问题**:Windows 上 `-DCMAKE_TOOLCHAIN_FILE=$env:VCPKG_INSTALLATION_ROOT/...` 未被 shell 展开,CMake 收到字面量 `$env:...` 报 "Could not find toolchain file"。

**修复**:配置步骤拆分为二——`配置(Windows)`(`shell: pwsh`,双引号包裹 `"$env:VCPKG_INSTALLATION_ROOT/scripts/buildsystems/vcpkg.cmake"`,微软 vcpkg 官方推荐写法)与 `配置(Linux/macOS)`(沿用 matrix.cmake_args);matrix Windows 行 `cmake_args` 置空。act 重跑 Linux job 全绿(6 步 Success)。

**本地 PowerShell 复现命令**:
```powershell
cmake -B build -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_INSTALLATION_ROOT/scripts/buildsystems/vcpkg.cmake"
```

## 遗留
- macOS/Windows job 首次真实运行在 GitHub runner 上(推送后触发)
- 仓库无远端:推送 GitHub 后 push 即自动触发;本地 act 预览命令见 README
