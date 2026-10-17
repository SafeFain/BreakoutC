cmake_minimum_required(VERSION 3.20)

# ---------------- Version (single source of truth: VERSION) ----------------
set(NEON_VERSION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/VERSION")
if(NOT EXISTS "${NEON_VERSION_FILE}")
  message(FATAL_ERROR "VERSION file not found: ${NEON_VERSION_FILE}")
endif()
file(STRINGS "${NEON_VERSION_FILE}" NEON_VERSION_RAW LIMIT_COUNT 1)
string(REGEX REPLACE "\r$" "" NEON_VERSION_RAW "${NEON_VERSION_RAW}")
string(STRIP "${NEON_VERSION_RAW}" NEON_VERSION_RAW)
if(NOT NEON_VERSION_RAW MATCHES "^v([0-9]+)\\.([0-9]+)\\.([0-9]+)-(alpha|beta|rc|release)\\.([0-9]+)$")
  message(FATAL_ERROR "Invalid VERSION '${NEON_VERSION_RAW}': expected v1.<minor>.<patch>-<alpha|beta|rc|release>.<build>")
endif()
set(NEON_VERSION_MAJOR "${CMAKE_MATCH_1}")
set(NEON_VERSION_MINOR "${CMAKE_MATCH_2}")
set(NEON_VERSION_PATCH "${CMAKE_MATCH_3}")
set(NEON_VERSION_STAGE "${CMAKE_MATCH_4}")
set(NEON_VERSION_BUILD "${CMAKE_MATCH_5}")
set(NEON_VERSION_SEMVER "${NEON_VERSION_MAJOR}.${NEON_VERSION_MINOR}.${NEON_VERSION_PATCH}")

if(NEON_VERSION_STAGE STREQUAL "alpha")
  set(NEON_VERSION_STAGE_RANK 1)
elseif(NEON_VERSION_STAGE STREQUAL "beta")
  set(NEON_VERSION_STAGE_RANK 2)
elseif(NEON_VERSION_STAGE STREQUAL "rc")
  set(NEON_VERSION_STAGE_RANK 3)
else()
  set(NEON_VERSION_STAGE_RANK 4)
endif()

if(NEON_VERSION_BUILD LESS 1)
  message(FATAL_ERROR "VERSION build number must be >= 1")
endif()

if(NEON_VERSION_STAGE STREQUAL "release")
  # release is final: the per-stage build counter is ignored everywhere
  set(NEON_VERSION_EFFECTIVE_BUILD 0)
  set(NEON_VERSION_DISPLAY "v${NEON_VERSION_SEMVER}-release")
else()
  set(NEON_VERSION_EFFECTIVE_BUILD "${NEON_VERSION_BUILD}")
  set(NEON_VERSION_DISPLAY "v${NEON_VERSION_SEMVER}-${NEON_VERSION_STAGE}.${NEON_VERSION_BUILD}")
endif()

if(NEON_VERSION_MINOR GREATER 999 OR NEON_VERSION_PATCH GREATER 999 OR NEON_VERSION_EFFECTIVE_BUILD GREATER 999)
  message(FATAL_ERROR "VERSION components exceed the versionCode mapping (minor/patch/build <= 999)")
endif()

# Android versionCode / iOS CFBundleVersion: major*1e8 + minor*1e6 + patch*1e4 + stage*1e3 + build
math(EXPR NEON_VERSION_CODE "${NEON_VERSION_MAJOR} * 100000000 + ${NEON_VERSION_MINOR} * 1000000 + ${NEON_VERSION_PATCH} * 10000 + ${NEON_VERSION_STAGE_RANK} * 1000 + ${NEON_VERSION_EFFECTIVE_BUILD}")
math(EXPR NEON_VERSION_WIN_BUILD "${NEON_VERSION_STAGE_RANK} * 1000 + ${NEON_VERSION_EFFECTIVE_BUILD}")
if(NEON_VERSION_CODE GREATER 2100000000)
  message(FATAL_ERROR "VERSION maps to versionCode ${NEON_VERSION_CODE}, exceeding the Android maximum")
endif()

project(neon-breaker VERSION "${NEON_VERSION_SEMVER}" LANGUAGES CXX)

function(neon_apply_version target)
  target_compile_definitions(${target} PRIVATE
    NEON_VERSION_STRING="${NEON_VERSION_DISPLAY}"
    NEON_VERSION_SEMVER="${NEON_VERSION_SEMVER}"
    NEON_VERSION_CODE=${NEON_VERSION_CODE}
  )
endfunction()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

if(MSVC)
  # /utf-8  源码为 UTF-8(无 BOM)且含中文字面量,默认按系统 ANSI 代码页解析会乱码/编译失败
  # /EHsc   启用 C++ 异常展开语义(storage.cpp 等处使用 try/catch,缺省会触发 C4530)
  add_compile_options(/W4 /utf-8 /EHsc)
else()
  add_compile_options(-Wall -Wextra)
endif()

# ---------------- 平台分支 ----------------
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  # Linux:本地 vendor(无需系统安装);也可改用系统 SDL3
  set(VENDOR_DIR ${CMAKE_SOURCE_DIR}/vendor/usr)
  set(CMAKE_PREFIX_PATH ${VENDOR_DIR})
  set(USE_VENDOR_RPATH ON)
  find_package(SDL3 CONFIG REQUIRED)
  find_package(SDL3_ttf CONFIG REQUIRED)
elseif(WIN32)
  # Windows:优先使用 vcpkg(vcpkg install sdl3 sdl3-ttf);
  # 也可用 tools/fetch-win-deps.ps1 下载官方 MSVC 开发包到 tools/win-deps/vc/(自动加入搜索路径)
  file(GLOB _win_vc_sdl "${CMAKE_SOURCE_DIR}/tools/win-deps/vc/SDL3-*")
  file(GLOB _win_vc_ttf "${CMAKE_SOURCE_DIR}/tools/win-deps/vc/SDL3_ttf-*")
  list(APPEND CMAKE_PREFIX_PATH ${_win_vc_sdl} ${_win_vc_ttf})
  find_package(SDL3 CONFIG REQUIRED)
  find_package(SDL3_ttf CONFIG REQUIRED)
  if(MSVC)
    enable_language(RC)
    set(NEON_VERSION_RC ON)
  endif()
elseif(ANDROID)
  # Android:交叉编译 libmain.so(静态链接 SDL3/SDL3_ttf/freetype)。
  # 依赖源码路径(下载与说明见 README / android-project/README.md)
  set(SDL3_SOURCE_DIR "${CMAKE_SOURCE_DIR}/tools/android/SDL-release-3.2.10" CACHE PATH "SDL3 源码目录")
  set(SDL3_TTF_SOURCE_DIR "${CMAKE_SOURCE_DIR}/tools/android/SDL_ttf-release-3.2.2" CACHE PATH "SDL3_ttf 源码目录")
  set(ANDROID_SDL_STATIC ON)
  set(SDL_SHARED OFF CACHE BOOL "" FORCE)
  set(SDL_STATIC ON CACHE BOOL "" FORCE)
  set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
  # SDL_ttf 依赖:显式启用 vendored freetype(交叉环境无系统包),关闭 harfbuzz/plutosvg(目录未提供)
  set(SDLTTF_VENDORED ON CACHE BOOL "" FORCE)
  set(SDLTTF_HARFBUZZ OFF CACHE BOOL "" FORCE)
  set(SDLTTF_PLUTOSVG OFF CACHE BOOL "" FORCE)
  add_subdirectory(${SDL3_SOURCE_DIR} SDL3 EXCLUDE_FROM_ALL)           # 先构建 SDL3(SDL3-static + SDL3::SDL3 别名)
  add_subdirectory(${SDL3_TTF_SOURCE_DIR} SDL3_ttf EXCLUDE_FROM_ALL)  # 检测到 SDL3 target 后跳过 find_package,使用 vendored freetype
  add_library(neon-breaker SHARED
    src/main.cpp src/audio.cpp src/fx.cpp src/game.cpp src/input.cpp
    src/renderer.cpp src/storage.cpp src/ui.cpp
  )
  target_include_directories(neon-breaker PRIVATE src)
  target_link_libraries(neon-breaker PRIVATE SDL3::SDL3-static SDL3_ttf::SDL3_ttf-static)
  set_target_properties(neon-breaker PROPERTIES OUTPUT_NAME main)      # libmain.so(SDL Android 启动器约定)
  neon_apply_version(neon-breaker)
  # 内置字体打包进 APK assets 由 android-project/Gradle 处理;此处仅构建库
  return()  # Android 专用构建,跳过下方桌面公共部分
elseif(IOS)
  # iOS:交叉编译(CI 于 macOS runner 用模拟器 SDK 验证;iOS 工程集成见 docs/platform/ios.md)
  set(SDL3_SOURCE_DIR "${CMAKE_SOURCE_DIR}/tools/android/SDL-release-3.2.10" CACHE PATH "SDL3 源码目录")
  set(SDL3_TTF_SOURCE_DIR "${CMAKE_SOURCE_DIR}/tools/android/SDL_ttf-release-3.2.2" CACHE PATH "SDL3_ttf 源码目录")
  set(SDL_SHARED OFF CACHE BOOL "" FORCE)
  set(SDL_STATIC ON CACHE BOOL "" FORCE)
  set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
  # SDL_ttf 依赖:显式启用 vendored freetype(交叉环境无系统包),关闭 harfbuzz/plutosvg(目录未提供)
  set(SDLTTF_VENDORED ON CACHE BOOL "" FORCE)
  set(SDLTTF_HARFBUZZ OFF CACHE BOOL "" FORCE)
  set(SDLTTF_PLUTOSVG OFF CACHE BOOL "" FORCE)
  add_subdirectory(${SDL3_SOURCE_DIR} SDL3 EXCLUDE_FROM_ALL)           # 先构建 SDL3(SDL3-static + SDL3::SDL3 别名,含 iOS UIKit main)
  add_subdirectory(${SDL3_TTF_SOURCE_DIR} SDL3_ttf EXCLUDE_FROM_ALL)  # 检测到 SDL3 target 后跳过 find_package,使用 vendored freetype
  add_executable(neon-breaker
    src/main.cpp src/audio.cpp src/fx.cpp src/game.cpp src/input.cpp
    src/renderer.cpp src/storage.cpp src/ui.cpp
  )
  target_include_directories(neon-breaker PRIVATE src)
  target_link_libraries(neon-breaker PRIVATE SDL3::SDL3-static SDL3_ttf::SDL3_ttf-static)
  # Info.plist 是模板:版本字段在 configure 时由 VERSION 生成到构建目录
  file(READ "${CMAKE_SOURCE_DIR}/assets/ios/Info.plist" _neon_ios_plist)
  string(REGEX REPLACE
    "(<key>CFBundleShortVersionString</key>[^<]*<string>)[^<]*(</string>)"
    "\\1${NEON_VERSION_SEMVER}\\2" _neon_ios_plist "${_neon_ios_plist}")
  string(REGEX REPLACE
    "(<key>CFBundleVersion</key>[^<]*<string>)[^<]*(</string>)"
    "\\1${NEON_VERSION_CODE}\\2" _neon_ios_plist "${_neon_ios_plist}")
  if(NOT _neon_ios_plist MATCHES "<string>${NEON_VERSION_SEMVER}</string>" OR
     NOT _neon_ios_plist MATCHES "<string>${NEON_VERSION_CODE}</string>")
    message(FATAL_ERROR "failed to inject version ${NEON_VERSION_SEMVER} (${NEON_VERSION_CODE}) into iOS Info.plist")
  endif()
  file(WRITE "${CMAKE_BINARY_DIR}/Info.plist" "${_neon_ios_plist}")
  set_target_properties(neon-breaker PROPERTIES
    MACOSX_BUNDLE ON
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_BINARY_DIR}/Info.plist"
  )
  neon_apply_version(neon-breaker)
  return()
