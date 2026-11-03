# COD6-NX
Call of Duty: Modern Warfare 2 (COD6) ported to Nintendo Switch. ReXGlue (Xenia-based) recompilation with a custom native Vulkan renderer: reads the PM4 ring directly, native texture/pipeline caches, disk-persisted SPIR-V. 20-60 fps, shadows and some dynamic lighting still WIP. Research only; no game data included.
<div align="center">

# cod6-nx

**《使命召唤：现代战争2》(COD6) 的 Nintendo Switch 移植**

Xbox 360 代码重编译为 ARM64 · 自研原生 Vulkan 渲染器 · 不含任何游戏数据

[![状态](https://img.shields.io/badge/状态-WIP-yellow)]()
[![帧数](https://img.shields.io/badge/实机-20~60%20fps-blue)]()
[![平台](https://img.shields.io/badge/平台-Switch%20(NVK)-red)]()
[![授权](https://img.shields.io/badge/授权-GPL--3.0-green)](LICENSE)

[简介](#简介) · [状态](#当前状态) · [安装](#安装) · [配置](#配置) · [诊断](#诊断) · [构建](#构建) · [已知问题](#已知问题) · [授权](#授权)

</div>

---

## 简介

把 Xbox 360 版 **Call of Duty: Modern Warfare 2** 带到 Switch 上运行。

不是模拟器套壳：游戏的 Xbox 360 可执行代码由 **ReXGlue**（[Xenia](https://xenia.jp/) 血统）
重编译为 ARM64；绘制由一套**自己写的原生 Vulkan 渲染器**完成——它直接读游戏的
PM4 命令环，自己管理纹理、管线和渲染目标，不经过模拟器的通用图形路径。

> [!IMPORTANT]
> **本仓库不含任何游戏数据。** 你需要自备正版游戏本体，并自行提取 `game_root/`。
> 仅供学习与技术研究。

---

## 当前状态

实机：Switch，NVK / Mesa 26.x；CPU 1785 MHz / GPU 768 MHz。

| 项目 | 状态 | 说明 |
|---|:---:|---|
| 启动 / 进入游戏 | ✅ | 初始化 9 个阶段全过，可进入关卡与选枪界面 |
| 帧数 | 🟡 | 默认画质 22~32 fps；`very_low` 预设实测 **35~60 fps** |
| 静态贴图 | ✅ | DXT1 / DXT5 / DXN 上传正确（导出贴图逐像素验证） |
| 着色器缓存 | ✅ | SPIR-V 翻译落盘复用，二次启动读回 700+ 条目 / 98 MB |
| 顶点数据 | ✅ | 按绘制范围拷贝，约 **0.1 MB/帧**、哈希 0.7 ms |
| 法线 / 平表面 | ❌ | 部分平表面出现蓝紫噪纹，**未解决** |
| 动态阴影 | ❌ | 当前关闭（`sm_enable 0`） |
| 枪焰 / 尘土粒子 | ❌ | 显示为巨型方块（memexport 通路调试中） |
| 远距离光照 | ❌ | 仅角色附近明亮，光照体积（lightgrid）未移植 |
| 长时间游玩 | 🟡 | 显存持续增长，触顶后新贴图失真 |

图例：✅ 可用　🟡 部分可用　❌ 未完成

---

## 安装

### 包结构

```
sdmc:/switch/cod6-nx/
├── cod6.nro              主程序
├── cod6.toml             配置
├── game_root/            游戏数据（自备，不随包提供）
├── update/               标题更新设备（自动创建）
├── user/                 存档与设置（自动创建）
├── cache/                着色器与管线缓存（自动创建）
└── cap/                  诊断导出（可选）
```

> [!TIP]
> `update/`、`user/`、`cache/` 由程序自动创建。**不要手动删除它们**——
> 着色器缓存在目录缺失时会静默失败（日志表现为 `0 cached`）。

### 启动

把游戏数据放进 `game_root/`（即 `default.xex` 等文件所在目录），
在 hbmenu 里选择 `cod6.nro`。

---

## 配置

全部配置在 `cod6.toml`，与 NRO 同级。**改配置不需要重新编译。**

### 画质预设

`very_low`（实测帧数提升最明显的一组 dvar）：

```toml
cod6_dvar_set = "r_picmip 2; r_picmip_bump 2; r_picmip_spec 2; r_lodScaleRigid 6; r_lodScaleSkinned 6; r_lodBiasRigid 4000; r_lodBiasSkinned 4000; sm_enable 0; sm_sunShadowScale 0; sm_maxCascades 1; r_shadowMapSize 256; r_distortion 0; r_glow 0; r_glow_allowed 0; r_dof_enable 0; r_bloom 0; r_drawSun 0; r_specular 0; r_specularMap 0; r_fog 0; r_zfeather 0; r_detail 0; r_drawDecals 0; r_drawWater 0; r_texFilterMipMode 0; r_texFilterAnisoMin 1; r_texFilterAnisoMax 1; cg_brass 0; fx_marks 0; fx_marks_ents 0; fx_marks_smodels 0; ragdoll_enable 0; ragdoll_max_simulating 0; ai_corpseCount 0; r_cacheModelLighting 1; r_cacheSModelLighting 1"
```

| 组 | dvar | 效果 |
|---|---|---|
| 贴图锐度 | `r_picmip*` | 降纹理分辨率，省带宽 |
| 模型细节 | `r_lodScale*` / `r_lodBias*` | 远处模型降级 / 不显示 |
| 阴影 | `sm_enable` / `r_shadowMapSize` | 关闭阴影 |
| 屏幕特效 | `r_glow` / `r_bloom` / `r_dof_enable` / `r_distortion` | 全关 |
| 非必要绘制 | `r_drawDecals` / `r_drawWater` / `fx_marks*` / `ragdoll_*` | 减少绘制次数 |

### 渲染器与关键项

| 键 | 默认 | 说明 |
|---|---|---|
| `cod6_renderizador` | `"nativo2"` | 原生 Vulkan 渲染器。`"xenos"` 走 SDK 通用路径——**正确但极慢**（实测 6 fps → 0.1 fps），仅作对照 |
| `cod6_native_texture_pool` | `true` | 纹理显存池：32 MB 块，预热 384 MB，上限 800 MB |
| `cod6_native_texture_budget_mb` | `768` | 纹理预算 |
| `cod6_native_texture_evict_frames` | `100000000` | 淘汰阈值（帧）。极大值 = 关闭回收；长期游玩建议 `600`（约 20 秒） |
| `cod6_native_ranged_vertices` | `true` | 按绘制范围拷顶点（约 0.1 MB/帧）；`false` 为整块拷贝，正确但慢 |
| `cod6_native_memexport_copy` | `false` | CPU 补做 memexport 写入（拷贝前先比对，未变则不写） |
| `cod6_native_full_texture_hash` | `false` | 每帧全量哈希纹理，诊断用，开销大 |
| `cod6_native_max_anisotropy` | `1` | 各向异性过滤，1 = 关闭 |
| `cod6_native_gpu_backend` | `"any"` | GPU 后端；PC 上需设 `"vulkan"`，否则会选 D3D12 |

---

## 诊断

渲染问题难定位，工程里保留了一套可开关的诊断手段，全部通过 `cod6.toml` 打开。

### 抓帧与贴图导出

```toml
cod6_debug_capture_dir   = "sdmc:/switch/cod6-nx/cap"
cod6_debug_capture_every = 300    # 每 300 帧一个抓取点
cod6_debug_capture_max   = 2      # 抓 2 张画面
cod6_debug_tex_album     = 128    # 每帧最多导出 128 张被绑定的贴图
cod6_debug_album_frames  = 2      # 抓 2 个抓取点
cod6_debug_album_skip    = 4      # 跳过前 4 个点
cod6_debug_draw_table    = 2      # 记录这两帧的每个绘制与绑定的贴图地址
```

产物：

```
cap/frame_000300.rgbx          画面：8 字节头 + R8G8B8X8
cap/tex_f000300_0B434000.img   贴图：16 字节头 = 宽, 高, VkFormat, 每像素/块字节数
```

`[dtab]` 行把「哪个绘制 → 绑定哪些 guest 地址 / 什么尺寸」写进日志，
与导出的贴图文件名一一对应，用来定位「这个面为什么花」。

### 日志标记

| 标记 | 含义 |
|---|---|
| `[native] renderer: N drawn, M skipped …` | 每 10 秒绘制统计 |
| `[native] skipped by reason: …` | 被跳过的绘制及原因编号 |
| `[texmon] subidas=… hash: X cambio, Y igual` | 纹理上传次数与内容变化次数 |
| `[writes] …` | 写入戳（4 KB 页）命中 / 失配统计 |
| `[evict] frame N: … expulsadas` | 纹理淘汰情况 |
| `[dtab] …` | 绘制表（需开启） |
| `[native] SPIR-V cache: N translations …` | 着色器缓存加载情况 |

### 报问题请附

1. `logs/rex/rex_stderr.log` 与 `logs/rex/rex_perfil.log`
2. 使用的 `cod6.toml`
3. 出问题位置的截图（含 HUD 更好定位）
4. 版本行：`[cod6] VERSION 2026-…`

---

## 构建

Linux（推荐 WSL2 Ubuntu）+ devkitPro 的 devkitA64：

```bash
# 依赖：devkitPro (devkitA64 + switch-tools)、cmake、ninja
cd cod6-ng
ninja -C out/nx cod6          # 产物：out/nx/cod6.nro
```

版本号只有一处，NACP 版本由它派生：

```
sdk/src/core/exception_handler_switch.cpp  →  COD6_VERSION
```

### 目录

```
src/            应用层：启动、配置、渲染器选择
src/native/     原生渲染器（PM4 环、绘制、纹理、管线）
sdk/            ReXGlue SDK（Xenia 派生的运行时、重编译器、内核）
generated/      由 XEX 重编译生成的 PPC 代码（不入库）
```

开发与调试细节见 [docs/BUILD.md](docs/BUILD.md)。

---

## 已知问题

1. **平表面蓝紫噪纹** — 部分表面（选枪台、集装箱）呈彩色噪点。
   已排除：纹理内容被改坏（4.5 万次检查 0 变化）、显存池块复用、DXN 通道顺序。
2. **枪焰 / 尘土显示为巨型方块** — memexport 写入通路未完成。
3. **仅角色附近被照亮** — 光照体积（lightgrid）打包未移植。
4. **长时间游玩后新贴图失真** — 显存只增不减，触顶后新纹理分配失败。
5. **动态阴影** — 目前关闭。

排查过程与结论见 [CHANGELOG.md](CHANGELOG.md)。

---

## 授权

本项目以 **GPL-3.0** 分发（源码树中已含 `GPL-3.0-or-later` 文件）。见 [LICENSE](LICENSE)。

`ReXGlue` / `Xenia` 派生部分为 **BSD-3-Clause**，声明见 `sdk/LICENSE`，必须保留。
第三方库的授权见 [NOTICE](NOTICE)。

**不包含任何游戏数据**；游戏本体、`.ff`、`.bik` 等资源的版权属于其权利人。

## 致谢

- [Xenia](https://xenia.jp/) — Xbox 360 模拟器，ReXGlue 的前身
- ReXGlue — Xbox 360 代码重编译框架
- devkitPro / devkitA64 — Switch 自制程序工具链
- NVK / Mesa — Switch 上的 Vulkan 驱动
