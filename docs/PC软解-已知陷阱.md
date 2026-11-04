# PC 软解环境 · 已知陷阱（踩过就别再踩）

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


写于 2026-11-04。每一条都是实际踩过的，不是推测。

---

## 1. 多开进程会伪装成「渲染 bug」

**现象**：画面一黑一亮、贴图闪、像渲染坏了。

**原因**：同一台 Xvfb 上跑了**两个 `cod6` 进程**，两个程序抢同一个窗口交替画。
或者跑了**多个 `websockify`** 抢 6080，页面提示「无法连接到服务器」。

**规矩**：每次重启前先杀干净，然后**断言每个进程恰好 1 个**。

```bash
for pat in 'cod6 --game_data_root' x11vnc websockify padbridge 'vpad.py' 'Xvfb :99'; do
  pids=$(pgrep -f "$pat"); [ -n "$pids" ] && kill -9 $pids
done
sleep 3
# 启动后逐个断言
[ "$(pgrep -f 'cod6 --game_data_root' | wc -l)" = "1" ] || echo "有重复！"
```

脚本：`build/clean_restart.sh`（内含断言，输出「全部单实例 ✓」才算成功）。

---

## 2. 呈现模式默认会让画面一半时间全黑

**现象**：亮度在 `0.0097 ↔ 0.0315` 之间跳（3 倍）。

**原因**：lavapipe 默认 `IMMEDIATE` 呈现——提交后立刻显示，不等画面画完。软渲染每帧几百毫秒，于是显示的是没画完的缓冲。

**fix**：启动前设环境变量

```bash
MESA_VK_WSI_PRESENT_MODE=fifo
```

实测：亮度从 `0.0097↔0.0315` 变成稳定 `0.38~0.40`。

---

## 3. Xvfb 不保留窗口内容 → 慢渲染时闪黑

**现象**：只在**关卡里**闪（主菜单不闪），亮一帧暗十几帧。

**原因**：Xvfb 默认没有 backing store，两次渲染之间窗口内容被丢弃。

**fix**：

```bash
Xvfb :99 +bs -screen 0 1280x720x24      # +bs = backing store
```

---

## 4. VNC 用 XDAMAGE 会漏掉 Vulkan 更新

**现象**：浏览器里画面一块一块黑，但本地截图正常。

**原因**：`x11vnc` 默认用 XDAMAGE 追踪变化，Vulkan 交换链的更新它经常漏。

**fix**：

```bash
x11vnc -display :99 -noxdamage ...
```

---

## 5. 键盘进不了游戏（SDK 的 keybind_* 无效）

**事实**：这版 SDK 的 `keybind_*` cvar **没把键盘接到手柄模拟上**（实测无效）。

**链路**：

```
浏览器/虚拟手柄 → /tmp/pad_cmd → 生成代码里 XamInputGetState 的注入点 → 虚拟 360 手柄
```

**两个前端**：

| 前端 | 用途 | 地址 |
|---|---|---|
| `vpad.py` | **鼠标点按**的虚拟手柄（推荐） | http://localhost:6081/ |
| `padbridge.py` | 键盘映射（Enter=START 等） | 配合 noVNC 画面用 |

**格式陷阱**：注入点解析的是 `"%7s %d"`（两列），必须是

```
b 4096        ← 正确（A = 0x1000）
S             ← 错误，sscanf 返回 1，整行丢弃，等于没按键
```

**脉宽**：软解只有 2~5 fps，按键保持时间必须 ≥ 0.6 秒，否则轮询错过。
`padbridge.py` 现在是 **2.0 秒**。

---

## 6. 音频报错可以忽略

```
ALSA lib ... cannot find card '0'
```

WSL 没有声卡，游戏照样跑，和画面无关。

---

## 7. 启动要等 25 秒

软解从启动到出标题画面约 **20~25 秒**。脚本里 `sleep 22` 是这个原因，不是卡死。