elseif(APPLE)
  # macOS:Homebrew(sdl3 sdl3_ttf);也支持 Xcode 自带或自定义前缀
  foreach(_prefix "/opt/homebrew" "/usr/local")
    if(EXISTS "${_prefix}/lib/cmake")
      list(APPEND CMAKE_PREFIX_PATH "${_prefix}")
    endif()
  endforeach()
  find_package(SDL3 CONFIG REQUIRED)
  find_package(SDL3_ttf CONFIG REQUIRED)
else()
  message(FATAL_ERROR "未支持的平台: ${CMAKE_SYSTEM_NAME}")
endif()

# SDL3 target 名兼容(vcpkg/官方 config 提供 SDL3::SDL3 别名,部分版本仅 SDL3::SDL3-shared)
if(NOT TARGET SDL3::SDL3)
  set(SDL3_TARGET SDL3::SDL3-shared)
else()
  set(SDL3_TARGET SDL3::SDL3)
endif()
if(NOT TARGET SDL3_ttf::SDL3_ttf)
  set(SDL3_TTF_TARGET SDL3_ttf::SDL3_ttf-shared)
else()
  set(SDL3_TTF_TARGET SDL3_ttf::SDL3_ttf)
endif()

add_executable(neon-breaker
  src/main.cpp
  src/audio.cpp
  src/fx.cpp
  src/game.cpp
  src/input.cpp
  src/renderer.cpp
  src/storage.cpp
  src/ui.cpp
)

