# Neon Breaker

An Arkanoid-style arcade game — a cross-platform **C++20 + SDL3** implementation.

Neon visuals, 120 Hz fixed-step physics, procedurally synthesized sound effects (no external audio assets), and build pipelines for five platforms.

> **History**: the project once included a TypeScript (web) implementation. It was removed on 2026-08-06 and the C++ implementation is the only one maintained; a source backup of the web version is archived outside the repository at `/tmp/breakout-web-backup-20260806.tar.gz`.

---

## Features

- **Gameplay**: 5 levels, 7 brick types, 11 power-ups, combo multiplier scoring, 120 Hz fixed-step physics.
- **Free-aim sticky ball**: while a ball is stuck to the paddle, drag the mouse or finger — the line from the ball to the pointer defines the launch direction (dashed aim guide plus a minimum elevation angle that prevents tunneling through the paddle).
- **Interface**: main menu, level select, settings dialog (language / theme / ball trail / particle effects / ball color), HUD, overlays, level-complete toast.
- **Effects**: neon particles, brick debris, score popups, expanding rings, full-screen flashes, screen shake, animated background lights, combo edge glow.
- **Audio**: 12 procedurally synthesized sound effects through the SDL audio stream; no external audio files.
- **Persistent settings**: stored via `SDL_GetPrefPath` (`~/.local/share/NeonBreaker/settings/settings.kv` on Linux).
- **Input**: keyboard, mouse, and touch (SDL touch events; mobile-ready).
- **Themes**: light, dark, follow system.
- **Auto-pause** on window focus loss or minimize.
- **Resizable window**: free scaling with proportional letterboxing; minimum size 640×420.

---

## Quick start

### Linux

```bash
# SDL3 and SDL3_ttf are vendored under vendor/; no system packages are required
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/neon-breaker
```

Requirements: a C++20 compiler and CMake 3.20+. SDL3 3.2.10 and SDL3_ttf 3.2.2 are bundled in `vendor/`.

### Windows

```powershell
# Download the official MSVC development packages into tools/win-deps/vc/
powershell -ExecutionPolicy Bypass -File tools/fetch-win-deps.ps1

cmake -S . -B build -G "Visual Studio 18 2026" -A x64
cmake --build build --config Release
build\Release\neon-breaker.exe
```

Windows 11 with native MSVC is the primary development environment; Linux regressions run in CI and containers. Windows dependencies can also come from vcpkg (`vcpkg install sdl3 sdl3-ttf`).

---

## Gameplay guide

### Controls

| Action | Keyboard | Mouse / touch |
| --- | --- | --- |
| Move paddle | `A` / `D`, `←` / `→` | Move the pointer (the paddle follows) |
| Launch ball / release sticky ball | `Space` | Click / tap |
| Pause / resume | `Esc` (Android back button) | Pause button in the top bar |
| Restart current level | `R` (while playing) | — |
| Aim sticky ball | Launch direction = ball center → pointer | Pointer position is the aim point |

### Bricks (7 types)

| Brick | HP | Behavior |
| --- | --- | --- |
| Normal `·` | 1 | Base brick (100 points) |
| Tough `=` | 3 | Color changes as HP drops (180 points) |
| Indestructible `◆` | ∞ | Cannot be destroyed; only piercing balls pass through, lasers are blocked |
| Bonus `★` | 1 | Always drops a power-up (250 points) |
| Explosive `＊` | 1 | Chain-detonates breakable bricks within a 125 px radius (160 points) |
| Rainbow `R` | 3 | Changes color on every hit (cyan → purple → pink); shatters in a three-color burst (150 points) |
| Respawn `♻` | 1 | Respawns in place after 5 seconds, with a ghost frame countdown (120 points) |

### Power-ups (11 types)

| Power-up | Effect |
| --- | --- |
| `WIDE` | Paddle width ×1.45 for 12 s |
| `SHORT` | Paddle width ×0.72 for 12 s |
| `MULTI` | Splits into three balls total (up to 5 balls) |
| `SLOW` | Ball speed ×0.78 for 12 s |
| `FAST` | Ball speed ×1.25 for 12 s |
| `LIFE` | +1 life (maximum 6) |
| `PIERCE` | Balls pass through bricks for 12 s |
| `SHIELD` | One-time barrier at the bottom; absorbs the next miss |
| `LASER` | Twin lasers from the paddle for 12 s; 2.2 s cooldown |
| `STICKY` | Catches the ball on the paddle; launch with free aim |
| `MEGA` | Ball radius ×1.5 for 12 s |

Notes:

