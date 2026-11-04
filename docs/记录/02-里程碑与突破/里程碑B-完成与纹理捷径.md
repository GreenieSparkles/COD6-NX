# 里程碑 B 完成（含闪烁修复）+ 纹理的捷径（第 178 轮）

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


---

## 一、里程碑 B 真正完成

| 版本 | 现象 | 结论 |
|---|---|---|
| v77 | UI 变洋红框，位置正确 | 顶点解码 + 坐标正确 |
| v78/v79 | 洋红**疯狂闪烁** | 捕获表和呈现帧错位（多帧在飞） |
| **v80** | **洋红框稳定不动，位置与 UI 元素一致，背景正常** | **里程碑 B 完成** |

修法：四边形列表改成**双缓冲**（一张捕获、一张绘制），切换发生在游戏帧结束
（`IssueSwap`），不再在 present 里清空。

现在的能力链：

    读客机顶点（大端） → 解格式 → 像素坐标 → 自己的管线 → 屏幕上正确位置

## 二、关键测量（177 轮，已确认）

| 状态 | 主菜单 fps | 绘制/帧 |
|---|---|---|
| 正常（模拟层全跑） | 12.8~14.4 | 64~77 |
| 跳掉我们接管的 13 个 | 14.2~14.4 | 64 |
| **跳掉全部绘制** | **38.2~41.1（平均 44.13）** | 0 |

**模拟层绘制路径 ≈ 50 ms/帧；自研渲染器的天花板在 41~44 fps（菜单）。**

## 三、发现了一条纹理捷径

原本以为"纹理"要自己写：从 guest 内存取纹理 → 去 tile（NEON）→ 上传 →
采样缓存。参考项目为此写了 `nfsmw_nativo_texturas_pool.cpp`（15 KB）+ 去 tile 代码。

**但 SDK 已经把这些做好了**，而且有公开接口（`sdk/include/rex/graphics/vulkan/texture_cache.h`）：

```cpp
VkImageView GetActiveBindingOrNullImageView(uint32_t fetch_constant_index,
                                            xenos::FetchOpDimension dimension, bool is_signed);
SamplerParameters GetSamplerParameters(const VulkanShader::SamplerBinding& binding) const;
```

也就是说：**模拟层已经把客机纹理去 tile、上传成 VkImageView 了，我们可以直接拿来用。**
这省掉了自研纹理缓存这一整块工作。

### 实现方案（下一步）

1. **捕获时**：对每个 UI 四边形，从像素着色器里取纹理绑定（fetch constant + 维度），
   调 `GetActiveBindingOrNullImageView` 拿到 `VkImageView`，连同矩形和 4 个 UV 一起存下来；
2. **片段着色器**：保持现在"全屏三角 + scissor 裁剪"的做法，在 FS 里用
   `gl_FragCoord` 和 push constant 里的矩形换算 UV，然后采样纹理 —— **顶点着色器不用改**；
3. **管线**：新建一条带"描述符集（combined image sampler）+ push constant"的管线；
4. **验收**：洋红框变成**真实的文字和图标**，位置不变。

这一条走通之后，UI 部分就基本完成了；剩下的才是关卡里的 3D（渲染目标、深度、着色器常量）。

## 四、当前交付

```
D:\主机掌机\NSLITE\NSMW2\测试\当前\switch\cod6-nx\
    cod6.nro    md5 f4f026dfa0c85aa5e00f545d4118be18   版本 2026-10-31-80
    cod6.toml   md5 4745b44aa0ab3dae0f7341fc06cb9e99
```

行为：主菜单正常 + **稳定的洋红 UI 框**（我们的渲染器画的），fps ~13~14。
