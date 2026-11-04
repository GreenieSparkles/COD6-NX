# PC 环境：现状、可复现步骤、下一步

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


写于 2026-10-31（第 155 轮）。用户提醒："PC 上跑通过，D 盘能找到文件，用 CPU 软渲染跑过"。
核对后确认属实——**我上一版写"第 1 步从未完成"是错的，已在 `卡在哪-流程对照.md` 更正。**

---

## 一、结论先说

| 项目 | 状态 |
|---|---|
| 原版 PC 构建（只编译 `src/main.cpp`，用**预编译** Linux SDK） | ✅ **能跑**。已实测：MW2 标题画面正常显示（`frames\PC版-主菜单-实拍.png`），Xvfb + lavapipe 软渲染 |
| 我们自己的源码树（含渲染器代码 + 全部插桩 + 改过的 SDK）编成 PC 版 | ✅ **编译成功**：`/opt/switch/cod6-ng/out/pc7/cod6`（50.8 MB） |
| 我们的 PC 版**运行** | ❌ 启动后崩溃：带配置时进入 `RENDERIZADOR xenos` 之后 segfault；不带配置时卡在 guest 读零页 `0x0000000C` 的异常循环（`protect_zero` 默认 true） |
| Switch 构建 | ✅ 仍然正常（改了 SDK 可移植性后重新编过，见下） |

---

## 二、原版 PC 构建（已知能跑的那套）

```bash
# 二进制
/opt/build/cod6/build/cod6            # WSL/Linux + lavapipe（CPU 软渲染），36 MB
D:\rexglue-projects\cod6\out\build\win-clang\cod6.exe   # Windows 原生（D3D12），21.4 MB
D:\rexglue-projects\cod6\out\build\win-msvc\            # MSVC 试过，编不过

# 运行（我们实测通过的写法）
Xvfb :99 -screen 0 1280x720x24 &
DISPLAY=:99 openbox &
cd /opt/build/cod6/build
LD_LIBRARY_PATH=/opt/rexglue-linux/lib LD_PRELOAD=/opt/tools/netshim.so \
DISPLAY=:99 VK_ICD_FILENAMES=/usr/share/vulkan/icd.d/lvp_icd.json SDL_VIDEODRIVER=x11 \
./cod6 --game_data_root /opt/game/cod6_en --gpu_plugin xenos
# 截屏：xwd -display :99 -root -out s.xwd && convert s.xwd s.png
```

浏览器监控（用户当时用的就是这个）：

```bash
x11vnc -display :99 -forever -shared -nopw -rfbport 5900 &
websockify --web /usr/share/novnc 6080 localhost:5900 &
# 浏览器打开 http://localhost:6080/vnc.html
```

脚本都在：`D:\devkitPro\tools\wsllive.sh`、`wslstart.sh`、`liveview.py`、`padbridge.py`（手柄桥）。
帧率记录：**WSL+lavapipe 主菜单 35~58 fps、坑道关卡 1~5 fps**；Windows 原生版识别到 RTX 4060 但
D3D12 呈现 10~24 帧后死锁（详见 `原生Windows构建说明.md`）。

---

## 三、我们自己的树编 PC 版：可复现步骤

```bash
# 1) 依赖（这台机器已装好；换机需重来）
apt-get install -y libx11-dev libxext-dev libxrandr-dev libxi-dev libxfixes-dev \
    libxcursor-dev libxss-dev libxkbcommon-dev libwayland-dev libdecor-0-dev \
    libxtst-dev libx11-xcb-dev libc++-18-dev libc++abi-18-dev
# 2) SDK 的 thirdparty 需要 .git 标记（树是从别处拷来的，没有子模块元数据）
#    以及一份完整的 SDL3 源码（原来那份是残缺的）
cd /opt/switch/cod6-ng/sdk/thirdparty
git clone --depth 1 --branch release-3.4.18 https://github.com/libsdl-org/SDL.git sdl3
# 3) 配置 + 编译
cd /opt/switch/cod6-ng
cmake -S . -B out/pc7 -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_COMPILER=clang-18 -DCMAKE_CXX_COMPILER=clang++-18 \
  -DREXSDK_DIR=/opt/switch/cod6-ng/sdk \
  -DREXGLUE_ENABLE_TRACY=OFF \
  -DREXGLUE_GPU_PLUGIN_STATIC=ON \
  -DCMAKE_C_FLAGS='-march=native' -DCMAKE_CXX_FLAGS='-march=native'
ninja -C out/pc7 -j 14 cod6        # ~4 分钟（16 核），产物 out/pc7/cod6
```