- Catching a power-up awards 100 × multiplier points.
- `WIDE` and `SHORT` replace each other, as do `SLOW` and `FAST`; re-picking the same power-up refreshes its timer.
- Timed effects last 12 seconds (`EFFECT_DURATION`); `LIFE`, `MULTI`, and `SHIELD` apply instantly.

### Scoring

- Brick hits award brick points × multiplier; damaging (but not destroying) a multi-HP brick awards 25% of its value.
- The combo counter increases with every brick hit and resets after 3.5 s without a hit, when a life is lost, or when a new level starts.
- Multiplier = 1 + 0.5 × floor(combo / 5), capped at ×5. Every fifth combo triggers a burst effect and a score popup.
- Catching a power-up awards 100 × multiplier points.
- Clearing level N awards 1000 × N + 250 × remaining lives.
- The high score is persisted between runs.

---

## Interface and settings

- **Main menu**: Start game / Level select / Settings.
- **Level select**: all five levels are available as soon as the game starts.
- **Top bar**: pause/resume, sound toggle, settings.
- **Settings dialog**: language (Chinese / English), theme (light / dark / follow system), ball trail, particle effects, ball color (8-color palette). Changes are saved automatically.
- **HUD**: score, combo, lives, level, and active power-up tags.
- **Overlays**: ready, paused, level complete, and game over; all support keyboard and pointer interaction.

---

## Build and run

### Test flags

```bash
# Capture the screen (saved as BMP after ~0.5 s): menu / level N ready / playing after launch
./build/neon-breaker --screenshot shots/menu.bmp
./build/neon-breaker --level 3 --screenshot shots/level3.bmp
./build/neon-breaker --level 5 --auto --screenshot shots/play5.bmp
# Simulate a window resize before capturing
./build/neon-breaker --resize 1280x800 --screenshot shots/resize.bmp
# Headless self-test for the sticky-ball launch direction
./build/neon-breaker --aim-test
# Print the version derived from VERSION (no window / audio initialization)
./build/neon-breaker --version
# Headless smoke test using the SDL dummy video driver
SDL_VIDEODRIVER=dummy ./build/neon-breaker --level 3 --auto --screenshot shots/x.bmp
```

On Windows, replace `./build/neon-breaker` with `build\Release\neon-breaker.exe` and set the dummy driver with `$env:SDL_VIDEODRIVER='dummy'`.

### Platform support

| Platform | Status | Build | Verification |
| --- | --- | --- | --- |
| Linux (X11/Wayland) | ✅ Available | `cmake -B build && cmake --build build` (vendored SDL3) | CI: zero-warning build, `--aim-test`, headless screenshot |
| Windows 10/11 | ✅ Available | Native MSVC + CMake (VS generator; dependencies via `tools/fetch-win-deps.ps1` or vcpkg) | Local zero-warning build, self-test, and screenshots; CI build with vcpkg |
| macOS 11+ | ⚠️ Build chain ready | Homebrew `sdl3 sdl3_ttf` + CMake — see [`docs/platform/macos.md`](docs/platform/macos.md) | CI build and headless smoke test; not manually run on Mac hardware |
| Android 10+ (API 29) | ⚠️ Build chain ready | `tools/android/build-android.sh` → Gradle project in `android-project/` | CI cross-compiles `libmain.so` and assembles a debug APK; not device-tested |
| iOS 13+ | ⚠️ Config ready | Xcode / CMake cross-compilation — see [`docs/platform/ios.md`](docs/platform/ios.md) | CI builds the simulator app; release adds an unsigned device IPA; not device-tested |

⚠️ marks platforms whose build chain is verified but which have not been exercised on physical hardware.

### Continuous delivery (CD)

[`.github/workflows/release.yml`](.github/workflows/release.yml) is triggered only for tags matching `v*-release` (for example `v1.0.0-release`). A `Version` job re-reads `VERSION`, requires the stage to be `release`, and requires the tag to equal the version-derived tag before any packaging starts. There is no manual/nightly trigger.

| Job | Artifact | Contents |
| --- | --- | --- |
| Linux | `neon-breaker-linux-x86_64.tar.gz` | Executable + SDL runtime libraries (vendor layout preserved to match the RPATH) + fonts; extract and run |
| Windows | `neon-breaker-windows-x64.zip` | `neon-breaker.exe` + `SDL3.dll` / `SDL3_ttf.dll` (vcpkg applocal) + fonts |
| macOS | `neon-breaker-macos-arm64.zip` | Executable + transitive Homebrew dylib closure (relocated to `@loader_path`) + fonts |
| Android | `neon-breaker-android-arm64.apk` | NDK cross-compiled `libmain.so` packaged into a debug APK by Gradle |
| iOS | `neon-breaker-ios-simulator.zip` / `neon-breaker-unsigned.ipa` | Simulator `.app` (arm64 + x86_64) and an **unsigned device IPA** (iphoneos arm64, `CODE_SIGNING_ALLOWED=NO`, `Payload/` layout) for developers to re-sign |

