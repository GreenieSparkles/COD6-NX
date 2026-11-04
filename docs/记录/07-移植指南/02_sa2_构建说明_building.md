# Building sa2-nx

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


How to build the Windows PC build (used for development and testing), the Nintendo Switch NRO, the shader package and
the SD card folder, from your own copy of the game.

## What you need

- **Your own XBLA copy of Sonic Adventure 2:** the package
  `Content/0000000000000000/58411202/000D0000/525E7BB1E72A2DB99AB79281BA24938048198FAD58` from an Xbox 360. The
  Battle DLC (`58411202/00000002/SonicAdventure2Battle`) is optional.
- **Windows 10/11** with:
  - VS 2022 Build Tools (the MSVC libraries and the Windows SDK; CMake and Ninja come with it);
  - LLVM clang 20 (a portable copy is fine; by default `..\_tools\llvm`);
  - Python 3 and Git.

  `tools/win/env.ps1` finds them; override the paths with `SA2_LLVM`, `SA2_VSDIR` and `SA2_CMAKE_DIR`.
- **For the Switch:**
  - [devkitPro](https://devkitpro.org) with devkitA64 and libnx;
  - the NVK Vulkan driver built from [mesa-switch](https://github.com/danfromtico/mesa-switch), as a static
    `libvulkan.a`. SA2 links nfsmw-nx's build; set `NVK_SDK` to another one;
  - glslangValidator (`tools/win/build_glslang.ps1` builds it into `out/glslang`).

Run every build command from PowerShell, through `tools\win\run.ps1` or the scripts below. Running `cmake` from a
Bash shell picks up MSYS2's tools: the configure fails and wipes the build folder's cache.

## 1. Extract the game

```powershell
# Your package (it is only read, never written; ~2.4 GB)
python tools\stfs_extract.py "<...>\58411202\000D0000\525E7BB1E72A2DB99AB79281BA24938048198FAD58" assets\game_root
# Optional: the Battle DLC, installed by the game from dlc\ at start
mkdir out\pc\dlc; copy "<...>\58411202\00000002\SonicAdventure2Battle" out\pc\dlc\
```

## 2. Translate the game's code to C++

```powershell
python tools\fetch_thirdparty.py                       # third-party sources of the SDK
powershell -File tools\win\run.ps1 out\host_configure.log "cmake -S sdk -B out/host -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++"
powershell -File tools\win\run.ps1 out\host_build.log "cmake --build out/host --target rexglue"
powershell -File tools\codegen.ps1                     # default.xex and the stage DLL -> app/generated (git-ignored)
```

`tools/codegen.ps1` also runs the post-codegen steps of the optimised flavour (`tools/post_codegen.py`: the
no-volatile precompiled header, direct calls, the localized native copies).

## 3. The PC build

```powershell
powershell -File tools\win\run.ps1 out\pc_configure.log "cmake --preset win-pc -S app"
powershell -File tools\win\run.ps1 out\pc_build.log "cmake --build out/pc"
powershell -File tools\win\boot_test.ps1               # 45 s, muted and minimized, then the end of the log
```

The PC build runs the code generator again by itself when one of its inputs changed (a toml, the xex, the generator).
That build then stops once with "`sa2_pch.h` has been modified since the precompiled header ... was built" (the
precompiled header was made before the regeneration finished): run the build command again.

`out\pc\sa2.toml` holds the PC's settings. It starts **muted and minimized** (`audio_mute`, `window_start_minimized`)
so test runs stay out of the way; set both to `false` to play. `license_mask = 1` runs the full game rather than the
XBLA trial, and `sa2_renderer_native = true` (or the environment variable `REX_SA2_RENDERER_NATIVE=true`) uses the
native Vulkan renderer instead of the emulated GPU.

To record a video while it runs minimized, give `-VideoAt <seconds after start>` and optionally `-VideoFrames` (60
per second). The MP4 and a contact sheet land in `out\pc\captures\` (needs ffmpeg at `C:\ffmpeg\ffmpeg.exe`, or set
`FFMPEG`):

```powershell
powershell -File tools\win\boot_test.ps1 -VideoAt 8 -VideoFrames 600
```

## 4. The shader package

The native renderer draws with the game's shaders translated ahead of time (XenosRecomp to HLSL, DXC to SPIR-V) into
`sa2_shaders.mesp` + `sa2_shaders.mesp.idx`.

1. Run the PC build once with `sa2_dump_shaders = true`: every container the game creates is written to `shaders/`
   next to `sa2.exe`.
2. Build the package from those dumps:

   ```bash
   tools/build_shader_library.sh out/pc/shaders
   ```

Direct3D's own shaders and the variants Direct3D rewrites at run time are found with the discovery dumps
(`SA2_SHADER_DISCOVERY`, `SA2_VERTEX_VARIANTS`, `SA2_SHADER_MISSING`) and `tools/wrap_discovered.py`.
[porting-guide.md](porting-guide.md), chapter 6, explains the whole loop.

## 5. The Switch NRO

```powershell
powershell -File tools\win\build_switch.ps1            # out\nx\sa2.nro (LTO: allow 15-30 minutes)
python tools\check_switch_build.py                     # start-up code at 0, flavour, driver, no vector FMA, ...
powershell -File tools\win\package_switch.ps1          # the SD card folder: out\nx\sdcard\switch\sa2-nx
powershell -File tools\win\package_switch.ps1 -IncludeGame   # the same with the game as game_root.nxpack
```

`build_switch.ps1` configures `out\nx` the first time (`-Reconfigure` does it again). The NRO's name, author, version
and icon (`app/icon.jpg`) are in `app/CMakeLists.txt`. The settings compiled into the NRO are
`app/sa2_switch_defaults.toml`, and the player's `sa2.toml` is `app/sa2_switch.toml`.

The Switch build's own switches for the CPU round of 2026-10-12, all ON (`-DSA2_SWITCH_...=OFF` at the configure puts
the old form back; `docs/cpu-rounds.md` has what each does): `SA2_SWITCH_OMIT_FRAME_POINTER`,
`SA2_SWITCH_FLUSH_FLOW`, `SA2_SWITCH_GUEST_CAS`, `SA2_SWITCH_VEC_BYTESWAP`, `SA2_SWITCH_NEON_SHUFFLES`,
`SA2_SWITCH_NEON_VMX_HELPERS`. Version 1.0.1 is built with them.

The second round's switches work on every platform (a PC build can take one step out to find where a difference
comes from). ON: `SA2_GEN_RO_CONSTANTS`, `SA2_GEN_FLOAT_LOADS`, `SA2_GEN_LEAF_LOCALS`, `SA2_GEN_LOOP_LOCALS` (third
round), `SA2_GEN_COLD_TRAPS`, `SA2_GEN_PROMOTED_CALLS`, `SA2_GAME_NO_MATH_ERRNO`. OFF, because they are not exact: `SA2_GEN_NV_SAVES_ELIDE` (it
tinted SA2's characters), `SA2_GEN_LR_MOVES_ELIDE`, `SA2_GEN_BACK_CHAIN_ELIDE`, `SA2_GAME_NO_TRAPPING_MATH`.
The post-codegen steps need the generator's image dump `out\default_image.bin`; `tools\codegen.ps1` writes it.

### Profile-guided optimisation

`pgo/` holds the profile the released NRO was built with (one console session of version 1.0.0). To build with it:

```powershell
powershell -File tools\win\build_switch.ps1 -Pgo use
```

`-Pgo none` goes back to a build without PGO. The profile matches the code of this repository: a function you change
loses its part of it, and the rest still applies. [pgo/README.md](../pgo/README.md) explains its file names and what
applies when you build from another folder.

To record a new profile:

1. `build_switch.ps1 -Pgo generate` builds an instrumented NRO. Play it on the console: it writes
   `pgo/sa2_pgo_<N>.stream` next to the NRO (one file per session, saved every 2 minutes and on the HOME button).
2. Move the `.gcda` files of `pgo\` aside, copy the console's `pgo` folder to the PC, and run
   `powershell -File tools\win\pgo_merge.ps1 <folder>`. It merges every session into `pgo\` and fills in each file's
   summary. Merge on the PC that built the instrumented NRO: the streams name its profile folder.
3. `build_switch.ps1 -Pgo use` builds the optimised NRO.

Do not change the game or app code between steps 1 and 3: a changed function loses its profile. What the link
decides can still change, because GCC adds the counters before its interprocedural inliner runs: a build that uses a
profile gives the game code `--param=max-inline-insns-auto=64` and `--param=inline-unit-growth=12`
(`SA2_SWITCH_INLINE_AUTO`, `SA2_SWITCH_INLINE_UNIT_GROWTH`; empty for the compiler's 30 and 40).
`docs/cpu-rounds.md`, "Link-time optimisation of a recompiled program", has the measurements.

## Tests on the PC

| Script | What it does |
|---|---|
| `tools\win\boot_test.ps1` | One muted, minimized run; `-VideoAt` / `-VideoFrames` record it |
| `tools\make_enhancement_configs.py` | The A/B configurations (`out\nx\configs`) |
| `tools\compare_video.py`, `tools\compare_raw.py` | Line up two recordings frame by frame |
| `shaders\tests\test_vertex_fetch_reorder.cpp` | The run-time proof for reordered vertex fetches (`--orders` for every order) |
| `tools\check_switch_build.py` | The checks every Switch build must pass |
| `tools\win\run_sdk_tests.ps1` | Builds and runs the SDK's unit tests (`sdk\tests`): the any-mode float helpers, rotates, the signal protocol, the vector forms |
| `tools\arm_forms_check.py` | What the Switch compiler makes of the AArch64-only forms (lock words, NEON helpers), with the claims checked. No NRO is built |
| `tools\switch_compile_check.py <file> ...` | Compiles chosen files with the Switch compiler and the Switch build's flags into a scratch folder: code only the Switch builds, checked on the PC in a minute |
| `tools\one_file_ab.py <N> name=flags ...` | One recompiled file in several variants with the Switch compiler; instructions, loads and stores counted |
| `tools\pgo_fit.py <N> ...` | How many functions of a recompiled file still match the committed profile |
| `tools\scratch_ab.py copy / count N... / save NAME / compare NAME` | A scratch copy of the generated code (`out\gen_scratch`) to develop a post-codegen step on and measure it with the Switch compiler. `tools\post_codegen.py --gen out\gen_scratch` runs the whole sequence on it |
| `tools\indirect_promotions.py --census <file> --gen app\generated\default` | Makes `app\indirect_promotions.txt` from a PC run with `sa2_diag_indirect_census = "<file>"`: the indirect call sites that go to one target |
| `tools\loop_body_census.py --compare before.o after.o` | Instructions, loads and stores inside loops of two AArch64 objects: for a step that changes loops, where a file's size says nothing |
| `tools\switch_compile_check.py --like <known source> <new source>` | A source the configured Switch tree does not list yet, compiled with a sibling's flags |
| `tools\lto_census.py <elf>` | What the LTO link left in a Switch ELF: the calls between game functions that were not inlined (by the callee's size), functions with one call site left, calls out of the LTO unit, the clones and cold parts the optimiser made |
| `tools\lto_inline_report.py [--save NAME] [--against NAME]` | After a Switch link configured with `-DSA2_SWITCH_LTO_DUMP=ON`: the calls the inliner took and refused (by reason), counted as often as they ran in the profile's session, GCC's profile-weighted time estimate, the size of the hot functions, and against a saved report the copies a setting added |
| `tools\nro_set_version.py <in.nro> <out.nro> <version>` | Sets the version the homebrew menu shows (the NACP's display version) in a copy of an NRO, so a release is byte for byte the build that was tested. 1.0.1 was made this way from the tested build |
| `tools\address_taken_census.py` | An estimate of the game functions nobody takes the address of, and how many of them are called from one place (the code LTO cannot treat as it would in an ordinary program) |
| `tools\crt_signatures.py make / scan` | Signatures of the C runtime functions one port found (`tools\crt_signatures.json`), looked for in any game's image dump |
| `tools\census_summary.py <log>`, `tools\census_sites.py <log>` | A run with `sa2_diag_runtime_census = true`: float-mode switches and runtime calls per Swap, and the functions the switches come from |

Lockstep captures (`REX_VBLANK_PER_SWAP=true`, `REX_SA2_VIDEO_FRAME=6000`) pin every frame to a presented swap, and
`<name>.hash` keeps the XXH3 of each frame at full resolution. Two runs are equal when their hashes are, frame for
frame (align the labels by content first: each run numbers its swaps from its own start).
