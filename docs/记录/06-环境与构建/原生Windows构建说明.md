# 原生 Windows 版：构建说明与当前卡点

<!-- AUTHOR-BANNER -->
> # 🚫⛔ 致 Telegram「DarkNSP」搬运者的一封公开信 ⛔🚫
>
> 我知道你在看。你可能已经点开这个页面，扫了一眼，觉得「又是那个做半成品的人」，然后顺手把它和几个 G 的游戏文件塞进一个压缩包，配上「整合包已打包、解压即玩」的文案，发进群里。
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
> 我不反对搬运。这个项目放在 GitHub 上就是给所有人看的。你要转，可以——**只转仓库，附上声明，写清楚「半成品、有大量 bug、不保证能玩」**。别把游戏本体塞进去，那是侵权；别写「解压即玩」，那是骗人；别省略那句「不保证可玩」，那是我唯一能替玩家守住的东西。
>
> 如果你连这三件事都不愿意做，那就**别搬**。
>
> **我把它放到 GitHub 上，是因为我在这个阶段卡住了。** 贴图这个问题我已经绕了三百多轮还没绕出来，一个人做不下去了——**开源就是想让愿意一起弄的人进来**。我做这个东西，是因为我想知道一台 2019 年的掌机能不能跑动 2009 年的引擎；想知道 PM4 命令环能不能被重新读懂；想知道重编译出来的 ARM64 代码能不能把几十万面的场景推上屏幕。我不是为了流量，也清楚它现在拿不出手。
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
> I know you are reading this. Maybe you opened this page, skimmed it, thought “oh, that unfinished project again”, then dropped it into an archive together with a few gigabytes of game files, added the line “all-in-one pack, just extract and play”, and pushed it into your group.
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
> I am not against sharing. This project is on GitHub for everyone to read. Share it if you want — **share the repository, attach this notice, and state plainly: work in progress, many bugs, no guarantee that it is playable.** Do not bundle the game data in, that is infringement. Do not write “just extract and play”, that is a lie. Do not drop the “no guarantee that it is playable” line, because that sentence is the only thing I can still hold on to on behalf of the players.
>
> If you are not willing to do even those three things, then **do not share it at all**.
>
> **I put this on GitHub because I am stuck at this stage.** I have been going around the texture problem for more than three hundred test runs and I cannot get past it alone — **open-sourcing it is my way of asking for help**. I built this because I wanted to know whether a handheld from 2019 can run an engine from 2009; whether the PM4 command ring can be understood again; whether recompiled ARM64 code can push a scene of hundreds of thousands of triangles onto that screen. I am not doing it for traffic, and I know perfectly well that it is not presentable yet.
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
> 1. **只搬仓库，不搬游戏数据。** 不要附带任何 `.ff` / `.bik` / `.pak` / `game_root`。
> 2. **必须写明：半成品、有大量 bug、不保证能玩、非商业。**
> 3. **必须保留作者署名（BILIBILI UID 122607497）与仓库链接。**
>
> 1. **Share the repository, not the game data.** No `.ff` / `.bik` / `.pak` / `game_root` files.
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


> 时间：2026-10-28 凌晨
> 结论：**能编、能跑、真显卡/真声卡/真手柄全部识别，但 D3D12 呈现会在 10~24 帧后死锁。**

## 一、为什么必须走原生 Windows 版

WSL 里没有 GPU，只能用 lavapipe（CPU 软件光栅化）。实测：

| 场景 | 软件渲染帧率 |
|---|---|
| 主菜单 | 35~58 FPS（勉强可玩） |
| 坑道关卡（3D） | **1~5 FPS（完全不可玩）** |

ReXGlue 官方移植（Dante's Inferno 到 240 FPS、WWE'12 到 60 FPS）全部是**模拟 GPU + 真显卡**。
所以瓶颈不是渲染器架构，是**没有 GPU**。原生 Windows 版直接用 RTX 4060。

## 二、工具链要求（来自官方 Wiki Getting-Started）

> Windows: Visual Studio 2022 with Desktop development with C++, including:
> - **C++ Clang Compiler for Windows (20.x or newer)**
> - MSBuild support for LLVM (clang-cl) toolset

**MSVC 编不了** —— SDK 头文件用了 `__builtin_bswap32/16/64`、`__VA_OPT__`（clang/GCC 扩展）。
用 MSVC 报的错：

```
fmt/base.h(465): error C2338: 'Unicode support requires compiling with /utf-8'
rex/ppc/function.h(130): error C3861: "__builtin_bswap32": 找不到标识符
rex/ppc/stack.h(43): error C2146: 语法错误: "__VA_OPT__"
```

**实测 clang 19.1.5 也能编过**（Wiki 写 20+，我们这台装的是 19.1.5，一次编过、0 错误）。

安装方式（不需要单独下 LLVM）：

```
vs_BuildTools.exe --add Microsoft.VisualStudio.Component.VC.Llvm.Clang --quiet --wait --norestart
```

