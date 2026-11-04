# How bo2-zm-nx works

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


The Xbox 360's PowerPC code is translated ahead of time into C++, compiled for the Switch's ARM cores and linked with a
runtime that provides what the 360's kernel and hardware did. The GPU is not emulated: a renderer of its own reads
the game's GPU commands and draws them with Vulkan.

```
 your disc + TU18            build time (PC)                          run time (Switch)
 default_mp.xex  ──▶  loader: decrypt, decompress, patch       bo2zm.nro
 default_mp.xexp      code generator: 30,096 functions   ──▶    ├ recompiled game code
                      fix-up passes → C++ → GCC 16 (LTO, PGO)   ├ runtime: kernel, memory, threads, files, audio
 fastfiles (.ff) ──▶  shaders → XenosRecomp → dxc        ──▶    ├ native renderer → Vulkan (NVK)
                      → bo2zm_shaders.spvpack                   └ Switch layer: threads, cores, display
```

## 1. The executable

Zombies lives in `default_mp.xex`, targeted at Title Update 18, the game's final update. The SDK's loader (from Xenia)
decrypts the XEX2 image, decompresses it and applies the update's delta patch (`default_mp.xexp`). The result is a
47 MB PE image at 0x82000000 that imports 162 functions from `xam` and 215 from `xboxkrnl`. `tools/xexdump` writes it
to disk for the analysis tools.

## 2. The code generator

ReXGlue's generator (`sdk/src/codegen`), driven by `app/bo2zm_manifest.toml`:

1. **Analysis** finds every function from the entry point, exports, call targets, jump tables and exception tables,
   until nothing new appears: 28,358 functions, 30,096 emitted, no errors.
2. **Emission** writes one C++ function per PowerPC function, about 5 million lines in 140 files, each instruction
   kept as a comment. Guest memory is one base pointer with byte swaps; VMX128 maps to SIMDe (NEON on ARM, SSE on x86).

**Configuration.** `app/overrides.toml` holds what the analysis cannot find (boundaries, tail-branch-only thunks,
`setjmp` addresses, register options), each entry with its evidence. `app/gaps.toml` lists code no function covers.
`app/locals.txt` names hot leaf functions to copy with local registers.

**Registers.** Most PowerPC registers are C++ locals so the compiler can keep them in ARM registers: the non-volatile
general, floating-point and vector registers, condition registers, CTR, XER and the scratch vector registers. Only
argument and volatile registers stay in the context structure.

**Fix-up passes**, run by `tools/codegen.sh`:

| Pass | Why |
|---|---|
| `read_before_write.py --strict` | Finds functions that read a register before writing it; they must share registers |
| `setjmp_locals.py --fix` | `longjmp` error recovery must restore the register locals |
| `direct_calls.py` | Direct calls to unhooked functions, so the compiler can inline them |
| `localize.py` | Local-register copies of the functions in `app/locals.txt` |
| `guest_barriers.py` | Compiler barriers that make plain guest memory accesses safe |

## 3. Hooks

