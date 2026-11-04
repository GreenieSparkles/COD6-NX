# 本轮记录 · 第 307 轮（BO2 的方法：给快路径装校验；上版结论修正）

<!-- AUTHOR-BANNER -->
> # 🚫⛔ 致 Telegram「DarkNSP」搬运者的一封公开信 ⛔🚫
>
> 我知道你在看。你可能已经点开这个页面，扫了一眼，觉得「又是那个做半成品的人」，然后顺手把它和几个 G 的游戏文件塞进一个压缩包，配上「整合包已打包、解压即玩」的文案，发进群里。
>
> 我想先问你一句：**发布之前，你自己跑过一遍吗？**
>
> 哪怕只跑十分钟——进到第二个关卡，低头看一眼脚下的地面。你会看见贴图在你眼前一点一点地烂掉：桌面变成显示器的花纹，墙皮变成别人的广告牌，枪口焰炸成一整块巨大的方块。你会看见画面一黑一亮，像一盏坏了很久的日光灯。你会看见帧数在三十五和六十之间乱跳，跳到你根本瞄不准人。
>
> 你看见了，你还会发吗？
>
> 我想你不会。因为**你连一次都没跑过**。你只是看见一个 GitHub 链接，看见「COD6 移植 Switch」这七个字，就把它当成了流量。你没有下载过这个项目，没有在自己机器上试过哪怕一关，没有读过 README 里那句被我加粗了三遍的「这是半成品」。你甚至不知道它跑到第几帧会崩。
>
> 可我知道。**三百一十四轮实机测试，全在这儿。** 我知道它在选枪台会烂，知道光照只有角色两米以内是亮的，知道枪焰会变成一块砖头。我比任何人都清楚它现在有多糟——因为我就是那个盯着它烂了三百多个小时的人。
>
> 而你，把它包装成「可以玩」，发给了一群相信你的人。
>
> **我这两天已经看到太多这样的评论了。** 有人说「下载完进游戏就花屏」，有人说「走两步就崩」，有人说「根本没法玩，白下了六个 G」，也有人客客气气地问我：「作者，这个版本是不是还没做完？」
>
> 他们不是来骂我的，他们是**失望**。他们花几个小时等下载、等解压、等安装，最后拿到一个连菜单都撑不住的版本。他们的时间、他们的硬盘、他们那份期待，被你一次「顺手打包」消耗干净了。
>
> **这不是分享，这是转嫁。** 你把我的半成品，连同我的名声，一起打包送了出去：出了问题，骂的是我；拿到流量的是你。
>
> 我不反对搬运。这个项目放在 GitHub 上就是给所有人看的。你要转，可以——**只转仓库，附上声明，写清楚「半成品、有大量 bug、不保证能玩」**。别把游戏本体塞进去，那是侵权；别写「解压即玩」，那是骗人；别省略那句「不保证可玩」，那是我唯一能替玩家守住的东西。
>
> 如果你连这三件事都不愿意做，那就**别搬**。
>
> 我做这个东西，是因为我想知道一台 2019 年的掌机能不能跑动 2009 年的引擎；想知道 PM4 命令环能不能被重新读懂；想知道重编译出来的 ARM64 代码能不能把几十万面的场景推上屏幕。我不是为了流量，也清楚它现在拿不出手。
>
> 所以请你，别拿它去骗人。
>
> —— 一个没有人看的UP主　BILIBILI UID 122607497　2026-11
>
> ---
>
> # 🚫⛔ An Open Letter to the Repackers in the Telegram Group “DarkNSP” ⛔🚫
>
> I know you are reading this. Maybe you opened this page, skimmed it, thought “oh, that unfinished project again”, then dropped it into an archive together with a few gigabytes of game files, added the line “all-in-one pack, just extract and play”, and pushed it into your group.
>
> So let me ask you one thing first: **did you even run it once before you shipped it?**
>
> Ten minutes would have been enough — load the second mission, look down at the ground under your feet. You would watch the textures rot in front of you: a table turning into a monitor panel, a wall turning into somebody else’s billboard, the muzzle flash exploding into one giant solid block. You would watch the screen flip between black and bright like a dying fluorescent tube. You would watch the frame rate thrash between thirty-five and sixty until you cannot aim at anything.
>
> If you had seen that, would you still have posted it?
>
> I do not think so. Because **you never ran it once**. You saw a GitHub link, you saw “COD6 ported to Switch”, and you treated it as traffic. You never downloaded this project, never played a single mission on your own console, never read the line in the README that I had to bold three times: *this is a work in progress*. You do not even know at which frame it crashes.
>
> I do. **Three hundred and fourteen recorded test runs, all of them documented here.** I know it falls apart in the loadout screen, I know the lighting only reaches two metres around the player, I know the muzzle flash turns into a brick. Nobody on this planet knows how bad it currently is better than I do — because I am the one who has been staring at it rot for over three hundred hours.
>
> And you wrapped it up as “playable” and handed it to people who trusted you.
>
> **In the last two days I have seen far too many of these comments.** “Textures are corrupted as soon as the game starts.” “It crashed after two steps.” “Completely unplayable, six gigabytes wasted.” And, politely: “Author, is this version not finished yet?”
>
> They are not angry at me. They are **disappointed**. They spent hours waiting for the download, waiting for the extraction, waiting for the install, and ended up with something that cannot even hold a menu together. Their time, their disk space and their expectations were burned by your one careless repack.
>
> **This is not sharing. This is shifting the cost onto someone else.** You packed up my unfinished build — and my reputation — and shipped it out. When it breaks, I am the one who gets blamed, and you are the one who gets the attention.
>
> I am not against sharing. This project is on GitHub for everyone to read. Share it if you want — **share the repository, attach this notice, and state plainly: work in progress, many bugs, no guarantee that it is playable.** Do not bundle the game data in, that is infringement. Do not write “just extract and play”, that is a lie. Do not drop the “no guarantee that it is playable” line, because that sentence is the only thing I can still hold on to on behalf of the players.
>
> If you are not willing to do even those three things, then **do not share it at all**.
>
> I built this because I wanted to know whether a handheld from 2019 can run an engine from 2009; whether the PM4 command ring can be understood again; whether recompiled ARM64 code can push a scene of hundreds of thousands of triangles onto that screen. I am not doing it for traffic, and I know perfectly well that it is not presentable yet.
>
> So please — do not use it to deceive people.
>
> — 一个没有人看的UP主　BILIBILI UID 122607497　November 2026
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


