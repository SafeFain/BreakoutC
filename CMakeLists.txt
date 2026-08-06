cmake_minimum_required(VERSION 3.20)
project(neon-breaker CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

if(MSVC)
  add_compile_options(/W4)
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
  # Windows:通过 vcpkg 安装 SDL3 SDL3-ttf(vcpkg install sdl3 sdl3-ttf),或手动提供 CMake config
  find_package(SDL3 CONFIG REQUIRED)
  find_package(SDL3_ttf CONFIG REQUIRED)
elseif(APPLE)
  # macOS:Homebrew(sdl3 sdl3_ttf);也支持 Xcode 自带或自定义前缀
  foreach(_prefix "/opt/homebrew" "/usr/local")
    if(EXISTS "${_prefix}/lib/cmake")
      list(APPEND CMAKE_PREFIX_PATH "${_prefix}")
    endif()
  endforeach()
  find_package(SDL3 CONFIG REQUIRED)
  find_package(SDL3_ttf CONFIG REQUIRED)
elseif(ANDROID)
  # Android:交叉编译 libmain.so(静态链接 SDL3/SDL3_ttf/freetype)。
  # 依赖源码路径(下载与说明见 README / android-project/README.md)
  set(SDL3_SOURCE_DIR "${CMAKE_SOURCE_DIR}/tools/android/SDL-release-3.2.10" CACHE PATH "SDL3 源码目录")
  set(SDL3_TTF_SOURCE_DIR "${CMAKE_SOURCE_DIR}/tools/android/SDL_ttf-release-3.2.2" CACHE PATH "SDL3_ttf 源码目录")
  set(ANDROID_SDL_STATIC ON)
  set(SDL_SHARED OFF CACHE BOOL "" FORCE)
  set(SDL_STATIC ON CACHE BOOL "" FORCE)
  set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
  add_subdirectory(${SDL3_SOURCE_DIR} SDL3 EXCLUDE_FROM_ALL)           # 先构建 SDL3(SDL3-static + SDL3::SDL3 别名)
  add_subdirectory(${SDL3_TTF_SOURCE_DIR} SDL3_ttf EXCLUDE_FROM_ALL)  # 检测到 SDL3 target 后跳过 find_package,使用 vendored freetype
  add_library(neon-breaker SHARED
    src/main.cpp src/audio.cpp src/fx.cpp src/game.cpp src/input.cpp
    src/renderer.cpp src/storage.cpp src/ui.cpp
  )
  target_include_directories(neon-breaker PRIVATE src)
  target_link_libraries(neon-breaker PRIVATE SDL3::SDL3-static SDL3_ttf::SDL3_ttf-static)
  set_target_properties(neon-breaker PROPERTIES OUTPUT_NAME main)      # libmain.so(SDL Android 启动器约定)
  # 内置字体打包进 APK assets 由 android-project/Gradle 处理;此处仅构建库
  return()  # Android 专用构建,跳过下方桌面公共部分
elseif(IOS)
  # iOS:交叉编译(CI 于 macOS runner 用模拟器 SDK 验证;iOS 工程集成见 docs/platform/ios.md)
  set(SDL3_SOURCE_DIR "${CMAKE_SOURCE_DIR}/tools/android/SDL-release-3.2.10" CACHE PATH "SDL3 源码目录")
  set(SDL3_TTF_SOURCE_DIR "${CMAKE_SOURCE_DIR}/tools/android/SDL_ttf-release-3.2.2" CACHE PATH "SDL3_ttf 源码目录")
  set(SDL_SHARED OFF CACHE BOOL "" FORCE)
  set(SDL_STATIC ON CACHE BOOL "" FORCE)
  set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
  add_subdirectory(${SDL3_TTF_SOURCE_DIR} SDL3_ttf EXCLUDE_FROM_ALL)  # 先编译 freetype(其 vendored 子目录)
  add_subdirectory(${SDL3_SOURCE_DIR} SDL3 EXCLUDE_FROM_ALL)           # SDL3-static(含 iOS UIKit main)
  add_executable(neon-breaker
    src/main.cpp src/audio.cpp src/fx.cpp src/game.cpp src/input.cpp
    src/renderer.cpp src/storage.cpp src/ui.cpp
  )
  target_include_directories(neon-breaker PRIVATE src)
  target_link_libraries(neon-breaker PRIVATE SDL3::SDL3-static SDL3_ttf::SDL3_ttf-static)
  set_target_properties(neon-breaker PROPERTIES
    MACOSX_BUNDLE ON
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/assets/ios/Info.plist
  )
  return()
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

# 内置字体资源:开发时复制到构建目录,与运行时 SDL_GetBasePath 布局一致
add_custom_command(TARGET neon-breaker POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_SOURCE_DIR}/assets/fonts
          $<TARGET_FILE_DIR:neon-breaker>/assets/fonts
  COMMENT "复制内置字体到构建目录"
)

# 运行期库路径:仅 Linux vendor 需要(产物在 build/,SDL 库在 ../vendor/usr/lib/x86_64-linux-gnu)
if(USE_VENDOR_RPATH)
  set_target_properties(neon-breaker PROPERTIES
    BUILD_RPATH "$ORIGIN/../vendor/usr/lib/x86_64-linux-gnu"
    INSTALL_RPATH "$ORIGIN/../vendor/usr/lib/x86_64-linux-gnu"
  )
endif()
