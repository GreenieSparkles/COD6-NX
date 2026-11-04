# COD5 源码对照：我们要的"完整渲染器"长什么样

<!-- AUTHOR-BANNER -->
> # 🚫⛔ 致 Telegram「DarkNSP」搬运者的一封公开信 ⛔🚫
>
> 我知道你在看。你可能已经点开这个页面，扫了一眼，觉得「又是那个做半成品的人」，然后顺手把它和几个 G 的游戏文件塞进一个压缩包，配上三步「安装教程」——**第一步，把根目录复制到卡上；第二步，安装 NSP 前端；第三步，玩**——就发进群里了。
> **一句 bug 都没提。** 没提贴图会烂，没提帧数会掉到个位数，没提它根本还没做完。
>
> 我想先问你一句：**发布之前，你自己跑过一遍吗？**
>
> 哪怕只跑**一分钟**——在出生点附近看看就行，**不用进第二关**。你会看见贴图在你眼前一点一点地烂掉：**墙上长出人脸，桌子变成电视，弹匣变成砖头**，枪口焰炸成一整块巨大的方块。你会看见画面一黑一亮，像一盏坏了很久的日光灯。至于帧数——**那个「三十五到六十」是我在特别行动「坑道（The Pit）」那种小场景里测出来的；你但凡进的是剧情关卡，帧数就奇差无比，屏幕上有人物、有战斗的时候直接掉到个位数**，卡到你连准星都推不动。
>
> 你看见了，你还会发吗？
>
> 我想你不会。因为**你连一次都没跑过**。你只是看见一个 GitHub 链接，看见「COD6 移植 Switch」这七个字，就把它当成了流量。你没有下载过这个项目，没有在自己机器上试过哪怕一关，没有读过 README 里那句被我加粗了三遍的「这是半成品」。你甚至不知道它跑到第几帧会崩。
>
> 可我知道。**三百一十四轮实机测试，全在这儿。** 我知道它在选枪台会烂，知道光照只有角色两米以内是亮的，知道枪焰会变成一块砖头。我比任何人都清楚它现在有多糟——因为我就是那个从 **10 月 26 日下完 ISO** 起、一路盯着它烂了 **九天** 的人。
>
> 而你，把它包装成「可以玩」，发给了一群相信你的人。
>
> **电报群里已经有人下载完，发现它根本没法玩。原话我一个字都没有改：**

> 「Masih bnyak bug, misi tutorial aja bug masalah render, fps nya juga anjlok, tunggu update aja itupun klo ada, yg mau main cod mnding main yg black OPS 2 atau MW 1 aja」
>
> *（印尼语：bug 还很多，连教学任务都有渲染问题，帧数也崩了；等更新吧——如果有的话。真想玩 COD，不如去玩 Black Ops 2 或者 MW1。）*

