# 参与贡献

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
