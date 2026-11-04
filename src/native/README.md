# COD6 原生后端（自写渲染器）—— 骨架与移植计划

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


**状态：阶段 0（骨架）。** 这个目录现在只有一个空壳：`cod6_renderizador = "nativo"` 时会被创建、
打日志，然后**照旧走模拟层的命令处理**（零风险）。真正的渲染器从 COD5 移植进来。

---

## 一、为什么单独建目录

这几轮的血泪教训（v96 / v99 / v100 / v102 全部黑屏卡死）都来自同一件事：
**在正在工作的代码旁边加东西**。

所以规则定死：

| 规则 | 原因 |
|---|---|
| 新后端**绝不修改**模拟层路径 | 出问题只需把 toml 改回 `"xenos"` |
| 每次只搬一个模块，搬完立刻真机验证 | 定位快，回退便宜 |
| 任何"探针"都放在这个目录里 | 探针崩了也不能影响默认路径 |
| 先用 `"nativo"` 跑通启动，再谈画面 | 先证明挂钩成功 |

---

## 二、要移植什么（COD5 → 这里）

| COD5 文件 | 行数 | 目标文件 | 说明 |
|---|---|---|---|
| `cod5_native_system.cpp` | 2,124 | `cod6_native_system.cpp`（本目录） | 读 PM4 环 + 分发 LoadShader/Draw/Resolve/Swap |
| `native_renderer.cpp` | 7,340 | `cod6_native_renderer.cpp`（本目录） | 管线 / 渲染目标 / 绘制 / resolve / swap |
| `native_textures.cpp` | 2,992 | `cod6_native_textures.cpp` | 纹理去 tile / 缓存 / 淘汰 |
| `texture_pool.cpp` | 408 | `cod6_texture_pool.cpp` | 纹理池 |
| `native_geometry_shaders.cpp` | 1,124 | `cod6_native_geometry.cpp` | 程序化 quad / 点精灵展开（对应我们的 `A4A9`） |
| `native_nvk_draw.cpp` | 272 | `cod6_nvk_draw.cpp` | 打了补丁的 Mesa 快速路径 |
| `native_batch_recorder.h` 等小件 | ~800 | `cod6_native_cache.*` | 每绘制开销优化 |

COD5 源码在：`D:\主机掌机\NSLITE\NSMW2\参考资料-COD5-SP-NX-源码\app\src\native\`

---

## 三、阶段与验收

| 阶段 | 做什么 | 验收 |
|---|---|---|
| **0（已完成）** | 骨架 + 开关 + 打日志 | `"nativo"` 能启动，画面与 `"xenos"` 一致 |
| 1 | 搬 Reader（只读环、统计包数） | 包数 ≈ SDK 的统计；**画面不变** |
| 2 | 搬 LoadShader + 管线缓存 | 着色器能找到、管线能建；仍不画 |
| 3 | 搬 Draw（先 UI 小 quad） | 与模拟层截图逐像素对比 |
| 4 | 搬渲染目标 + resolve | 菜单 fps 上升 |
| 5 | 搬 NVK 桥接 + 缓存模块 + ZCULL | 逼近 30~40 fps |

---

## 四、我们已有的数据（做决定用）

| 事实 | 数值 | 来源 |
|---|---|---|
| 菜单 fps（模拟层） | 13~14 | 多轮 |
| 菜单 fps（跳过全部绘制） | 38~44 | 177 号 |
| 关卡 fps（零绘制） | 14~15 | 185 号 |
| 每帧绘制 | 73~74 | 190/192 号 |
| 每帧寄存器写包 | ~325 | 192 号（MAPA98） |
| 每帧切着色器 | ~25 | 192 号 |
| 每帧等 GPU | ~8.5 | 192 号 |
| 菜单像素着色器 | 4 个 | 190 号 |

---

## 五、当前实现说明

- `Cod6NativeGraphicsSystem` 继承 `Cod6GraphicsSystem`，`CreateCommandProcessor()`
  直接调用基类实现（= 现有 `Cod6NativoCommandProcessor`）；
- `NativeRenderer` 只是签名骨架（`Initialize/LoadShader/Draw/Resolve/Swap/GuestMemorySynchronized`），
  **没有被任何地方调用**，计数器全为 0；
- 开关：`cod6.toml` 里 `cod6_renderizador = "nativo"`（默认 `"xenos"`）。
