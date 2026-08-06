# 自动 CD 发布流水线(2026-08-06)

## 目标
在五平台 CI 全部跑通后,增加自动持续交付(CD):推送 `v*` tag 即自动构建五平台产物并发布到 GitHub Release。

## 完成内容
- 新增 `.github/workflows/release.yml`(触发:`push tags v*` + `workflow_dispatch`):
  - **linux**(ubuntu):构建 + aim-test → 打包 `neon-breaker-linux-x86_64.tar.gz`(可执行 + SDL3/SDL3_ttf 运行库,保持 vendor 布局匹配 `$ORIGIN/../vendor/...` RUNPATH + 字体,解压即跑)
  - **windows**(windows):vcpkg + MSVC 构建 → zip(exe + SDL3.dll/SDL3_ttf.dll + 字体)
  - **macos**(macos):构建 → 打包 dylib 传递闭包(otool 迭代收集 Homebrew dylib + `install_name_tool` 重写为 `@loader_path`,4 轮覆盖嵌套依赖)+ 字体 + README
  - **android**(ubuntu):NDK 交叉编译 + Gradle assembleDebug → `neon-breaker-android-arm64.apk`(复用 CI 的 cache key 与 SDK 自装逻辑)
  - **ios**(macos):模拟器交叉编译 .app + **真机未签名 IPA**(iphoneos arm64 构建 `CODE_SIGNING_ALLOWED=NO`,`Payload/` 结构 zip,开发者可自行重签名安装;字体进 bundle)
  - **release**(ubuntu,needs 全部):download-artifact 汇总 → `gh release create`(tag 已存在则 `--clobber` 覆盖),`--generate-notes`;手动触发生成 `nightly-<时间戳>`
- README 新增 CD 小节;AGENTS 目录结构更新

## 验证
- act 端到端跑通 Linux job:安装依赖/构建/打包全部 Success(tar.gz 597ms 生成)
- 本地模拟打包结构端到端验证:包内 RUNPATH `$ORIGIN/../vendor/...` 生效、`--aim-test` ALL PASS、dummy 截图 state=2、tar.gz 3.7MB
- Windows/macOS/Android/iOS 打包步骤为 CI 已验证逻辑的复用(编译链路均已绿),首次真实运行在 tag 推送后

## 遗留
- 首次 tag 推送触发真实 CD;iOS 真机包需开发者账号签名(模拟器包无需)
- act 本地限制:upload-artifact@v6 认证不兼容(与 CI 相同),act 验证到打包产物为止
- 发布权限:workflow 需 `contents: write`(release job 已声明)