> 「not playable yet, and having texture issues.」
>
> *（英语：还不能玩，而且有贴图问题。）*
>
> 他们不是来骂我的，他们是**失望**。他们花几个小时等下载、等解压、等安装，最后拿到一个连菜单都撑不住的版本。他们的时间、他们的硬盘、他们那份期待，被你一次「顺手打包」消耗干净了。
>
> **这不是分享，这是转嫁。** 你把我的半成品，连同我的名声，一起打包送了出去：出了问题，骂的是我；拿到流量的是你。
>
> 我不反对搬运。这个项目放在 GitHub 上就是给所有人看的。你要转，可以——**只转仓库，附上声明，写清楚「半成品、有大量 bug、不保证能玩」**。至于你把游戏本体一起打包——**那不是我的事**，那是动视暴雪和 Infinity Ward 的事（not my business），我不管。
> 但你**不能不负责任**：别写「解压即玩」，那是骗人；别把它说成成品；别省略那句「不保证可玩」——那是我唯一能替玩家守住的东西。
>
> 如果你连这三件事都不愿意做，那就**别搬**。
>
> **我把它放到 GitHub 上，是因为我在这个阶段卡住了。** 贴图这个问题我已经绕了三百多轮还没绕出来，一个人做不下去了——**开源就是想让愿意一起弄的人进来**。这套开源的移植框架——**ReXGlue**——已经诞生了一大批优秀的经典作品：**MW1、BO2、USF4、MK9** 等等，一个接一个被搬上了掌机，而且跑得都还算像样。既然它们可以，我相信一台 **2019 年的设备同样可以完美跑动 2009 年的这款作品**。
>
> 做这件事是出于热爱：我太喜欢这一代了，也太想看着它在这块屏幕上活过来。我不是为了流量，也清楚它现在拿不出手。
>
> 所以请你，别拿它去骗人。
>
> —— 一个没有人看的UP主
> BILIBILI（B站）UID **122607497**　｜　GitHub **GreenieSparkles**　｜　2026-11
>
> ---
>
> # 🚫⛔ An Open Letter to the Repackers in the Telegram Group “DarkNSP” ⛔🚫
>
> I know you are reading this. Maybe you opened this page, skimmed it, thought “oh, that unfinished project again”, then dropped it into an archive together with a few gigabytes of game files, added a three-step “installation guide” — **step one, copy the root folder to the SD card; step two, install the NSP front-end; step three, play** — and pushed it into your group.
> **Not a single word about the bugs.** Not about the textures rotting, not about the frame rate collapsing to single digits, not about the fact that it is not finished at all.
>
> So let me ask you one thing first: **did you even run it once before you shipped it?**
>
> **One minute** would have been enough — just look around the spawn area. You do not even need to reach the second mission. You would watch the textures rot in front of you: **faces growing out of the walls, tables turning into televisions, magazines turning into bricks**, the muzzle flash exploding into one giant solid block. You would watch the screen flip between black and bright like a dying fluorescent tube. And the frame rate? **That “thirty-five to sixty” was measured in a tiny Special Ops map, “The Pit”. The moment you enter a campaign mission the frame rate is dreadful; with characters and combat on screen it drops to single digits** — so slow that you cannot even push the crosshair around.
>
> If you had seen that, would you still have posted it?
>
> I do not think so. Because **you never ran it once**. You saw a GitHub link, you saw “COD6 ported to Switch”, and you treated it as traffic. You never downloaded this project, never played a single mission on your own console, never read the line in the README that I had to bold three times: *this is a work in progress*. You do not even know at which frame it crashes.
>
> I do. **Three hundred and fourteen recorded test runs, all of them documented here.** I know it falls apart in the loadout screen, I know the lighting only reaches two metres around the player, I know the muzzle flash turns into a brick. Nobody on this planet knows how bad it currently is better than I do — because I am the one who has been staring at it rot for **nine straight days, ever since I finished downloading the ISO on 26 October**.
>
> And you wrapped it up as “playable” and handed it to people who trusted you.
>
> **People in the Telegram group have already downloaded it and found that it simply cannot be played. These are their words, unedited:**

> “Masih bnyak bug, misi tutorial aja bug masalah render, fps nya juga anjlok, tunggu update aja itupun klo ada, yg mau main cod mnding main yg black OPS 2 atau MW 1 aja”
>
> *(Indonesian: still lots of bugs — even the tutorial mission has rendering problems, and the frame rate collapses. Just wait for an update, if there ever is one. If you actually want to play Call of Duty, go play Black Ops 2 or MW1 instead.)*