Each generated function is `__imp__sub_<address>`, called through a weak alias `sub_<address>`. A hook is a C++
function of that name, which the linker prefers; it can still call the original. Hooks replace hot functions with
native rewrites (each checked against the original first), observe the game (its Direct3D layer, the lighting
volume's writer, the job system) and add features (menus, cheats, presets, the benchmark).

## 4. The build

GCC 16 (devkitA64) compiles the generated code, the SDK and the port at `-O3` with LTO and PGO into one NRO of about
66 MB. Switch code is behind `__SWITCH__`; the same sources build for Windows and Linux with clang.

## 5. The runtime

The ReXGlue SDK (`sdk/`) carries Xenia's implementation of the 360's kernel and libraries, plus a Horizon layer.

- **Kernel**: the 377 imports (threads, events, semaphores, mutants, memory, files, content, sockets, profiles, system
  UI). On Horizon, sync objects wait on the address arbiter, so a signal never takes a lock.
- **Memory**: Horizon has no sparse memory and cannot change permissions on mirrored memory, so the window is reserved
  empty and memory is committed in chunks, turned into code-memory aliases and mapped into every view on first access.
  The exception handler emulates accesses to unmapped pages without system calls. The design is in
  `sdk/src/core/guest_memory_switch.h`.
- **Threads**: each guest thread is a Horizon thread. The game's job system is mapped in `app/src/bo2zm_waits.cpp`,
  where its polling loops sleep instead of spinning.
- **Audio**: the game mixes on its own threads; the runtime provides XAudio and the XMA decoder (FFmpeg, with a frame
  cache), out through `audout`.
- **Files**: the game, update and `user` folders; image pack reads are paced, because at memory speed the game's
  streaming re-reads the same images endlessly.
- **Input**: HID controllers presented as Xbox 360 controllers.
- **Sockets**: the solo match's local server, with the network errors Horizon reports after sleep treated as "no data
  yet".

## 6. The native renderer

The game's Direct3D layer, itself recompiled game code, writes PM4 packets for the Xenos GPU. The renderer's thread
reads them, keeps the register state, and for every draw identifies the shaders by hash in the shader library, finds
the Vulkan pipeline, resolves the textures (untiled and byte-swapped into the texture pool), uploads vertices, indices
and the constants the shaders read, and records the draw. Resolves, fences, memory waits, occlusion queries and
memexport are handled the same way.

- **Threads**: a recorder thread makes the Vulkan calls the ring thread queues; others copy vertex data and bind
  texture memory.
- **Write stamps**: each 4 KB page has an epoch advanced by the code that writes memory (copy and fill routines,
  skinning hooks, memexport). Cached vertex data, indices and textures stay valid while their pages are unchanged; a
  sample of hits is checked.
- **Shader library**: the game's 16,149 shaders are translated at build time (XenosRecomp, then `dxc`) into
  `bo2zm_shaders.spvpack`. An index built on the first start lets each shader be read on first use. Pipelines are
  compiled on a thread and cached next to the game, and the saved list is precompiled when the build or driver changes.
- **Texture pool**: large blocks with one GPU mapping per block and memory kind, sized for the 64 KB unit each texture
  takes, evicted in small batches.
- **GPU skinning**: the game's CPU skinning jobs become compute dispatches.
- **Direct3D layer**: register flushes become markers decoded in blocks, and four Direct3D functions run natively. A
  test mode, the D3D shadow, compares each draw's state with the game's Direct3D side.
- **Driver**: NVK, built for Horizon by mesa-switch, with this project's patch (state fingerprints, copy-engine copies,
  kind views, a leak fix).
- **Output**: the presenter scales the 1280x720 image to the screen and draws the FPS counter and credits on top;
  VSYNC follows the display's refresh signal.

On the PC the SDK's exact Xenos emulation (Direct3D 12) is the reference image every renderer change is checked
against.

## 7. The Switch layer

In the SDK's `*_switch.*` files and `app/src/bo2zm_switch.cpp`:

- start-up: paths, performance mode, network, the exception handler;
- cores: core 3 placement and its failsafe (`sdk/src/ui/switch_core3.cpp`);
- clocks: never set, only read for the logs;
- display: the window, VSYNC, docked and handheld;
- relaunch: the game's mode switches restart the NRO with the launch data carried across;
- logs: written by their own thread; warnings only for players, the full log for tests.

## 8. The game's own systems

- **Menus** are HavokScript (Lua 5.1) in the fastfiles; the game's own Lua compiler builds `app/src/ui/menus.lua` at
  start to add the port's pages. `tools/inspect/hks_dis.py` reads the game's menus.
- **Settings**: presets set the game's own dvars, and hooks hold the ones it forces back every frame.
- **Cheats and Extras** drive the game's script engine (notifies, function calls, field reads) on the Server thread,
  right after its frame, where the game runs its own scripts.
- **DLC** packs are mounted as the 360 did.