target_include_directories(neon-breaker PRIVATE src)
target_link_libraries(neon-breaker PRIVATE ${SDL3_TARGET} ${SDL3_TTF_TARGET})
neon_apply_version(neon-breaker)

# Windows:生成的 VERSIONINFO 资源,让资源管理器/属性页显示版本
if(WIN32 AND NEON_VERSION_RC)
  configure_file("${CMAKE_SOURCE_DIR}/tools/version.rc.in" "${CMAKE_BINARY_DIR}/version.rc" @ONLY)
  target_sources(neon-breaker PRIVATE "${CMAKE_BINARY_DIR}/version.rc")
endif()

# 内置字体资源:开发时复制到构建目录,与运行时 SDL_GetBasePath 布局一致
add_custom_command(TARGET neon-breaker POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_SOURCE_DIR}/assets/fonts
          $<TARGET_FILE_DIR:neon-breaker>/assets/fonts
  COMMENT "复制内置字体到构建目录"
)

# Windows 开发运行:把 SDL3/SDL3_ttf 运行库复制到 exe 同目录(等价于 vcpkg applocal)
if(WIN32 AND CMAKE_VERSION VERSION_GREATER_EQUAL 3.21)
  add_custom_command(TARGET neon-breaker POST_BUILD
    COMMAND ${CMAKE_COMMAND}
            -DDEST=$<TARGET_FILE_DIR:neon-breaker>
            "-DDLLS=$<TARGET_RUNTIME_DLLS:neon-breaker>"
            -P ${CMAKE_SOURCE_DIR}/tools/copy-runtime-dlls.cmake
    COMMENT "复制 SDL 运行库到构建目录"
  )
endif()

# 运行期库路径:仅 Linux vendor 需要(产物在 build/,SDL 库在 ../vendor/usr/lib/x86_64-linux-gnu)
if(USE_VENDOR_RPATH)
  set_target_properties(neon-breaker PROPERTIES
    BUILD_RPATH "$ORIGIN/../vendor/usr/lib/x86_64-linux-gnu"
    INSTALL_RPATH "$ORIGIN/../vendor/usr/lib/x86_64-linux-gnu"
  )
endif()