> “not playable yet, and having texture issues.”
>
> They are not angry at me. They are **disappointed**. They spent hours waiting for the download, waiting for the extraction, waiting for the install, and ended up with something that cannot even hold a menu together. Their time, their disk space and their expectations were burned by your one careless repack.
>
> **This is not sharing. This is shifting the cost onto someone else.** You packed up my unfinished build — and my reputation — and shipped it out. When it breaks, I am the one who gets blamed, and you are the one who gets the attention.
>
> I am not against sharing. This project is on GitHub for everyone to read. Share it if you want — **share the repository, attach this notice, and state plainly: work in progress, many bugs, no guarantee that it is playable.** As for bundling the game data in — **that is not my business**; it is between you and Activision Blizzard and Infinity Ward. I do not care about that.
> But you **cannot be irresponsible**: do not write “just extract and play”, that is a lie; do not present it as finished; do not drop the “no guarantee that it is playable” line, because that sentence is the only thing I can still hold on to on behalf of the players.
>
> If you are not willing to do even those three things, then **do not share it at all**.
>
> **I put this on GitHub because I am stuck at this stage.** I have been going around the texture problem for more than three hundred test runs and I cannot get past it alone — **open-sourcing it is my way of asking for help**. This open-source porting framework — **ReXGlue** — has already produced a whole line of excellent classic titles: **MW1, BO2, USF4, MK9** and more, brought onto handheld hardware one after another, and running well enough. If those can do it, then a device from **2019 can certainly run a game from 2009 perfectly**.
>
> I am doing this out of love: I adore this generation of games, and I badly want to see it come alive on this screen. I am not doing it for traffic, and I know perfectly well that it is not presentable yet.
>
> So please — do not use it to deceive people.
>
> — 一个没有人看的UP主
> BILIBILI UID **122607497**　｜　GitHub **GreenieSparkles**　｜　November 2026
>
> ---
>
> ### 🛑 三条底线 / Three bottom lines
>
> 1. **把话说清楚。** 你附不附游戏数据我不管，但必须写明：**半成品、未经完整测试、有大量 bug**。
> 2. **必须写明：半成品、有大量 bug、不保证能玩、非商业。**
> 3. **必须保留作者署名（BILIBILI UID 122607497）与仓库链接。**
>
> 1. **Say it straight.** I do not care what you attach, but you must state: **work in progress, not fully tested, many bugs**.
> 2. **State it plainly:** work in progress, many bugs, no guarantee it is playable, non-commercial.
> 3. **Keep the author credit (BILIBILI UID 122607497) and the repository link.**
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


分析对象：`D:\主机掌机\NSLITE\COD5-SP-NX-源码分享版-e2e7dba.zip`
（来源分支 `feature/nfsmw-nx-port`，**和我们同一个 SDK fork**，日期 2026-11-01）
已解压到 `/opt/switch/cod5-ref`（WSL）。

---

## 一、它的架构（关键发现）

```
NativeGraphicsSystem : public rex::system::IGraphicsSystem   ← 他们替换掉的是"图形系统"
  ├─ Reader（自己读 PM4 环）
  │    └─ 分发：LoadShader / Draw / Resolve / Swap
  ├─ Renderer（native_renderer.cpp，7,340 行）← 真正干活
  │    ├─ 着色器翻译 + 磁盘 SPIR-V 缓存 + 后台线程
  │    ├─ 管线缓存 + 预热 + 后台线程 + 驱动 PipelineCache
  │    ├─ 渲染目标缓存（clear-alias = resolve 不拷贝）
  │    ├─ 每绘制：BuildDrawSetup + Draw
  │    ├─ Resolve / ResolveDownsampled / FlushResolveBarrier
  │    └─ Swap（presenter + frontbuffer）
  ├─ native_textures.cpp（2,992 行）+ texture_pool.cpp（408）← 自己的纹理管线
  ├─ native_geometry_shaders.cpp（1,124 行）← 几何着色器替代（quad/point sprite 展开）
  └─ native_nvk_draw.cpp ← 薄封装，用打了补丁的 Mesa 快速路径（nvk_switch_draw / set4）
```

**要点：他们不是在 Xenia 的命令处理器旁边"寄生"，而是整条替换**——自己读环、自己管状态、自己发绘制。
这正是我们实测出来的唯一可行路线（全跳过模拟绘制 → 菜单 38~44 fps）。

## 二、模块对照表（COD5 → 我们）

