# 本轮记录 · 第 307 轮（BO2 的方法：给快路径装校验；上版结论修正）

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
