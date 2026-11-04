# TAG · 新会话交接（COD6 → Switch 移植）

<!-- AUTHOR-BANNER -->
> # ⚠️ 半成品 · 严禁商用 · 严禁盗用
> # ⚠️ WORK IN PROGRESS · NO COMMERCIAL USE · DO NOT REUPLOAD AS YOUR OWN
>
> **作者 / Author：** BILIBILI（B站）**一个没有人看的UP主**　UID **122607497**
> ｜ GitHub **GreenieSparkles**
>
> - **这是半成品 / This is a work in progress.** 大量已知与未知 bug，参数和实现随时会变，不保证能通关、能玩完。
>   Expect plenty of known and unknown bugs; parameters and implementation change without notice. No guarantee that it is playable to the end.
> - **严禁商业用途 / NO COMMERCIAL USE.** 不得售卖、不得用于付费内容、不得捆绑进任何收费产品或服务、不得用于引流变现。
>   May not be sold, paywalled, bundled into any paid product or service, or used for monetised traffic.
> - **严禁盗用与冒名 / DO NOT REUPLOAD AS YOUR OWN.** 转载、搬运、二次打包必须保留本署名与声明，并注明来源仓库。
>   Any repost, mirror or repack must keep this attribution and notice, and must link back to the source repository.
>
> ---
>
> ### 致 Telegram「DarkNSP」群的搬运者
>
> 看到这个 GitHub 后，请**不要再打包包含游戏资源的整合包**。
>
> 这里只是**半成品**，bug 一大堆还没修。把游戏本体塞进去一起发，既**侵权**，也是在**欺骗玩家和粉丝**——他们花时间下载好几个 GB，拿到的却是一个走两步就花屏、随时崩的版本，**白费功夫下个不能玩的版本，图什么？**
>
> 这样做**不负责任**。要搬运，就只搬**仓库本身和这份声明**，并且必须写清楚：**半成品、有大量 bug、不保证能玩**。
>
> ### To the repackers in the Telegram group “DarkNSP”
>
> If you see this GitHub repository, **stop repacking it together with the game files**.
>
> This is an **unfinished build** with a pile of bugs still unfixed. Bundling the game data with it is **copyright infringement**, and it is also **deceiving players and fans** — they spend hours downloading several gigabytes only to get a build that corrupts its textures after a few steps and crashes at random. **Why waste people's time with a version that cannot be played?**
>
> Doing that is **irresponsible**. If you must share it, share **the repository and this notice only**, and state it plainly: **work in progress, many bugs, no guarantee that it is playable**.
>
> ---
>
> 仓库 / Repository：https://github.com/GreenieSparkles/COD6-NX
<!-- /AUTHOR-BANNER -->


> **读这一份就够了。** 它规定了：拉日志、存备份、出包、验收的全部固定流程，以及当前状态和下一步。

---

## 0. 一句话现状（2026-10-31 收盘）

自研 PM4 解析器**已完成并验证**；**帧率零提升**（13~15 fps）。
下一步只有一件事：写 `BeginPass` + 连发绘制，一次做完。

---

## 1. 固定路径

| 用途 | 路径 |
|---|---|
| 交付目录（用户拷到卡上的文件） | `D:\主机掌机\NSLITE\NSMW2\测试\当前\switch\cod6-nx\` |
| **日志归档**（每轮一个文件夹，纯阿拉伯数字） | `D:\主机掌机\NSLITE\NSMW2\SD测试包\每次日志都在这\<轮次>\logs\` |
| 文档归档 | `D:\主机掌机\NSLITE\NSMW2\测试\当前\*.md` |
| 工程（WSL） | `/opt/switch/cod6-ng`（构建：`ninja -C out/nx cod6`） |
| COD5 参考源码（已拷入工程，不参与编译） | `/opt/switch/cod6-ng/src/native/referencia/` |
| COD4 参考 NRO（用户机器上） | `D:\Downloads\codmw.nro` |
| 构建脚本 | `D:\devkitPro\tools\buildNNN.sh`（每轮一个） |

---

## 2. 拉日志（固定流程，不要问用户）

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File 'D:\devkitPro\tools\mtp_pull_logs.ps1' -Destino 'D:\devkitPro\tmp\mtpNNN'
```