装到 `C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\Llvm\x64\bin`。
注意同目录下还有 `ARM64\` 版本，**不能用**（"不是此操作系统平台的有效应用程序"）。

## 三、构建步骤

```bat
call "...\VC\Auxiliary\Build\vcvars64.bat"
set "LLVM=...\VC\Tools\Llvm\x64\bin"
set PATH=%LLVM%;...\CMake\Ninja;%PATH%
set CMAKE_PREFIX_PATH=D:\devkitPro\rexglue\win-amd64
cd /d D:\rexglue-projects\cod6
cmake -S . -B out\build\win-clang -G Ninja ^
      -DCMAKE_BUILD_TYPE=Release ^
      -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
cmake --build out\build\win-clang --parallel 16
```

产物：`out\build\win-clang\cod6.exe`（20.4 MB）+ 需要从 SDK bin 拷的 DLL：
`rexgpu-xenos.dll`、`TracyClient.dll`。

## 四、中文路径的坑（务必看）

Win32 程序从 PowerShell/CMD 传参时，中文路径会被编码成乱码：

```
[error] [core] --game_data_root does not exist: D:\涓绘満鎺屾満\NSLITE\_cod6_extract
```

**解法：建一个 ASCII 目录联接，不用复制 6.8 GB。**

```bat
mklink /J "D:\cod6_data" "D:\主机掌机\NSLITE\_cod6_extract"
```

启动用 `--game_data_root D:\cod6_data`。

## 五、原生版的补丁差异（重要）

`apply_all_patches.py` 加了 `--native` 模式：

```bat
python apply_all_patches.py --gen D:\rexglue-projects\cod6\generated\default --native
```

它**不打 PAD 补丁，并清掉已存在的 PAD 块**。原因：

> PAD 补丁会把 `/tmp/pad_cmd` 的状态无条件写进 `X_INPUT_STATE`。
> Windows 上没有这个文件，读出来全零，**等于把玩家真实手柄输入每帧清零**。

所以原生版必须去掉 PAD，让 SDL/XInput 的真实手柄直接生效。

## 六、已经跑起来的部分（日志证据）

```
[info] [gpu] DXGI adapter: NVIDIA GeForce RTX 4060 Laptop GPU (vendor 0x10DE, device 0x28A0)
[info] [gpu] Direct3D 12 device and OS features: resource binding tier 3, tiled resources tier 3
[info] [sys] Guest memory arena mapped: virtual base 0x0000000100000000
[info] [apu] audio endpoint '耳机 (High Definition Audio Device)': 2 ch, 48000 Hz
[info] [core] SDL JoystickAdded: "Xbox Series X Controller", VendorID(0x045E), ProductID(0x0B13)
[info] [core] SDL Controller 0: 0300fa67..., *,a:b0,b:b1,back:b6,dpdown:h0.4,...
[info] [core] New controller connected to slot 0.
```

**显卡、声卡、蓝牙手柄全部识别并初始化成功。**

## 七、当前卡点：D3D12 呈现死锁

在生成代码里加了跟踪探针（`D:\cod6_trace.txt`，因为 WIN32 子系统没有控制台）：

```
[1] xstart-ENTER
[1] gamemain-ENTER
... VdSwap-CALL 共 10 次 ... 然后永久停止
```

| 配置 | VdSwap 次数（9 秒内） |
|---|---|
| 默认 | **10（卡死）** |
| `--present_render_pass_clear=false` | 10 |
| `--present_effect=none` | 9 |
| `--d3d12_allow_variable_refresh_rate_and_tearing=false` | 10 |
| `--async_shader_compilation=false` | 10 |
| **`--vsync=false`** | **22（有改善，但仍在 ~22 帧后停）** |
| `SDL_GPU_DRIVER=vulkan` 环境变量 | 24（没换后端，日志仍是 DXGI） |

**现象**：客户机跑到第 10~24 次 `VdSwap` 就阻塞在 Present 里，进程 CPU 占用降到 ~20%，56 个线程都睡着。
**没有**任何 D3D12 错误日志。

### 下一步排查方向

1. **用 SDK 的 debug 运行时**（`rexruntimed.dll`，20 MB，带符号）+ 调试器挂上去看阻塞线程的调用栈。
   （`--gpu_plugin xenosd` 试过，debug 插件启动即退出，返回码 1）
2. 查 ReXGlue 的 Discord / issues 有没有 Windows D3D12 Present 卡死的已知问题。
3. 试 `--d3d12_pipeline_creation_threads=1`、`--d3d12_queue_priority` 等 D3D12 专项 cvar。

## 八、对比：WSL/Vulkan 版

| 版本 | 渲染 | 帧率 | 状态 |
|---|---|---|---|
| WSL + lavapipe | Vulkan（CPU） | 菜单 35~58，关卡 1~5 | **能完整跑，但 3D 不可玩** |
| 原生 Windows | D3D12（RTX 4060） | 10~24 帧后卡死 | **基础设施全通，呈现死锁待解** |

WSL 版是完整的参考实现：主菜单、特别行动、坑道关卡、HUD、武器都能渲染，
只是 3D 场景被 CPU 光栅化拖死。原生版一旦解决 Present 死锁，就是可玩的成品。
