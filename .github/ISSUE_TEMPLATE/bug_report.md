<!-- AUTHOR-BANNER -->
> # ⚠️ 半成品 · 严禁商用 · 严禁盗用
> # ⚠️ WORK IN PROGRESS · NO COMMERCIAL USE · DO NOT REUPLOAD AS YOUR OWN
>
> **作者 / Author：** BILIBILI（B站）**一个没有人看的UP主**　UID **122607497**
> ｜ GitHub **GreenieSparkles**
>
> - **这是半成品 / This is a work in progress.** 大量已知与未知 bug，参数和实现随时会变，不保证能通关、能玩完。
>   Expect plenty of known and unknown bugs; parameters and implementation change without notice. No guarantee that it is playable to the end.
> - **严禁商业用途 / NO COMMERCIAL USE.** 不得售卖、不得用于付费内容、不得捆绑进任何收费产品或服务、不得用于引流变现。
>   May not be sold, paywalled, bundled into any paid product or service, or used for monetised traffic.
> - **严禁盗用与冒名 / DO NOT REUPLOAD AS YOUR OWN.** 转载、搬运、二次打包必须保留本署名与声明，并注明来源仓库。
>   Any repost, mirror or repack must keep this attribution and notice, and must link back to the source repository.
>
> ---
>
> ### 致 Telegram「DarkNSP」群的搬运者
>
> 看到这个 GitHub 后，请**不要再打包包含游戏资源的整合包**。
>
> 这里只是**半成品**，bug 一大堆还没修。把游戏本体塞进去一起发，既**侵权**，也是在**欺骗玩家和粉丝**——他们花时间下载好几个 GB，拿到的却是一个走两步就花屏、随时崩的版本，**白费功夫下个不能玩的版本，图什么？**
>
> 这样做**不负责任**。要搬运，就只搬**仓库本身和这份声明**，并且必须写清楚：**半成品、有大量 bug、不保证能玩**。
>
> ### To the repackers in the Telegram group “DarkNSP”
>
> If you see this GitHub repository, **stop repacking it together with the game files**.
>
> This is an **unfinished build** with a pile of bugs still unfixed. Bundling the game data with it is **copyright infringement**, and it is also **deceiving players and fans** — they spend hours downloading several gigabytes only to get a build that corrupts its textures after a few steps and crashes at random. **Why waste people's time with a version that cannot be played?**
>
> Doing that is **irresponsible**. If you must share it, share **the repository and this notice only**, and state it plainly: **work in progress, many bugs, no guarantee that it is playable**.
>
> ---
>
> 仓库 / Repository：https://github.com/GreenieSparkles/COD6-NX
<!-- /AUTHOR-BANNER -->

---
name: Bug 报告
about: 画面、帧数、崩溃等问题
title: "[Bug] "
labels: bug
assignees: ''
---

## 现象

<!-- 一句话说清：哪里、什么样子 -->

## 复现步骤

1.
2.
3.

## 环境

| 项 | 值 |
|---|---|
| 版本（日志里 `[cod6] VERSION`） | |
| Switch 型号 / 系统版本 | |
| 时钟（sys-clk 设置） | |
| 游戏数据来源（自备份） | |

## 必须附上的文件

- [ ] `logs/rex/rex_stderr.log`
- [ ] `logs/rex/rex_perfil.log`
- [ ] `cod6.toml`
- [ ] 出问题位置的截图（**含 HUD**，便于定位场景）

## 补充信息

- 稳定复现？还是玩久了才出现？
- 是否只在特定画质预设下出现？
- 是否试过 `cod6_renderizador = "xenos"`（对照用，很慢但正确）？

## 如果抓了贴图（可选但很有用）

- [ ] `cap/` 里的 `tex_*.img`
- [ ] 日志里的 `[dtab]` 行

<!-- 抓取方法见 README 的「诊断」一节 -->