### 为了编过，改了这些地方（全部可用 git 反向撤销；Switch 行为不变）

| 文件 | 改动 | 为什么 |
|---|---|---|
| `CMakeLists.txt`（项目） | PC 分支加 `COD6_GPU_INCLUDES` + imgui/renderdoc/thirdparty include，`REXGLUE_GPU_PLUGIN_STATIC` 时链 `rexgpu-xenos`，加 `src/switch_stubs_pc.cpp` | Switch 分支本来就加了这些 include；PC 分支缺 |
| `src/cod6_app.h` | `RexSwitchWatchdogStart()` 用 `__SWITCH__` 包起来 | 该符号只有 Switch 版 SDK 提供 |
| `src/switch_stubs_pc.cpp`（新） | `RexSwitch*` 空实现 + `g_rex_log_extra_fd` | 生成代码和我们的插桩都调这些 |
| `sdk/CMakeLists.txt` | 新选项 `REXGLUE_GPU_PLUGIN_STATIC`（默认 OFF）+ Linux 下定义 `REXGPU_PLUGIN_STATIC` | 让 PC 也能把插件编进可执行文件 |
| `sdk/src/graphics/CMakeLists.txt` | 该选项时 `rexgpu-xenos` 编成 STATIC | 原来只有 Switch 是 STATIC |
| `sdk/src/system/CMakeLists.txt` | 同上，`rexruntime` 编成 STATIC（**只改 STATIC/SHARED 那一处**） | 共享库会缺 `rex_gpu_create` 符号 |
| `sdk/src/system/gpu_plugin_loader.cpp` | 静态分支条件加 `|| defined(REXGPU_PLUGIN_STATIC)` | 走 `rex_gpu_create` 直连而不是 dlopen |
| `sdk/src/system/xthread.cpp` | `#include <switch.h>` 和 `threading_switch.h` 用 `__SWITCH__` 包起来 | Linux 上没有 libnx |
| `sdk/include/rex/system/expected_compat.h`（新） | `rex::compat::expected`：有 std::expected 就用，没有就给个最小实现 | clang 18 + libstdc++ 13 拿不到 std::expected（libstdc++ 对 clang 隐藏了它） |
| `sdk/include/rex/system/map_parser.h`、`sdk/src/system/map_parser.cpp` | `std::expected`/`std::unexpected` → `rex::compat::*` | 同上 |

> 注意：`-march=native` 是必须的，否则 SDK 的 SSE 内联函数报 "requires target feature 'ssse3'"。
> 用 GCC 不行：SDK 硬性要求 Clang。用 libc++ 也不行：`from_chars`/`clock_time_conversion` 缺失。

### 现在的运行状态

```bash
# 不带 toml：卡在 guest 读 0x0000000C 的异常循环（protect_zero 默认 true）
# 带最小 toml（protect_zero=false, log_level=info）：进入 "RENDERIZADOR xenos" 之后 segfault
```

---

## 四、下一步（按顺序）

1. **修我们 PC 版的启动 segfault**（当前唯一挡路的东西）：
   `cd /opt/switch/cod6-ng/out/pc7 && gdb -batch -ex run -ex bt --args ./cod6 --game_data_root /opt/game/cod6_en`
   （需要 DISPLAY=:99 + lavapipe 环境变量）。最可能是静态插件路径与 `cod6_renderizador` 的
   `Cod6GraphicsSystem` 选择逻辑冲突，或 lavapipe 缺某个 Vulkan 特性。
2. 跑起来后，把 `Xvfb + lavapipe + 截图 + 日志` 做成一条命令（脚本已备好 `pc_run2.sh`），
   **迭代就从"插卡一次"变成"编译 + 跑 50 秒"**。
3. 然后用它做渲染器开发：顶点解码 → 自己的管线 → 接管 UI 小绘制（验收看像素，不看 fps）。

## 五、遗留

- 改了 SDK 之后，Switch 重新编出的 nro 是 `1819346f20692e7a455d318f432122c3`（原 `4b0d8d13…`）。
  两者功能等价（只是可移植性改动），但**卡上那版仍是 `4b0d8d13…`（版本 60）**，v61 的 toml 测试照用。
- 备份：`备份\完整备份-2026-10-31-61\`（文档 + toml）。这一轮还改了源码，源码备份见
  `备份\完整备份-2026-10-31-61-PC移植源码\`（只放这次改过的 10 个文件）。
