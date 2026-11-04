# 变更记录

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


版本号格式：`YYYY-MM-DD-NNN`，唯一来源是
`sdk/src/core/exception_handler_switch.cpp` 里的 `COD6_VERSION`。

只记录**影响行为**的改动。每一条都附上验证依据。

---

## 2026-11-03-325

### 新增
- `cod6_native_dxn_ra`：DXN（法线贴图）的通道顺序可切换。
  依据：SDK 的 `cache.h` 明确写「DXN is read as RG in 4D5307E6, but as RA in 415607E6」，
  即该格式**按游戏而异**。默认 `false`（RG）。

### 结论
- 实测 `true`（RA）对画面无改善 → 通道顺序**不是**平表面噪纹的原因，已回退默认。

---

## 2026-11-03-324

### 修复
- **顶点拷贝恢复为「按绘制范围」，但去掉 fetch 声明 size 的截断。**
  上一版把整个绑定区间都缓存，实测每帧拷贝 **61.7 MB**、哈希 **46.2 ms**，
  缓存几乎不命中，导致流缓冲写满回绕、已记录的副本被覆盖（画面更花）。
  现在：`vertex copies 0.1 MB/帧`、`hashing 0.7 ms`。

---

## 2026-11-03-323

### 修复
- **着色器存储初始化时序。** `InitializeShaderStorage` 在渲染器创建之前被调用，
  旧代码 `if (renderer_ready_)` 直接丢弃请求，导致 `cache/shaders/driver/` 从未被
  加载或写入——这就是日志里长期出现 `N shaders (0 cached)` 的原因。
  现在记录请求，渲染器就绪后补做。

### 实测
```
SPIR-V cache: 710 translations (98338 KB) loaded from
  /switch/cod6-nx/cache/shaders/driver/native-41560817-000010DE-00000FE0-06802004.spirv
```

---

## 2026-11-03-322

### 新增
- `cod6_native_memexport_copy` 可用：由 CPU 补做 memexport 写入，
  **拷贝前先比对**几个采样点，字节未变则不写、不失效。
  之前的实现每拷贝一次就 `InvalidateRange` 一次，导致每帧数千贴图重上传、掉到 1 fps。

---

## 2026-11-03-321

### 变更（已被 324 修正）
- 顶点改为整绑定内容寻址缓存。方向正确（对齐 BO2 的 `bo2zm_native_vertex_cache`），
  但导致拷贝量暴涨，见 324。

---

## 2026-11-03-318

### 修复
- **非索引绘制不再套用 `VGT_MIN/MAX_VTX_INDX`。**
  这两个寄存器只对索引绘制有意义；套到 auto-index 绘制上会沿用上一帧索引绘制的值，
  把顶点范围截断，让绘制读到别人的顶点。

---

## 2026-11-03-317

### 修复
- 程序自动创建 `user/`、`cache/`（此前只创建 `update/`）。
  缓存目录不存在时，着色器缓存会**静默失败**。

---

## 2026-11-03-315/316

### 修复
- 删除 `cod6_diag_stubs.cpp` 中与 `cod6_page_writes.cpp` 重复的 cvar 注册。
  `cod6_native_vertex_by_writes` 一直被旧 stub 抢注（日志里那三条
  `duplicate registration`），因此 **toml 里写它从未生效**。

---

## 更早

- `v310`：`force_mip0` 语义修正（真正钳到 mip 0）
- `v308/v309`：停止按年龄驱逐纹理；安全驱逐前校验来源指纹
- `v305/v306`：mip 重建、resolve 旧图复用调查
- `v287`：写入戳跟踪（`cod6_page_writes.cpp`）加入
- `v298`：首次尝试 memexport CPU 拷贝（实现有误，写出界，已回退）

---

## 已排除的方向（避免重复劳动）

| 假设 | 否掉的依据 |
|---|---|
| 纹理内容被游戏改坏 | 45,178 次检查，0 次变化 |
| 显存池块复用导致串味 | 关闭池后现象不变 |
| DXN 通道顺序（RG/RA） | 切换后画面无变化 |
| 顶点范围截断导致平表面噪纹 | 去掉截断后仍存在 |
| 绑定的地址在逐帧变化 | `[dtab]` 逐帧对照，地址稳定 |
| recoder 线程死锁 | `record_thread=false` 更糟 |
| 音频回调阻塞 guest | `audio_mute=true` 现象不变 |
| 第三方 Vulkan 层干扰 | 禁用 OBS/GamePP/Bandicam 后不变 |
