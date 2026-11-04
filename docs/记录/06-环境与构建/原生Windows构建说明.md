# 原生 Windows 版：构建说明与当前卡点

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


> 时间：2026-10-28 凌晨
> 结论：**能编、能跑、真显卡/真声卡/真手柄全部识别，但 D3D12 呈现会在 10~24 帧后死锁。**

## 一、为什么必须走原生 Windows 版

WSL 里没有 GPU，只能用 lavapipe（CPU 软件光栅化）。实测：

| 场景 | 软件渲染帧率 |
|---|---|
| 主菜单 | 35~58 FPS（勉强可玩） |
| 坑道关卡（3D） | **1~5 FPS（完全不可玩）** |

ReXGlue 官方移植（Dante's Inferno 到 240 FPS、WWE'12 到 60 FPS）全部是**模拟 GPU + 真显卡**。
所以瓶颈不是渲染器架构，是**没有 GPU**。原生 Windows 版直接用 RTX 4060。

## 二、工具链要求（来自官方 Wiki Getting-Started）

> Windows: Visual Studio 2022 with Desktop development with C++, including:
> - **C++ Clang Compiler for Windows (20.x or newer)**
> - MSBuild support for LLVM (clang-cl) toolset

**MSVC 编不了** —— SDK 头文件用了 `__builtin_bswap32/16/64`、`__VA_OPT__`（clang/GCC 扩展）。
用 MSVC 报的错：

```
fmt/base.h(465): error C2338: 'Unicode support requires compiling with /utf-8'
rex/ppc/function.h(130): error C3861: "__builtin_bswap32": 找不到标识符
rex/ppc/stack.h(43): error C2146: 语法错误: "__VA_OPT__"
```

**实测 clang 19.1.5 也能编过**（Wiki 写 20+，我们这台装的是 19.1.5，一次编过、0 错误）。

安装方式（不需要单独下 LLVM）：

```
vs_BuildTools.exe --add Microsoft.VisualStudio.Component.VC.Llvm.Clang --quiet --wait --norestart
```

装到 `C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\Llvm\x64\bin`。
注意同目录下还有 `ARM64\` 版本，**不能用**（"不是此操作系统平台的有效应用程序"）。

## 三、构建步骤

```bat
call "...\VC\Auxiliary\Build\vcvars64.bat"
set "LLVM=...\VC\Tools\Llvm\x64\bin"
set PATH=%LLVM%;...\CMake\Ninja;%PATH%
set CMAKE_PREFIX_PATH=D:\devkitPro\rexglue\win-amd64
cd /d D:\rexglue-projects\cod6
cmake -S . -B out\build\win-clang -G Ninja ^
      -DCMAKE_BUILD_TYPE=Release ^
      -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
cmake --build out\build\win-clang --parallel 16
```

产物：`out\build\win-clang\cod6.exe`（20.4 MB）+ 需要从 SDK bin 拷的 DLL：
`rexgpu-xenos.dll`、`TracyClient.dll`。

## 四、中文路径的坑（务必看）

Win32 程序从 PowerShell/CMD 传参时，中文路径会被编码成乱码：

```
[error] [core] --game_data_root does not exist: D:\涓绘満鎺屾満\NSLITE\_cod6_extract
```

**解法：建一个 ASCII 目录联接，不用复制 6.8 GB。**

```bat
mklink /J "D:\cod6_data" "D:\主机掌机\NSLITE\_cod6_extract"
```

启动用 `--game_data_root D:\cod6_data`。

## 五、原生版的补丁差异（重要）

`apply_all_patches.py` 加了 `--native` 模式：

```bat
python apply_all_patches.py --gen D:\rexglue-projects\cod6\generated\default --native
```

它**不打 PAD 补丁，并清掉已存在的 PAD 块**。原因：

> PAD 补丁会把 `/tmp/pad_cmd` 的状态无条件写进 `X_INPUT_STATE`。
> Windows 上没有这个文件，读出来全零，**等于把玩家真实手柄输入每帧清零**。

所以原生版必须去掉 PAD，让 SDL/XInput 的真实手柄直接生效。

## 六、已经跑起来的部分（日志证据）

```
[info] [gpu] DXGI adapter: NVIDIA GeForce RTX 4060 Laptop GPU (vendor 0x10DE, device 0x28A0)
[info] [gpu] Direct3D 12 device and OS features: resource binding tier 3, tiled resources tier 3
[info] [sys] Guest memory arena mapped: virtual base 0x0000000100000000
[info] [apu] audio endpoint '耳机 (High Definition Audio Device)': 2 ch, 48000 Hz
[info] [core] SDL JoystickAdded: "Xbox Series X Controller", VendorID(0x045E), ProductID(0x0B13)
[info] [core] SDL Controller 0: 0300fa67..., *,a:b0,b:b1,back:b6,dpdown:h0.4,...
[info] [core] New controller connected to slot 0.
```

**显卡、声卡、蓝牙手柄全部识别并初始化成功。**

## 七、当前卡点：D3D12 呈现死锁

在生成代码里加了跟踪探针（`D:\cod6_trace.txt`，因为 WIN32 子系统没有控制台）：

```
[1] xstart-ENTER
[1] gamemain-ENTER
... VdSwap-CALL 共 10 次 ... 然后永久停止
```

| 配置 | VdSwap 次数（9 秒内） |
|---|---|
| 默认 | **10（卡死）** |
| `--present_render_pass_clear=false` | 10 |
| `--present_effect=none` | 9 |
| `--d3d12_allow_variable_refresh_rate_and_tearing=false` | 10 |
| `--async_shader_compilation=false` | 10 |
| **`--vsync=false`** | **22（有改善，但仍在 ~22 帧后停）** |
| `SDL_GPU_DRIVER=vulkan` 环境变量 | 24（没换后端，日志仍是 DXGI） |

**现象**：客户机跑到第 10~24 次 `VdSwap` 就阻塞在 Present 里，进程 CPU 占用降到 ~20%，56 个线程都睡着。
**没有**任何 D3D12 错误日志。

### 下一步排查方向

1. **用 SDK 的 debug 运行时**（`rexruntimed.dll`，20 MB，带符号）+ 调试器挂上去看阻塞线程的调用栈。
   （`--gpu_plugin xenosd` 试过，debug 插件启动即退出，返回码 1）
2. 查 ReXGlue 的 Discord / issues 有没有 Windows D3D12 Present 卡死的已知问题。
3. 试 `--d3d12_pipeline_creation_threads=1`、`--d3d12_queue_priority` 等 D3D12 专项 cvar。

## 八、对比：WSL/Vulkan 版

| 版本 | 渲染 | 帧率 | 状态 |
|---|---|---|---|
| WSL + lavapipe | Vulkan（CPU） | 菜单 35~58，关卡 1~5 | **能完整跑，但 3D 不可玩** |
| 原生 Windows | D3D12（RTX 4060） | 10~24 帧后卡死 | **基础设施全通，呈现死锁待解** |

WSL 版是完整的参考实现：主菜单、特别行动、坑道关卡、HUD、武器都能渲染，
只是 3D 场景被 CPU 光栅化拖死。原生版一旦解决 Present 死锁，就是可玩的成品。
