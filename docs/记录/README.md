# 项目记录

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


共 **62** 份原始笔记，按类别整理。另有 [全部 314 轮实机测试](00-全部实机测试/全部314轮.md)（自动从每次运行的日志生成）。

---

## 分类

| 类别 | 份数 | 内容 |
|---|---|---|
| [01-开发日记](01-开发日记/) | 3 | 按时间写的开发日记 |
| [02-里程碑与突破](02-里程碑与突破/) | 7 | 关键节点：第一次画出几何体、接管绘制、阶段成果 |
| [03-每轮记录](03-每轮记录/) | 14 | 单轮的详细记录（做了什么、看到什么） |
| [04-实测与归档](04-实测与归档/) | 4 | 实测记录汇总、按轮次归档的总结、版本 fps 对比 |
| [05-排查与分析](05-排查与分析/) | 12 | 卡点分析、日志全量分析、方向判断 |
| [06-环境与构建](06-环境与构建/) | 6 | PC 软解环境、Windows 构建、Switch 构建与部署 |
| [07-移植指南](07-移植指南/) | 8 | 移植总纲、平台笔记、原生渲染器设计、参考项目对照 |
| [08-交接与总结](08-交接与总结/) | 5 | 交接文档、状态总览（新会话先看这些） |
| [09-避雷](09-避雷/) | 1 | 踩过的坑 |
| [10-其他](10-其他/) | 2 | 未归类的 |
| [00-全部实机测试](00-全部实机测试/全部314轮.md) | 314 | 314 轮逐条记录：时间、版本、跑到第几帧、纹理统计、有无崩溃 |

---

## 建议阅读顺序

1. [全部 314 轮实机测试](00-全部实机测试/全部314轮.md) —— 这个项目到底跑了多少、跑成什么样
2. [08-交接与总结](08-交接与总结/) —— 当前状态
3. [01-开发日记](01-开发日记/) —— 从头到尾
4. [09-避雷](09-避雷/) —— 别再踩
5. [06-环境与构建](06-环境与构建/) —— 怎么复现

