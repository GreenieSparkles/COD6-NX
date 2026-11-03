# 把源码装进 GitHub 仓库目录
#
# 用法（PowerShell）：
#   cd D:\Documents\deepseek-harness\default-workspace\build
#   .\assemble_repo.ps1
#
# 默认从 WSL 里的 /opt/switch/cod6-ng 取源码，装到
# D:\主机掌机\NSLITE\NSMW2\github\cod6-nx\。
#
# 只复制「应用层」：src/、CMakeLists.txt、cod6_manifest.toml、overrides.toml。
# 不复制：
#   generated/    —— 由你自己的游戏副本重编译产生，不进仓库
#   sdk/          —— 体积巨大（thirdparty 393 MB），按 README 作为依赖/子模块处理
#   out/  game_root/  cache/  user/  —— 构建产物与游戏数据

param(
    [string]$Source = '\\wsl.localhost\ubuntu24\opt\switch\cod6-ng',
    [string]$Repo   = 'D:\主机掌机\NSLITE\NSMW2\github\cod6-nx'
)

$ErrorActionPreference = 'Stop'

if (-not (Test-Path -LiteralPath $Repo)) {
    throw "仓库目录不存在: $Repo"
}
if (-not (Test-Path -LiteralPath $Source)) {
    throw "源码目录不可访问: $Source（WSL 是否启动？）"
}

Write-Host "源码: $Source"
Write-Host "目标: $Repo"

# 1) 应用层源码
$dstSrc = Join-Path $Repo 'src'
if (Test-Path -LiteralPath $dstSrc) { Remove-Item -LiteralPath $dstSrc -Recurse -Force }
Copy-Item -LiteralPath (Join-Path $Source 'src') -Destination $dstSrc -Recurse -Force
$n = (Get-ChildItem -LiteralPath $dstSrc -Recurse -File | Measure-Object).Count
Write-Host "  src/  $n 个文件"

# 2) 顶层构建文件
foreach ($f in @('CMakeLists.txt', 'cod6_manifest.toml', 'overrides.toml')) {
    $p = Join-Path $Source $f
    if (Test-Path -LiteralPath $p) {
        Copy-Item -LiteralPath $p -Destination (Join-Path $Repo $f) -Force
        Write-Host "  $f"
    }
}

# 3) 兼容头（PC 构建用）
$compat = Join-Path $Source 'compat_win'
if (Test-Path -LiteralPath $compat) {
    $dst = Join-Path $Repo 'compat_win'
    if (Test-Path -LiteralPath $dst) { Remove-Item -LiteralPath $dst -Recurse -Force }
    Copy-Item -LiteralPath $compat -Destination $dst -Recurse -Force
    Write-Host "  compat_win/"
}

Write-Host ""
Write-Host "完成。下一步："
Write-Host "  cd `"$Repo`""
Write-Host "  git init"
Write-Host "  git add ."
Write-Host "  git commit -m `"initial import`""
Write-Host "  git remote add origin <你的仓库地址>"
Write-Host "  git push -u origin main"
Write-Host ""
Write-Host "注意："
Write-Host "  - generated/ 与 sdk/ 不在仓库里；构建前需自行生成/放置，见 docs/BUILD.md"
Write-Host "  - 推送前确认没有 game_root/ 或任何 .ff/.bik 文件"
