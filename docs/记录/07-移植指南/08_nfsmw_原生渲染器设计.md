# How the port draws the game (the native renderer)

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


## In short

- [ReXGlue](glossary.md#rexglue) normally imitates the Xbox 360 GPU. On the Switch that was far too slow for this game:
  a race ran at about 2 frames per second.
- So this port draws the game its own way. The game still believes it is talking to an Xbox 360 GPU; the port reads
  the game's GPU commands and draws the same frame with [Vulkan](glossary.md#vulkan).
- This page explains why the imitation was replaced, how the new renderer works, and what made it fast.
- The code is in `app/src/nfsmw_nativo_*` ("nativo" means native).

## Why the imitated GPU was replaced

Measured on the console in September 2026, with the first build that worked:

| Scene | Imitated Xbox 360 GPU | Without resolves | Without any GPU work |
|---|---|---|---|
| Title screen | 10 FPS | 26.7 FPS | 58.6 FPS |
| Menu (about 800 draws) | 3.3 FPS | | 14 FPS |
| Race | 2.0-2.2 FPS | | 2.1 FPS |

What the numbers say:

- Removing **all** the GPU work did not make the race faster. So the problem was the CPU: about 80 µs of CPU per draw
  to imitate the GPU, plus the game's own [Direct3D](glossary.md#direct3d), plus handling the memory faults of the
  game's memory (see [platform-notes.md](platform-notes.md#exceptions)).
- Each [resolve](glossary.md#resolve) cost 3 to 4 ms of GPU time.
- Two days of cuts on the imitation side (turning off the game's drawing in strips, removing passes in races, updating
  the shadows less often) only reached about 3.7 FPS in a race.

The limit was far below 30 FPS, so the decision was to stop imitating the GPU and draw the game's frames with Vulkan
directly.

## How it works

### The idea: read the game's commands, keep its Direct3D

Other recompiled ports, like Sonic Unleashed and Marathon, replace the game's Direct3D functions: they put their own
objects in the game's memory and [hook](glossary.md#hook) the functions that create and lock resources. That does not
fit this game:

- many textures come with their Direct3D description already inside the game data, so they never go through
  `CreateTexture`, the function you would hook;
- `SetTexture` copies the texture's description (its "fetch constant") straight into the Direct3D device;
- the device keeps its own copy of the GPU's settings, which the game updates itself.

So the port lets the game's Direct3D run as it is, and reads what it writes: the list of GPU commands (the
[PM4 ring](glossary.md#pm4-ring)), with its settings, draws, copies and the "frame finished" command (Swap). It never
imitates the [EDRAM](glossary.md#edram).

A few hooks on the game's thread add what the command list does not say:

- **Which shader is which.** When vertex shaders reach the command list, Direct3D has already modified them, so they
  cannot be recognized by their microcode (see [shaders.md](shaders.md)). Hooks on the functions that create shaders
  note which original shader each one is. Each Direct3D draw call also notes which shaders it used, and the ring
  thread pairs those notes with the draws it reads: by type and number of vertices, checked against the last shader
  load in the list (`IM_LOAD`). One full-screen quad that Direct3D draws on its own is recognized from the command list
  alone.
- **Keeping the game in step.** The game waits (`WAIT_REG_MEM`) until the GPU writes a value (`SCRATCH_REG` to
  `SCRATCH_ADDR`). The port writes it, as the real GPU would; otherwise the game would freeze within a second.

### The threads

| Thread | What it does |
|---|---|
| Game threads ([guest](glossary.md#guest)) | Run the game and its Direct3D, which writes the command list |
| [Ring thread](glossary.md#ring-thread) | Reads the commands, keeps track of the GPU settings and records the Vulkan commands |
| Vertex copy thread | Copies vertex data into the upload buffer, turning the Xbox 360's byte order into the Switch's |
| Presentation | Composes the final image and shows it |

The ring thread is the heart of the renderer. For most of the project, a frame took exactly as long as one loop of
that thread.

### One frame

- **Three work slots.** Each frame is recorded into one of three slots. Each slot has its own command buffers, its own
  fence (a signal the GPU gives when it finishes), a 64 MB upload buffer and its own read-back buffers. So the CPU
  records the next frame while the GPU draws the current one, and only waits if the slot it needs is still busy. The
  read-back buffers must be per slot, or two frames would write over each other.
- **Render targets instead of EDRAM.** The game's [render targets](glossary.md#render-target) become normal Vulkan
  images, and resolves become copies into textures. Many of those copies are not needed: when the next thing the game
  does is clear the target anyway, the port swaps the two images instead of copying ("resolve without copy"). If the
  old content is needed later, it is brought back.
- **Some images must go back to the game.** The game measures how bright the scene is (for its automatic exposure) by
  reading a small image on the CPU. So a few resolved images are written back into the game's memory, in the exact
  Xbox 360 format. Only the 64x64 ones are needed for the exposure to look right, which removed 98 % of this cost.
- **Showing the frame** goes through ReXGlue's presenter in IMMEDIATE mode (see
  [platform-notes.md](platform-notes.md#presentation)).

### Each draw

For each draw, the ring thread:

1. reads the drawing settings from its copy of the GPU settings, and works out which [pipeline](glossary.md#pipeline)
   it needs;
2. finds the textures in a two-level cache;
3. copies the vertices (without duplicates) and the indices into the upload buffer, turning their byte order;
4. writes the shader constants that changed into a buffer the shaders read (a dynamic uniform buffer);
5. binds what changed and records the draw.

Lessons that shaped the code (each one cost time to find):

- **Check textures cheaply.** Each texture gets a fingerprint (a hash) of its raw bytes in the game's memory, and is
  only put back in normal order ("untiled", see [tiling](glossary.md#tiling)) when the fingerprint changes. Textures
  that do not change are checked less and less often, down to once every 32 frames. On the PC this took the texture
  work from 66 µs to 4.2 µs per draw in the menu.
- **Structures that are fingerprinted or compared byte by byte must have no gaps.** A pipeline key with four
  uninitialized gap bytes ("padding") created duplicate pipelines from leftover memory: 125 to 203, depending on the
  build. Every such structure has `static_assert(std::has_unique_object_representations_v<T>)`, which makes the
  compiler refuse a structure with gaps.
- **In busy loops, copy what you need into local variables first.** A vertex copy loop that read the source, the
  destination and the count from a structure passed by reference got 1.9 times slower: the compiler could not be sure
  that the writes did not change the structure, so it read it again every time.
- **Do not time everything.** Timers are only read on a sample: one in 128 packets, and one in 8 or 64 draws depending
  on the timer. See [measuring.md](measuring.md#counters-that-lie).
- **Counters that only one thread uses must not be atomic.** The Switch's processor (Cortex-A57, ARMv8.0) has no fast
  atomic instructions, so each atomic `fetch_add` is a slow loop that also disturbs the other cores. About 83,000
  packets per frame were counted that way: 1.1 ms per frame. Before removing an atomic, check every place that writes
  it: one of these counters turned out to be written by another thread too (the vblank thread).

### Pipelines

- A [pipeline](glossary.md#pipeline) is found by its drawing settings, shaders and specialization constants, and
  created through a Vulkan [pipeline cache](glossary.md#pipeline-cache) saved on the SD card. Without it, creating one
  on NVK took about 59 ms (82 ms for the ones created during the first race).
- On top of that, the port saves **the list of pipelines the game uses** and creates them again on a background thread
  when the game starts, so the first race does not stutter: 113 of 113 pipelines ready in 0.3 s, and none created
  slowly during the race. Both things live in one file, `cache/nfsmw_nativo_pipelines.bin`.
- Tried and dropped: Vulkan's dynamic state extensions (`VK_EXT_extended_dynamic_state` 1, 2 and 3), to change
  pipelines less often. On NVK with this GPU each change got slower (from 2.95 to 4.36 µs) for only 13 % fewer
  changes: a loss.

### Textures and memory

- The texture cache has a limit: 512 MB in the released version (setting `nfsmw_nativo_texturas_mb_max`). It was
  384 MB until the resolution became automatic, because larger resolutions need more room. Above the limit, textures
  not used for at least 120 frames are released until the cache is back under 75 %. Without a limit, the cache grew by
  about 20 MB every 40 s of racing, because the game streams new textures into new addresses as the car drives through
  the city.
- Textures take their memory from big blocks (16 MB each in the released settings) instead of one allocation each,
  because every allocation is slow on the Switch (see [platform-notes.md](platform-notes.md#nvk-on-horizon)).
- Untiling works on groups of 16 bytes with NEON (the processor's instructions for several values at once). The first
  version, block by block, cost around 30 ms each time a burst of new textures arrived.

### Game functions in native code

Where the game's own thread was the slowest part, some of its busiest functions were rewritten in C++ and installed as
[hooks](glossary.md#hook): the material setup, the effect setup, the matrices of each draw, the visibility check, the
drawing of the scenery, the dump of Direct3D settings into the command list, and the code that issues each draw.

Every one is protected by the same [guard](glossary.md#guard):

- for the first 50,000 to 200,000 calls, and then one in 4,096, the port runs both the native and the original
  function and compares everything they produce;
- the original's result is the one used;
- any difference turns the native version off until the game is closed, and writes `DIFERENCIA` in the log.

Getting exactly the same results needs care:

- The game was compiled with FMA (a multiply and an add in one instruction, which rounds slightly differently). The
  native code must write each expression in the same shape and order, so that GCC fuses it the same way. Test code
  placed between a multiply and an add changes that, and makes the test report false differences.
- Only the registers that something reads later need to match. An analysis of the translated code finds which ones.
- Inputs with NaNs (invalid numbers) are left to the original function.

### Guards that check themselves

Some changes depend on recognizing a draw, a shader or a render pass by its signature. They switch on in three stages,
decided while the game runs:

1. watch, without changing anything;
2. apply, but only after the signature has been seen cleanly for a number of frames;
3. switch off until the game is closed if the known sign of failure ever appears.

So the worst case is "it does nothing", never a broken image. This was adopted after two builds broke the image by
moving the sky pass without checking first.

## What made it fast

From the first native build to the release, the work went back and forth between two limits: the GPU (shading pixels,
at 307.2 MHz in handheld mode for most of the project) and the ring thread (CPU time per draw).
[performance-history.md](performance-history.md) tells it step by step, with the measurements. The main changes:

**GPU**

- shader constants through one buffer (a dynamic uniform buffer) instead of reading them from memory one by one;
- resolves without copies, and one shadow map copy replaced by taking the minimum of two shadow maps;
- [ZCULL](glossary.md#zcull) turned on in the driver;
- cleaner translated shaders: merged conditional blocks, no `max(a, a)` moves, no texture size queries;
- resetting only the GPU query pools the frame will use (2,144 resets per frame were costing 1.2 ms of GPU idle time);
- less geometry in the shadow map: level of detail and distance limits;
- a slope bias for the shadows. It removed "shadow acne", a pattern of dots that a cheaper shadow filter had made
  visible.

**CPU**

- no busy waits in the game's Direct3D;
- the ring thread at a priority that does not starve the presentation;
- vertex uploads without duplicates, a helper that copies when the copy thread falls behind, and texture fingerprints
  made on another thread;
- caches for shader loads, texture descriptions and pipelines;
- native versions of the busiest game functions;
- log lines written by another thread (the periodic report used to be formatted by the ring thread);
- a read cache for the game's streaming, which read the same zone packs from the SD card again on every lap.

**Build**

- registers as C++ variables, direct calls, [LTO](glossary.md#lto), [PGO](glossary.md#pgo) and
  [function ordering](glossary.md#function-ordering) (see [toolchain.md](toolchain.md)).
