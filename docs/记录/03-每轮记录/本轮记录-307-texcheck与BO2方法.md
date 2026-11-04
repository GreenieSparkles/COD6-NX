# 本轮记录 · 第 307 轮（BO2 的方法：给快路径装校验；上版结论修正）

<!-- AUTHOR-BANNER -->
> **⚠️ 半成品 · 严禁商用 · 严禁盗用**　｜　作者：BILIBILI（B站）**一个没有人看的UP主** UID **122607497**　｜　GitHub **GreenieSparkles**
>
> 本项目是半成品，存在大量已知与未知 bug，不保证可玩。严禁商业用途、严禁冒名转载。
> [致搬运者的公开信](https://github.com/GreenieSparkles/COD6-NX/blob/main/docs/致DarkNSP搬运者的公开信.md)　｜　[仓库](https://github.com/GreenieSparkles/COD6-NX)
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
