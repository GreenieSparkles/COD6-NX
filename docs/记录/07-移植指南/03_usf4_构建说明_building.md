# Building usf4-nx

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


How to build and run the PC version from your own copy of the game. Windows is the main PC platform (the guide's
toolchain); Linux works too (section "Linux"). Nothing from the game is in this repository: you extract your disc,
translate its code and, for the native renderer, use a shader package made from your copy.

## What you need (Windows 10/11)

- **Your own disc image** of Ultra Street Fighter IV for Xbox 360 (`Ultra Street Fighter IV (World).iso`, title
  434307FC; ~8.7 GB).
- **Visual Studio 2022 Build Tools** with the *Desktop development with C++* workload (MSVC libraries, Windows SDK;
  CMake and Ninja come with it). <https://visualstudio.microsoft.com/visual-cpp-build-tools/>
- **LLVM clang 20** (the SDK needs clang): the Windows x64 archive or installer from
  <https://github.com/llvm/llvm-project/releases> (e.g. `clang+llvm-20.1.8-x86_64-pc-windows-msvc.tar.xz`).
  Put it in a folder `_tools\llvm` **next to** the `usf4-nx` folder (so `..\_tools\llvm\bin\clang++.exe`), or point
  `$env:SA2_LLVM` at it.
- **Python 3** and **Git** on the PATH.
- About **30 GB** free: 8 GB for the extracted game, the rest for the build.
- A GPU with a current **Vulkan** driver (any NVIDIA/AMD/Intel card from the last ten years).

Run every command from **PowerShell** in the `usf4-nx` folder. Build commands go through `tools\win\run.ps1`, which
loads the compiler environment and keeps the whole output in a log (`out\*.log`).

## Steps

```powershell
# 0. Allow the scripts in this window (once per window)
Set-ExecutionPolicy -Scope Process Bypass

# 1. The SDK's third-party sources (~19,000 files from the ReXGlue SDK release, needs Git; 2-5 minutes)
python tools\fetch_thirdparty.py

# 2. Extract your disc (only reads the .iso; ~8 GB, a few minutes)
python tools\extract_iso.py "D:\path\to\Ultra Street Fighter IV (World) (En,Ja,Fr,De,Es,It,Ko).iso" -o assets\game_root

# 3. Build the code generator (10-20 minutes the first time)
tools\win\run.ps1 out\host_configure.log "cmake -S sdk -B out/host -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_FLAGS=-march=x86-64-v2 -DCMAKE_CXX_FLAGS=-march=x86-64-v2"
tools\win\run.ps1 out\host_build.log "cmake --build out/host --target rexglue"

# 4. Translate the game's code to C++ (about 2 minutes; writes app\generated, 157 MB)
tools\codegen.ps1

# 5. glslangValidator, for the native renderer's own shaders (a few minutes)
tools\win\build_glslang.ps1

# 6. Build the game (20-40 minutes the first time: ~160 MB of generated C++)
tools\win\run.ps1 out\pc_configure.log "cmake --preset win-pc -S app"
tools\win\run.ps1 out\pc_build.log "cmake --build out/pc"
```

The result is `out\pc\usf4.exe`, with `usf4.toml` (settings) next to it.