| COD5 模块 | 行数 | 作用 | 我们的现状 | 可复用性 |
|---|---|---|---|---|
| `cod5_native_system.cpp` 的 Reader | ~2,100 | 读 PM4 环、分发 LoadShader/Draw/Resolve/Swap | ❌ 0（我们靠 SDK 的 CP） | **高**：Xenos 包格式与 COD6 相同，机械移植 |
| `native_renderer.cpp` | 7,340 | 管线/RT/绘制/resolve/swap | ❌ 0 | **中高**：结构可照搬，着色器与状态假设要按 COD6 改 |
| `native_textures.cpp` + `texture_pool.cpp` | 3,400 | 纹理去 tile/缓存/淘汰 | 🟡 借用 SDK 的 texture cache | **中**：我们已证明能拿到 `VkImageView`，但他们这套更适合接管 |
| `native_geometry_shaders.cpp` | 1,124 | quad/point sprite 展开 | ❌ 0 | **中**：COD6 的 UI 大量用程序化全屏三角（`A4A9`），这块直接相关 |
| `native_nvk_draw.cpp` | 272 | 用 Mesa 的快速路径（NFS 补丁） | ❌ 0 | **高**：配合我们已经有的 NVK，直接调 |
| 各种缓存头（batch/binding memo/descriptor pool/resolve tracker） | ~800 | 每绘制开销优化 | ❌ 0 | **高**：都是独立小模块，逐个搬 |
| `docs/` 80 份中文复盘 | — | 决策依据与验收门槛 | — | **高**：避免重踩 |

## 三、我们和他们差在哪里（同一坐标系）

| 能力 | COD5 | 我们 |
|---|---|---|
| 读 PM4 环 | ✅ 自己读 | ❌ 用 SDK 的 CP |
| 着色器翻译 | ✅ 后台线程 + 磁盘缓存 | 🟡 SDK 的翻译器在用，但我们没接管 |
| 管线管理 | ✅ 缓存 + 预热 + 后台创建 | 🟡 SDK 的缓存在用 |
| 渲染目标 / resolve | ✅ clear-alias（不拷贝） | 🟡 SDK 的实现（慢，占 80% 成本） |
| 纹理 | ✅ 自己的池 + 指纹 + 淘汰 | 🟡 借 SDK 的缓存（已验证可拿 view） |
| 每绘制路径 | ✅ BuildDrawSetup + Draw | ❌ 0（我们只画了 10 个 UI quad） |
| 驱动层桥接 | ✅ nvk_switch_draw | ❌ 0 |
| ZCULL / MSAA 别名 / 深度 24 位 | ✅ 有候选与开关 | ❌ 0 |

## 四、给我们排的计划（照他们的路线图裁剪）