The `Release` job collects every artifact and runs `gh release create` with the verified release tag (uploading with `--clobber` if the release already exists) and generated notes.

### Per-platform builds

```bash
# Linux: see "Quick start"

# Windows (native MSVC; Visual Studio 2022/2026 and a Windows SDK are required)
# Dependencies, option 1: vcpkg install sdl3 sdl3-ttf
# Dependencies, option 2: download the official development packages
powershell -ExecutionPolicy Bypass -File tools/fetch-win-deps.ps1
cmake -S . -B build -G "Visual Studio 18 2026" -A x64   # use "Visual Studio 17 2022" for VS 2022
cmake --build build --config Release
# Output: build/Release/neon-breaker.exe; SDL DLLs and fonts are copied next to it automatically
# Behind a proxy: powershell -File tools/fetch-win-deps.ps1 -Proxy http://127.0.0.1:7897

# macOS: see docs/platform/macos.md
# Android: tools/android/fetch-deps.sh, then tools/android/build-android.sh, then android-project/ (see android-project/README.md)
# iOS: see docs/platform/ios.md
```

### Cross-platform notes

- **Fonts**: the bundled subset `assets/fonts/NotoSansSC-Bold-subset.otf` (~113 KB, generated by `tools/make_font.py`, including the space glyph and symbols such as `＊`/`♻`) is preferred; desktop builds fall back to system fonts (Microsoft YaHei on Windows, PingFang on macOS, Noto CJK on Linux).
- **MSVC options**: `/utf-8` (sources are UTF-8 without a BOM; without it the compiler decodes them with the system ANSI code page) and `/EHsc` (exception unwinding) are added centrally by `CMakeLists.txt`.
- **Save data**: located per platform via `SDL_GetPrefPath`.
- **Touch, back button, backgrounding**: handled uniformly through SDL3 events and already wired up.
- **Window**: desktop builds use proportional letterbox scaling with a 640×420 minimum; mobile builds are fullscreen with a landscape lock configured in the platform project.

---

## Project structure

```
CMakeLists.txt            Platform-branching build (Linux vendor / Windows find_package / Android & iOS sub-builds)
VERSION                   Canonical version string (v1.x.x-<stage>.<build>)
vcpkg.json                Windows (vcpkg) dependency manifest: sdl3 / sdl3-ttf
vendor/                   Vendored SDL3 + SDL3_ttf (Linux, unpacked from Debian packages)
assets/fonts/             Bundled subset font (generated by tools/make_font.py; do not edit by hand)
assets/ios/Info.plist     iOS bundle metadata
android-project/          Android Gradle project (prebuilt .so mode)
src/
  main.cpp                Entry point: assembly, event loop, per-frame step + render, test flags
  game.h/cpp              State machine / physics / scoring / power-ups / sticky-ball aiming
  renderer.h/cpp          SDL rendering: gradients / rounded rectangles / text cache / neon effects / window scaling
  input.h/cpp             Keyboard / mouse / touch → InputActions
  audio.h/cpp             PCM sound synthesis + SDL audio stream
  fx.h/cpp                Particles / debris / popups / rings / flashes / screen shake
  ui.h/cpp                Button system / HUD / overlays / menu / level select / settings dialog
  storage.h/cpp           Settings key-value storage
  types.h / constants.h / i18n.h / levels.h
tools/
  make_font.py            Bundled-font subsetting script (--src selects the source font; requires fontTools)
  fetch-win-deps.ps1      Windows: downloads the official MSVC SDL3/SDL3_ttf packages into tools/win-deps/vc/
  copy-runtime-dlls.cmake Windows: copies SDL runtime DLLs next to the executable after the build
  version.rc.in           Windows: VERSIONINFO template filled from VERSION at configure time
  mingw-toolchain.cmake   mingw-w64 cross-compilation toolchain (legacy path, kept for reference)
  android/                fetch-deps.sh (dependency bootstrap) + build-android.sh (NDK cross-compilation)
docs/platform/            Platform build guides (macOS, iOS)
.github/workflows/        CI and CD pipelines
```

**Repository hygiene**: build outputs (`build*`, `shots/`) and local agent notes (`AGENTS.md`, `PLAN.md`, `PROGRESS.md`, `TASKS.md`, `docs/tasks/`) are ignored by Git through [`.gitignore`](.gitignore); only product sources and user-facing documentation are committed.

