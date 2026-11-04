# 本轮记录 · 第 306 轮（代码级定位：采样优先用了「渲染目标的旧图」）

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


> 上一轮我拿放大后的画面下结论，这不算证据 —— 这一轮全部从代码出发。
> 行号对应 `src/native/nativo/native_textures.cpp`（v305 源码，`备份\源码\v305_src.tar.gz`）。

---

## 一、代码里的事实

### 事实 1：采样路径的第一道分支，按**地址**命中渲染目标，直接返回那张图

`GetViewUncached()`，约 2844 行：

```cpp
// A texture a resolve wrote: the image, not the memory.
auto it = im.resolve_destinations.find(im.SampleKey(base_page << 12));
...
if (it != im.resolve_destinations.end()) {
  ResolveDestination& dest = *it->second;
  uint32_t swizzle = GuestToHostSwizzle(fetch.swizzle, dest.host_swizzle);
  ...
  return v->second;          // 渲染目标的图像
}
```

**键只有 guest 地址**（`base_page << 12`）。这一步在贴图上传/布局计算**之前**，
所以 guest 内存里的内容根本没被看过。

### 事实 2：唯一「内存被写了」的入口，不会失效这些图

`InvalidateRange()`，约 2460 行 —— 已知写者（文件读入、guest 填充、GPU 拷贝）都从这里上报：

```cpp
for (uint64_t key : list->second) {
  ...
  if (base_hit || mip_hit) {
    if (Impl::Checked* c = im.checked.find(key)) *c = Impl::Checked{};   // 内容哈希作废
    auto t = im.textures.find(key);
    if (t != im.textures.end()) t->second->invalidated = true;           // 贴图作废
    any = true;
  }
}
if (any) ++im.answers_epoch;                                            // 绑定/槽位缓存作废
```

全文 grep `resolve_destinations`：只在 `GetResolveDestination()` 里 erase
（格式/尺寸不符时），**没有任何一处因为「guest 内存被写」而失效它**。

### 结论（由事实 1+2 直接推出，不依赖画面）

> 一个地址一旦当过渲染目标，之后贴图流把另一张贴图写进同一块内存，
> 采样**仍然**返回那张渲染目标的图；代码里没有任何路径会复核这个决定。

「哪个物体中招」取决于贴图流复用了哪个曾经是渲染目标的地址 —— 与
「任意物体、随机变化、近处也有、越玩越糟」四个特征全部吻合。

---

## 二、护栏为什么这样写（我否掉了第一版）

### 第一版（已丢弃）：用页写戳判断「之后有没有被写」

```cpp
if (cod6::writes::Enabled() && candidate.stamp_serial &&
    !cod6::writes::g_pages.Unchanged(addr, bytes, candidate.stamp_serial)) { 绕过 }
```

**读 `cod6_page_writes.h` 后发现这是错的**：`Unchanged()` 在
`unknown_ > since` 或 range 非法时直接 `return false`。而实测日志一直是
`[texmon] marcas: aciertos 0 fallos 29500` —— **它几乎恒假**。
拿它当条件等于「所有渲染目标采样全部绕过」，后处理（辉光/模糊/HUD）会立刻崩。

> 这也顺带解释了一个旧谜：写戳系统的 `aciertos 0` 不是「没通电」，
> 是 `Unchanged()` 的保守判据在真实工作负载下恒假。

### 第二版（已交付）：用 fetch 常量本身的语义，构造上不可能误伤

```cpp
const xenos::TextureFormat base_format = rex::graphics::GetBaseFormat(fetch.format);
const bool compressed_fetch =
    base_format == xenos::TextureFormat::k_DXT1  || base_format == k_DXT2_3 ||
    base_format == xenos::TextureFormat::k_DXT4_5 || base_format == k_DXN;
if (compressed_fetch) it = im.resolve_destinations.end();   // 走正常贴图路径
```

**理由**：渲染目标解析出来的是**未压缩**颜色数据。一个请求 DXT1/DXT3/DXT5/DXN 的
fetch，逻辑上不可能是要渲染目标。所以「压缩 fetch 撞上渲染目标地址」只有一种解释：
那块内存已经被贴图复用了。**没有假阳性。**

同时它不会伤害后处理：后处理采样的 fetch 是未压缩格式，照旧走渲染目标图。

---

## 三、这一版怎么判断成败（不看画，看计数）

日志里新增（每 256 次打一行）：

```
[resolve] rancio: 地址 是 resolve 目的地(宽x高)；fetch 要 DXT n 的 宽x高，说明内存已经是贴图（N 次）
```

| 结果 | 含义 | 下一步 |
|---|---|---|
| `N = 0` | 这条路径被洗清：压缩贴图从没撞上渲染目标地址 | 换下一处：见第四节 |
| `N > 0` | 就是它，而且能拿到具体地址与次数 | 让用户复测同一场景，看串图是否消失 |

---

## 四、如果 N=0，下一处要看什么（已在代码里定位到候选）

`GetViewUncached()` 的贴图缓存键 `TextureLayoutKey`（约 280 行）包含
`base_page / mip_page / 宽高深 / pitch / mip_max_level / format / dimension /
endianness / tiled / packed_mips`；

`slot_views[32][8]` 的复用判据是「6 个 fetch dword 全等 + views_epoch + answers_epoch」；

绑定描述符集缓存 `texture_set_cache` 的键是「layout 种子 + 按 `texture_bindings` 顺序的
view id 序列」。

这三处**都是完整键**（`mask_missing=0` 已证绑定键覆盖所有采样槽），剩下唯一没被审计的是
**`GetTextureBindingsAfterTranslation()` 的数组顺序是否严格等于 SPIR-V 里的 descriptor 索引** ——
若不等，同一个绘制内部的贴图会整体错位（把 detail/normal 当 diffuse）。
这条下一轮用「同一个绘制：槽位 vs 描述符索引」的一次性打印来判定。

---

## 五、产物

```
测试\当前\switch\cod6-nx\
  cod6.nro    41,613,509  md5 AEC95622B5ED2DD3B783CA2ED22F0954   2026-11-03-306
  cod6.toml        4,846  md5 E66F79FDA4DF0EE2C3BFFB47A5201EC0
  cod6.nacp       16,384  （hbmenu 里显示 2026-11-03-306）

单变量：这一版 mip 重建关闭、force_mip0 保持 true，画面风格与 v302/v303 一致，
只暴露「resolve 旧图护栏」一个变化。

备份\源码\ v302 v303 v304 v305 各一份（源码，约 445 KB/份）
```
