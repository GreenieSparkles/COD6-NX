# 对照结论（用已有日志，不需要再跑）

<!-- AUTHOR-BANNER -->
> # 🚫⛔ 致 Telegram「DarkNSP」群的搬运者：请立刻停止 ⛔🚫
> # 🚫⛔ TO THE REPACKERS IN THE TELEGRAM GROUP “DarkNSP”: STOP NOW ⛔🚫
>
> ### ❌ 不要再打包包含游戏资源的整合包 / Stop bundling the game files
>
> 看到这个 GitHub 后，请**立刻停止**把本项目和游戏本体打包成「整合包」的行为。
> **Stop packaging this project together with the game data. Immediately.**
>
> 这里只是**半成品**——贴图会花、光影不对、随时崩，bug 一大堆还没修。
> **This is an unfinished work in progress.** Textures corrupt, lighting is wrong, it crashes at random, and a long list of bugs is still unfixed.
>
> 把游戏本体塞进去一起发，**既侵权，也是在欺骗玩家和粉丝**：
> 他们花几个小时下载好几个 GB，拿到的却是一个走两步就花屏、进关卡就出错的版本。
> **白费功夫下个不能玩的版本，究竟图什么？**
>
> **Bundling the game data is copyright infringement — and it is deceiving players and fans.** They spend hours downloading several gigabytes and end up with a build that corrupts its textures after a few steps and breaks down as soon as a level loads. **Why waste people's time with a version that cannot be played?**
>
> ### 🛑 这样做是不负责任的 / This is irresponsible
>
> 你说你在"分享"，实际是在拿别人的时间和期待换流量。
> 玩家以为下到的是成品，结果是半成品；出了问题，骂的是这个项目，不是你的搬运。
> **You call it sharing. In reality you are trading other people's time and expectations for attention.** Players think they downloaded something finished, and when it breaks, the project gets blamed — not your repack.
>
> ### ✅ 要搬运，请这样做 / If you must share it, do this
>
> 1. **只搬仓库本身和这份声明**，不要附带任何 `.ff` / `.bik` / `.pak` / `game_root`。
> 2. 必须写清楚：**半成品、有大量 bug、不保证能玩、非商业**。
> 3. 必须保留作者署名与仓库链接。
>
> **1.** Share **the repository and this notice only** — no `.ff` / `.bik` / `.pak` / `game_root` files.
> **2.** State it plainly: **work in progress, many bugs, no guarantee that it is playable, non-commercial**.
> **3.** Keep the author credit and the repository link.
>
> ---
>
> # ⚠️ 半成品 · 严禁商用 · 严禁盗用
> # ⚠️ WORK IN PROGRESS · NO COMMERCIAL USE · DO NOT REUPLOAD AS YOUR OWN
>
> **作者 / Author：** BILIBILI（B站）**一个没有人看的UP主**　UID **122607497**
> ｜ GitHub **GreenieSparkles**
>
> - 🚧 **这是半成品 / This is a work in progress.** 大量已知与未知 bug，参数和实现随时会变，不保证能通关、能玩完。
>   Expect plenty of known and unknown bugs; parameters and implementation change without notice. No guarantee that it is playable to the end.
> - 💰 **严禁商业用途 / NO COMMERCIAL USE.** 不得售卖、不得用于付费内容、不得捆绑进任何收费产品或服务、不得用于引流变现。
>   May not be sold, paywalled, bundled into any paid product or service, or used for monetised traffic.
> - 🪪 **严禁盗用与冒名 / DO NOT REUPLOAD AS YOUR OWN.** 转载、搬运、二次打包必须保留本署名与声明，并注明来源仓库。
>   Any repost, mirror or repack must keep this attribution and notice, and must link back to the source repository.
>
> ---
>
> ## 📜 免责声明 / Disclaimer
>
> **中文**
>
> 1. 本项目**仅供学习与技术研究**，目的是研究 Xbox 360 程序的重编译与图形 API 的移植，不提供、不包含、不分发任何游戏数据。
> 2. 使用本项目需要你**自己合法拥有**《使命召唤：现代战争2》的正版游戏。请自行准备游戏文件。
> 3. 本项目与 Activision Publishing, Inc.、Infinity Ward、id Software、Nintendo **没有任何关系**，未获其授权或认可。所有商标与版权归各自所有者。
> 4. 本项目**按现状提供，不附带任何担保**，包括但不限于可用性、稳定性、特定用途适用性。使用风险由使用者自行承担。
> 5. 因使用本项目导致的**任何直接或间接损失**（存档损坏、主机异常、账号问题、数据丢失等），作者**不承担责任**。
> 6. 本项目**可能随时停止更新**，作者没有义务修复任何问题、回复任何提问或提供任何支持。
> 7. 若权利人认为本项目侵犯其权益，请联系删除，本项目会立即配合处理。
>
> **English**
>
> 1. This project is **for study and technical research only** — it explores recompiling Xbox 360 code and porting a graphics API. It does not provide, contain or distribute any game data.
> 2. You must **legally own** a genuine copy of *Call of Duty: Modern Warfare 2*. Supply the game files yourself.
> 3. This project is **not affiliated with, authorised by, or endorsed by** Activision Publishing, Inc., Infinity Ward, id Software or Nintendo. All trademarks and copyrights belong to their respective owners.
> 4. Provided **“as is”, without warranty of any kind**, express or implied, including but not limited to availability, stability or fitness for a particular purpose. You use it at your own risk.
> 5. The author **accepts no liability** for any direct or indirect damage arising from its use (corrupted saves, console anomalies, account issues, data loss, etc.).
> 6. Development **may stop at any time**. The author is under no obligation to fix anything, answer questions or provide support.
> 7. If a rights holder believes this project infringes their rights, please get in touch and it will be removed promptly.
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
