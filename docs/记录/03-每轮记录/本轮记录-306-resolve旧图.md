# 本轮记录 · 第 306 轮（代码级定位：采样优先用了「渲染目标的旧图」）

<!-- AUTHOR-BANNER -->
> **⚠️ 半成品 · 严禁商用 · 严禁盗用**　｜　作者：BILIBILI（B站）**一个没有人看的UP主** UID **122607497**　｜　GitHub **GreenieSparkles**
>
> 本项目是半成品，存在大量已知与未知 bug，不保证可玩。严禁商业用途、严禁冒名转载。
> [致搬运者的公开信](https://github.com/GreenieSparkles/COD6-NX/blob/main/docs/致DarkNSP搬运者的公开信.md)　｜　[仓库](https://github.com/GreenieSparkles/COD6-NX)
<!-- /AUTHOR-BANNER -->


> 上一轮我拿放大后的画面下结论，这不算证据 —— 这一轮全部从代码出发。
> 行号对应 `src/native/nativo/native_textures.cpp`（v305 源码，`备份\源码\v305_src.tar.gz`）。

---

## 一、代码里的事实

### 事实 1：采样路径的第一道分支，按**地址**命中渲染目标，直接返回那张图

`GetViewUncached()`，约 2844 行：

```cpp
// A texture a resolve wrote: the image, not the memory.
auto it = im.resolve_destinations.find(im.SampleKey(base_page << 12));
...
if (it != im.resolve_destinations.end()) {
  ResolveDestination& dest = *it->second;
  uint32_t swizzle = GuestToHostSwizzle(fetch.swizzle, dest.host_swizzle);
  ...
  return v->second;          // 渲染目标的图像
}
```

**键只有 guest 地址**（`base_page << 12`）。这一步在贴图上传/布局计算**之前**，
所以 guest 内存里的内容根本没被看过。

### 事实 2：唯一「内存被写了」的入口，不会失效这些图

`InvalidateRange()`，约 2460 行 —— 已知写者（文件读入、guest 填充、GPU 拷贝）都从这里上报：

```cpp
for (uint64_t key : list->second) {
  ...
  if (base_hit || mip_hit) {
    if (Impl::Checked* c = im.checked.find(key)) *c = Impl::Checked{};   // 内容哈希作废
    auto t = im.textures.find(key);
    if (t != im.textures.end()) t->second->invalidated = true;           // 贴图作废
    any = true;
  }
}
if (any) ++im.answers_epoch;                                            // 绑定/槽位缓存作废
```

全文 grep `resolve_destinations`：只在 `GetResolveDestination()` 里 erase
（格式/尺寸不符时），**没有任何一处因为「guest 内存被写」而失效它**。

### 结论（由事实 1+2 直接推出，不依赖画面）

> 一个地址一旦当过渲染目标，之后贴图流把另一张贴图写进同一块内存，
> 采样**仍然**返回那张渲染目标的图；代码里没有任何路径会复核这个决定。

「哪个物体中招」取决于贴图流复用了哪个曾经是渲染目标的地址 —— 与
「任意物体、随机变化、近处也有、越玩越糟」四个特征全部吻合。

---

## 二、护栏为什么这样写（我否掉了第一版）

### 第一版（已丢弃）：用页写戳判断「之后有没有被写」

```cpp
if (cod6::writes::Enabled() && candidate.stamp_serial &&
    !cod6::writes::g_pages.Unchanged(addr, bytes, candidate.stamp_serial)) { 绕过 }
```

**读 `cod6_page_writes.h` 后发现这是错的**：`Unchanged()` 在
`unknown_ > since` 或 range 非法时直接 `return false`。而实测日志一直是
`[texmon] marcas: aciertos 0 fallos 29500` —— **它几乎恒假**。
拿它当条件等于「所有渲染目标采样全部绕过」，后处理（辉光/模糊/HUD）会立刻崩。

> 这也顺带解释了一个旧谜：写戳系统的 `aciertos 0` 不是「没通电」，
> 是 `Unchanged()` 的保守判据在真实工作负载下恒假。

### 第二版（已交付）：用 fetch 常量本身的语义，构造上不可能误伤

```cpp
const xenos::TextureFormat base_format = rex::graphics::GetBaseFormat(fetch.format);
const bool compressed_fetch =
    base_format == xenos::TextureFormat::k_DXT1  || base_format == k_DXT2_3 ||
    base_format == xenos::TextureFormat::k_DXT4_5 || base_format == k_DXN;
if (compressed_fetch) it = im.resolve_destinations.end();   // 走正常贴图路径
```

**理由**：渲染目标解析出来的是**未压缩**颜色数据。一个请求 DXT1/DXT3/DXT5/DXN 的
fetch，逻辑上不可能是要渲染目标。所以「压缩 fetch 撞上渲染目标地址」只有一种解释：
那块内存已经被贴图复用了。**没有假阳性。**

同时它不会伤害后处理：后处理采样的 fetch 是未压缩格式，照旧走渲染目标图。

---

## 三、这一版怎么判断成败（不看画，看计数）

日志里新增（每 256 次打一行）：

```
[resolve] rancio: 地址 是 resolve 目的地(宽x高)；fetch 要 DXT n 的 宽x高，说明内存已经是贴图（N 次）
```

| 结果 | 含义 | 下一步 |
|---|---|---|
| `N = 0` | 这条路径被洗清：压缩贴图从没撞上渲染目标地址 | 换下一处：见第四节 |
| `N > 0` | 就是它，而且能拿到具体地址与次数 | 让用户复测同一场景，看串图是否消失 |

---

## 四、如果 N=0，下一处要看什么（已在代码里定位到候选）

`GetViewUncached()` 的贴图缓存键 `TextureLayoutKey`（约 280 行）包含
`base_page / mip_page / 宽高深 / pitch / mip_max_level / format / dimension /
endianness / tiled / packed_mips`；

`slot_views[32][8]` 的复用判据是「6 个 fetch dword 全等 + views_epoch + answers_epoch」；

绑定描述符集缓存 `texture_set_cache` 的键是「layout 种子 + 按 `texture_bindings` 顺序的
view id 序列」。

这三处**都是完整键**（`mask_missing=0` 已证绑定键覆盖所有采样槽），剩下唯一没被审计的是
**`GetTextureBindingsAfterTranslation()` 的数组顺序是否严格等于 SPIR-V 里的 descriptor 索引** ——
若不等，同一个绘制内部的贴图会整体错位（把 detail/normal 当 diffuse）。
这条下一轮用「同一个绘制：槽位 vs 描述符索引」的一次性打印来判定。

---

## 五、产物

```
测试\当前\switch\cod6-nx\
  cod6.nro    41,613,509  md5 AEC95622B5ED2DD3B783CA2ED22F0954   2026-11-03-306
  cod6.toml        4,846  md5 E66F79FDA4DF0EE2C3BFFB47A5201EC0
  cod6.nacp       16,384  （hbmenu 里显示 2026-11-03-306）

单变量：这一版 mip 重建关闭、force_mip0 保持 true，画面风格与 v302/v303 一致，
只暴露「resolve 旧图护栏」一个变化。

备份\源码\ v302 v303 v304 v305 各一份（源码，约 445 KB/份）
```
