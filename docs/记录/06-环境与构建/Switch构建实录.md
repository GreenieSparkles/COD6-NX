# COD6 → Switch 构建实录

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


> **现状（2026-10-28）**：可运行的 `.nro` 已产出 —— **39.28 MB**，含真实 Mesa/NVK Vulkan 驱动、
> Xenos GPU 插件、完整运行时着色器翻译链路（glslang 1465 个符号）。链接零未定义符号。
> **只差上机验证。** 所有不需要游戏机的部分都已完成。

## 一页纸现状

| 项目 | 状态 | 证据 |
|---|---|---|
| 交叉编译链 | ✅ | 612 个目标，aarch64 + devkitA64 GCC 15.2 |
| ReXGlue 运行时（Horizon 层） | ✅ | `threading_switch` / `guest_memory_switch` / `fiber_switch` / `seh_switch` / `dynlib_switch` 全部链入 |
| NVK Vulkan 驱动 | ✅ | `libvulkan.a` 88.2 MB，1533 个成员，从 Mesa 26.2.4 交叉编译 |
| Xenos GPU 插件 | ✅ | `librexgpu-xenos.a` 6.0 MB，`rex_gpu_create` 由它提供 |
| 运行时着色器翻译 | ✅ | `TShader` / `TProgram` / `GlslangToSpv` / `SpirvShaderTranslator` 在符号表里 |
| 输入（蓝牙手柄） | ✅ | `SwitchInputDriver` + `padInitialize` / `hidInitialize`，59 个 pad 符号 |
| 音频 | ✅ | `audoutInitialize` / `audoutAppendAudioOutBuffer` + XMA 解码 |
| NRO 合法性 | ✅ | `HOMEBREW` + `NRO0`，`.text` 26.9 MB / `.rodata` 13.2 MB |
| SD 目录结构 | ✅ | `sd_root\switch\cod6-nx\`（`部署说明.md`） |
| 可复现性 | ✅ | 清洁重建 0 错误 / 4 分钟；WSL 与 Windows 工程 15/15 关键文件校验和一致，62 个生成 cpp 校验和一致 |
| **上机验证** | ⏳ | **需要真 Switch** |

**NRO 位置**：`switch_build\cod6.nro`

**重建**：
```bash
bash NSMW2/build_nvk_switch.sh    # 编 NVK（约 15 分钟，只需一次）
bash NSMW2/build_cod6_switch.sh   # 编 NRO（约 10 分钟）
```

---

# COD6 → Switch：`.nro` 构建实录

> 时间：2026-10-28
> 产物：`switch_build\cod6.nro`（**20.47 MB**，NRO 头校验通过）
> 状态：**交叉编译完全打通**，可启动的 NRO 已产出；画面需要 NVK + 原生渲染器（见末节）

---

## 一、最终结果

```
cod6.nro            20.47 MB   Switch 可执行文件
cod6.nacp           16 KB      元数据（标题/作者/版本）
cod6                28.66 MB   未 strip 的 ELF
cod6.stripped.elf   21.51 MB   strip 后
```

NRO 头校验（Windows 侧独立复核）：

```
0x08: "HOMEBREW"   标准 homebrew 标记
0x10: "NRO0"       NRO 魔数
size = 21,442,560  与实际文件大小一致
.text = 18,907,136 字节
ELF: AArch64, PIE, 26030 个全局函数符号
```

对比参考成品：`codmw.nro` 39 MB、`t6zm.nro` 40 MB（都带 NVK 驱动，约 +20 MB）。
我们的 20.5 MB 因为用的是**占位 NVK**。

---

## 二、关键发现：真正的构建环境在 WSL 的 rootfs 里

Windows 上的 `D:\devkitPro` 是**残缺**的：

| | Windows `D:\devkitPro` | WSL rootfs `/opt/devkitpro` |
|---|---|---|
| GCC | 13.3.1（xPack） | **15.2.0（devkitA64 官方）** |
| C++ 线程支持 | ✗ `_GLIBCXX_HAS_GTHREADS` 未定义 | **✓ 定义了** |
| `libpthread.a` | ✗ | ✓ |
| portlibs | ✗ 空 | **✓ 173 个库**（含 zlib/expat/zstd） |
| nacptool / elf2nro | ✗ 需自己编 | **✓ 官方 Linux 二进制** |
| libnx | ✓ | ✓ |

**rootfs 位置**：`D:\devkitPro\wsl\inst\rootfs\opt\devkitpro`
**挂载方式**：`ln -s /mnt/d/devkitPro/wsl/inst/rootfs/opt/devkitpro /opt/devkitpro`

> 这就是为什么之前在 Windows 上编到 `std::mutex` 就死 —— Windows 那套工具链的 libstdc++ 没编线程支持。**换到 rootfs 的 GCC 15.2 立刻解决。**

---

## 三、完整依赖链

```
PC (clang) — rexglue codegen —► 62 个生成的 C++（跨平台共享，不能再生成）
                                        │
