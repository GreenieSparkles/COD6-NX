# COD5 源码对照：我们要的"完整渲染器"长什么样

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


分析对象：`D:\主机掌机\NSLITE\COD5-SP-NX-源码分享版-e2e7dba.zip`
（来源分支 `feature/nfsmw-nx-port`，**和我们同一个 SDK fork**，日期 2026-11-01）
已解压到 `/opt/switch/cod5-ref`（WSL）。

---

## 一、它的架构（关键发现）

```
NativeGraphicsSystem : public rex::system::IGraphicsSystem   ← 他们替换掉的是"图形系统"
  ├─ Reader（自己读 PM4 环）
  │    └─ 分发：LoadShader / Draw / Resolve / Swap
  ├─ Renderer（native_renderer.cpp，7,340 行）← 真正干活
  │    ├─ 着色器翻译 + 磁盘 SPIR-V 缓存 + 后台线程
  │    ├─ 管线缓存 + 预热 + 后台线程 + 驱动 PipelineCache
  │    ├─ 渲染目标缓存（clear-alias = resolve 不拷贝）
  │    ├─ 每绘制：BuildDrawSetup + Draw
  │    ├─ Resolve / ResolveDownsampled / FlushResolveBarrier
  │    └─ Swap（presenter + frontbuffer）
  ├─ native_textures.cpp（2,992 行）+ texture_pool.cpp（408）← 自己的纹理管线
  ├─ native_geometry_shaders.cpp（1,124 行）← 几何着色器替代（quad/point sprite 展开）
  └─ native_nvk_draw.cpp ← 薄封装，用打了补丁的 Mesa 快速路径（nvk_switch_draw / set4）
```

**要点：他们不是在 Xenia 的命令处理器旁边"寄生"，而是整条替换**——自己读环、自己管状态、自己发绘制。
这正是我们实测出来的唯一可行路线（全跳过模拟绘制 → 菜单 38~44 fps）。

## 二、模块对照表（COD5 → 我们）

| COD5 模块 | 行数 | 作用 | 我们的现状 | 可复用性 |
|---|---|---|---|---|
| `cod5_native_system.cpp` 的 Reader | ~2,100 | 读 PM4 环、分发 LoadShader/Draw/Resolve/Swap | ❌ 0（我们靠 SDK 的 CP） | **高**：Xenos 包格式与 COD6 相同，机械移植 |
| `native_renderer.cpp` | 7,340 | 管线/RT/绘制/resolve/swap | ❌ 0 | **中高**：结构可照搬，着色器与状态假设要按 COD6 改 |
| `native_textures.cpp` + `texture_pool.cpp` | 3,400 | 纹理去 tile/缓存/淘汰 | 🟡 借用 SDK 的 texture cache | **中**：我们已证明能拿到 `VkImageView`，但他们这套更适合接管 |
| `native_geometry_shaders.cpp` | 1,124 | quad/point sprite 展开 | ❌ 0 | **中**：COD6 的 UI 大量用程序化全屏三角（`A4A9`），这块直接相关 |
| `native_nvk_draw.cpp` | 272 | 用 Mesa 的快速路径（NFS 补丁） | ❌ 0 | **高**：配合我们已经有的 NVK，直接调 |
| 各种缓存头（batch/binding memo/descriptor pool/resolve tracker） | ~800 | 每绘制开销优化 | ❌ 0 | **高**：都是独立小模块，逐个搬 |
| `docs/` 80 份中文复盘 | — | 决策依据与验收门槛 | — | **高**：避免重踩 |

## 三、我们和他们差在哪里（同一坐标系）

| 能力 | COD5 | 我们 |
|---|---|---|
| 读 PM4 环 | ✅ 自己读 | ❌ 用 SDK 的 CP |
| 着色器翻译 | ✅ 后台线程 + 磁盘缓存 | 🟡 SDK 的翻译器在用，但我们没接管 |
| 管线管理 | ✅ 缓存 + 预热 + 后台创建 | 🟡 SDK 的缓存在用 |
| 渲染目标 / resolve | ✅ clear-alias（不拷贝） | 🟡 SDK 的实现（慢，占 80% 成本） |
| 纹理 | ✅ 自己的池 + 指纹 + 淘汰 | 🟡 借 SDK 的缓存（已验证可拿 view） |
| 每绘制路径 | ✅ BuildDrawSetup + Draw | ❌ 0（我们只画了 10 个 UI quad） |
| 驱动层桥接 | ✅ nvk_switch_draw | ❌ 0 |
| ZCULL / MSAA 别名 / 深度 24 位 | ✅ 有候选与开关 | ❌ 0 |

## 四、给我们排的计划（照他们的路线图裁剪）

