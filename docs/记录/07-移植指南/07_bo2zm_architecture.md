# How bo2-zm-nx works

<!-- AUTHOR-BANNER -->
> # 🚫⛔ 致 Telegram「DarkNSP」搬运者的一封公开信 ⛔🚫
>
> 我知道你在看。你可能已经点开这个页面，扫了一眼，觉得「又是那个做半成品的人」，然后顺手把它和几个 G 的游戏文件塞进一个压缩包，配上三步「安装教程」——**第一步，把根目录复制到卡上；第二步，安装 NSP 前端；第三步，玩**——就发进群里了。
> **一句 bug 都没提。** 没提贴图会烂，没提帧数会掉到个位数，没提它根本还没做完。
>
> 我想先问你一句：**发布之前，你自己跑过一遍吗？**
>
> 哪怕只跑十分钟——进到第二个关卡，低头看一眼脚下的地面。你会看见贴图在你眼前一点一点地烂掉：**墙上长出人脸，桌子变成电视，弹匣变成砖头**，枪口焰炸成一整块巨大的方块。你会看见画面一黑一亮，像一盏坏了很久的日光灯。至于帧数——**那个「三十五到六十」是我在特别行动「坑道（The Pit）」那种小场景里测出来的；一到真正有人物、有战斗的关卡，帧数直接掉到个位数**，卡到你连准星都推不动。
>
> 你看见了，你还会发吗？
>
> 我想你不会。因为**你连一次都没跑过**。你只是看见一个 GitHub 链接，看见「COD6 移植 Switch」这七个字，就把它当成了流量。你没有下载过这个项目，没有在自己机器上试过哪怕一关，没有读过 README 里那句被我加粗了三遍的「这是半成品」。你甚至不知道它跑到第几帧会崩。
>
> 可我知道。**三百一十四轮实机测试，全在这儿。** 我知道它在选枪台会烂，知道光照只有角色两米以内是亮的，知道枪焰会变成一块砖头。我比任何人都清楚它现在有多糟——因为我就是那个盯着它烂了三百多个小时的人。
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
> Ten minutes would have been enough — load the second mission, look down at the ground under your feet. You would watch the textures rot in front of you: **faces growing out of the walls, tables turning into televisions, magazines turning into bricks**, the muzzle flash exploding into one giant solid block. You would watch the screen flip between black and bright like a dying fluorescent tube. And the frame rate? **That “thirty-five to sixty” was measured in a tiny Special Ops map, “The Pit”. In any real level with characters and combat it drops to single digits** — so slow that you cannot even push the crosshair around.
>
> If you had seen that, would you still have posted it?
>
> I do not think so. Because **you never ran it once**. You saw a GitHub link, you saw “COD6 ported to Switch”, and you treated it as traffic. You never downloaded this project, never played a single mission on your own console, never read the line in the README that I had to bold three times: *this is a work in progress*. You do not even know at which frame it crashes.
>
> I do. **Three hundred and fourteen recorded test runs, all of them documented here.** I know it falls apart in the loadout screen, I know the lighting only reaches two metres around the player, I know the muzzle flash turns into a brick. Nobody on this planet knows how bad it currently is better than I do — because I am the one who has been staring at it rot for over three hundred hours.
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


The Xbox 360's PowerPC code is translated ahead of time into C++, compiled for the Switch's ARM cores and linked with a
runtime that provides what the 360's kernel and hardware did. The GPU is not emulated: a renderer of its own reads
the game's GPU commands and draws them with Vulkan.

```
 your disc + TU18            build time (PC)                          run time (Switch)
 default_mp.xex  ──▶  loader: decrypt, decompress, patch       bo2zm.nro
 default_mp.xexp      code generator: 30,096 functions   ──▶    ├ recompiled game code
                      fix-up passes → C++ → GCC 16 (LTO, PGO)   ├ runtime: kernel, memory, threads, files, audio
 fastfiles (.ff) ──▶  shaders → XenosRecomp → dxc        ──▶    ├ native renderer → Vulkan (NVK)
                      → bo2zm_shaders.spvpack                   └ Switch layer: threads, cores, display
```

## 1. The executable

Zombies lives in `default_mp.xex`, targeted at Title Update 18, the game's final update. The SDK's loader (from Xenia)
decrypts the XEX2 image, decompresses it and applies the update's delta patch (`default_mp.xexp`). The result is a
47 MB PE image at 0x82000000 that imports 162 functions from `xam` and 215 from `xboxkrnl`. `tools/xexdump` writes it
to disk for the analysis tools.

## 2. The code generator

ReXGlue's generator (`sdk/src/codegen`), driven by `app/bo2zm_manifest.toml`:

1. **Analysis** finds every function from the entry point, exports, call targets, jump tables and exception tables,
   until nothing new appears: 28,358 functions, 30,096 emitted, no errors.
2. **Emission** writes one C++ function per PowerPC function, about 5 million lines in 140 files, each instruction
   kept as a comment. Guest memory is one base pointer with byte swaps; VMX128 maps to SIMDe (NEON on ARM, SSE on x86).

**Configuration.** `app/overrides.toml` holds what the analysis cannot find (boundaries, tail-branch-only thunks,
`setjmp` addresses, register options), each entry with its evidence. `app/gaps.toml` lists code no function covers.
`app/locals.txt` names hot leaf functions to copy with local registers.

**Registers.** Most PowerPC registers are C++ locals so the compiler can keep them in ARM registers: the non-volatile
general, floating-point and vector registers, condition registers, CTR, XER and the scratch vector registers. Only
argument and volatile registers stay in the context structure.

