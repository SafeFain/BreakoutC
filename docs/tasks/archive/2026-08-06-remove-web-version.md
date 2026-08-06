# 任务归档:移除 web 版(TypeScript)

- **日期**:2026-08-06
- **状态**:已完成(归档)
- **决定**:用户明确"不用维护 WEB 版了,可以将其移除"

## 实施内容

1. **源码备份**(无 Git 仓库,删除不可逆):`tar czf /tmp/breakout-web-backup-20260806.tar.gz`(排除 `node_modules`/`dist` 可再生物,19 个文件,32KB)
2. **删除**:`rm -rf web/`(含 node_modules 42MB 等)
3. **文档清理**:`README.md`(移除双实现说明/目录/差异对照表,标注历史)、`AGENTS.md`(移除 web 模块映射/命令/双实现一致性原则/验证规则,技术栈标注"已移除")
4. **状态文件**:TASKS/PLAN/PROGRESS 同步;归档本任务

## 验证

- `cmake --build build -j8` 构建无回归(web/ 与根项目相互独立)
- `./build/neon-breaker --aim-test` ALL PASS
- 工作区仅剩 C++ 单实现

## 涉及文件

删除:`web/`;更新:`README.md`、`AGENTS.md`、`TASKS.md`、`PLAN.md`、`PROGRESS.md`

## 备注

- 备份在 `/tmp`(tmpfs,重启丢失);如需长期保留请自行移出
- 归档 `docs/tasks/archive/` 中 web 版相关历史任务记录保留原样(历史事实)
- web 版不得恢复为维护对象
