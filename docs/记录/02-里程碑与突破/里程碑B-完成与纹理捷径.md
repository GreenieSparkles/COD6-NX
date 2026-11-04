# 里程碑 B 完成（含闪烁修复）+ 纹理的捷径（第 178 轮）

---

## 一、里程碑 B 真正完成

| 版本 | 现象 | 结论 |
|---|---|---|
| v77 | UI 变洋红框，位置正确 | 顶点解码 + 坐标正确 |
| v78/v79 | 洋红**疯狂闪烁** | 捕获表和呈现帧错位（多帧在飞） |
| **v80** | **洋红框稳定不动，位置与 UI 元素一致，背景正常** | **里程碑 B 完成** |

修法：四边形列表改成**双缓冲**（一张捕获、一张绘制），切换发生在游戏帧结束
（`IssueSwap`），不再在 present 里清空。

现在的能力链：

    读客机顶点（大端） → 解格式 → 像素坐标 → 自己的管线 → 屏幕上正确位置

## 二、关键测量（177 轮，已确认）

| 状态 | 主菜单 fps | 绘制/帧 |
|---|---|---|
| 正常（模拟层全跑） | 12.8~14.4 | 64~77 |
| 跳掉我们接管的 13 个 | 14.2~14.4 | 64 |
| **跳掉全部绘制** | **38.2~41.1（平均 44.13）** | 0 |

**模拟层绘制路径 ≈ 50 ms/帧；自研渲染器的天花板在 41~44 fps（菜单）。**

## 三、发现了一条纹理捷径

原本以为"纹理"要自己写：从 guest 内存取纹理 → 去 tile（NEON）→ 上传 →
采样缓存。参考项目为此写了 `nfsmw_nativo_texturas_pool.cpp`（15 KB）+ 去 tile 代码。

**但 SDK 已经把这些做好了**，而且有公开接口（`sdk/include/rex/graphics/vulkan/texture_cache.h`）：

```cpp
VkImageView GetActiveBindingOrNullImageView(uint32_t fetch_constant_index,
                                            xenos::FetchOpDimension dimension, bool is_signed);
SamplerParameters GetSamplerParameters(const VulkanShader::SamplerBinding& binding) const;
```

也就是说：**模拟层已经把客机纹理去 tile、上传成 VkImageView 了，我们可以直接拿来用。**
这省掉了自研纹理缓存这一整块工作。

### 实现方案（下一步）

1. **捕获时**：对每个 UI 四边形，从像素着色器里取纹理绑定（fetch constant + 维度），
   调 `GetActiveBindingOrNullImageView` 拿到 `VkImageView`，连同矩形和 4 个 UV 一起存下来；
2. **片段着色器**：保持现在"全屏三角 + scissor 裁剪"的做法，在 FS 里用
   `gl_FragCoord` 和 push constant 里的矩形换算 UV，然后采样纹理 —— **顶点着色器不用改**；
3. **管线**：新建一条带"描述符集（combined image sampler）+ push constant"的管线；
4. **验收**：洋红框变成**真实的文字和图标**，位置不变。

这一条走通之后，UI 部分就基本完成了；剩下的才是关卡里的 3D（渲染目标、深度、着色器常量）。

## 四、当前交付

```
D:\主机掌机\NSLITE\NSMW2\测试\当前\switch\cod6-nx\
    cod6.nro    md5 f4f026dfa0c85aa5e00f545d4118be18   版本 2026-10-31-80
    cod6.toml   md5 4745b44aa0ab3dae0f7341fc06cb9e99
```

行为：主菜单正常 + **稳定的洋红 UI 框**（我们的渲染器画的），fps ~13~14。
