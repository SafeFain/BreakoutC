# 任务归档:C++ 版窗口自由缩放

- **日期**:2026-08-06
- **状态**:已完成(归档)
- **关联版本**:根目录 C++ 项目

## 目标

桌面版支持自由调节窗口大小(此前固定 960×700)。

## 实施内容

- **等比缩放 + 居中 letterbox**(不变形):逻辑坐标恒为 960×700
- `Renderer::onWindowResized(w, h)`:计算 `scale = min(w/960, h/700)` 与居中偏移,`SDL_SetRenderScale` + `SDL_SetRenderViewport` 使全部绘制自动缩放
- `InputManager::setScreenTransform`:鼠标/触屏事件窗口像素 → 逻辑坐标(与渲染同一套 transform)
- `UI::handleEvent` 点击坐标经 `Renderer::screenToLogical` 转换
- 窗口 `SDL_WINDOW_RESIZABLE`,`SDL_SetWindowMinimumSize(640, 420)`
- 监听 `SDL_EVENT_WINDOW_RESIZED`(事件自带新尺寸)同步 renderer 与 input
- 每帧 `SDL_RenderClear` 清除黑边;新增 `--resize WxH` 测试参数

## 验证

- 构建零警告;dummy + 真实显示 resize 截图(1280×800、700×900、1100×750 等),scale/offset 计算正确
- ASCII 灰度图检视:resize 后 HUD/砖块/挡板/footer 等比缩放、布局完整无变形
- 默认尺寸无回归

## 涉及文件

`src/{renderer,input,ui,main}.{h,cpp}`、`README.md`

## 备注

非整数倍缩放由 SDL 渲染器插值,轻微模糊属正常;真实环境窗口尺寸可能受虚拟显示/WM 约束。
