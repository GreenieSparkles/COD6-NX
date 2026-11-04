# PC 软解环境 · 已知陷阱（踩过就别再踩）

<!-- AUTHOR-BANNER -->
> **⚠️ 半成品 · 严禁商用 · 严禁盗用**　｜　作者：BILIBILI（B站）**一个没有人看的UP主** UID **122607497**　｜　GitHub **GreenieSparkles**
>
> 本项目是半成品，存在大量已知与未知 bug，不保证可玩。严禁商业用途、严禁冒名转载。
> [致搬运者的公开信](https://github.com/GreenieSparkles/COD6-NX/blob/main/docs/致DarkNSP搬运者的公开信.md)　｜　[仓库](https://github.com/GreenieSparkles/COD6-NX)
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
