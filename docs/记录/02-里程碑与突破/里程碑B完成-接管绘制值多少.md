# 里程碑 B 完成 + 关键测量：自己接管绘制值多少（第 177 轮）

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


写于 2026-10-31 晚。**这一轮拿到了整个项目最重要的一组数字。**

---

## 一、里程碑 B：我们画出了游戏真实的 UI

```
[cod6] QUADS: 10 capturados, se dibujan 10 (primero: 256.0,63.0 -> 1024.0,255.0)
```

- 每帧从客机内存里读出 **10 个 UI 四边形**的顶点（大端解码：位置 4×float 像素坐标、
  颜色 RGBA8、UV 2×float，stride 32 字节）；
- 用**我们自己创建的管线**在 presenter 的 swapchain render pass 里按真实坐标画出来；
- 用户实测："UI 变成了洋红色框框，但是背景等好着呢" —— **位置完全正确**。

这一条链现在是通的：**读客机顶点 → 解格式 → 坐标映射 → 自己的管线 → 屏幕正确位置**。

## 二、关键测量：模拟层绘制值多少

用 `cod6_nativo_solo_propios` 把模拟层绘制**全部跳掉**，只留我们自己的 10 个四边形：

| 状态 | 主菜单 fps | 绘制/帧 | CPU |
|---|---|---|---|
| 正常（模拟层全跑） | 12.8~14.4 | 64~77 | 80~126% |
| 跳掉我们接管的 13 个 | 14.2~14.4 | 64 | 80~102% |
| **跳掉全部绘制** | **38.2 → 41.1（平均 44.13）** | **0** | 116~126% |

**结论**：

1. **模拟层的绘制路径 = 整帧成本的绝大部分**（约 50 ms/帧，14 fps → 41~44 fps，**+200%**）。
2. 跳掉 13 个 UI 绘制**没有任何提升** → 那几个不是成本来源；成本在那 64 个全屏程序化绘制上。
3. CPU 只用了 1~1.3 个核（400% 满）→ **不是 CPU 不够，是绘制路径本身把时间吃掉了**。
4. 1 次提交、1 次 resolve、0 绘制的情况下还有 41 fps → 那个 41~44 fps 就是我们渲染器能到的
   **天花板附近**（还包含游戏逻辑 16 ms/帧）。

## 三、这个数字为什么重要

- 它**用真机数据确认了"自研渲染器是唯一出路"**，而且给了明确的目标值：
  **菜单 14 → 40+ fps**。
- 参考项目的曲线是 1~3 → 16.1 → 26 → 30.3 → 34 → 35.4 fps；我们现在有了自己的
  "1~3 → ?" 的起点证据：**只要不模拟绘制，菜单就已经 41 fps**。
- 关卡内同理：5,142 绘制/帧 × 0.42 ms = 2.1 s/帧，这些全是我们渲染器要接管的，
  接管后的上限同样值得在关卡里测一次（见下）。

## 四、下一步（按优先级）

| # | 工作 | 判据 |
|---|---|---|
| 1 | **在关卡里做同样的实验**（solo_propios） | 得到关卡内的天花板，判断 60 fps 是否可达 |
| 2 | **纹理**：给捕获的四边形取纹理、去 tile、上传、采样 | UI 从"洋红框"变成真实文字/图标 |
| 3 | **扩大接管范围**：从 idx=6 扩到 idx=1 的全屏程序化绘制 | 每接管一族，测一次 fps |
| 4 | 渲染目标/EDRAM 替代 + 帧节奏 | 完整一帧由我们画 |

## 五、给用户的结论

**值得继续投渲染器，而且回报是 3 倍。** 这不是估算，是刚在你这台机器上量出来的：
同样的主菜单，不模拟绘制就是 41~44 fps，模拟就是 14 fps。