### 阶段 0：资料与基线（1 天）
- 把 COD5 包纳入 `参考资料-COD5-SP-NX-源码\`（只源码 + 文档，不带游戏文件）；
- 读 `docs/中文文档索引.md` 里这 6 份：`原生NVK绘制桥接`、`原生批次记录器`、
  `原生绑定键备忘录`、`原生描述符池寿命`、`原生场景ZCULL`、`deko3d迁移`。

### 阶段 1：ring 骨架（1~2 周，零风险）
- 照 `cod5_native_system.cpp` 的 `Reader` 写一个**只读不画**的版本：
  读 PM4、统计包类型分布、验证环指针推进；
- **画面完全不变**（仍然让 SDK 的 CP 画），只输出统计；
- 产物：COD6 的包类型分布 + 每帧状态变更次数 = 后面所有工作的地图。

### 阶段 2：接管第一类绘制（2~4 周）
- 用他们 `Renderer::Draw` 的结构，先只处理**我们已经验证过的那类 UI quad**（idx=6、有顶点）；
- 与 SDK 的 CP **二选一**（不能并行，v96 的教训）：同一帧里，我们的绘制由我们自己发；
- 验收：画面一致（截图对比）、帧率不变或更好。

### 阶段 3：接管程序化全屏绘制（`A4A9`，64 次/帧）——**收益最大的一步**
- 需要解出它的顶点着色器常量（我们已拿到 `c0/c1 = 像素→NDC`）；
- 参考 `native_geometry_shaders.cpp` 的做法（quad 展开）。

### 阶段 4：渲染目标与 resolve（决定 GPU 侧）
- 照他们的 `GetRenderTargetCanonical` + `FindClearAlias` 实现"resolve 不拷贝"；
- 我们实测里 `PREP_LENTO rt=5.1 ms` 就是这块。

### 阶段 5：驱动桥接 + ZCULL + 缓存模块（持续）
- `nvk_switch_draw`、binding memo、descriptor pool 寿命、batch recorder。

## 五、诚实的代价与风险

| 项 | 估计 |
|---|---|
| 总工作量 | **3~6 个月**（一个人的节奏），阶段 1~2 可见结果，阶段 3 才有帧率跳变 |
| 最大风险 | 半成品状态机（v96 的教训）；必须"整条路径替换"，不能混跑 |
| 关卡上限 | 不变：**14~15 fps**（游戏自身 CPU），除非再做游戏函数原生化 |
| 我们能拿到的确定收益 | **菜单 14 → 30~40 fps** |

## 六、结论

**这个包把"要写什么"从模糊变成了清单**：7,340 行渲染器 + 2,100 行环读取 + 3,400 行纹理
+ 1,124 行几何展开 + 一堆小缓存模块，而且有 80 份复盘告诉我们哪里会踩坑。

我们不需要从零设计——需要的是**按阶段移植 + 按他们的门槛验收**。

---

# 补充（第 193~197 轮）：为什么"自己读环"一直崩

## 结论

**根因不是我的解析代码，而是"读客机内存"这件事本身。**

这个 SDK 打开了**页写监控**（为了纹理失效跟踪）：游戏把某些页设为只读，我们一读就触发一次"模拟缺页"。
主菜单下这类缺页可达 **每秒 20 万次**。

- 我在 SDK 的 GPU 线程里、或在自己起的线程里读环 → 每秒几十万次模拟缺页 →
  线程卡死/崩溃 → 黑屏 + 洋红崩溃屏（洋红是 SDK 异常处理器的崩溃画面，不是我们画的）；

## COD5 项目怎么解决的

他们在自己的 SDK fork 里加了一个开关，并在启动时调用：

```cpp
RexGmSetReadOnlyMapped(true);   // 不再对只读页做写入监控，读到就直接读
```

（cod5_native_system.cpp 构造函数里，注释原文："Nothing watches guest pages for writes
without the SDK's shared memory, so the pages the game makes read-only can stay mapped.
Otherwise every read of them is an emulated fault: 200,000 a second at the menu, which
held it at 8 fps." —— 他们那边不关这个开关，菜单只有 **8 fps**。）

## 我们的现状

```
grep -rn "RexGmSetReadOnlyMapped" sdk/include sdk/src   →   没有任何结果
```

**说明我们的 SDK fork 里没有这个函数**，它在 COD5 那套 fork 里是新增的。

## 所以阶段 1 的正确顺序是

1. **先在我们的 SDK 里加这个开关**（内存层：mapped_memory / shared_memory 的只读页监控，
   加一个全局标志 + RexGmSetReadOnlyMapped(bool) c 接口）；
2. 只在 cod6_renderizador = "nativo" 时打开它；
3. 然后才做"自己读环、自己统计"——那时读环就是纯内存读，不会再有每秒几十万次缺页。

**这一步不做，后面所有"自写渲染器"的工作都会撞同一堵墙**：渲染器要频繁读客机内存
（顶点、索引、纹理、着色器），每次读都触发模拟缺页，性能会比现在更差。

（这条也解释了为什么参考项目 nfsmw-nx 能在 Switch 上跑到 35 fps：他们同样关掉了页监控。）
