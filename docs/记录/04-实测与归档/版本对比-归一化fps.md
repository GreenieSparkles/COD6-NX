# 版本对比：把"场景权重"扣掉之后，fps 到底有没有变

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


写于第 155 轮末。起因：用户质疑"稳态帧率在提高？我认为只是波动"。
结论先行：**用户是对的。v53 → v60 的 fps 在扣掉"绘制/帧"之后基本是平的。**

统计脚本：`D:\devkitPro\tools\normalizar_fps.py`（可重跑）。

## 一、数据（7 次运行，剔除启动窗口和卡顿窗口）

| 号 | 版本 | 绘制/帧 | fps 中位 | 拟合期望 | **残差** |
|---|---|---|---|---|---|
| 147 | 53 | 72.9 | 14.92 | 14.11 | **+0.81** |
| 148 | 53 | 83.3 | 12.59 | 13.07 | **−0.48** |
| 149 | 54 | 82.2 | 13.18 | 13.18 | **−0.00** |
| 150 | 57 | 85.5 | 11.77 | 12.85 | **−1.08** |
| 151 | 58 | 83.0 | 14.00 | 13.10 | **+0.90** |
| 152 | 59 | 74.5 | 14.25 | 13.95 | **+0.30** |
| 153 | 60 | 53.1 | 15.62 | 16.08 | **−0.46** |

回归：**fps = 21.36 − 0.100 × （绘制/帧）**，相关系数 **r = −0.84**。

也就是说：**fps 主要由"这次停在哪个菜单画面"决定**——每多 10 个绘制/帧，fps 掉约 1.0。

## 二、三个必须承认的问题

1. **153 号的"15.62 fps"不是进步，是场景更轻。** 那一次平均只有 53.1 个绘制/帧，
   拟合期望是 16.08，实测 15.62，**比趋势线还略低**。
2. **版本间的残差范围只有 −1.08 ~ +0.90（约 ±7%）**，和同版本不同次运行的波动同量级
   （149 与 150 行为几乎相同，却是 13.18 vs 11.77）。
3. 所以"稳态帧率在提高"这个说法**是错的**，那是拿不同场景的数在比。

## 三、哪些结论仍然成立（因为它们不依赖 fps）

这两条是**在绘制路径里直接测出来的**，与停在哪个画面无关：

| 事实 | 证据 |
|---|---|
| memexport 的 CPU 读回被去掉了 | `media_cola`（读回那一段）0.43 → **0.00 ms/绘制**；带 memexport 的慢绘制 350 次/41.0 s → **16 次/0.13 s** |
| CPU 与 GPU 在串行，而不是重叠 | FENCE：每帧阻塞 **28.5 ms**，而 GPU 只落后 **1.66 个提交**（3 帧在飞的环应是 ~6） |

这两条改变了后续方向（去同步、恢复重叠），但**不等于 fps 已经变好**。

## 四、以后的比较口径（从第 156 轮起照此执行）

1. **主指标不是 fps，而是"与场景无关的机制指标"**：
   - `[perf] FENCE` 的 `retraso`（GPU 落后几个提交）与 `media`（阻塞毫秒）
   - `[perf] PREP4 / COLA` 的每绘制毫秒（分项）
   - `[perf] DIBUJOS` 的 `en_draw` 占墙上时间比例
2. **fps 必须报告"绘制/帧"并做归一化**：`归一化 fps = 实测 fps + 0.100 ×（82 − 实测绘制/帧）`。
   82 是这些运行的典型值。归一化后的数才有可比性。
3. **同一画面、同一停留时长**：测试时停在主菜单不动 30 秒，别翻页（翻页会换场景）。
   这一条会写进每轮的 `说明.txt`。
4. **能做 A/B 就做 A/B**：参考项目 `measuring.md` 的建议是在**同一次会话**里比两个版本，
   而不是两次运行互相比。
