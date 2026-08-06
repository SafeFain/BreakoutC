# 任务归档:粘球自由方向发射(C++ 版 + web 版同步)

- **日期**:2026-08-06
- **状态**:已完成(归档)
- **关联版本**:根目录 C++ 项目为主,web/ 同步(双实现一致性原则)

## 目标

粘球(sticky)增益的发射方向可由玩家自由选择:球与鼠标所在位置的连线即为发射方向(鼠标在球上方即向上打,左右即斜向)。

## 实施内容

### C++ 版(src/)

1. **输入层**:`InputActions` 增加 `pointerY`;`InputManager` 跟踪指针世界坐标 Y(鼠标移动/按下、触屏),离开游戏区清空
2. **方向计算**:`Game::computeLaunchDirection(dx, dy, ballX, ballY, aimX, aimY)` 静态方法:
   - 球→鼠标向量归一化即为发射方向
   - 鼠标与球距离 ≤ 4px:回退默认方向(0.55, -0.835)
   - 方向接近水平或向下(vy > -0.15):强制最小仰角,防止向下击穿挡板
3. **发射**:`launchStuckBall` 使用瞄准方向;顺手修复 slow/fast 效果未作用于发球速度的问题(速度 × 0.78/1.25)
4. **瞄准线渲染**:粘球时绘制球→鼠标的虚线瞄准线(2.5px,8-5 虚线)+ 鼠标位置光圈环(`Renderer::thickLine` 新原语)
5. **自检**:`--aim-test` 无头测试,5 个用例(右上/正下方抬升/重合回退/左下抬升/水平抬升)

### web 版同步(web/src/)

- `types.ts` / `input.ts`:pointerY 跟踪(mapY 基于 WORLD_HEIGHT)
- `game.ts`:aim 字段 + `launchStuckBall` 同构实现(含最小仰角与速度因子)
- `renderer.ts`:瞄准线用 Canvas `setLineDash` 绘制

## 验证

- C++ 构建零错误零警告;`--aim-test` 5 用例全部 PASS
- dummy 截图回归(state=2,退出码 0)
- `cd web && npm run build`(tsc strict)通过

## 涉及文件

C++:`src/{types,input,game,renderer,main}.{h,cpp}`;web:`web/src/{types,input,game,renderer}.ts`;`README.md`

## 备注

- 键盘玩家(无鼠标移动)发射回退原固定方向;触屏以触摸点作为方向
- 瞄准方向使用"最后有效指针位置"(指针移出游戏区后保留)
