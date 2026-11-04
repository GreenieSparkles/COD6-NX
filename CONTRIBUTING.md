# 参与贡献

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


感谢愿意帮忙。这个项目处于早期阶段，**渲染问题是主要工作**，
所以最有价值的贡献是「一份能复现的问题报告」。

---

## 提 Issue 之前

1. 先看 [README 的已知问题](../README.md#已知问题) 和 [CHANGELOG](../CHANGELOG.md)，
   也许已经记录过了
2. 确认用的是**最新版本**（日志里的 `[cod6] VERSION` 行）

---

## Bug 报告需要包含

### 必需

| 项 | 说明 |
|---|---|
| `logs/rex/rex_stderr.log` | 完整日志，不要截断 |
| `logs/rex/rex_perfil.log` | 帧时间与时钟信息 |
| `cod6.toml` | 你实际使用的配置 |
| 截图 | 出问题的画面；**带 HUD 的截图**能帮助定位场景 |
| 版本行 | 日志里的 `[cod6] VERSION 2026-…` |

### 如果能提供，更好

- 出问题的**位置**（哪一关、哪个界面）
- 用 `cod6_debug_draw_table` + `cod6_debug_tex_album` 导出的贴图与 `[dtab]` 行
- 现象是**稳定复现**还是**随时间变差**

---

## 抓日志

日志在 `sdmc:/switch/cod6-nx/logs/rex/`：

```
rex_stderr.log    主日志
rex_perfil.log    帧率/时钟曲线
```

想拿到更详细的日志：

```toml
log_level = "info"     # 默认 "error"，只记错误
cod6_sin_logs = false
```

---

## 抓贴图（定位「这个面为什么花」）

```toml
cod6_debug_capture_dir   = "sdmc:/switch/cod6-nx/cap"
cod6_debug_capture_every = 300     # 抓取点间隔（帧）
cod6_debug_album_skip    = 4       # 跳过前 4 个点 → 从第 1500 帧开始
cod6_debug_tex_album     = 128     # 每帧最多导出 128 张贴图
cod6_debug_draw_table    = 2       # 同时记录这两帧的每个绘制
```

然后在出问题的画面前**停留约 10 秒**，退出。`cap/` 里会出现：

```
frame_001500.rgbx          画面（8 字节头 + R8G8B8X8，宽高在头里）
tex_f001500_0B434000.img   贴图（16 字节头 = 宽, 高, VkFormat, 每像素字节数）
```

日志里的 `[dtab]` 行会列出「绘制 → 绑定地址 → 尺寸」，
地址与 `tex_*.img` 文件名里的地址对应。

---

## 代码贡献

- **一个 PR 一件事**，附上「为什么这么改」的依据（日志、贴图、实测数字）
- 渲染相关改动请附**修改前后**的日志数字，例如：
  ```
  修改前: vertex copies 61.7 MB, hashing 46.2 ms
  修改后: vertex copies  0.1 MB, hashing  0.7 ms
  ```
- 不要提交 `generated/`、`game_root/`、`out/` 或任何游戏数据
- 每个源文件保留 SPDX 头（新文件用 `GPL-3.0-or-later`）

### 提交信息

```
<范围>: <做了什么>

<为什么：现象 + 依据>
```

例：

```
native_textures: stop cropping vertex copies to the fetch size

The fetch constant's size field is only a bounds hint; the game sets it
smaller than the data the shader reads, so the tail of the range was read
out of the stream buffer. Measured: confetti on flat surfaces disappears
when the clamp is removed.
```

---

## 不接受的贡献

- 任何游戏数据（`.ff` / `.bik` / `game_root` / 提取物）
- 绕过授权的二进制分发
- 只有「好像变好了」而没有日志/贴图佐证的渲染改动

---

## 授权

提交的代码默认以 **GPL-3.0** 授权（见 [LICENSE](../LICENSE)）。
