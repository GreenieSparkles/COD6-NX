# TAG · 新会话交接（COD6 → Switch 移植）

<!-- AUTHOR-BANNER -->
> # 🚫⛔ 致 Telegram「DarkNSP」搬运者的一封公开信 ⛔🚫
>
> 我知道你在看。你可能已经点开这个页面，扫了一眼，觉得「又是那个做半成品的人」，然后顺手把它和几个 G 的游戏文件塞进一个压缩包，配上三步「安装教程」——**第一步，把根目录复制到卡上；第二步，安装 NSP 前端；第三步，玩**——就发进群里了。
> **一句 bug 都没提。** 没提贴图会烂，没提帧数会掉到个位数，没提它根本还没做完。
>
> 我想先问你一句：**发布之前，你自己跑过一遍吗？**
>
> 哪怕只跑十分钟——进到第二个关卡，低头看一眼脚下的地面。你会看见贴图在你眼前一点一点地烂掉：**墙上长出人脸，桌子变成电视，弹匣变成砖头**，枪口焰炸成一整块巨大的方块。你会看见画面一黑一亮，像一盏坏了很久的日光灯。至于帧数——**那个「三十五到六十」是我在特别行动「坑道（The Pit）」那种小场景里测出来的；一到真正有人物、有战斗的关卡，帧数直接掉到个位数**，卡到你连准星都推不动。
>
> 你看见了，你还会发吗？
>
> 我想你不会。因为**你连一次都没跑过**。你只是看见一个 GitHub 链接，看见「COD6 移植 Switch」这七个字，就把它当成了流量。你没有下载过这个项目，没有在自己机器上试过哪怕一关，没有读过 README 里那句被我加粗了三遍的「这是半成品」。你甚至不知道它跑到第几帧会崩。
>
> 可我知道。**三百一十四轮实机测试，全在这儿。** 我知道它在选枪台会烂，知道光照只有角色两米以内是亮的，知道枪焰会变成一块砖头。我比任何人都清楚它现在有多糟——因为我就是那个盯着它烂了三百多个小时的人。
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
> Ten minutes would have been enough — load the second mission, look down at the ground under your feet. You would watch the textures rot in front of you: **faces growing out of the walls, tables turning into televisions, magazines turning into bricks**, the muzzle flash exploding into one giant solid block. You would watch the screen flip between black and bright like a dying fluorescent tube. And the frame rate? **That “thirty-five to sixty” was measured in a tiny Special Ops map, “The Pit”. In any real level with characters and combat it drops to single digits** — so slow that you cannot even push the crosshair around.
>
> If you had seen that, would you still have posted it?
>
> I do not think so. Because **you never ran it once**. You saw a GitHub link, you saw “COD6 ported to Switch”, and you treated it as traffic. You never downloaded this project, never played a single mission on your own console, never read the line in the README that I had to bold three times: *this is a work in progress*. You do not even know at which frame it crashes.
>
> I do. **Three hundred and fourteen recorded test runs, all of them documented here.** I know it falls apart in the loadout screen, I know the lighting only reaches two metres around the player, I know the muzzle flash turns into a brick. Nobody on this planet knows how bad it currently is better than I do — because I am the one who has been staring at it rot for over three hundred hours.
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


> **读这一份就够了。** 它规定了：拉日志、存备份、出包、验收的全部固定流程，以及当前状态和下一步。

---

## 0. 一句话现状（2026-10-31 收盘）

自研 PM4 解析器**已完成并验证**；**帧率零提升**（13~15 fps）。
下一步只有一件事：写 `BeginPass` + 连发绘制，一次做完。

---

## 1. 固定路径

| 用途 | 路径 |
|---|---|
| 交付目录（用户拷到卡上的文件） | `D:\主机掌机\NSLITE\NSMW2\测试\当前\switch\cod6-nx\` |
| **日志归档**（每轮一个文件夹，纯阿拉伯数字） | `D:\主机掌机\NSLITE\NSMW2\SD测试包\每次日志都在这\<轮次>\logs\` |
| 文档归档 | `D:\主机掌机\NSLITE\NSMW2\测试\当前\*.md` |
| 工程（WSL） | `/opt/switch/cod6-ng`（构建：`ninja -C out/nx cod6`） |
| COD5 参考源码（已拷入工程，不参与编译） | `/opt/switch/cod6-ng/src/native/referencia/` |
| COD4 参考 NRO（用户机器上） | `D:\Downloads\codmw.nro` |
| 构建脚本 | `D:\devkitPro\tools\buildNNN.sh`（每轮一个） |

---

## 2. 拉日志（固定流程，不要问用户）

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File 'D:\devkitPro\tools\mtp_pull_logs.ps1' -Destino 'D:\devkitPro\tmp\mtpNNN'
```