WSL (GCC 15.2 aarch64) ─────────────────┘
   + Horizon 层（nfsmw-nx 的 sdk/，1017 文件）
   + FFmpeg / glslang / spirv-tools / imgui / fmt / spdlog …
   + 占位 libvulkan.a（真 NVK 待补）
   → cod6.elf → strip → elf2nro → cod6.nro
```

| 组件 | 来源 |
|---|---|
| 交叉编译器 | `/opt/devkitpro/devkitA64`（GCC 15.2.0） |
| libnx | `/opt/devkitpro/libnx` |
| Horizon 层 | `D:\devkitPro\_dl\bo2zm\...` 同款 `sdk/`（nfsmw-nx 的 fork，从 GitHub 下的 tarball） |
| 第三方依赖 | 14 个子模块（按上游固定 SHA 下载）+ 10 个 SDK 本地库 |
| 工具链文件 | nfsmw-nx 的 `tools/switch/cmake/switch-devkitA64.cmake` |
| 打包 | `/opt/devkitpro/tools/bin/{nacptool,elf2nro}` |

---

## 四、踩过的 12 个坑（每个都记了原因）

| # | 现象 | 原因 | 修复 |
|---|---|---|---|
| 1 | MSVC 编不过 | SDK 用 `__builtin_bswap32`/`__VA_OPT__`（clang/GCC 扩展） | 换 clang/GCC（官方 Wiki 也要求 Clang 20+） |
| 2 | `-mtp=soft` 不认识 | devkitA64 GCC 不支持 | 从工具链文件删掉 |
| 3 | `Git submodule 'cli11' is not initialized` | tarball 不含子模块 | 按上游固定 SHA 下 14 个子模块 + 造 `.git` 标记 |
| 4 | `thirdparty/disasm` 缺失 | SDK 自带本地库，tarball 也缺 | 从上游 SDK tarball 拷 10 个本地库 |
| 5 | `disasm`、`oobheap` 等 0 字节 | PowerShell 从 `\\wsl$` UNC 拷贝会截断 | 一律用 WSL 内 `cp` 经 `/mnt/d` |
| 6 | `lzxd.c: system.h: Invalid argument` | tarball 里是符号链接，NTFS 上变空 reparse point | 用真实文件覆盖 13 个链接 |
| 7 | `#error "no config"`（FFmpeg） | FFmpeg 只认 android/linux/macos/windows | 加 `__SWITCH__` 分支 + `config_switch_aarch64.h` |
| 8 | `bsf_list.c: unknown type 'FFBitStreamFilter'` | 生成文件要匹配宿主文件的数组名/类型 | `parser_list[]` / `bitstream_filters[]`，不带 include |
| 9 | `PTHREAD_ONCE_INIT undeclared` | `pthread.h` 整段被 `#if defined(_POSIX_THREADS)` 关掉 | 编译加 `-D_POSIX_THREADS -D_UNIX98_THREAD_MUTEX_ATTRIBUTES` |
| 10 | `std::mutex` 不存在 | Windows 的 GCC 13.3.1 没编 libstdc++ 线程支持 | **换 rootfs 的 GCC 15.2** |
| 11 | `sys/mman.h` / `memalign` / `posix_memalign` / `sysconf` 缺失 | libnx 的 libc 没有 | 关掉 FFmpeg 的 `HAVE_MMAP`/`HAVE_MEMALIGN`/`HAVE_POSIX_MEMALIGN`，`sysconf` 写桩 |
| 12 | 链接报 19000+ `undefined reference to sub_XXX` | **GCC 会静默丢弃 `extern "C"` 前的 attribute**，weak alias 不生成 | 给 `cod6_pch.h` 补 SDK 模板里那条 GCC 分支（attribute 放到声明符之后） |

