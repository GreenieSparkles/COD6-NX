# PC 环境：现状、可复现步骤、下一步

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
