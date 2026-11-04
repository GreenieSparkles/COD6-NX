# How bo2-zm-nx works

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
