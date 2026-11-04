# Prompt: port an Xbox 360 game to PC, then Switch (ReXGlue method)

<!-- AUTHOR-BANNER -->
> # 🚫⛔ 致 Telegram「DarkNSP」群的搬运者：请立刻停止 ⛔🚫
> # 🚫⛔ TO THE REPACKERS IN THE TELEGRAM GROUP “DarkNSP”: STOP NOW ⛔🚫
>
> ### ❌ 不要再打包包含游戏资源的整合包 / Stop bundling the game files
>
> 看到这个 GitHub 后，请**立刻停止**把本项目和游戏本体打包成「整合包」的行为。
> **Stop packaging this project together with the game data. Immediately.**
>
> 这里只是**半成品**——贴图会花、光影不对、随时崩，bug 一大堆还没修。
> **This is an unfinished work in progress.** Textures corrupt, lighting is wrong, it crashes at random, and a long list of bugs is still unfixed.
>
> 把游戏本体塞进去一起发，**既侵权，也是在欺骗玩家和粉丝**：
> 他们花几个小时下载好几个 GB，拿到的却是一个走两步就花屏、进关卡就出错的版本。
> **白费功夫下个不能玩的版本，究竟图什么？**
>
> **Bundling the game data is copyright infringement — and it is deceiving players and fans.** They spend hours downloading several gigabytes and end up with a build that corrupts its textures after a few steps and breaks down as soon as a level loads. **Why waste people's time with a version that cannot be played?**
>
> ### 🛑 这样做是不负责任的 / This is irresponsible
>
> 你说你在"分享"，实际是在拿别人的时间和期待换流量。
> 玩家以为下到的是成品，结果是半成品；出了问题，骂的是这个项目，不是你的搬运。
> **You call it sharing. In reality you are trading other people's time and expectations for attention.** Players think they downloaded something finished, and when it breaks, the project gets blamed — not your repack.
>
> ### ✅ 要搬运，请这样做 / If you must share it, do this
>
> 1. **只搬仓库本身和这份声明**，不要附带任何 `.ff` / `.bik` / `.pak` / `game_root`。
> 2. 必须写清楚：**半成品、有大量 bug、不保证能玩、非商业**。
> 3. 必须保留作者署名与仓库链接。
>
> **1.** Share **the repository and this notice only** — no `.ff` / `.bik` / `.pak` / `game_root` files.
> **2.** State it plainly: **work in progress, many bugs, no guarantee that it is playable, non-commercial**.
> **3.** Keep the author credit and the repository link.
>
> ---
>
> # ⚠️ 半成品 · 严禁商用 · 严禁盗用
> # ⚠️ WORK IN PROGRESS · NO COMMERCIAL USE · DO NOT REUPLOAD AS YOUR OWN
>
> **作者 / Author：** BILIBILI（B站）**一个没有人看的UP主**　UID **122607497**
> ｜ GitHub **GreenieSparkles**
>
> - 🚧 **这是半成品 / This is a work in progress.** 大量已知与未知 bug，参数和实现随时会变，不保证能通关、能玩完。
>   Expect plenty of known and unknown bugs; parameters and implementation change without notice. No guarantee that it is playable to the end.
> - 💰 **严禁商业用途 / NO COMMERCIAL USE.** 不得售卖、不得用于付费内容、不得捆绑进任何收费产品或服务、不得用于引流变现。
>   May not be sold, paywalled, bundled into any paid product or service, or used for monetised traffic.
> - 🪪 **严禁盗用与冒名 / DO NOT REUPLOAD AS YOUR OWN.** 转载、搬运、二次打包必须保留本署名与声明，并注明来源仓库。
>   Any repost, mirror or repack must keep this attribution and notice, and must link back to the source repository.
>
> ---
>
> ## 📜 免责声明 / Disclaimer
>
> **中文**
>
> 1. 本项目**仅供学习与技术研究**，目的是研究 Xbox 360 程序的重编译与图形 API 的移植，不提供、不包含、不分发任何游戏数据。
> 2. 使用本项目需要你**自己合法拥有**《使命召唤：现代战争2》的正版游戏。请自行准备游戏文件。
> 3. 本项目与 Activision Publishing, Inc.、Infinity Ward、id Software、Nintendo **没有任何关系**，未获其授权或认可。所有商标与版权归各自所有者。
> 4. 本项目**按现状提供，不附带任何担保**，包括但不限于可用性、稳定性、特定用途适用性。使用风险由使用者自行承担。
> 5. 因使用本项目导致的**任何直接或间接损失**（存档损坏、主机异常、账号问题、数据丢失等），作者**不承担责任**。
> 6. 本项目**可能随时停止更新**，作者没有义务修复任何问题、回复任何提问或提供任何支持。
> 7. 若权利人认为本项目侵犯其权益，请联系删除，本项目会立即配合处理。
>
> **English**
>
> 1. This project is **for study and technical research only** — it explores recompiling Xbox 360 code and porting a graphics API. It does not provide, contain or distribute any game data.
> 2. You must **legally own** a genuine copy of *Call of Duty: Modern Warfare 2*. Supply the game files yourself.
> 3. This project is **not affiliated with, authorised by, or endorsed by** Activision Publishing, Inc., Infinity Ward, id Software or Nintendo. All trademarks and copyrights belong to their respective owners.
> 4. Provided **“as is”, without warranty of any kind**, express or implied, including but not limited to availability, stability or fitness for a particular purpose. You use it at your own risk.
> 5. The author **accepts no liability** for any direct or indirect damage arising from its use (corrupted saves, console anomalies, account issues, data loss, etc.).
> 6. Development **may stop at any time**. The author is under no obligation to fix anything, answer questions or provide support.
> 7. If a rights holder believes this project infringes their rights, please get in touch and it will be removed promptly.
>
> ---
>
> 仓库 / Repository：https://github.com/GreenieSparkles/COD6-NX
<!-- /AUTHOR-BANNER -->


