# Switch 网络部署（netloader / nxlink）—— 迭代从"插卡"变"秒级"

<!-- AUTHOR-BANNER -->
> **⚠️ 半成品 · 严禁商用 · 严禁盗用**　｜　作者：BILIBILI（B站）**一个没有人看的UP主** UID **122607497**　｜　GitHub **GreenieSparkles**
>
> 本项目是半成品，存在大量已知与未知 bug，不保证可玩。严禁商业用途、严禁冒名转载。
> [致搬运者的公开信](https://github.com/GreenieSparkles/COD6-NX/blob/main/docs/致DarkNSP搬运者的公开信.md)　｜　[仓库](https://github.com/GreenieSparkles/COD6-NX)
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
