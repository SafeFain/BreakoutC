# PLAN.md — 当前任务计划

更新时机:任务开始/计划变更时更新;步骤完成时同步到 `PROGRESS.md`。

## 当前任务

**无进行中任务**。上一任务(Windows 开发环境迁移修复,2026-10-17)已完成并归档:

- 归档:`docs/tasks/archive/2026-10-17-windows-migration.md`
- 内容:MSVC `/utf-8` `/EHsc`、源码警告清零、内置字体空格与符号、`.gitattributes`/符号链接/执行位、Windows 依赖获取脚本、README/AGENTS 同步、迁移垃圾清理
- 验证:Windows 原生 MSVC 零警告构建、aim-test、dummy/真实窗口截图;Linux 回归待 CI

新任务开始时:在此登记目标/验收标准/实施步骤,并在 `TASKS.md` Active 表登记。

## 历史决策记录

- 2026-10-17:Windows 为首要开发环境;构建路径改为原生 MSVC + 官方 VC 开发包(`tools/win-deps/vc/`),mingw 交叉降级为备用
- 2026-10-17:内置字体字符集含空格;UI 符号只选 Noto Sans CJK 实际存在的字形(`＊`/`♻` 替代 `✦`/`↻`)
- 2026-08-06:跨平台验证深度=全量交叉编译(用户确认);移动端字体=内置子集(107KB);Android 用预编译 so 模式工程;macOS/iOS 本环境无法验证,交付配置+文档
- 2026-08-06:移除 web 版,项目转为 C++ 单实现(详见归档)
- 2026-08-06:粘球方向控制采用"球→鼠标连线"语义(详见归档)
- 2026-08-06:状态文件体系按 1.txt 规范建立(详见 `PROGRESS.md` 整理记录)