**第 12 条是 SDK 自己在 `pch_h.inja` 里记录过的坑**，原文：

> GCC rejects attributes ahead of extern "C" ... the weak alias is never emitted and the link fails much later with thousands of undefined sub_ symbols.

---

## 五、怎么重建

```bash
# 环境
export DEVKITPRO=/opt/devkitpro
export PATH=/opt/devkitpro/devkitA64/bin:/opt/devkitpro/tools/bin:$PATH

# 工程
cd /opt/switch/cod6-ng

# 配置
cmake -S . -B out/nx -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$PWD/tools/switch/cmake/switch-devkitA64.cmake" \
  -DDEVKITPRO=/opt/devkitpro \
  -DREXGLUE_SWITCH_NVK_SDK="$PWD/nvk-stub" \
  -DREXSDK_DIR="$PWD/sdk" \
  -DCMAKE_BUILD_TYPE=Release

# 编译（约 8~12 分钟，612 个目标）
cmake --build out/nx -j $(nproc)

# 产物：out/nx/cod6.nro
```

**首次配置时的前提**（都已在这个工程里做好）：
- `sdk/thirdparty/` 下 14 个子模块 + 10 个本地库齐备（每个都带 `.git` 标记文件）
- `sdk/thirdparty/FFmpeg/config.h` 有 `__SWITCH__` 分支，`config_switch_aarch64.h` 关掉了 mmap/memalign 家族
- `sdk/thirdparty/ffmpeg-overlay/{parser_list,bsf_list}.c` 是空列表
- `sdk/switch_compat/` 有 `semaphore.h`、`sys/resource.h`、`sys/mman.h`、`endian.h`、`netinet/ip.h`
- `generated/default/cod6_pch.h` 有 GCC 的 `DEFINE_REX_FUNC` 分支
- `generated/rexglue.cmake` 在 Switch 下跳过 codegen 与 `add_dependencies(cod6_codegen)`
- `sdk/cmake/rexglue_switch.cmake` 的 NRO 打包用真 `elf2nro`
- `src/switch_stubs.cpp` 提供 `rex_gpu_abi_version`/`rex_gpu_create`/`sysconf`
- `nvk-stub/lib/libvulkan.a` 是占位归档

---

## ★★ 里程碑 B（2026-10-28）：真 NVK 编出来了，NRO 现在带着 Vulkan 驱动

**新产物**：`switch_build\cod6.nro` —— **39.28 MB**（原 20.47 MB），里面是真的 Mesa/NVK。

```
NRO 头: HOMEBREW ✓  NRO0 ✓
.text 25.6 MB   .rodata 12.6 MB   .data 1.1 MB   .bss 5.1 MB
内含: Mesa / NVK / nvk_ / nouveau / nak_ 符号
rex_gpu_create / rex_gpu_abi_version / vkCreateInstance 均来自真插件
```

对比参考成品：`codmw.nro` 39.0 MB、`t6zm.nro` 40.0 MB —— **同一量级，说明这次是真的把驱动链进去了**。

### NVK 从哪来：`libvulkan.a` = 88.2 MB，1533 个成员

**不能取，只能编。** 之前查清：NRO 里是已链接的机器码，无法还原成库；任何仓库都没有预编译。
所以从 `danfromtico/mesa-switch`（Mesa 26.2.4）交叉编译出来。

### 构建链路（Windows 的 MSYS2 路线完全不需要）

WSL 里那套 Linux devkitPro 本来就能跑，反而比 MSYS2 简单：

