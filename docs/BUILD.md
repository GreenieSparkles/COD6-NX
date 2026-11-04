# 构建与调试

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


## 环境

| 项 | 版本 |
|---|---|
| 系统 | Linux（推荐 WSL2 Ubuntu 24.04） |
| 工具链 | devkitPro `devkitA64`（GCC 15.2）+ `switch-tools` |
| 构建 | CMake ≥ 3.20、Ninja |
| GPU 驱动（实机） | NVK / Mesa 26.x |

环境变量（devkitPro 的 `switchvars.sh` 会设置）：

```bash
export DEVKITPRO=/opt/devkitpro
export DEVKITA64=$DEVKITPRO/devkitA64
```

---

## 编译

```bash
# 首次配置（已有 out/nx 则可跳过）
cmake -S . -B out/nx -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/cmake/Switch.cmake \
      -DCMAKE_BUILD_TYPE=Release

# 构建
ninja -C out/nx cod6
```

产物：

```
out/nx/cod6.nro      可执行（放到 sdmc:/switch/cod6-nx/）
out/nx/cod6.nacp     NACP，版本号由 COD6_VERSION 派生
```

> `cod6.nacp` **不需要随包更新**——只要 NRO 里的版本号变了即可。

---

## 版本号

唯一来源：

```
sdk/src/core/exception_handler_switch.cpp → COD6_VERSION "YYYY-MM-DD-NNN"
```

`CMakeLists.txt` 用 `configure_file` 从它派生出 NACP 版本，
并已加 `CMAKE_CONFIGURE_DEPENDS`，改完直接 `ninja` 会重新配置。

---

## 部署到实机

有三种方式：

### 1. SD 卡（最稳）

```
sdmc:/switch/cod6-nx/
├── cod6.nro
├── cod6.toml
└── game_root/
```

### 2. netloader（迭代最快）

```bash
nxlink -a <Switch的IP> out/nx/cod6.nro
```

Switch 端在 hbmenu 里按 `Y` 启动 netloader。改一行代码 → 编译 → 推过去，十几秒一轮。

> 注意：netloader 会把参数截断到约 256 字符，所以 `cod6.toml` 里不要放超长命令行。

### 3. PC 端对照（可选）

同一份源码在 Linux 上用 lavapipe（软件 Vulkan）+ Xvfb 可以跑起来，
用于**渲染逻辑**的对照调试（驱动相关的问题不会复现）：

```bash
cmake -S . -B out/pc -G Ninja -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
      -DREXGLUE_USE_VULKAN=ON -DREXGLUE_USE_D3D12=OFF
ninja -C out/pc cod6
VK_ICD_FILENAMES=/usr/share/vulkan/icd.d/lvp_icd.json \
  xvfb-run -a -s "-screen 0 1280x720x24" ./out/pc/cod6 \
  --game_data_root=/path/to/game_root
```

> PC 版在 Windows 上需要 `REXGLUE_GPU_PLUGIN_STATIC=ON`
> （PE 的 DLL 不能引用 exe 里的符号），并设 `cod6_gpu_backend = "vulkan"`。

---

## 代码结构

```
src/
├── main.cpp                    入口
├── cod6_app.h                  路径配置、启动阶段标记
├── cod6_renderizador.cpp       渲染器选择
├── cod6_nativo.cpp             原生路径的钩子实现
└── native/
    ├── nativo/
    │   ├── native_renderer.cpp      PM4 环读取、绘制、管线、提交
    │   ├── native_textures.cpp      纹理缓存、反平铺、上传
    │   ├── texture_pool.cpp         显存池
    │   ├── native_hash.h            内容变更检测（采样 + 信任窗口）
    │   ├── cod6_page_writes.cpp     写入戳（4 KB 页）
    │   └── cod6_native_system_nativo.cpp  系统层、周期报告
    └── cod6_native_reader.cpp    命令环读取
```

### 两条渲染路径

| `cod6_renderizador` | 说明 |
|---|---|
| `"nativo2"` | 自研原生 Vulkan 渲染器：直读 PM4 环，自己管状态、纹理、管线、渲染目标 |
| `"xenos"` | SDK 的通用 Vulkan 路径（Xenia 血统）——**正确但极慢**，仅用于对照 |

---

## 调试要点

### 先确认进程状态

黑屏时先看日志的阶段标记：

```
[cod6] phase 1/9 … phase 9/9
```

卡在哪个 phase 就是哪个子系统的问题。

### 缓存目录必须存在

`cache/shaders/driver/` 不存在时，着色器缓存**静默失败**：

```
[native] renderer: … 346 shaders (0 cached) …
SPIR-V cache: 710 translations (98318 KB) loaded from …   ← 正常时应有这行
```

### 性能指标基线

正常水平（关卡内）：

```
vertex copies 0.1 MB/帧      copying 0.0 ms      hashing 0.7 ms
texture lookups 2~8 ms       （超过 20 ms 说明哈希在空转）
renderer CPU 26~34 ms        GPU 18~23 ms
```

### 已知的侵入性诊断（会掉帧）

| 开关 | 代价 |
|---|---|
| `cod6_native_full_texture_hash = true` | 每帧全量哈希，+20 ms/帧 |
| `cod6_native_texture_pool = false` | 每贴图独立分配，约 1.9 ms/张 |
| `cod6_debug_tex_album > 0` | 导出贴图，每个抓取点一次卡顿 |
| `cod6_native_texture_evict_frames` 极小值 | 频繁回收，导致重上传 |

诊断完记得关掉，否则会误判为「渲染慢」。
