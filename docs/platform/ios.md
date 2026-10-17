# iOS 13+ build guide (Neon Breaker)

> **Status**: CI cross-compiles the game for the iOS simulator and the release workflow also produces an unsigned device build. The game has not been run on physical iOS hardware, and signing/App Store packaging is not configured.

## Prerequisites

- macOS with Xcode (deployment target iOS 13+)
- SDL3 and SDL3_ttf sources: `tools/android/SDL-release-3.2.10/` and `tools/android/SDL_ttf-release-3.2.2/`. Fetch them with:

```bash
tools/android/fetch-deps.sh --no-ndk
```

The `--no-ndk` flag skips the ~660 MB Android NDK download; the iOS build only needs the SDL sources and the vendored FreeType tree that the script prepares.

## Build with CMake (used by CI)

The `IOS` branch in `CMakeLists.txt` builds SDL3, SDL3_ttf (with vendored FreeType), and the game into a `.app` bundle using [`assets/ios/Info.plist`](../../assets/ios/Info.plist).

### Simulator (arm64 + x86_64)

```bash
cmake -G Xcode -B build-ios -DCMAKE_SYSTEM_NAME=iOS \
  -DCMAKE_OSX_SYSROOT=iphonesimulator \
  -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
cmake --build build-ios --config Release -- -sdk iphonesimulator
```

The bundle is written to `build-ios/Release-iphonesimulator/neon-breaker.app`.

### Device (arm64, unsigned)

```bash
cmake -G Xcode -B build-ios-device -DCMAKE_SYSTEM_NAME=iOS \
  -DCMAKE_OSX_SYSROOT=iphoneos \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED=NO
cmake --build build-ios-device --config Release -- \
  -sdk iphoneos CODE_SIGNING_ALLOWED=NO CODE_SIGNING_REQUIRED=NO
```

The output is `build-ios-device/Release-iphoneos/neon-breaker.app`. Re-sign it with your own identity to install on a device.

### Bundle resources

Copy the bundled font into the app bundle so the game can find it at runtime:

```bash
APP=build-ios/Release-iphonesimulator/neon-breaker.app
mkdir -p "$APP/assets/fonts"
cp assets/fonts/*.otf "$APP/assets/fonts/"
```

At runtime the renderer checks `<bundle>/assets/fonts/`, then `<bundle>/fonts/`. If neither exists, it falls back to the system PingFang font.

## Integration with an Xcode project (alternative)

SDL3 ships an Xcode project in its source tree:

1. Open `tools/android/SDL-release-3.2.10/Xcode/SDL/SDL.xcodeproj` (see the upstream `docs/README-ios.md`).
2. Add the SDL Xcode project or the generated `SDL3.xcframework` to your app target.
3. Add `src/*.cpp` and `src/*.h` to the app target.
4. Link the static SDL3 / SDL3_ttf / FreeType libraries and the system frameworks required by SDL3 (UIKit, CoreGraphics, QuartzCore, AVFoundation, and others listed by the SDL template).
5. Add `assets/fonts/NotoSansSC-Bold-subset.otf` to the bundle resources (`assets/fonts/` or `fonts/`).
6. Set `IPHONEOS_DEPLOYMENT_TARGET=13.0`.

## Info.plist

The bundled [`assets/ios/Info.plist`](../../assets/ios/Info.plist) declares the landscape orientations used by the game:

```xml
<key>UISupportedInterfaceOrientations</key>
<array>
    <string>UIInterfaceOrientationLandscapeLeft</string>
    <string>UIInterfaceOrientationLandscapeRight</string>
</array>
```

Add `UIRequiresFullScreen` (or the appropriate scene configuration) if your distribution requires it.

## Input and lifecycle

- Touch input, UI taps, pause-on-background, and save paths behave the same as on Android; SDL3 handles them uniformly and the game code is already adapted.
- iOS has no hardware back button; the pause button in the top bar covers that case.

## Known limitations

- CI verifies simulator compilation and produces an unsigned device build; nothing has been run on physical hardware, so `__IPHONEOS__` code paths (fullscreen flags, font paths) are compile-tested only.
- App Store distribution requires signing, notarization, and privacy declarations, none of which are configured.
- Static SDL3_ttf requires FreeType; use the vendored FreeType tree prepared by `tools/android/fetch-deps.sh`.