Paste the section below into a new chat, filling in the bracketed parts.

---

Port the Xbox 360 game **[name]** (title ID **[xxxxxxxx]**) to Windows PC and then Nintendo Switch by static
recompilation with **ReXGlue** (Xbox 360 PPC to C++), the method of nfsmw-nx, masseffect-nx and sa2-nx
([sa2-nx path]). My own game files are at **[path]** (ISO, or XBLA STFS packages). Work in a new folder
**[path]**, reuse the sa2-nx SDK fork (`sdk/`), tools and renderer, and keep game files and generated code out of the
repository. Order of work, one stage at a time, verify each before moving on:

**1. Extract and inspect**
- STFS (XBLA, DLC): `tools/stfs_extract.py` (only reads the package). ISO: `tools/extract_iso.py`.
- Look for extra code modules: guest DLLs (`*.xex` inside the data, loaded with XexLoadImage) go in the manifest as
  `[[modules]]`. Overlay files from older consoles (GameCube `.rel` etc.) may be leftovers: check the prologue style
  (`mflr r12` = Xenon MSVC, `mflr r0` = CodeWarrior) before assuming they are live code.

**2. Build the code generator and translate** (Windows: portable LLVM clang 20, VS Build Tools libs, `tools/win/env.ps1`
and `run.ps1`; `-march=x86-64-v2`; fix Git symlink stubs in `sdk/thirdparty`, `fetch_thirdparty.py` does it)
- `rexglue codegen <manifest>`; fix each "target not in any function" with an `[functions]` entry plus evidence.
- Before booting, sweep the missed-code classes, each with its tool, and re-run after every codegen:
  - functions reached only by pointer (CRT initializer table, vtables, callbacks built with `lis/addi`):
    `tools/pointer_fixpoint.py`, `tools/pointer_audit.py`
  - truncated jump tables (crash = illegal instruction, "Switch case out of range"): `tools/switch_check.py`
  - `setjmp`/`longjmp` (crash = r1/r31 corrupted after a call returns): find them by shape
    (`std`/`stfd`/`stvlx128` into r3 vs the mirrored loads) and set `setjmp_address` / `longjmp_address`.

**3. Boot on PC with the emulated GPU** (`sa2_renderer_native = false`)
- Test harness: `tools/win/boot_test.ps1` (always muted + minimized; never pipe it into `Select-Object -First`), video
  with `-VideoAt N -VideoFrames M`, MP4 + contact sheet with `tools/make_video.sh`, frame matching against a reference
  recording with `tools/compare_video.py`.
- Crash triage: unregistered call -> pointer/gap class; guest access violation -> symbolize `sa2.exe+0xRVA` with
  `llvm-symbolizer` (`app/src/sa2_crash_log.cpp` logs illegal instructions too); temporary caller-logging hooks in
  a debug-hooks file.
- XBLA: `license_mask = 1` for the full game; DLC packages installed from a `dlc/` folder (`app/src/sa2_dlc.cpp`).
- Send a subagent to audit crash classes in a COPY of the tree while you continue.

**4. Native renderer on PC** (the PM4-ring Vulkan renderer from masseffect-nx via banjo-nx, `app/src/me`)
- Find the game's D3D functions: `tools/d3d_match.py` against a game with named XDK functions, confirm by call
  alignment (`tools/listing_diff.py`). Hooks needed: draw-time shader load (+ the device's bound VS/PS offsets, which
  move between XDK versions; pass them to `SetDeviceShaderOffsets`), DrawVertices/DrawIndexedVertices/DrawVerticesUP
  (pass the device, r3, to `NoteDrawCall`), CreateVertexShader/CreatePixelShader (shader dump), the flip (frames).
- Shader library loop until the log says 0 unidentified loads: dump containers (`*_dump_shaders`), run native with
  `SA2_SHADER_DISCOVERY`, `SA2_VERTEX_VARIANTS`, `SA2_SHADER_MISSING`; repair D3D-patched
  variants (`repair_vertex_variant_declarations`, binary-mode output); wrap ONLY shaders no real container covers
  (`tools/wrap_discovered.py`); build with `tools/build_shader_library.sh` (XenosRecomp + DXC + packer).
- Compare native against emulated frame by frame (`compare_video.py` on two raw captures of the same window), and
  trace single frames (`banjo_trace_frame_s`, `SA2_DUMP_TEXTURE`) for every difference. Done = no visible
  difference across boot, menus, gameplay, movies.
- Known XenosRecomp fixes in sa2-nx: fetches whose swizzle is all constants; interpolators linked by hardware
  register for every shader.

**5. Then GPU optimizations, then Switch** (devkitA64 + libnx, NVK from mesa-switch, the SDK's Horizon layer; read
nfsmw-nx `docs/platform-notes.md`, `toolchain.md`, `measuring.md` first). Every optimization must be pixel-identical
(verify with `compare_video.py`) and measured on the console.

Rules: every analysis entry carries its evidence; test runs never take over the desktop (muted, minimized); no game
data in the repo; report results faithfully, including what was not verified.
