# macOS 11+ build guide (Neon Breaker)

> **Status**: CI builds and smoke-tests the game on `macos-latest` with Homebrew SDL3 packages, but the build has not been run manually on Mac hardware. The steps below follow the standard Homebrew workflow; adjust them if your environment differs.

## Prerequisites

- macOS 11 (Big Sur) or newer (Intel or Apple Silicon)
- [Homebrew](https://brew.sh)
- Xcode Command Line Tools (`xcode-select --install`), or the full Xcode installation

```bash
brew install sdl3 sdl3_ttf
```

## Build (CMake)

```bash
cd BreakOut
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$(brew --prefix)"
cmake --build build -j
./build/neon-breaker
```

Notes:

- The `APPLE` branch in `CMakeLists.txt` automatically searches `/opt/homebrew` (Apple Silicon) and `/usr/local` (Intel); `-DCMAKE_PREFIX_PATH="$(brew --prefix)"` is a fallback.
- The build produces a plain `neon-breaker` executable. It also runs inside an app bundle (see below).
- The bundled font is copied to `assets/fonts/` next to the binary after the build; at runtime the game loads it through `SDL_GetBasePath()`, which is app-bundle compatible.
- The version comes from the repository-root `VERSION` file; `./build/neon-breaker --version` prints the derived version (for example `v1.0.0-release`).

## Packaging as a .app (optional)

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
  <key>CFBundleDisplayName</key><string>Neon Breaker</string>
  <key>CFBundleIdentifier</key><string>com.neonbreaker.app</string>
  <key>CFBundleExecutable</key><string>neon-breaker</string>
  <key>CFBundlePackageType</key><string>APPL</string>
  <key>LSMinimumSystemVersion</key><string>11.0</string>
  <key>NSHighResolutionCapable</key><true/>
</dict>
</plist>
EOF
```

Inside the app bundle, `SDL_GetBasePath()` returns the bundle's `Contents/Resources` directory, so the renderer looks for `Contents/Resources/assets/fonts/`; place the files as shown above. If the font is missing, the renderer falls back to the system PingFang font (`/System/Library/Fonts/PingFang.ttc`).

If you distribute the optional bundle, set `CFBundleShortVersionString` to the semantic version from `VERSION` and `CFBundleVersion` to the numeric version code (the same mapping the iOS build uses).

## Fonts

Priority: 1) the bundled subset (`assets/fonts/NotoSansSC-Bold-subset.otf`, ~113 KB, shipped with the build); 2) system fonts (PingFang / STHeiti / Hiragino Sans GB / Songti).

## Known limitations

- CI only compiles and smoke-tests the macOS build; the `APPLE` branch (font paths, CMake configuration) has not been exercised manually on macOS hardware.
- Distribution outside the App Store requires ad-hoc signing: `codesign --force --deep -s - neon-breaker.app` (or a Developer ID certificate). See Apple's Gatekeeper documentation for the full flow.
- App Sandbox and notarization are not configured; entitlements must be added before distribution through the App Store or a notarized channel.

## iOS 13+

See [`docs/platform/ios.md`](ios.md) for iOS (Xcode plus macOS required; this repository only documents the cross-compilation setup used by CI).
