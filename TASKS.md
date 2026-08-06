# TASKS.md — 任务索引

维护规则:任务启动时加入 Active 表并同步 `PLAN.md`;完成后移入 Completed 表,归档详情写入 `docs/tasks/archive/`。需要历史背景时按需读取相关归档,不要默认加载全部。

## Active(进行中)

当前无进行中任务。新任务启动时在此登记并同步 `PLAN.md` / `PROGRESS.md`。

## Completed(已完成)

| 任务 | 完成日期 | 归档 |
| --- | --- | --- |
| web 版玩法与特效扩展(彩虹/再生砖、激光/粘球/巨球、fx 特效、音效) | 2026-08-06 | `docs/tasks/archive/2026-08-06-web-gameplay-expansion.md` |
| web 版主菜单 / 关卡选择 / 粒子特效开关 | 2026-08-06 | `docs/tasks/archive/2026-08-06-web-menu-level-select-settings.md` |
| C++ / SDL3 桌面版重写(功能与 web 版一致) | 2026-08-06 | `docs/tasks/archive/2026-08-06-cpp-sdl3-rewrite.md` |
| C++ 版窗口自由缩放(等比 letterbox) | 2026-08-06 | `docs/tasks/archive/2026-08-06-resizable-window.md` |
| 工作区状态文件体系整理 | 2026-08-06 | (状态文件本身:AGENTS/PLAN/PROGRESS/TASKS) |
| 粘球自由方向发射(C++ 版 + web 同步) | 2026-08-06 | `docs/tasks/archive/2026-08-06-sticky-aim-direction.md` |
| 移除 web 版(TypeScript 实现) | 2026-08-06 | `docs/tasks/archive/2026-08-06-remove-web-version.md` |
| 跨平台兼容性拓展(Windows/macOS/Linux/iOS/Android) | 2026-08-06 | `docs/tasks/archive/2026-08-06-cross-platform.md` |

## 备注

- 本项目无 Git 仓库,归档不引用提交/分支;删除性操作前先备份(web 版源码备份:`/tmp/breakout-web-backup-20260806.tar.gz`,tmpfs 临时)
- 当前为 C++ 单实现项目,web 版历史归档保留原样(历史事实),不得恢复为维护对象