**Fix-up passes**, run by `tools/codegen.sh`:

| Pass | Why |
|---|---|
| `read_before_write.py --strict` | Finds functions that read a register before writing it; they must share registers |
| `setjmp_locals.py --fix` | `longjmp` error recovery must restore the register locals |
| `direct_calls.py` | Direct calls to unhooked functions, so the compiler can inline them |
| `localize.py` | Local-register copies of the functions in `app/locals.txt` |
| `guest_barriers.py` | Compiler barriers that make plain guest memory accesses safe |

## 3. Hooks

Each generated function is `__imp__sub_<address>`, called through a weak alias `sub_<address>`. A hook is a C++
function of that name, which the linker prefers; it can still call the original. Hooks replace hot functions with
native rewrites (each checked against the original first), observe the game (its Direct3D layer, the lighting
volume's writer, the job system) and add features (menus, cheats, presets, the benchmark).

## 4. The build

GCC 16 (devkitA64) compiles the generated code, the SDK and the port at `-O3` with LTO and PGO into one NRO of about
66 MB. Switch code is behind `__SWITCH__`; the same sources build for Windows and Linux with clang.

## 5. The runtime

The ReXGlue SDK (`sdk/`) carries Xenia's implementation of the 360's kernel and libraries, plus a Horizon layer.

- **Kernel**: the 377 imports (threads, events, semaphores, mutants, memory, files, content, sockets, profiles, system
  UI). On Horizon, sync objects wait on the address arbiter, so a signal never takes a lock.
- **Memory**: Horizon has no sparse memory and cannot change permissions on mirrored memory, so the window is reserved
  empty and memory is committed in chunks, turned into code-memory aliases and mapped into every view on first access.
  The exception handler emulates accesses to unmapped pages without system calls. The design is in
  `sdk/src/core/guest_memory_switch.h`.
- **Threads**: each guest thread is a Horizon thread. The game's job system is mapped in `app/src/bo2zm_waits.cpp`,
  where its polling loops sleep instead of spinning.
- **Audio**: the game mixes on its own threads; the runtime provides XAudio and the XMA decoder (FFmpeg, with a frame
  cache), out through `audout`.
- **Files**: the game, update and `user` folders; image pack reads are paced, because at memory speed the game's
  streaming re-reads the same images endlessly.
- **Input**: HID controllers presented as Xbox 360 controllers.
- **Sockets**: the solo match's local server, with the network errors Horizon reports after sleep treated as "no data
  yet".

## 6. The native renderer

The game's Direct3D layer, itself recompiled game code, writes PM4 packets for the Xenos GPU. The renderer's thread
reads them, keeps the register state, and for every draw identifies the shaders by hash in the shader library, finds
the Vulkan pipeline, resolves the textures (untiled and byte-swapped into the texture pool), uploads vertices, indices
and the constants the shaders read, and records the draw. Resolves, fences, memory waits, occlusion queries and
memexport are handled the same way.

- **Threads**: a recorder thread makes the Vulkan calls the ring thread queues; others copy vertex data and bind
  texture memory.
- **Write stamps**: each 4 KB page has an epoch advanced by the code that writes memory (copy and fill routines,
  skinning hooks, memexport). Cached vertex data, indices and textures stay valid while their pages are unchanged; a
  sample of hits is checked.
- **Shader library**: the game's 16,149 shaders are translated at build time (XenosRecomp, then `dxc`) into
  `bo2zm_shaders.spvpack`. An index built on the first start lets each shader be read on first use. Pipelines are
  compiled on a thread and cached next to the game, and the saved list is precompiled when the build or driver changes.
- **Texture pool**: large blocks with one GPU mapping per block and memory kind, sized for the 64 KB unit each texture
  takes, evicted in small batches.
- **GPU skinning**: the game's CPU skinning jobs become compute dispatches.
- **Direct3D layer**: register flushes become markers decoded in blocks, and four Direct3D functions run natively. A
  test mode, the D3D shadow, compares each draw's state with the game's Direct3D side.
- **Driver**: NVK, built for Horizon by mesa-switch, with this project's patch (state fingerprints, copy-engine copies,
  kind views, a leak fix).
- **Output**: the presenter scales the 1280x720 image to the screen and draws the FPS counter and credits on top;
  VSYNC follows the display's refresh signal.

On the PC the SDK's exact Xenos emulation (Direct3D 12) is the reference image every renderer change is checked
against.

## 7. The Switch layer

In the SDK's `*_switch.*` files and `app/src/bo2zm_switch.cpp`:

- start-up: paths, performance mode, network, the exception handler;
- cores: core 3 placement and its failsafe (`sdk/src/ui/switch_core3.cpp`);
- clocks: never set, only read for the logs;
- display: the window, VSYNC, docked and handheld;
- relaunch: the game's mode switches restart the NRO with the launch data carried across;
- logs: written by their own thread; warnings only for players, the full log for tests.

## 8. The game's own systems

- **Menus** are HavokScript (Lua 5.1) in the fastfiles; the game's own Lua compiler builds `app/src/ui/menus.lua` at
  start to add the port's pages. `tools/inspect/hks_dis.py` reads the game's menus.
- **Settings**: presets set the game's own dvars, and hooks hold the ones it forces back every frame.
- **Cheats and Extras** drive the game's script engine (notifies, function calls, field reads) on the Server thread,
  right after its frame, where the game runs its own scripts.
- **DLC** packs are mounted as the 360 did.
