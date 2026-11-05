# 变更记录

<!-- AUTHOR-BANNER -->
> **⚠️ 半成品 · 严禁商用 · 严禁盗用**　｜　作者：BILIBILI（B站）**一个没有人看的UP主** UID **122607497**　｜　GitHub **GreenieSparkles**
>
> 本项目是半成品，存在大量已知与未知 bug，不保证可玩。严禁商业用途、严禁冒名转载。
> [致搬运者的公开信](https://github.com/GreenieSparkles/COD6-NX/blob/main/docs/致DarkNSP搬运者的公开信.md)　｜　[仓库](https://github.com/GreenieSparkles/COD6-NX)
<!-- /AUTHOR-BANNER -->


版本号格式：`YYYY-MM-DD-NNN`，唯一来源是
`sdk/src/core/exception_handler_switch.cpp` 里的 `COD6_VERSION`。

只记录**影响行为**的改动。每一条都附上验证依据。

> 🤖 **本项目使用 AI 开发工具：Deepseek-V41-Flash。** 大量排查轮次、补丁与文档由 AI 参与完成。

---

## 2026-11-05-355

这一段是"贴图在运行中崩坏"的排查记录。**结论是没修好**，但把三种猜测用日志排除了，
并把一直缺的那半边数据（guest 侧原始字节）接上了。

### 新增（都在 `cod6.toml` 里开关，默认关闭）
- `[fraw]`：抓 16/200 个绘制时，打印每个纹理槽的 fetch 常量**全部布局字段**（type / tiled /
  dimension / 尺寸 / pitch / format / packed mips / mip 范围 / endianness / swizzle / mip 地址）
  **外加 6 个原始 dword**。原来的 `[dtab]` 只打印"我们解读后"的地址格式尺寸，
  位域解错了日志里天然看不出来。
- `[up]`：每次贴图上传一行（地址、格式、尺寸、层数、dimension、pitch、tiled、packed、
  mipmax、level 数、endianness、内容哈希、所属帧）。`[album]` 同时打印每张图的内容哈希。
- `[alias]`：记录每张池内图的 `(block, offset, size)`，新分配与**活着的**图重叠就报警。
- `cod6_native_null_slots`：把指定 fetch 常量绑成空视图，用来做"A/B 摘掉一个槽位"的实验。
- `cod6_native_force_mip0` 打开时**只建、只上传 level 0**（采样本来就钉在 level 0）。
- 贴图上传的 barrier / copy 改为**记录时**取当前命令缓冲，而不是取内存时快照的那个。
- **接上 guest 侧贴图导出**：`cod6::texdump::Dump()` 一直存在但**从没有任何调用者**，
  `cod6_texdump.cpp` 甚至不在 `COD6_SOURCES` 里 —— 所以 `cod6_texdump_count` 是个空开关，
  我们从来没有 guest 侧的原始字节可对照。现在会写到 `switch/cod6-nx/texdump/tNN.txt`。

### 已排除（都有日志或回读证据，别再重复验证）
- **fetch 常量解码没错**：80 个绘制的每个槽位，用 6 个原始 dword 独立重解，逐字段一致。
- **不是图像对象别名**：同一 `VkImage` 没有被两个地址共用；池内 `(block, offset)` 无重叠；
  把整个显存池关掉（每张图独立分配）后，现象没有任何变化。
- **不是原地重写 / 录制线程 / memexport / mip 链上传**：逐项关掉，现象都在。
- **渲染是确定性的**：两次独立运行的 21 帧逐字节相同 —— 排掉竞态类解释。

### 仍未解决
- 一批贴图在显存里以**对齐小块**（4~36 个 4×4 块）在"真数据 ↔ 全 0"之间翻转，
  而这些贴图的**上传次数 ≤ 1**；改动的块多落在第 0 行。
- 另有整张全黑的贴图（512×256 全 0），以及"纯深红 / 紫底黑条"这类块级损坏。
  这几种形态不像"抓错了图"，更像**同一个块里混进了两次写入的字节**。
- 下一步：对同一地址比较 `texdump/`（guest）与 `cap/`（显存）两份数据，
  确定是"guest 本来就是空的"还是"拷贝没落地"。

---

## 2026-11-03-325

### 新增
- `cod6_native_dxn_ra`：DXN（法线贴图）的通道顺序可切换。
  依据：SDK 的 `cache.h` 明确写「DXN is read as RG in 4D5307E6, but as RA in 415607E6」，
  即该格式**按游戏而异**。默认 `false`（RG）。

### 结论
- 实测 `true`（RA）对画面无改善 → 通道顺序**不是**平表面噪纹的原因，已回退默认。

---

## 2026-11-03-324

### 修复
- **顶点拷贝恢复为「按绘制范围」，但去掉 fetch 声明 size 的截断。**
  上一版把整个绑定区间都缓存，实测每帧拷贝 **61.7 MB**、哈希 **46.2 ms**，
  缓存几乎不命中，导致流缓冲写满回绕、已记录的副本被覆盖（画面更花）。
  现在：`vertex copies 0.1 MB/帧`、`hashing 0.7 ms`。

---

## 2026-11-03-323

### 修复
- **着色器存储初始化时序。** `InitializeShaderStorage` 在渲染器创建之前被调用，
  旧代码 `if (renderer_ready_)` 直接丢弃请求，导致 `cache/shaders/driver/` 从未被
  加载或写入——这就是日志里长期出现 `N shaders (0 cached)` 的原因。
  现在记录请求，渲染器就绪后补做。

### 实测
```
SPIR-V cache: 710 translations (98338 KB) loaded from
  /switch/cod6-nx/cache/shaders/driver/native-41560817-000010DE-00000FE0-06802004.spirv
```

---

## 2026-11-03-322

### 新增
- `cod6_native_memexport_copy` 可用：由 CPU 补做 memexport 写入，
  **拷贝前先比对**几个采样点，字节未变则不写、不失效。
  之前的实现每拷贝一次就 `InvalidateRange` 一次，导致每帧数千贴图重上传、掉到 1 fps。

---

## 2026-11-03-321

### 变更（已被 324 修正）
- 顶点改为整绑定内容寻址缓存。方向正确（对齐 BO2 的 `bo2zm_native_vertex_cache`），
  但导致拷贝量暴涨，见 324。

---

## 2026-11-03-318

### 修复
- **非索引绘制不再套用 `VGT_MIN/MAX_VTX_INDX`。**
  这两个寄存器只对索引绘制有意义；套到 auto-index 绘制上会沿用上一帧索引绘制的值，
  把顶点范围截断，让绘制读到别人的顶点。

---

## 2026-11-03-317

### 修复
- 程序自动创建 `user/`、`cache/`（此前只创建 `update/`）。
  缓存目录不存在时，着色器缓存会**静默失败**。

---

## 2026-11-03-315/316

### 修复
- 删除 `cod6_diag_stubs.cpp` 中与 `cod6_page_writes.cpp` 重复的 cvar 注册。
  `cod6_native_vertex_by_writes` 一直被旧 stub 抢注（日志里那三条
  `duplicate registration`），因此 **toml 里写它从未生效**。

---

## 更早

- `v310`：`force_mip0` 语义修正（真正钳到 mip 0）
- `v308/v309`：停止按年龄驱逐纹理；安全驱逐前校验来源指纹
- `v305/v306`：mip 重建、resolve 旧图复用调查
- `v287`：写入戳跟踪（`cod6_page_writes.cpp`）加入
- `v298`：首次尝试 memexport CPU 拷贝（实现有误，写出界，已回退）

---

## 已排除的方向（避免重复劳动）

| 假设 | 否掉的依据 |
|---|---|
| 纹理内容被游戏改坏 | 45,178 次检查，0 次变化 |
| 显存池块复用导致串味 | 关闭池后现象不变 |
| DXN 通道顺序（RG/RA） | 切换后画面无变化 |
| 顶点范围截断导致平表面噪纹 | 去掉截断后仍存在 |
| 绑定的地址在逐帧变化 | `[dtab]` 逐帧对照，地址稳定 |
| recoder 线程死锁 | `record_thread=false` 更糟 |
| 音频回调阻塞 guest | `audio_mute=true` 现象不变 |
| 第三方 Vulkan 层干扰 | 禁用 OBS/GamePP/Bandicam 后不变 |