```bash
# 1) 依赖
apt install meson bison flex gcc g++ clang llvm-dev libclang-dev \
            libelf-dev libexpat1-dev zlib1g-dev libzstd-dev \
            libllvmspirvlib-18-dev spirv-tools python3-mako python3-yaml
pip install "meson>=1.4"          # Ubuntu 的 1.3.2 不够，Mesa 26.2 要 1.4
cargo install bindgen-cli cbindgen

# 2) 本机工具（mesa_clc + vtn_bindgen2，要链 LLVM）
cd mesa-switch-main
meson setup builddir-native --wipe \
    -Dvulkan-drivers= -Dgallium-drivers= -Dplatforms= -Dopengl=false \
    -Dllvm=enabled -Dmesa-clc=enabled -Dprecomp-compiler=enabled
ninja -C builddir-native src/compiler/clc/mesa_clc src/compiler/spirv/vtn_bindgen2

# 3) 交叉构建 Switch 版
export DEVKITPRO=/opt/devkitpro
export MESA_SWITCH_RUST_TARGET=aarch64-unknown-linux-gnu
meson setup builddir-switch --wipe \
    --cross-file switch_cross_file.txt \
    --native-file /opt/switch/mesa-build/native_tools.txt \
    --buildtype=release -Doptimization=1 -Db_lto=false \
    -Dvulkan-drivers=nouveau -Dgallium-drivers=nouveau \
    -Dplatforms=switch -Dopengl=false -Dgles1=disabled -Dgles2=disabled \
    -Dllvm=disabled -Dmesa-clc=system -Dprecomp-compiler=system -Dcpp_rtti=false
ninja -C builddir-switch
# → builddir-switch/src/nouveau/vulkan/libvulkan.a  (88.2 MB)
```

### 又踩了 5 个坑

| # | 现象 | 原因 | 修复 |
|---|---|---|---|
| 13 | `Dependency "LLVMSPIRVLib" not found` | 本机工具要 SPIRV-LLVM-Translator | `apt install libllvmspirvlib-18-dev spirv-tools` |
| 14 | `Subproject syn-2-rs is buildable: NO` | 我加了 `--wrap-mode=nodownload` 把 41 个 wrap 挡住了 | 去掉该参数；`static.crates.io` 和 GitHub releases 都能下 |
| 15 | `Program 'cbindgen' not found` | nouveau 的 NIL 层要 cbindgen（和 bindgen 是两个工具） | `cargo install cbindgen` |
| 16 | `custom targets are unstable` / `can't find crate core` | 自定义 target JSON 需要 nightly 且没有预编译 core | **改用内置 `aarch64-unknown-linux-gnu`** —— tier 1，自带 std，输出 aarch64 ELF |
| 17 | `--fix-cortex-a53-843419 is only supported on AArch64` | proc-macro crate 要按**本机**编译，但 meson 用了交叉 rustc | 加 `--native-file`，把 `rust`/`bindgen`/`cbindgen` 指向本机版本 |

**第 16 条的关键**：`aarch64-unknown-none` 虽然名字更像 Switch（`os: none`），但 **`acorn` 这类 crate 需要 std**，
而 `none` 目标没有。`aarch64-unknown-linux-gnu` 有 std、输出仍是 aarch64 ELF，最终由 devkitA64 链接，完全可用。
（BO2 的构建脚本用的也是这个 target。）

### 最后一环：让 GPU 插件真的链进去

`librexgpu-xenos.a` 编出来了却没进链接命令行 —— 因为 `CMakeLists.txt` 的 Switch 分支还在用 milestone A 的
`rexglue_setup_target(cod6)`（不传 `GPU_PLUGINS`）。改成：

```cmake
rexglue_setup_target(cod6 GPU_PLUGINS xenos)
```

`rexglue_switch_configure_target` 会把它放进 `--whole-archive`，`rex_gpu_create` 从此由插件提供，
`src/switch_stubs.cpp` 里的 GPU 桩必须删掉（否则重复定义）。

---

## ★★★ 里程碑 C（2026-10-28）：SD 布局 + 渲染链路验证，NRO 功能完整

### SD 目录结构（`sd_root\switch\cod6-nx\`）