> 本轮全部依据卡上文件：`SD测试包\每次日志都在这\307-v306resolve\`（日志 + 44 个捕获文件）。

---

## 一、先修正我上一版的结论（重要）

v306 的「resolve 旧图护栏」在实机 **一次都没触发**：

```
[resolve] rancio: ...        ← 全文 0 行
```

所以：

| 我说过的 | 实际情况 |
|---|---|
| resolve 旧图复用是串图的原因 | **未证实，且实测为 0 次，等于洗清这条路径** |
| 用户感觉「比之前好一点」 | **不能算我修好的**（该改动没有生效） |

护栏代码保留（构造上不会误伤：渲染目标不可能被以 DXT 格式采样），但它不是原因。

---

## 二、上一版的绘制表抓错了地方

解析 v306 的 2500 行 `[dtab]`：

```
draws parsed: 2500
distinct pixel shaders: 2

ps                  draws   vertices       vp  slot0 addresses
0000000000000000     2376     950739  512x512  0 distinct      ← 无像素着色器
38456BFA91A51EAB      124     152418  512x512  12 distinct
```

**2376 个绘制 `ps=0`、视口 512×512 —— 全是阴影 pass**，颜色 pass 只抓到最后 124 个。
所以 v307 加了 `cod6_debug_draw_table_skip = 2200`，跳过阴影再抓 2000 个。

（顺带：这一帧真正被采样的贴图地址只有 12 个，格式 18/19/20 = DXT1/3/5。）

---

## 三、BO2 到底哪三条能照抄 —— 以及我逐条核对了我们的代码

BO2 的 NRO 里自己的运行期诊断把规矩写死了：

```
[native] C6 texture words reuse: {} reuses checked against the sampler loop,
   0 differences: APPLYING (1 in 1,024 still checked)
[native] C6 texture words reuse MISMATCH after {} checks: word {} is {:08X}, the
   reuse had {:08X}: reuse off for this session
[native] C3 texture rechecks by write stamps ... {} changed without a stamp,
   their pages now count as always written
[native] C6 framebuffers: MISMATCH when destroying view {:016X}: {}. For the rest
   of the session the affected framebuffers are retired without destroying them
```

| BO2 的机制 | 我们的代码 | 本轮动作 |
|---|---|---|
| **每条快路径抽样复核（1/1024），第一次不符就永久关** | `slot_views[32][8]` 用上次绘制的答案回答这次，**零校验、无自关** | **v307 已实现**（见第四节） |
| **写戳自愈**：变了却没戳 → 那些页从此算「一直脏」 | 写戳 `Unchanged()` 在真实负载下恒假（`aciertos 0`），等于没有 | 待办：换成按地址的「脏代」判据 |
| **退役不销毁**：销毁 view 时若还有引用，就不销毁 | **已经做对了**：`retired` 队列只在 `r.frame <= completed_frame` 之后才真正销毁（约 1855 行） | 这条假设排除 |

---

## 四、v307 的新东西：`[texcheck]`

每 1024 次槽位缓存命中，用同一组 fetch 字**走慢路径重算**并比对：

```cpp
if (!texcheck_disabled && (slot_view_hits & 1023) == 0) {
  VkImageView fresh = textures.GetView(regs->GetTextureFetch(slot), dimension, is_signed, upload);
  if (fresh != e.view) {
    REXLOG_ERROR("[texcheck] MISS: slot {} cached {} but the fetch words give {} "
                 "(base {:08X} fmt {} {}x{} dim {} words {:08X} {:08X} {:08X}) - "
                 "slot view reuse OFF for this session", ...);
    texcheck_disabled = true;   // 照 BO2 的 "reuse off for this session"
  }
}
```

**这直接回答你的问题**：绘制拿到的贴图，和 fetch 常量描述的是不是同一张。

- `[texcheck] MISS` 出现 → 就是它，槽位、fetch 字、地址、格式尺寸全在日志里
- 一直不出现 → 这条快路径洗清，样本量（每 1024 次一次）也能说明查过

---

## 五、产物

```
测试\当前\switch\cod6-nx\
  cod6.nro    41,613,509  md5 CB119C9BEFA1547681D7E058392D087D   2026-11-03-307
  cod6.toml        4,316  md5 92A409020032044B04C094E997B104CB
```

v307 仍是**单变量**：渲染设置与 v302/v303 完全一致（mip 重建关、force_mip0 开），
只多两件观测：`[texcheck]` 与「跳过阴影后的绘制表」。

卡上的 `cap` 目录会累积（v303 的帧和图还在里面），下次抓完我会先清空再跑，
否则分不清哪张是新一版的。
