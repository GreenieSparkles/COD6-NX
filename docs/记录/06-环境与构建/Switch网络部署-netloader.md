# Switch 网络部署（netloader / nxlink）—— 迭代从"插卡"变"秒级"

<!-- AUTHOR-BANNER -->
> # ⚠️ 半成品 · 严禁商用 · 严禁盗用
> # ⚠️ WORK IN PROGRESS · NO COMMERCIAL USE · DO NOT REUPLOAD AS YOUR OWN
>
> **作者 / Author：** BILIBILI（B站）**一个没有人看的UP主**　UID **122607497**
> ｜ GitHub **GreenieSparkles**
>
> - **这是半成品 / This is a work in progress.** 大量已知与未知 bug，参数和实现随时会变，不保证能通关、能玩完。
>   Expect plenty of known and unknown bugs; parameters and implementation change without notice. No guarantee that it is playable to the end.
> - **严禁商业用途 / NO COMMERCIAL USE.** 不得售卖、不得用于付费内容、不得捆绑进任何收费产品或服务、不得用于引流变现。
>   May not be sold, paywalled, bundled into any paid product or service, or used for monetised traffic.
> - **严禁盗用与冒名 / DO NOT REUPLOAD AS YOUR OWN.** 转载、搬运、二次打包必须保留本署名与声明，并注明来源仓库。
>   Any repost, mirror or repack must keep this attribution and notice, and must link back to the source repository.
>
> ---
>
> ### 致 Telegram「DarkNSP」群的搬运者
>
> 看到这个 GitHub 后，请**不要再打包包含游戏资源的整合包**。
>
> 这里只是**半成品**，bug 一大堆还没修。把游戏本体塞进去一起发，既**侵权**，也是在**欺骗玩家和粉丝**——他们花时间下载好几个 GB，拿到的却是一个走两步就花屏、随时崩的版本，**白费功夫下个不能玩的版本，图什么？**
>
> 这样做**不负责任**。要搬运，就只搬**仓库本身和这份声明**，并且必须写清楚：**半成品、有大量 bug、不保证能玩**。
>
> ### To the repackers in the Telegram group “DarkNSP”
>
> If you see this GitHub repository, **stop repacking it together with the game files**.
>
> This is an **unfinished build** with a pile of bugs still unfixed. Bundling the game data with it is **copyright infringement**, and it is also **deceiving players and fans** — they spend hours downloading several gigabytes only to get a build that corrupts its textures after a few steps and crashes at random. **Why waste people's time with a version that cannot be played?**
>
> Doing that is **irresponsible**. If you must share it, share **the repository and this notice only**, and state it plainly: **work in progress, many bugs, no guarantee that it is playable**.
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
