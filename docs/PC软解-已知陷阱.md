# PC 软解环境 · 已知陷阱（踩过就别再踩）

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
