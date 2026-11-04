# 本轮记录（PC 版推进）

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


时间：2026-10-31

## 做成了

1. rexglue codegen 成功：D:\devkitPro\rexglue-projects\cod6\generated\ 共 131 个文件
   （60 个 cod6_recomp.*.cpp + 60 个 funcs 头 + init/register + sources.cmake + rexglue.cmake）
2. **卡了两天的根因找到了：中文路径**。rexglue.exe 读不了
   D:\主机掌机\...\game_root，换成 D:\cod6data（junction，ASCII）后 6 秒跑完。
3. 工具链全部就位（只是不在 PATH）：
   - cl.exe: C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64\
   - cmake:  ...\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe
   - ninja:  ...\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe
   - WSL:    /usr/bin/{cmake,ninja,clang++,g++}
   - SDL3:   D:\devkitPro\rexglue\win-amd64\lib\SDL3-static.lib + cmake\SDL3Config.cmake

## 没做成

- 
exglue init 生成工程模板：**失败且无报错**（share 目录里没有模板）。
  所以工程根目录没有 CMakeLists.txt。

## 下一步

手写 CMakeLists.txt（用 generated/rexglue.cmake + generated/default/sources.cmake），
然后 cmake 配置 + 编译。或者从 Switch 工程的 CMakeLists.txt 改一份。

## 需要的开关

PC 版运行：cod6.exe --game_data_root D:\cod6data