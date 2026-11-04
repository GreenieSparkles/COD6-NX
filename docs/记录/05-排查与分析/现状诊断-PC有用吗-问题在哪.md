# 现状诊断：PC 到底有没有用，Switch 现在的问题在哪

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


写于第 156 轮。所有数字都来自本项目的实测日志（154 号 / 152 号 / 118 号 / 128 号）。

---

# 一、"PC 版跑通并高帧数"对 Switch 有用吗？

## 短答

**有用，但只对一件事：写渲染器的迭代成本。它对"Switch 上会跑多快"没有任何预测力。**

## 为什么不能拿 PC 的帧数当参考

| 项目 | PC（WSL/lavapipe 或 Windows+4060） | Switch |
|---|---|---|
| CPU 单核 | x86，比 A57 快约 5~10 倍 | Cortex-A57，1026 MHz，**只有 3 核** |
| GPU | lavapipe（CPU 软渲染）或 RTX 4060 | GM20B，307~768 MHz |
| 内存带宽 | 十几~几十 GB/s | 25.6 GB/s，和 CPU 共用 |
| 我们量的对象 | 每绘制 CPU 成本、逻辑正确性 | **每绘制 CPU 成本 × 5142 次/帧** |

参考项目自己的 `measuring.md` 写得最直白：
> "The PC tells you where the work is, but never how much it costs on the Switch."

而且他们的高性能数字全是**主机上、标准频率**测的；PC 只用来看逻辑对不对。

## 那 PC 到底值在哪

只值这一点：**渲染器要靠几百次迭代写出来。**

    现在（插卡）：构建 → 拷卡 → 用户拔插 → 启动 → 停/玩 → 退出 → 取日志 ≈ 分钟级，且要用户配合
    PC 上：      改代码 → ninja → Xvfb + lavapipe 跑 50 秒 → 截图 + 日志 ≈ 十秒级，全程我自己能做

参考项目正是这么干的："It was developed on the PC first. In two days it drew videos,
menus, the HUD and full races"。**他们的原生渲染器是在 PC 上长出来的，然后搬到 Switch 上测。**

## 但有一条更便宜的路，比 PC 更好

**用 netloader / nxlink 直接把 nro 推到真机。** 本机已经有 `/opt/devkitpro/tools/bin/nxlink`。
它的好处是：**同一台 Switch、同一块 GPU、同样的频率**——迭代速度接近 PC，但量到的数字**就是真机数字**。

所以优先级应该是：

    1. netloader（真机 + 秒级迭代）  ← 最好
    2. PC（秒级迭代，但只能验逻辑）  ← 备选
    3. 插卡（现状）                  ← 最慢，且要用户配合

## 一个必须说清的风险

如果目标还是"测指标、调参数"，**PC 完全没用**——它量不出 Switch 的成本。
PC 只有在"要写渲染器"这个前提下才值得投入。这一点必须先定下来，否则就是又一次绕路
（前面第 141~155 轮已经绕了 15 轮"能测出问题但改不动架构"的圈）。

---

# 二、Switch 现在的问题到底在哪

主菜单（154 号实测，13.22 fps，75.7 ms/帧，80.8 绘制/帧）的时间去哪了：

| 项 | 每帧 | 依据 |
|---|---|---|
| **等 GPU（fence 阻塞）** | **≈30 ms** | `[perf] FENCE media=29.96 ms`，每帧约 1 次 |
| 绘制函数内的 CPU | ≈14 ms | `[perf] PREP4`：begin 0.159 + prims 0.005 + tex 0.001 + rt 0.014 ≈ 0.18 ms/绘制 × 80 |
| 游戏自身逻辑 / 音频 / 输入 | ≈16 ms | 118 号：跳过全部绘制时 61.7 fps = 16.2 ms/帧 |
| 呈现 + 其它 | ≈15 ms | 剩下的差额 |

## 三个问题，按重要性

### 问题 1（最根本）：模拟层的每绘制成本 × 绘制数量

    主菜单：82 次绘制/帧 × ~0.18~0.5 ms  = 15~40 ms/帧
    关卡内：5142 次绘制/帧 × 0.42 ms     = 2.1 秒/帧  ← 这就是 0.5 fps 的全部原因

参考项目自研渲染器是 **1.5~1.8 µs/绘制**，我们是 **420 µs/绘制**，差 **250 倍**。
关卡内再乘以 5142，差距就是"能不能玩"。
**这一条不换架构就永远解决不了。**

### 问题 2：帧环只有不到 1 帧深 → CPU 和 GPU 在轮流干活

代码声明 3 帧在飞、每帧 2 个提交，本应等到 ~6 个提交；实测只等 **1.69**。
所以每帧 30 ms 的等待不是"GPU 慢三帧"，而是**串行**：帧长 = CPU + GPU，而不是 max(CPU, GPU)。
若重叠正常，按 30 ms GPU + 30 ms CPU 算，帧长约 35 ms ≈ **28 fps**。
版本 62 就是改这个（3 → 6 帧 + `[perf] ANILLO` 计数）。

### 问题 3：GPU 每帧的活有多少，现在还没有拆开

主菜单：83 次绘制、**剪刀面积 39.2 个屏幕**、1 次 resolve、2 个渲染目标、10 次 transfer 绘制。
39 个屏幕的过绘制，对片元着色器（运行时翻译出来的，带着 Xenos 的裁剪/alpha 逻辑）来说不便宜。
**但我们还没有把 GPU 时间按 pass 拆开**——这正是参考项目第 3 步（GPU 60 → 33 ms）做的事。

### 次要（不是现在的主要矛盾）

- 构建层：55,205 次弱别名调用、0 处 `-flto` → 游戏自身代码还能快 10~15%
- `Cinematic` 忙等：主菜单 46%、关卡内 77%
- 这些都影响"游戏自身逻辑"那 16 ms，而它现在只占 1/5

---

# 三、结论：两句话

1. **PC 有用，但它的价值是"让我能快速写渲染器"，不是"PC 跑得快所以 Switch 也会快"。**
   而且如果 netloader 能用，netloader 比 PC 更好（真机 + 快迭代）。
2. **Switch 的问题不是"还没测够"，是"每次绘制 420 µs"和"帧环不重叠"。**
   前者只有自研渲染器能解（关卡内的唯一出路），后者版本 62 正在测。
   继续在模拟层上调参数，最多把主菜单从 13 fps 挪到 20 fps 左右，**关卡内永远是 0.5 fps**。