然后归档到 `SD测试包\每次日志都在这\<轮次>\logs\`（`cod6_lastrun.log` + `rex\` 四个文件）。

**注意**：
- MTP 只读，**绝不写卡**（nro 一律用户线缆复制）；
- MTP 报的文件大小恒为 0，不能用来判断是否写完；
- 若提示 `no se encontro el dispositivo Switch` → 主机休眠或 USB 断开，让用户重连后 **retry**；
- 判断这一轮是否真的跑了新包：先看 `rex_stderr.log` 里的 `VERSION 2026-10-31-NNN`；
  **版本不对 = 用户还没跑，别分析旧日志**。

---

## 3. 出包规则（用户反复强调的硬要求）

1. **每读完一次日志，必须交付一个新包**（放交付目录，报 md5 + 版本号）；
2. 每轮同时给出：`cod6.nro` 的 md5、大小、版本号；`cod6.toml` 是否要换；
3. `cod6.toml` 当前内容：13 个正常键 + `cod6_renderizador = "nativo"`；
4. **默认路径（`xenos`）永不破坏**；
5. **热路径里只许整数运算**（不许日志、不许字符串格式化）——违反者必黑屏（v128 教训）；
6. **任何实验路径必须自带"状态不对就落回模拟层"的保护**——这样黑屏不会发生，也就不需要回退包；
7. 黑屏时**只回退一次**，不额外发第三个包（v136~v140 教���：堆包没用）。

---

## 4. 当前可用包

| 版本 | md5 | 说明 |
|---|---|---|
| **v138** | `ab57f43d6cdbe255c984f349e2e2aaaa` | 画面正常，13~15 fps（**推荐保底**） |
| v140 | `bcf2e0352cce8bf4fd42931581c79d1c` | 同上 + 渲染目标模块骨架（未启用） |

---

## 5. 已验证的事实（别再重复测量）

| 事实 | 数据 | 来源轮次 |
|---|---|---|
| 自研解析器正确 | `self-test OK (paquetes=6 … registros=3 dwords=17)` | 202 |
| 自研环读取真实数据 | `paquetes=880000 dibujos=144369 shaders=61155 registros=6466390` | 207 |
| 菜单着色器 | **4 个**：`A4A9` 68%（idx=1）、`3C15` 31%（idx=3）、`C60E`、`39EF` | 208 |
| 每帧绘制 | ≈76 次 3 索引 + ≈35 次 6 索引 | 207 |
| 环几何 | `base=0x1FC98000 bytes=32768` | 111 |
| 缺页 | 12 次/秒，**不是瓶颈** | 210 |
| 绘制位置 | 主环只有寄存器写，绘制在**间接缓冲**（`0x3F`）里 | 123/207 |
| 天花板 | 全跳过模拟绘制 = **38~44 fps**；关卡零绘制 = **14~15 fps** | 177/185 |
| 黑屏根因 | 跳过 `IssueDraw` 会连"进入渲染目标"一起跳过 | v137 |

---

## 6. 下一步（唯一正确的做法）

**三件一起做，一次上真机，不发中间验证包：**

```
src/native/cod6_native_destinos.*   渲染目标缓存（v140 已有 DestinoKey + DestinoCache）
src/native/cod6_native_pass.*       BeginPass：自己开 render pass + barrier + 布局转换
src/native/cod6_native_lote.*       连发：同一管线/同一目标的多条 A4A9 一次发出
                                    保护：状态不一致 → 整条落回模拟层
```

参考：`src/native/referencia/native_renderer.cpp` 里的 `BeginPass`、`GetRenderTargetCanonical`、`FindClearAlias`（约 3800~3900 行）。

**验收**：画面与 v138 一致 **且** fps 上升。二者缺一即失败，回退到 v138。

---

## 7. 用户偏好（务必遵守）

- 不要问"要不要继续"——**直接做，做完报结果**；
- 不要中途反复发包做单点验证；
- 每次读完日志先给新包，再解释；
- 中文汇报，先给结论和数据，再给原因；
- 不要用 zip 做备份；文档用中文，路径用中文；
- 用户会自己拷卡（线缆），不要用 MTP 写。
