# PC 环境进展与卡点（第 157 轮）

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


## 一、已经做到的

| 步骤 | 状态 |
|---|---|
| 我们的源码树编成 PC 版（含渲染器代码 + 全部插桩 + 改过的 SDK） | ✅ `out/pc7/cod6`，50.8 MB |
| 程序跑起来：进到 guest 线程 | ✅ 日志 `phase 1..7/9` 全过，音频、输入、文件系统都通 |
| 找到并修掉"画面全黑"的第一个原因 | ✅ toml 少了 `gpu_plugin = "xenos"`，日志里有 `no GPU emulation loaded (gpu_plugin not set)` |
| 打开 GPU 插件后 | ❌ **崩在 lavapipe 里面** |

## 二、当前卡点（栈）

```
#0 libc
#1 libvulkan_lvp.so          ← lavapipe（CPU 软件 Vulkan）
#2 libvulkan_lvp.so
#3 rex::ui::vulkan::VulkanPresenter::InitializeSurfaceIndependent()
#4 VulkanPresenter::Create(...)
#5 VulkanProvider::CreatePresenter(...)
#6 GraphicsSystem::SetupPresentation(...)
#10 rex::ReXApp::OnInitialize()
```

**不是我们的代码、也不是插件加载**，是 lavapipe 在 presenter 建管线/描述符时崩了。
同一个 lavapipe 跑**原版 PC 构建**（`/opt/build/cod6`，用预编译 SDK）是正常的，
所以差别在"我们这套 SDK fork 的 presenter 代码"和"预编译 SDK 的 presenter 代码"之间。

## 三、四条可选路线（按性价比）

### A. 用 netloader 在真机上迭代（首选，但需要用户配合）

本机有 `/opt/devkitpro/tools/bin/nxlink`。Switch 与电脑同一 WiFi、用 hbmenu 的 netloader 启动后：

```
nxlink -a <Switch的IP> out/nx/cod6.nro
```

**这样根本不需要 PC 版**：迭代是秒级，而且量到的就是真机数字。渲染器开发就在真机上调。
代价：需要用户开 hbmenu（按住 R 点游戏图标）+ 告诉我 IP。

### B. 在 Windows 上用 Vulkan 后端跑（RTX 4060，真显卡）

插件支持 `backend = "d3d12" | "vulkan" | "any"`，但 `LoadGpuPlugin` 目前只传了名字
（`rex_app.cpp:343` 只传 `config_.gpu_plugin`），所以 Windows 上会选 D3D12 → 死锁。
可以给 app 加一个 cvar（如 `cod6_backend="vulkan"`），让它直接调
`LoadGpuPlugin("xenos", "vulkan")`。代价：要配 Windows 工具链（VS Clang + CMake + Ninja），
且要确认预编译的 `rexgpu-xenos.dll` 里编译了 Vulkan 后端。

### C. 继续修 lavapipe 那个崩溃

在 `InitializeSurfaceIndependent()` 里逐个 Vulkan 调用打点，找出是哪一句崩的。
可能半小时，也可能几小时；而且就算修好，lavapipe 也只能验逻辑、不能量性能。

### D. 回插卡模式，但每轮只发"有可见/可测变化"的包

最稳，最慢。第一里程碑（顶点解码）本身每一步都是小验证，插卡也能推进，只是慢。

## 四、我的建议

**先问用户能不能开 netloader（A）。** 能 → 直接走 A，PC 这条线的必要性立刻下降；
不能 → 走 C（我本地调试，不占用用户的卡），修完再回到渲染器主线。

无论走哪条，**渲染器第一里程碑的工作内容不变**：

    ① 顶点/索引解码（照 nfsmw_nativo_dibujos.cpp）→ 日志打出真实顶点值
    ② 自己的管线 + 绑定 → 屏幕上出现我们画的方块，位置正确
    ③ 接管 index_count ≤ 8 的 UI 绘制 → 菜单 UI 由我们画
