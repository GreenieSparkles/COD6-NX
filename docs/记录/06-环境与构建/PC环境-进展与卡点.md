# PC 环境进展与卡点（第 157 轮）

## 一、已经做到的

| 步骤 | 状态 |
|---|---|
| 我们的源码树编成 PC 版（含渲染器代码 + 全部插桩 + 改过的 SDK） | ✅ `out/pc7/cod6`，50.8 MB |
| 程序跑起来：进到 guest 线程 | ✅ 日志 `phase 1..7/9` 全过，音频、输入、文件系统都通 |
| 找到并修掉"画面全黑"的第一个原因 | ✅ toml 少了 `gpu_plugin = "xenos"`，日志里有 `no GPU emulation loaded (gpu_plugin not set)` |
| 打开 GPU 插件后 | ❌ **崩在 lavapipe 里面** |

## 二、当前卡点（栈）

```
#0 libc
#1 libvulkan_lvp.so          ← lavapipe（CPU 软件 Vulkan）
#2 libvulkan_lvp.so
#3 rex::ui::vulkan::VulkanPresenter::InitializeSurfaceIndependent()
#4 VulkanPresenter::Create(...)
#5 VulkanProvider::CreatePresenter(...)
#6 GraphicsSystem::SetupPresentation(...)
#10 rex::ReXApp::OnInitialize()
```

**不是我们的代码、也不是插件加载**，是 lavapipe 在 presenter 建管线/描述符时崩了。
同一个 lavapipe 跑**原版 PC 构建**（`/opt/build/cod6`，用预编译 SDK）是正常的，
所以差别在"我们这套 SDK fork 的 presenter 代码"和"预编译 SDK 的 presenter 代码"之间。

## 三、四条可选路线（按性价比）

### A. 用 netloader 在真机上迭代（首选，但需要用户配合）

本机有 `/opt/devkitpro/tools/bin/nxlink`。Switch 与电脑同一 WiFi、用 hbmenu 的 netloader 启动后：

```
nxlink -a <Switch的IP> out/nx/cod6.nro
```

**这样根本不需要 PC 版**：迭代是秒级，而且量到的就是真机数字。渲染器开发就在真机上调。
代价：需要用户开 hbmenu（按住 R 点游戏图标）+ 告诉我 IP。

### B. 在 Windows 上用 Vulkan 后端跑（RTX 4060，真显卡）

插件支持 `backend = "d3d12" | "vulkan" | "any"`，但 `LoadGpuPlugin` 目前只传了名字
（`rex_app.cpp:343` 只传 `config_.gpu_plugin`），所以 Windows 上会选 D3D12 → 死锁。
可以给 app 加一个 cvar（如 `cod6_backend="vulkan"`），让它直接调
`LoadGpuPlugin("xenos", "vulkan")`。代价：要配 Windows 工具链（VS Clang + CMake + Ninja），
且要确认预编译的 `rexgpu-xenos.dll` 里编译了 Vulkan 后端。

### C. 继续修 lavapipe 那个崩溃

在 `InitializeSurfaceIndependent()` 里逐个 Vulkan 调用打点，找出是哪一句崩的。
可能半小时，也可能几小时；而且就算修好，lavapipe 也只能验逻辑、不能量性能。

### D. 回插卡模式，但每轮只发"有可见/可测变化"的包

最稳，最慢。第一里程碑（顶点解码）本身每一步都是小验证，插卡也能推进，只是慢。

## 四、我的建议

**先问用户能不能开 netloader（A）。** 能 → 直接走 A，PC 这条线的必要性立刻下降；
不能 → 走 C（我本地调试，不占用用户的卡），修完再回到渲染器主线。

无论走哪条，**渲染器第一里程碑的工作内容不变**：

    ① 顶点/索引解码（照 nfsmw_nativo_dibujos.cpp）→ 日志打出真实顶点值
    ② 自己的管线 + 绑定 → 屏幕上出现我们画的方块，位置正确
    ③ 接管 index_count ≤ 8 的 UI 绘制 → 菜单 UI 由我们画