---

## Development and verification

```bash
# Full verification sequence (run before committing)
cmake --build build -j                # zero warnings
./build/neon-breaker --aim-test       # sticky-ball aiming self-test: ALL PASS
SDL_VIDEODRIVER=dummy ./build/neon-breaker --level 5 --auto --screenshot shots/x.bmp  # headless screenshot regression
```

- Builds must be warning-free (`-Wall -Wextra`; `/W4` under MSVC).
- Screenshot regression: after any change, capture the three states (menu / ready / playing) and compare them manually.
- Font subset: to extend the character set, edit `tools/make_font.py`, regenerate the font, then copy it to `android-project/app/src/main/assets/fonts/` so the Android build uses the same subset.

### Continuous integration (CI)

[`.github/workflows/ci.yml`](.github/workflows/ci.yml) runs on pushes to `main` and on pull requests. Five jobs run in parallel:

| Job | Runner | Dependencies | Checks |
| --- | --- | --- | --- |
| Linux | `ubuntu-latest` | Vendored SDL3 + apt runtime libraries | Zero-warning build → `--version` → `--aim-test` → dummy-driver screenshot |
| macOS | `macos-latest` | Homebrew `sdl3 sdl3_ttf` | Same as Linux |
| Windows | `windows-latest` | vcpkg (`vcpkg.json`: `sdl3`, `sdl3-ttf`) | Same as Linux (`--config Release`) |
| Android | `ubuntu-latest` | `fetch-deps.sh` (NDK + SDL sources, cached) and Android SDK 35 / build-tools 35 | NDK cross-compile → `libmain.so` → Gradle `assembleDebug` → APK |
| iOS | `macos-latest` | `fetch-deps.sh --no-ndk` (SDL sources) | Xcode simulator cross-compilation (`CMAKE_SYSTEM_NAME=iOS`) → `.app` validation |

Desktop jobs upload their smoke screenshots as `smoke-<platform>.bmp` artifacts; the Android job uploads `libmain.so` and `neon-breaker-debug.apk`. Preview locally with act: `act -j build-test --matrix "os:ubuntu-latest"` or `act -j android`.

---

## Known limitations

- Non-integer window scaling produces slight interpolation blur on desktop.
- Visual effects are enabled by default (the system reduced-motion setting is not read); particle effects can be disabled in the settings dialog.
- Ball color is limited to an 8-color palette.
- Android and iOS builds require their platform toolchains (Android SDK/Gradle, Xcode); CI covers the build chain but no physical-device testing is performed.
- CI compiles both the macOS and iOS builds, but neither has been confirmed on real hardware (font paths and project settings in particular).

---

## Versioning

[`VERSION`](VERSION) is the single source of truth for the game version. It contains one line:

```
v1.<minor>.<patch>-<stage>.<build>
```

- `v1.x.x` is the semantic version.
- `<stage>` is one of `alpha`, `beta`, `rc`, or `release`.
- `<build>` is the sequential number of that stage for the same version — for example, `v1.0.0-rc.2` is the second release candidate for 1.0.0.
- For `release`, the build counter is **ignored**: the derived version and release tag drop it. The released version of `v1.0.0-release.1` is `v1.0.0-release`.

The following build outputs are derived from it:

| Output | Derived value for `v1.0.0-release.1` |
| --- | --- |
| CMake / desktop CLI | project version `1.0.0`; `--version` prints `v1.0.0-release` |
| Android | `versionName 1.0.0`; `versionCode 100004000` |
| iOS | `CFBundleShortVersionString 1.0.0`; `CFBundleVersion 100004000` |
| Windows | resource `FILEVERSION 1,0,0,4000`; product version `v1.0.0-release` |
| Release tag | `v1.0.0-release` |

The Android `versionCode` / iOS `CFBundleVersion` encoding is:

```text
major*100000000 + minor*1000000 + patch*10000 + stage_rank*1000 + build
stage_rank: alpha=1, beta=2, rc=3, release=4 (build=0 for release)
```

`vcpkg.json` keeps its own `version-string` as vcpkg manifest metadata; it is not used for the game version and must be kept in sync numerically when the semantic version changes.

Release publishing is restricted to release tags: pushing `v1.0.0-release` starts the release workflow, which re-reads `VERSION`, rejects a non-`release` stage, and requires the tag to match the file exactly. Tags such as `v1.0.0-rc.1` or `v1.0.0-release.1` do not publish a release.

---

## License

No open-source license has been specified for this project (all rights reserved). Dependencies: SDL3 / SDL3_ttf (zlib license); the bundled font subset is derived from Noto Sans CJK (SIL OFL 1.1).
