# 本轮记录（PC 版推进）

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