然后归档到 `SD测试包\每次日志都在这\<轮次>\logs\`（`cod6_lastrun.log` + `rex\` 四个文件）。

**注意**：
- MTP 只读，**绝不写卡**（nro 一律用户线缆复制）；
- MTP 报的文件大小恒为 0，不能用来判断是否写完；
- 若提示 `no se encontro el dispositivo Switch` → 主机休眠或 USB 断开，让用户重连后 **retry**；
- 判断这一轮是否真的跑了新包：先看 `rex_stderr.log` 里的 `VERSION 2026-10-31-NNN`；
  **版本不对 = 用户还没跑，别分析旧日志**。

---

## 3. 出包规则（用户反复强调的硬要求）

1. **每读完一次日志，必须交付一个新包**（放交付目录，报 md5 + 版本号）；
2. 每轮同时给出：`cod6.nro` 的 md5、大小、版本号；`cod6.toml` 是否要换；
3. `cod6.toml` 当前内容：13 个正常键 + `cod6_renderizador = "nativo"`；
4. **默认路径（`xenos`）永不破坏**；
5. **热路径里只许整数运算**（不许日志、不许字符串格式化）——违反者必黑屏（v128 教训）；
6. **任何实验路径必须自带"状态不对就落回模拟层"的保护**——这样黑屏不会发生，也就不需要回退包；
7. 黑屏时**只回退一次**，不额外发第三个包（v136~v140 教���：堆包没用）。

---

## 4. 当前可用包

| 版本 | md5 | 说明 |
|---|---|---|
| **v138** | `ab57f43d6cdbe255c984f349e2e2aaaa` | 画面正常，13~15 fps（**推荐保底**） |
| v140 | `bcf2e0352cce8bf4fd42931581c79d1c` | 同上 + 渲染目标模块骨架（未启用） |

---

## 5. 已验证的事实（别再重复测量）

| 事实 | 数据 | 来源轮次 |
|---|---|---|
| 自研解析器正确 | `self-test OK (paquetes=6 … registros=3 dwords=17)` | 202 |
| 自研环读取真实数据 | `paquetes=880000 dibujos=144369 shaders=61155 registros=6466390` | 207 |
| 菜单着色器 | **4 个**：`A4A9` 68%（idx=1）、`3C15` 31%（idx=3）、`C60E`、`39EF` | 208 |
| 每帧绘制 | ≈76 次 3 索引 + ≈35 次 6 索引 | 207 |
| 环几何 | `base=0x1FC98000 bytes=32768` | 111 |
| 缺页 | 12 次/秒，**不是瓶颈** | 210 |
| 绘制位置 | 主环只有寄存器写，绘制在**间接缓冲**（`0x3F`）里 | 123/207 |
| 天花板 | 全跳过模拟绘制 = **38~44 fps**；关卡零绘制 = **14~15 fps** | 177/185 |
| 黑屏根因 | 跳过 `IssueDraw` 会连"进入渲染目标"一起跳过 | v137 |

---

## 6. 下一步（唯一正确的做法）

**三件一起做，一次上真机，不发中间验证包：**

```
src/native/cod6_native_destinos.*   渲染目标缓存（v140 已有 DestinoKey + DestinoCache）
src/native/cod6_native_pass.*       BeginPass：自己开 render pass + barrier + 布局转换
src/native/cod6_native_lote.*       连发：同一管线/同一目标的多条 A4A9 一次发出
                                    保护：状态不一致 → 整条落回模拟层
```

参考：`src/native/referencia/native_renderer.cpp` 里的 `BeginPass`、`GetRenderTargetCanonical`、`FindClearAlias`（约 3800~3900 行）。

**验收**：画面与 v138 一致 **且** fps 上升。二者缺一即失败，回退到 v138。

---

## 7. 用户偏好（务必遵守）

- 不要问"要不要继续"——**直接做，做完报结果**；
- 不要中途反复发包做单点验证；
- 每次读完日志先给新包，再解释；
- 中文汇报，先给结论和数据，再给原因；
- 不要用 zip 做备份；文档用中文，路径用中文；
- 用户会自己拷卡（线缆），不要用 MTP 写。
