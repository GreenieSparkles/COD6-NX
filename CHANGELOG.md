# 变更记录

<!-- AUTHOR-BANNER -->
> # ⚠️ 半成品 · 严禁商用 · 严禁盗用
>
> **作者：一个没有人看的UP主**　｜　**B站 UID 122607497**　｜　**GitHub：GreenieSparkles**
>
> - **这是半成品。** 大量已知与未知 bug，参数和实现随时会变，不保证能通关、能玩完。
> - **严禁商业用途。** 不得售卖、不得用于付费内容、不得捆绑进任何收费产品或服务、不得用于引流变现。
> - **严禁盗用与冒名。** 转载 / 搬运 / 二次打包**必须保留本署名与声明**，并注明来源仓库。
> - **致 Telegram「DarkNSP」群的搬运者：** 看到这个 GitHub 后，请**不要再打包包含游戏资源的整合包**。
>   这里只是半成品，bug 一大堆还没修；把游戏本体塞进去一起发，既侵权，也会让玩家误以为是成品。要搬运请只搬仓库本身和这份声明。
>
> 仓库：https://github.com/GreenieSparkles/COD6-NX
<!-- /AUTHOR-BANNER -->


版本号格式：`YYYY-MM-DD-NNN`，唯一来源是
`sdk/src/core/exception_handler_switch.cpp` 里的 `COD6_VERSION`。

只记录**影响行为**的改动。每一条都附上验证依据。

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
