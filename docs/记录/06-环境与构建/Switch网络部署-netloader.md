# Switch 网络部署（netloader / nxlink）—— 迭代从"插卡"变"秒级"

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


写于第 157 轮，已实测通。

## 一、前置条件

1. Switch 和这台电脑在同一个网段（用户给的地址：`192.168.31.160`）；
2. Switch 上**hbmenu 正在运行且 netloader 在监听**（端口 28280）；
   - 建议用 **title override**（按住 R 点任意游戏图标）进 hbmenu，
     这样是完整内存；从相册（applet 模式）进可能内存不够、跑不动 3 GB 客机内存。
3. 注意：**一旦有一次连接失败，hbmenu 的 netloader 可能不再监听**，需要重新进一次 hbmenu。

## 二、命令（实测通过）

```bash
wsl -d ubuntu24 -- bash /mnt/d/devkitPro/tools/switch_push.sh
# 或者直接：
/opt/devkitpro/tools/bin/nxlink -a 192.168.31.160 -p cod6-nx/cod6.nro out/nx/cod6.nro
```

`-p cod6-nx/cod6.nro` 是关键：文件会被放到 `sdmc:/switch/cod6-nx/`，
和插卡安装的路径**完全一样**，所以：

- `game_root` / `user` / `cache` / `logs` 的路径都不用改；
- 日志仍然写到卡上的 `switch/cod6-nx/logs/`，取日志的方式和以前一样。

实测输出：

```
Sending cod6-nx/cod6.nro, 39815365 bytes
... sent (...%), N blocks
(codigo 0)
```

## 三、现在的迭代流程

```
我改代码 → ninja -C out/nx cod6   (~22 秒)
        → nxlink 推送             (~10 秒，40 MB 走 WiFi)
        → Switch 上自动启动
        → 用户看画面 / 我读日志
```

**不用再插卡、不用再拷文件。** 一次迭代从"几分钟 + 用户操作"变成"半分钟、全自动"。

## 四、下一步（可选，去掉最后的插卡依赖）

日志目前还写在卡上，需要用户拷出来。可以给 app 加一个"日志走网络"的小钩子：
在 Switch 上开一个 UDP socket，把 `[perf]` 行直接发到电脑的 IP。
这样构建 → 推送 → 运行 → 读日志全自动，渲染器开发可以完全由我自己闭环。

## 五、注意

- 上传的 NRO 版本必须和你说的一致：当前 `out/nx/cod6.nro` = **版本 62**，
  md5 `2dba206dd04c7eca3893997eda3b184c`（和卡上那份一样）。
- netloader 上传只覆盖 NRO；`cod6.toml` 用卡上现有的（v61：`readback_memexport=false`、
  `async_skip=false`）。
