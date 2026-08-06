# PLAN.md — 当前任务计划

更新时机:任务开始/计划变更时更新;步骤完成时同步到 `PROGRESS.md`。

## 当前任务

**无进行中任务**。上一任务(跨平台兼容性拓展,2026-08-06)已完成并归档:

- 归档:`docs/tasks/archive/2026-08-06-cross-platform.md`
- 内容:五平台(Windows/macOS/Linux/iOS/Android)代码适配、桌面构建链、移动构建链、平台矩阵文档
- 验证:Linux 回归 + Windows mingw exe + Android libmain.so 交叉编译零警告;macOS/iOS 需实测

新任务开始时:在此登记目标/验收标准/实施步骤,并在 `TASKS.md` Active 表登记。

## 历史决策记录

- 2026-08-06:跨平台验证深度=全量交叉编译(用户确认);移动端字体=内置子集(107KB);Android 用预编译 so 模式工程;macOS/iOS 本环境无法验证,交付配置+文档
- 2026-08-06:移除 web 版,项目转为 C++ 单实现(详见归档)
- 2026-08-06:粘球方向控制采用"球→鼠标连线"语义(详见归档)
- 2026-08-06:状态文件体系按 1.txt 规范建立(详见 `PROGRESS.md` 整理记录)