### 阶段 0：资料与基线（1 天）
- 把 COD5 包纳入 `参考资料-COD5-SP-NX-源码\`（只源码 + 文档，不带游戏文件）；
- 读 `docs/中文文档索引.md` 里这 6 份：`原生NVK绘制桥接`、`原生批次记录器`、
  `原生绑定键备忘录`、`原生描述符池寿命`、`原生场景ZCULL`、`deko3d迁移`。

### 阶段 1：ring 骨架（1~2 周，零风险）
- 照 `cod5_native_system.cpp` 的 `Reader` 写一个**只读不画**的版本：
  读 PM4、统计包类型分布、验证环指针推进；
- **画面完全不变**（仍然让 SDK 的 CP 画），只输出统计；
- 产物：COD6 的包类型分布 + 每帧状态变更次数 = 后面所有工作的地图。

### 阶段 2：接管第一类绘制（2~4 周）
- 用他们 `Renderer::Draw` 的结构，先只处理**我们已经验证过的那类 UI quad**（idx=6、有顶点）；
- 与 SDK 的 CP **二选一**（不能并行，v96 的教训）：同一帧里，我们的绘制由我们自己发；
- 验收：画面一致（截图对比）、帧率不变或更好。

### 阶段 3：接管程序化全屏绘制（`A4A9`，64 次/帧）——**收益最大的一步**
- 需要解出它的顶点着色器常量（我们已拿到 `c0/c1 = 像素→NDC`）；
- 参考 `native_geometry_shaders.cpp` 的做法（quad 展开）。

### 阶段 4：渲染目标与 resolve（决定 GPU 侧）
- 照他们的 `GetRenderTargetCanonical` + `FindClearAlias` 实现"resolve 不拷贝"；
- 我们实测里 `PREP_LENTO rt=5.1 ms` 就是这块。

### 阶段 5：驱动桥接 + ZCULL + 缓存模块（持续）
- `nvk_switch_draw`、binding memo、descriptor pool 寿命、batch recorder。

## 五、诚实的代价与风险

| 项 | 估计 |
|---|---|
| 总工作量 | **3~6 个月**（一个人的节奏），阶段 1~2 可见结果，阶段 3 才有帧率跳变 |
| 最大风险 | 半成品状态机（v96 的教训）；必须"整条路径替换"，不能混跑 |
| 关卡上限 | 不变：**14~15 fps**（游戏自身 CPU），除非再做游戏函数原生化 |
| 我们能拿到的确定收益 | **菜单 14 → 30~40 fps** |

## 六、结论

**这个包把"要写什么"从模糊变成了清单**：7,340 行渲染器 + 2,100 行环读取 + 3,400 行纹理
+ 1,124 行几何展开 + 一堆小缓存模块，而且有 80 份复盘告诉我们哪里会踩坑。

我们不需要从零设计——需要的是**按阶段移植 + 按他们的门槛验收**。

---

# 补充（第 193~197 轮）：为什么"自己读环"一直崩

## 结论

**根因不是我的解析代码，而是"读客机内存"这件事本身。**

这个 SDK 打开了**页写监控**（为了纹理失效跟踪）：游戏把某些页设为只读，我们一读就触发一次"模拟缺页"。
主菜单下这类缺页可达 **每秒 20 万次**。

- 我在 SDK 的 GPU 线程里、或在自己起的线程里读环 → 每秒几十万次模拟缺页 →
  线程卡死/崩溃 → 黑屏 + 洋红崩溃屏（洋红是 SDK 异常处理器的崩溃画面，不是我们画的）；

## COD5 项目怎么解决的

他们在自己的 SDK fork 里加了一个开关，并在启动时调用：

```cpp
RexGmSetReadOnlyMapped(true);   // 不再对只读页做写入监控，读到就直接读
```

（cod5_native_system.cpp 构造函数里，注释原文："Nothing watches guest pages for writes
without the SDK's shared memory, so the pages the game makes read-only can stay mapped.
Otherwise every read of them is an emulated fault: 200,000 a second at the menu, which
held it at 8 fps." —— 他们那边不关这个开关，菜单只有 **8 fps**。）

## 我们的现状

```
grep -rn "RexGmSetReadOnlyMapped" sdk/include sdk/src   →   没有任何结果
```

**说明我们的 SDK fork 里没有这个函数**，它在 COD5 那套 fork 里是新增的。

## 所以阶段 1 的正确顺序是

1. **先在我们的 SDK 里加这个开关**（内存层：mapped_memory / shared_memory 的只读页监控，
   加一个全局标志 + RexGmSetReadOnlyMapped(bool) c 接口）；
2. 只在 cod6_renderizador = "nativo" 时打开它；
3. 然后才做"自己读环、自己统计"——那时读环就是纯内存读，不会再有每秒几十万次缺页。

**这一步不做，后面所有"自写渲染器"的工作都会撞同一堵墙**：渲染器要频繁读客机内存
（顶点、索引、纹理、着色器），每次读都触发模拟缺页，性能会比现在更差。

（这条也解释了为什么参考项目 nfsmw-nx 能在 Switch 上跑到 35 fps：他们同样关掉了页监控。）