**7. The shader package** for the native renderer: copy `sa2_shaders.mesp` and `sa2_shaders.mesp.idx` (made from
your copy of the game, see "Making the shader package") into `out\pc\`.

## Playing

```powershell
tools\win\play.ps1            # native Vulkan renderer, window, sound
tools\win\play.ps1 -Emulated  # ReXGlue's emulated GPU instead (slower; the reference image)
```

A controller (Xbox / XInput or any SDL-supported pad) works as player 1. The keyboard works too (`play.ps1` turns on
the SDK's keyboard driver, `mnk_mode = true`; click the window first):

| Key | Pad | USF4 |
|---|---|---|
| Enter (or X) | Start | menus, skip movies |
| Space (or ;) | A | confirm, Light Kick |
| Backspace (or ') | B | back, Medium Kick |
| L | X | Light Punch |
| P | Y | Medium Punch |
| 3 | RB | Heavy Punch |
| E (or O) | RT | Heavy Kick |
| W A S D | left stick | move |
| Shift + arrows | d-pad | menus |
| Tab (or Z) | Back | |
 The first run asks to create save data:
choose **YES**. Saves go to `Documents\usf4\` (the SDK's user folder: `GetUserFolder()` / the app name; the `user_data_root` setting moves it).

When you close the game, `play.ps1` prints the frame-rate lines of the log (`[fps] ... fps over 5.0 s`, every 5
seconds). The full log is in `out\pc\logs\`.

`usf4.toml` starts muted and minimized for automated tests; `play.ps1` overrides both. To play without the script,
set `audio_mute = false`, `window_start_minimized = false` and `sa2_renderer_native = true` in `out\pc\usf4.toml`.

### First Windows test (2026-10-11)

Windows 11, Visual Studio 2026 Community (`18\Community`: set `$env:SA2_VSDIR` to it, the scripts default to the 2022
Build Tools folder), LLVM 20.1.8: every step built; the native renderer ran a fight at a steady **60 fps**. Long
commands copied from a chat window can arrive split over several lines: PowerShell then waits with `>>` (Ctrl+C);
building the command in a variable (`$cfg = "..."; $cfg += "..."`) avoids it.

### What to look at (state on 2026-10-11)

- Known wrong with the native renderer: horizontal stripes over the top-left HUD (player 1's health bar and name)
  and a flat yellow sky on the volcano stage. Anything else that looks different from the emulated GPU
  (`-Emulated`) is worth a note, with the place it happens.
- The frame rate in a fight (`[fps]` lines): on a desktop GPU the test machine's limits (an idle laptop GPU and a
  virtual display) are gone, so this is the first real measurement.

## Automated tests

```powershell
# 45 s muted and minimized, then the end of the log and the [fps] lines
tools\win\boot_test.ps1
# native renderer, snapshots at 10/20/30 s (out\pc\captures\snap_<s>s.ppm), scripted gamepad
tools\win\boot_test.ps1 -Native -Seconds 60 -Snapshots "10,20,30" -Script "20 START;26 START"
```

The scripted gamepad (`usf4_input_script`, `usf4_input_script_file`): events `<seconds> <BUTTON[+BUTTON]> [hold]`
separated by `;` or one per line, buttons `A B X Y START BACK LB RB LT RT UP DOWN LEFT RIGHT LS RS LS_UP ...`
(`app/src/usf4_script_input.cpp`).

## Making the shader package

The package holds the game's shaders translated ahead of time (XenosRecomp to HLSL, DXC to SPIR-V). A run only
creates the shaders of the stages and characters it loads, so the package is made from three sources:

1. **A run with `usf4_dump_shaders = true`** (boot, menus, a fight): every container the game creates goes to
   `out/pc/shaders/` (the game's in-memory form of each container).
2. **The game files:** `python3 tools/extract_game_shaders.py assets/game_root out/pc/shaders_game --skip out/pc/shaders`
   pulls every container out of the EMZ streams (`archive/shader/*.emz` and the `.eaf` archives: every stage and
   character). Keep only those whose microcode the dump does not already have (the game rewrites container headers
   when it loads them, so the same shader can differ in its header bytes).
3. **Direct3D's own shaders:** run with `sa2_shader_misses = misses` and wrap what it finds:
   `python3 tools/wrap_discovered.py --raw out/pc/misses/discovery --containers <the folders above> --out <a NEW folder>`
   (it empties its output folder first: one folder per round). Repeat until the log says
   `unidentified loads: ... immediate 0` and no `unresolved draw shader pairs` line appears.

Then `tools/build_shader_library.sh <all the folders>` (bash; on Windows it expects MSYS2's clang64 `clang++` and a
DXC build, see the variables at the top) writes `out/shaders/sa2_shaders.mesp` + `.idx`. The second package
(2026-10-11) has 865 shaders: 565 dumped, 290 from the game files, 9 wrapped Direct3D shaders, 1 rebuilt container.

The file names keep sa2-nx's (`sa2_shaders.mesp`, `sa2_*` settings) so fixes can be taken over from it.

## Linux

Same steps with `tools/codegen.sh`, the `linux-pc` preset and `tools/linux/boot_test.sh` (Xvfb); needs clang >= 18,
CMake, Ninja, `libx11-xcb-dev` and `libwayland-dev`. The SDK's shared libraries are loaded from `sdk/out/linux-amd64`
(`LD_LIBRARY_PATH`, the boot test sets it). glslangValidator:
`cmake -S sdk/thirdparty/glslang -B out/glslang -G Ninja -DENABLE_OPT=OFF -DGLSLANG_TESTS=OFF && ninja -C out/glslang glslangValidator`.
DXC: Microsoft's Linux release unpacked into `out/tools/dxc`.

## Switch (stage 5)

Built on Linux in devkitPro's Docker image, with the repository mounted at the same path:

```sh
D=$PWD/..; R=$PWD
docker run --rm -v $D:$D -w $R devkitpro/devkita64 bash -c "cmake -S sdk/thirdparty/glslang -B out/glslang-dkp -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DENABLE_OPT=OFF -DGLSLANG_TESTS=OFF -DENABLE_HLSL=OFF && ninja -C out/glslang-dkp glslangValidator"
# The Mesa Horizon SDK (nfsmw-nx's Mesa 26.2.2), with libvulkan.a repacked (docs/usf4-notes.md, "Stage 5"):
python3 tools/switch/repack_archive.py <sdk>/lib/libvulkan.a <copy of the sdk>/lib/libvulkan.a --ar aarch64-none-elf-ar
docker run --rm -v $D:$D -w $R -e DEVKITPRO=/opt/devkitpro devkitpro/devkita64 bash -c "cmake -S app -B out/nx -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=$R/tools/switch/cmake/switch-devkitA64.cmake \
  -DREXGLUE_SWITCH_NVK_SDK=<copy of the sdk> -DSA2_GLSLANG_VALIDATOR=$R/out/glslang-dkp/StandAlone/glslangValidator \
  -DDEVKITPRO=/opt/devkitpro [-DUSF4_SWITCH_LTO=OFF] && cmake --build out/nx -j 8"
tools/linux/package_switch.sh          # out/nx/usf4-nx-switch.zip
```

On the SD card: `sdmc:/switch/usf4-nx/` with the NRO and the shader package, and your extracted disc as
`sdmc:/switch/usf4-nx/game_root/` (without `$SystemUpdate`). Launch through a title takeover (hold R while starting a
game) or a forwarder with full memory.
