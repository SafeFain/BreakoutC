# macOS 11+ 构建指南(Neon Breaker)

> ⚠️ 本文件由 Linux 交叉开发环境编写,未在真实 macOS 上实测。构建命令基于 Homebrew 标准流程,如遇问题按报错调整。

## 前置条件

- macOS 11(Big Sur)或更高(Intel 或 Apple Silicon)
- [Homebrew](https://brew.sh)
- Xcode Command Line Tools(`xcode-select --install`),或完整 Xcode

```bash
brew install sdl3 sdl3_ttf
```

## 构建(CMake 方式)

```bash
cd BreakOut
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$(brew --prefix)"
cmake --build build -j
./build/neon-breaker
```

说明:
- `CMakeLists.txt` 的 `APPLE` 分支会自动把 `/opt/homebrew`(Apple Silicon)与 `/usr/local`(Intel)加入搜索路径;`-DCMAKE_PREFIX_PATH="$(brew --prefix)"` 为兜底
- 构建产物 `neon-breaker` 是 macOS 可执行文件(bundle 内运行亦可,见下)
- 内置字体通过 POST_BUILD 复制到产物旁 `assets/fonts/`,运行时 `SDL_GetBasePath()` 加载(与 App Bundle 兼容,见下)

## 打包为 .app(可选)

```bash
mkdir -p neon-breaker.app/Contents/MacOS
cp build/neon-breaker neon-breaker.app/Contents/MacOS/
mkdir -p neon-breaker.app/Contents/Resources/assets/fonts
cp assets/fonts/*.otf neon-breaker.app/Contents/Resources/assets/fonts/
cat > neon-breaker.app/Contents/Info.plist <<'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>CFBundleName</key><string>Neon Breaker</string>
  <key>CFBundleDisplayName</key><string>霓虹破界</string>
  <key>CFBundleIdentifier</key><string>com.neonbreaker.app</string>
  <key>CFBundleExecutable</key><string>neon-breaker</string>
  <key>CFBundlePackageType</key><string>APPL</string>
  <key>LSMinimumSystemVersion</key><string>11.0</string>
  <key>NSHighResolutionCapable</key><true/>
</dict>
</plist>
EOF
```

注意:App Bundle 内运行时,内置字体路径相对可执行文件为 `../Resources/assets/fonts/`,需按上面目录结构放置;若字体缺失,渲染器会回退系统苹方(`/System/Library/Fonts/PingFang.ttc`)。

## 字体

优先级:1) 内置子集字体(`assets/fonts/NotoSansSC-Bold-subset.otf`,107KB,随产物分发);2) 系统字体(PingFang / STHeiti / Hiragino Sans GB / Songti)。

## 已知限制(未实测项)

- 本仓库开发环境为 Linux,`APPLE` 分支代码(字体路径、CMake 分支)未在 macOS 编译验证,首次构建如报错请记录并反馈
- 非 App Store 分发需 ad-hoc 签名:`codesign --force --deep -s - neon-breaker.app`(或开发者证书);Gatekeeper 相关流程见 Apple 文档
- 沙盒/公证(App Sandbox、notarization)未配置,如需上架需补充 entitlements

## iOS 13+ 说明

iOS 构建请见 `docs/platform/ios.md`(需 Xcode + macOS,本仓库仅提供工程配置说明)。
