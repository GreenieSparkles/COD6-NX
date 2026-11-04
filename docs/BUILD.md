# 构建与调试

<!-- AUTHOR-BANNER -->
> **⚠️ 半成品 · 严禁商用 · 严禁盗用**　｜　作者：BILIBILI（B站）**一个没有人看的UP主** UID **122607497**　｜　GitHub **GreenieSparkles**
>
> 本项目是半成品，存在大量已知与未知 bug，不保证可玩。严禁商业用途、严禁冒名转载。
> [致搬运者的公开信](https://github.com/GreenieSparkles/COD6-NX/blob/main/docs/致DarkNSP搬运者的公开信.md)　｜　[仓库](https://github.com/GreenieSparkles/COD6-NX)
<!-- /AUTHOR-BANNER -->


## 环境

| 项 | 版本 |
|---|---|
| 系统 | Linux（推荐 WSL2 Ubuntu 24.04） |
| 工具链 | devkitPro `devkitA64`（GCC 15.2）+ `switch-tools` |
| 构建 | CMake ≥ 3.20、Ninja |
| GPU 驱动（实机） | NVK / Mesa 26.x |

环境变量（devkitPro 的 `switchvars.sh` 会设置）：

```bash
export DEVKITPRO=/opt/devkitpro
export DEVKITA64=$DEVKITPRO/devkitA64
```

---

## 编译

```bash
# 首次配置（已有 out/nx 则可跳过）
cmake -S . -B out/nx -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/cmake/Switch.cmake \
      -DCMAKE_BUILD_TYPE=Release

# 构建
ninja -C out/nx cod6
```

产物：

```
out/nx/cod6.nro      可执行（放到 sdmc:/switch/cod6-nx/）
out/nx/cod6.nacp     NACP，版本号由 COD6_VERSION 派生
```

> `cod6.nacp` **不需要随包更新**——只要 NRO 里的版本号变了即可。

---

## 版本号

唯一来源：

```
sdk/src/core/exception_handler_switch.cpp → COD6_VERSION "YYYY-MM-DD-NNN"
```

`CMakeLists.txt` 用 `configure_file` 从它派生出 NACP 版本，
并已加 `CMAKE_CONFIGURE_DEPENDS`，改完直接 `ninja` 会重新配置。

---

## 部署到实机

有三种方式：

### 1. SD 卡（最稳）

```
sdmc:/switch/cod6-nx/
├── cod6.nro
├── cod6.toml
└── game_root/
```

### 2. netloader（迭代最快）

```bash
nxlink -a <Switch的IP> out/nx/cod6.nro
```

Switch 端在 hbmenu 里按 `Y` 启动 netloader。改一行代码 → 编译 → 推过去，十几秒一轮。

> 注意：netloader 会把参数截断到约 256 字符，所以 `cod6.toml` 里不要放超长命令行。

### 3. PC 端对照（可选）

同一份源码在 Linux 上用 lavapipe（软件 Vulkan）+ Xvfb 可以跑起来，
用于**渲染逻辑**的对照调试（驱动相关的问题不会复现）：

```bash
cmake -S . -B out/pc -G Ninja -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
      -DREXGLUE_USE_VULKAN=ON -DREXGLUE_USE_D3D12=OFF
ninja -C out/pc cod6
VK_ICD_FILENAMES=/usr/share/vulkan/icd.d/lvp_icd.json \
  xvfb-run -a -s "-screen 0 1280x720x24" ./out/pc/cod6 \
  --game_data_root=/path/to/game_root
```

> PC 版在 Windows 上需要 `REXGLUE_GPU_PLUGIN_STATIC=ON`
> （PE 的 DLL 不能引用 exe 里的符号），并设 `cod6_gpu_backend = "vulkan"`。

---

## 代码结构

```
src/
├── main.cpp                    入口
├── cod6_app.h                  路径配置、启动阶段标记
├── cod6_renderizador.cpp       渲染器选择
├── cod6_nativo.cpp             原生路径的钩子实现
└── native/
    ├── nativo/
    │   ├── native_renderer.cpp      PM4 环读取、绘制、管线、提交
    │   ├── native_textures.cpp      纹理缓存、反平铺、上传
    │   ├── texture_pool.cpp         显存池
    │   ├── native_hash.h            内容变更检测（采样 + 信任窗口）
    │   ├── cod6_page_writes.cpp     写入戳（4 KB 页）
    │   └── cod6_native_system_nativo.cpp  系统层、周期报告
    └── cod6_native_reader.cpp    命令环读取
```

### 两条渲染路径

| `cod6_renderizador` | 说明 |
|---|---|
| `"nativo2"` | 自研原生 Vulkan 渲染器：直读 PM4 环，自己管状态、纹理、管线、渲染目标 |
| `"xenos"` | SDK 的通用 Vulkan 路径（Xenia 血统）——**正确但极慢**，仅用于对照 |

---

## 调试要点

### 先确认进程状态

黑屏时先看日志的阶段标记：

```
[cod6] phase 1/9 … phase 9/9
```

卡在哪个 phase 就是哪个子系统的问题。

### 缓存目录必须存在

`cache/shaders/driver/` 不存在时，着色器缓存**静默失败**：

```
[native] renderer: … 346 shaders (0 cached) …
SPIR-V cache: 710 translations (98318 KB) loaded from …   ← 正常时应有这行
```

### 性能指标基线

正常水平（关卡内）：

```
vertex copies 0.1 MB/帧      copying 0.0 ms      hashing 0.7 ms
texture lookups 2~8 ms       （超过 20 ms 说明哈希在空转）
renderer CPU 26~34 ms        GPU 18~23 ms
```

### 已知的侵入性诊断（会掉帧）

| 开关 | 代价 |
|---|---|
| `cod6_native_full_texture_hash = true` | 每帧全量哈希，+20 ms/帧 |
| `cod6_native_texture_pool = false` | 每贴图独立分配，约 1.9 ms/张 |
| `cod6_debug_tex_album > 0` | 导出贴图，每个抓取点一次卡顿 |
| `cod6_native_texture_evict_frames` 极小值 | 频繁回收，导致重上传 |

诊断完记得关掉，否则会误判为「渲染慢」。
