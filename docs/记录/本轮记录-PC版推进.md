# 本轮记录（PC 版推进）

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

- exglue init 生成工程模板：**失败且无报错**（share 目录里没有模板）。
  所以工程根目录没有 CMakeLists.txt。

## 下一步

手写 CMakeLists.txt（用 generated/rexglue.cmake + generated/default/sources.cmake），
然后 cmake 配置 + 编译。或者从 Switch 工程的 CMakeLists.txt 改一份。

## 需要的开关

PC 版运行：cod6.exe --game_data_root D:\cod6data