```
cod6.nro        39.3 MB    程序（含 NVK）
cod6.nacp       16 KB      元数据
game_root/      6.9 GB     游戏数据（149 个文件，本机用目录联接零拷贝）
user/                     存档
cache/                    着色器缓存
logs/                     日志
```

程序按 **NRO 所在目录**找同级的 `game_root` —— `cod6_app.h` 里重写了 `OnConfigurePaths`：

```cpp
void OnConfigurePaths(rex::PathConfig& paths) override {
#if defined(__SWITCH__)
  const auto exe_dir = rex::filesystem::GetExecutableFolder();
  if (paths.game_data_root.empty()) paths.game_data_root = exe_dir / "game_root";
  if (paths.user_data_root.empty()) paths.user_data_root = exe_dir / "user";
  if (paths.cache_root.empty())    paths.cache_root    = exe_dir / "cache";
  if (paths.config_path.empty())   paths.config_path   = exe_dir / "cod6.toml";
#endif
}
```

NRO 里已确认有 `game_root` / `cod6.toml` / `sdmc:` 字符串。

### 关键结论：不需要预建着色器库

原以为要像 BO2 那样从 `.ff` 提出上万个 shader。**源码证明不需要**：

```
command_processor.cpp:508  glslang::InitializeProcess()
command_processor.cpp:512  glslang::TShader shader(stage)
command_processor.cpp:34   #include <rex/graphics/pipeline/shader/spirv_translator.h>
```

运行时链路：**Xenos 微码 → SpirvShaderTranslator → glslang → NVK**。
NRO 符号表里 1465 个 glslang 符号 + 310 个 spirv 符号（`TShader`/`TProgram`/
`GlslangToSpv`/`SpirvShaderTranslator`/`SpirvBuilder` 全在）。

### 最终链接状态

```
未定义符号: 0
rex_gpu_create              ✓ T（真插件）
vkCreateInstance            ✓ T
vk_icdGetPhysicalDeviceProcAddr            ✓ T（NVK loaderless 入口）
vk_icdNegotiateLoaderICDInterfaceVersion   ✓ T
```

### 为什么有信心它能跑

Switch 用的 Vulkan 渲染器（`sdk/src/graphics/vulkan/` 那 9 个文件）
**正是之前在 PC 上验证过、菜单能跑 35~58 fps 的那套代码**。
新增的只有三样，而每样都有已发布作品作证：

| 新增部分 | 验证来源 |
|---|---|
| NVK（Mesa 26.2.4 for Horizon） | nfsmw-nx / bo2-zm-nx / codmw 都在用 |
| GPU 插件静态链接 | 同一 SDK 的 Switch 分支 |
| Horizon 层（libnx 启动/内存/线程/音频/输入） | 同上 |

---

### 可复现性验证（2026-10-28）

删掉 `out/nx` 从零重编，证明整套流程可重复：

```
配置:  05:03:05 → 05:03:15   (10 秒)
编译:  05:03:15 → 05:07:11   (3 分 56 秒, 612 个目标)
错误:  0
产物:  cod6.nro  39.28 MB
md5:   f63bce46f2a0fed75ea55529a161c5ad
```

WSL 工程与 Windows 工程（`D:\360switch\cod6`）的一致性：

```
关键文件校验和:  15 项全部相同（CMakeLists / cod6_app.h / cod6_pch.h /
                rexglue.cmake / rexglue_switch.cmake / switch_compat 全部垫片 /
                FFmpeg config / ffmpeg-overlay / 工具链文件 / overrides.toml）
62 个生成 cpp:   总校验和相同 (a1fc99f91ee4004f02d6a2e000dd0e57)
产物 md5:        WSL / switch_build / sd_root 三处一致
```

**两个工程现在完全等价**，任选一个都能构建。

## 六、这个 NRO 能做什么、不能做什么

**能**：
- 通过 NRO 头校验，能被 hbmenu 识别
- 走真实的 Horizon 层：libnx 启动、客户机 4GB 地址空间映射、线程与栈池、音频（audout）、HID 输入、资产包读取
- 在真机上能**启动**（标题权限需要 title override：按住 R 启动一个游戏，或用 39 位地址空间的 forwarder）

