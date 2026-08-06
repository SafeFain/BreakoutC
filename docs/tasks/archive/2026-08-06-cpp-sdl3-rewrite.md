# 任务归档:C++ / SDL3 桌面版重写

- **日期**:2026-08-06
- **状态**:已完成(归档)
- **关联版本**:根目录 C++ 项目(功能以 web 版为准,完全一致)

## 目标

用 C++20 + SDL3 重写整个项目,功能与 TypeScript 版完全一致;原 TS 项目分离至 `web/`。

## 实施内容

1. **项目分离**:TS 版全部文件(除 `.reasonix/`)移至 `web/`,根目录为 C++ 项目
2. **依赖本地化**:apt 下载解包 SDL3 3.2.10 + SDL3_ttf 3.2.2 至 `vendor/`(含 cmake config 与 .so),CMake `CMAKE_PREFIX_PATH` + rpath 引用,无需系统安装
3. **模块移植**(11 个源文件,约 3200 行):`game`(状态机/物理/7 砖种/11 道具/连击)、`renderer`(文本纹理缓存/渐变/圆角/字体加载)、`fx`、`audio`(12 音效 PCM 合成)、`input`(键盘/鼠标/触屏)、`ui`(按钮系统/HUD/overlay/菜单/选关/设置对话框)、`storage`(KV 存档)、`types/constants/i18n/levels`
4. **测试参数**:`--screenshot` / `--level` / `--auto`(headless 冒烟基础设施)

## 修复的关键问题

- `std::vector` erase 后引用失效:`hitBrick`/`explode` 改按 id 重查 + 先拷贝
- SDL 资源退出顺序 use-after-free:`Renderer::shutdown`/`AudioManager::shutdown` 在 `SDL_DestroyRenderer`/`SDL_Quit` 前显式清理
- dummy 驱动无焦点导致 `autoPause` 立即暂停:`--auto` 模式忽略焦点事件
- SDL3 API 差异:`SDL_SetRenderScale`/`SDL_SetRenderViewport` 命名、`e.tfinger` 触摸事件、`TTF_OpenFontWithProperties` 属性、`TTF_GetStringSize`

## 验证

- 构建零错误零警告;dummy + 真实显示双环境截图冒烟(菜单/选关/发球状态流转,state=2 确认)
- 截图像素与 ASCII 检视:砖块/挡板/HUD/overlay 渲染正确
- 退出码 0(修复前段错误 139)

## 涉及文件

根目录 `CMakeLists.txt`、`src/*`(11 模块)、`vendor/`、`README.md`;`web/` 原项目迁移

## 后续

窗口自由缩放功能见 `2026-08-06-resizable-window.md`。
