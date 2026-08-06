# 任务归档:web 版主菜单 / 关卡选择 / 粒子特效开关

- **日期**:2026-08-06
- **状态**:已完成(归档)
- **关联版本**:`web/`(TypeScript + Vite)

## 目标

主菜单界面(开始游戏按顺序、关卡选择任意一关、设置);设置新增「粒子特效」开关。用户确认:关卡全部解锁、粒子特效开关独立于拖尾/球色。

## 实施内容

1. **任意关卡开始**:`Game.selectLevel(index)` — 校验范围、定位关卡、进入 `ready`;选关后按顺序进入后续关卡
2. **主菜单双视图**:`#menuActions`(开始 / 关卡选择 / 设置)与 `#levelSelectPanel`(5 关卡按钮 + 返回),`menuView` 状态驱动切换,非 menu 状态保持原单按钮布局
3. **选关面板**:5 个关卡按钮由 `LEVELS` 动态生成,中英文名按语言切换,2 列网格样式
4. **粒子特效开关 `impactFx`**:贯穿 `StorageData`/`GameSnapshot`/`FxSystem.enabled`/设置 UI;关闭后碎片/飘字/屏震/连击爆发/背景光点消失,拖尾与球色独立不受影响

## 验证

- `npm run build` 通过;`vite preview` + curl HTTP 200,新元素(`levelSelectButton`/`levelSelectPanel`/`fxToggle`)进产物
- 遗留:浏览器人工试玩(三入口、选关即玩、开关即时生效、中英切换、刷新持久化)

## 涉及文件

`web/src/{game,main,i18n,storage,types,fx}.ts`、`web/index.html`、`web/src/style.css`

## 后续

本任务之后进行了 C++/SDL3 桌面版重写(见 `2026-08-06-cpp-sdl3-rewrite.md`),功能以本任务后的 web 版为准。