**不能**：
- **没有画面**。`rex_gpu_create` 返回 nullptr（占位 NVK），`LoadGpuPlugin` 报 "no graphics system"
- 所以这是**验证地基**的产物，不是可玩版本

---

## 七、下一步：要画面需要两件事

### 1. 真实 NVK（`libvulkan.a`，约 +20 MB）

前面查清了：**不能从任何现成产物里提取**（NRO 里是已链接的机器码）。必须自己编。

**需要的环境**（BO2 移植的 `tools/build_mesa.sh` 头部原文）：
```
MSYS2 at C:\msys64 with clang64 packages:
  clang, llvm, lld, spirv-llvm-translator, libclc, spirv-tools/headers,
  meson, ninja, python + mako/yaml/packaging, pkgconf, cmake,
  cbindgen, rust-bindgen, bison, flex, git
junction C:\msys64\opt\devkitpro -> C:\devkitPro
rustup toolchain stable-x86_64-pc-windows-gnu with target aarch64-unknown-linux-gnu
```

源码：`danfromtico/mesa-switch` 的 `1a8c1a66d6f`（+ 可选 `patches/mesa-bo2zm.patch`）
输出：`C:\dev\mesa-switch-bo2zm\mesa-unified-install\opt\devkitpro\portlibs\switch\lib\libvulkan.a`

**注意**：rootfs 的 portlibs 里已有 `libz.a` / `libexpat.a` / `libzstd.a`（NVK 的三个依赖），
所以只要 NVK 本身编出来，链接就齐了。

### 2. 原生渲染器（指南第 4 阶段，最大的工作量）

