# 对照结论（用已有日志，不需要再跑）

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


数据源：日志全量分析.md（148 份日志自动汇总）

## 一、主菜单：纯 SDK（无任何 cod6_ 开关）

| 版本 | fps | 绘制/帧 |
|---|---|---|
| 2026-10-30-5（93/94/95/96/97/98） | **12.43 / 12.91 / 12.56 / 12.12 / 12.42 / 12.56** | 70~84 |
| 2026-10-30-6（99） | 11.97 | 70.0 |
| 2026-10-30-7（100/101） | 12.59 / 12.35 | 69.8 |
| 2026-10-30-8（102） | 11.93 | 70.0 |
| 2026-10-30-9（103） | 12.33 | 70.4 |
| 2026-10-30-10（104） | 12.02 | 69.8 |

→ 纯 SDK 稳定在 **11.9 ~ 12.9 fps**（中位数 12.4）

## 二、主菜单：带我们自己代码的版本

| 版本 | fps | 说明 |
|---|---|---|
| 2026-10-30-14（108） | 5.35 | 第一次接 native，未超频 |
| 2026-10-30-16（110） | **12.10** | native，超频前 |
| 2026-10-30-23（116） | **18.69** | native + 超频 |
| 2026-10-30-26（120） | 15.87 | native + 超频 |
| 2026-10-31-32（127） | 15.77 | native + 超频 |
| 2026-10-31-39（136） | 0.56 | 关卡内 |
| 2026-10-31-51（146） | **13.20** | native 图形系统 |
| 2026-10-31-53（148） | 12.59 | 同上 |
| 2026-10-31-54（149） | 13.18 | 同上（带插桩） |

→ 带我们代码时：**12.1 / 13.2 / 12.6 / 13.2**（同为超频关闭或相近状态）
→ 超频时最高 18.69

## 三、结论（这就是你问的"不是都实验过吗"）

    1. 纯 SDK 主菜单 = 11.9 ~ 12.9 fps
    2. 我们的代码（自写图形系统 + 插桩）在主菜单 = 12.1 ~ 13.2 fps
       → 我们的代码没有拖慢，甚至略快。
    3. 主菜单的天花板（跳过全部绘制）= 61.74 fps
       → 12 fps 和 61 fps 之间的差距全部来自「70 次绘制/帧」的 SDK 绘制成本。

    所以：
        我们自己的层是"免费"的（版本 56 那个对照实验已经没有意义，不必再跑）
        真正的钱仍在 SDK 的每绘制成本里（resto = 150 ms / 120 次 = 1.25 ms/次）

## 四、下一步（唯一有意义的方向）

    149 号日志把 150 ms 的来源定死了一半（详见 分析-150毫秒在哪.md）：
        每 120 次绘制里 112~115 次 <0.1 ms，只有 0.9 次 >=20 ms
        「管线配好 → 发出 vkCmdDraw」= 0.010 ms/次（累计 18840 次）
    → 钱在「发出 vkCmdDraw 之后 → return」，而且只在那一两次超大绘制上。

    版本 57 把这个区间切成四段（[perf] COLA / [perf] COLA_LENTO）：
        draw   = 发出绘制那一刻（可能在与 GPU 同步）
        inval  = memexport 失效（最可疑：else 分支 RangeWrittenByGpu(0, 512 MB)）
        cola   = 读回 / 收尾
        pausa  = 与上一次绘制的间隔（区分是不是帧边界现象）

    注意：版本 54 那次插桩有 bug（fase5 写在 return 之后是死代码），
    FASES 行的 bind/sysconst/bindings/vfetch_resto 与括号里的 vert/tex/resto_prep 都不可信，
    57 已修 fase5，但括号那三个字段仍是坏的，别再引用。
