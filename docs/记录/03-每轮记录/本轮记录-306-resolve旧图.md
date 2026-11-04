# 本轮记录 · 第 306 轮（代码级定位：采样优先用了「渲染目标的旧图」）

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