参考材料（都在本机）：
- `D:\devkitPro\_dl\bo2zm\bo2-zm-nx-main\docs\architecture.md`（8.2 KB）
- `...\docs\optimizations.md`（17.9 KB）
- `...\tools\xenosrecomp\` —— Xenos 微码 → HLSL → SPIR-V 翻译器
- `...\app\src\native\render\` —— BO2 的原生渲染器（游戏专用，不能直接复用，但结构可参考）
- `...\tools\spot_flicker.py` —— 闪烁诊断工具
- 我们自己 docs 里的 `08_nfsmw_原生渲染器设计.md`、`10_nfsmw_帧捕获.cpp`、`11_nfsmw_呈现.cpp`

### 3. 顺带：PC 版的那个 D3D12 呈现死锁

PC 原生 Windows 版（RTX 4060）能识别显卡/声卡/手柄，但 `VdSwap` 在 10~24 帧后阻塞。
BO2 工程里有 `tools/spot_flicker.py` 和 `docs/architecture.md` 可能提到这个，值得对照。

---

## 八、文件位置速查

| 内容 | 路径 |
|---|---|
| **最终 NRO** | `D:\主机掌机\NSLITE\NSMW2\switch_build\cod6.nro` |
| NACP 元数据 | `D:\主机掌机\NSLITE\NSMW2\switch_build\cod6.nacp` |
| WSL 工程（可构建） | `/opt/switch/cod6-ng`（= `D:\360switch\cod6` 的修好版） |
| Windows 工程（参考） | `D:\360switch\cod6` |
| rootfs devkitPro | `D:\devkitPro\wsl\inst\rootfs\opt\devkitpro` |
| BO2 Switch 参考工程 | `D:\devkitPro\_dl\bo2zm\bo2-zm-nx-main` |
| nfsmw-nx（SDK fork 来源） | `/opt/switch/dl/nfsmw-nx-main` |
| mesa-switch 源码 | `/opt/switch/dl/mesa-switch-main` |
| 子模块 tarball | `/opt/switch/submods/*.tar.gz` |
| 构建脚本 | `D:\devkitPro\tools\sw_*.sh` |

---

# 第 154~167 轮：构建与部署的变化（2026-10-31 晚）

## 部署方式

| 方式 | 结论 |
|---|---|
| 线缆（拷卡） | **可靠，nro 一律用这个** |
| `nxlink`（netloader） | 会传坏：输出停在 `34.42%` 那行就返回 0，**没有完整性校验**；卡上 md5 与编译结果对不上，表现为启动黑屏、日志不新增 |
| 读日志 | 真机 HTTP 服务器 `http://192.168.31.160:1234/switch/cod6-nx/logs/`（**只读，PUT 返回 501**），用 `tools/pull_logs2.sh` 拉取 |

## 日志与自检（这几处是这轮加的，保留）

1. **构建指纹**（`sdk/src/core/exception_handler_switch.cpp`）：

       [cod6] VERSION 2026-10-31-72  (compilado Oct 31 2026 20:14:23)
       resumen.txt 里也有 build=…

   同一版本号被重编多次时，靠编译时间戳区分。

2. **启动模式自检**：`[cod6] MODO applet=0 (aplicacion (memoria completa))  invitado=55/38 MB`
   （`applet=0` 就是 `AppletType_Application`；后面带的 MB 是启动瞬间的值，不是最终值）。

3. **GPU 插件兜底**：`gpu_plugin` 为空（toml 读不到）时程序自己用 `xenos`，并打印

       [cod6] AVISO: gpu_plugin vacio (cod6.toml ilegible?) -> se usa 'xenos' por defecto

## 渲染器相关（新增代码位置）

| 文件 | 作用 |
|---|---|
| `sdk/src/ui/vulkan/vulkan_presenter.cpp` | **落点**：在 swapchain render pass 里画（`paint_context_.swapchain_render_pass` / `…_format`） |
| `sdk/src/graphics/vulkan/command_processor.cpp` | `IssueDraw` 里的诊断（`VSHIST` 统计小绘制） |
| `src/cod6_shaders_debug.h` | 手写 SPIR-V（用 `spirv-as` 汇编，不需要 glslang） |
| `src/cod6_nativo.cpp` | 项目侧的原生路径（开关：`cod6_nativo_envio_propio`） |

## 版本与 md5（本轮）

| 版本 | md5 | 说明 |
|---|---|---|
| 63 | `71b79296…` / `2701d35d…` | 管线诊断、构建指纹、模式自检 |
| 64 | `dc73e6ca…` | `DIAG64` |
| 65 | `a83380d8…` | 双格式管线 |
| 66 | `b13ea600…` | **洋红矩形（落点验证通过）** |
| 67 | `0266dbe7…` | 我们的管线画三角形（未成功加载，netloader 传坏） |
| 68 | `efe46d37…` | 回到矩形（线缆验证通过） |
| 69 | `51b2d83b…` | **三角形成功（里程碑②）** |
| 70 | `afdfa50a…` | 干净可用版（测试绘制全关） |
| 71 | `0b68ece1…` | 顶点格式导出 |
| 72 | `3df19722…` | 小绘制统计（当前） |

---

## 第 187~197 轮：新增的构建与调试设施（2026-10-31 深夜）

| 设施 | 说明 |
|---|---|
| **MTP 只读拉日志** | 	ools/mtp_pull_logs.ps1（Explorer Shell 走 MTP）。注意：MTP 报的 Size 恒为 0；-Destino 必须用纯 ASCII 路径，否则中文被 PowerShell 5.1 读成乱码 |
| HTTP 只读拉日志 | 	ools/pull_logs2.sh（http://192.168.31.160:1234/switch/cod6-nx/logs/，PUT 返回 501，不可写） |
| 每轮构建脚本 | 	ools/buildNN.sh：改版本号 → 
inja -C out/nx cod6 → 拷到 测试\当前\switch\cod6-nx\ |
| 手写 SPIR-V | 	ools/cod6_quad.frag.spvasm → spirv-as → src/cod6_shaders_quad.h（	ools/gen_quad_shader.sh 一键生成） |
| 版本与 md5（本段） | v88 1c8bfb49（稳定）、v93 3be5066（UI 我们画）、v95 8d752416（四段实验）、v96 c7f1ec52（失败，文字乱）、**v97 dd0ef9b4（当前交付）** |

**规矩（血的教训）**：

1. nro 一律线缆复制（netloader 会在 34.42% 处"成功"返回但文件不完整）；
2. MTP 只读，不写卡；
3. 出包前检查 toml 有效键，实验开关必须清掉；
4. 黑屏先查 toml，再怀疑传输。
