# 下载 Windows(MSVC)构建所需的 SDL3 / SDL3_ttf 官方开发包,解压到 tools/win-deps/vc/。
# CMakeLists.txt 会自动把该目录加入 CMAKE_PREFIX_PATH,因此构建无需再传 -DCMAKE_PREFIX_PATH。
#
# 用法:
#   powershell -ExecutionPolicy Bypass -File tools/fetch-win-deps.ps1
#   powershell -ExecutionPolicy Bypass -File tools/fetch-win-deps.ps1 -Proxy http://127.0.0.1:7897
#
# 依赖:Windows PowerShell 5.1+(可访问 github.com;受限网络用 -Proxy 指定 HTTP 代理)。

param(
  [string]$Proxy = "",
  [string]$Dest = (Join-Path $PSScriptRoot "win-deps\vc")
)

$ErrorActionPreference = "Stop"
# Windows PowerShell 5.1 默认不启用 TLS 1.2,显式设置以免 GitHub 下载失败
[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12

$packages = @(
  @{
    Name = "SDL3-devel-3.2.10-VC.zip"
    Url  = "https://github.com/libsdl-org/SDL/releases/download/release-3.2.10/SDL3-devel-3.2.10-VC.zip"
    Dir  = "SDL3-3.2.10"
  },
  @{
    Name = "SDL3_ttf-devel-3.2.2-VC.zip"
    Url  = "https://github.com/libsdl-org/SDL_ttf/releases/download/release-3.2.2/SDL3_ttf-devel-3.2.2-VC.zip"
    Dir  = "SDL3_ttf-3.2.2"
  }
)

New-Item -ItemType Directory -Force $Dest | Out-Null
$tmp = Join-Path $Dest ".download"

foreach ($p in $packages) {
  $target = Join-Path $Dest $p.Dir
  if (Test-Path (Join-Path $target "cmake")) {
    Write-Host "[skip] $($p.Dir) 已存在"
    continue
  }
  New-Item -ItemType Directory -Force $tmp | Out-Null
  $zip = Join-Path $tmp $p.Name
  Write-Host "[get ] $($p.Name)"
  $req = @{ Uri = $p.Url; OutFile = $zip; UseBasicParsing = $true }
  if ($Proxy) { $req.Proxy = $Proxy }
  Invoke-WebRequest @req
  Write-Host "[unzip] $($p.Dir)"
  Expand-Archive -Path $zip -DestinationPath $Dest -Force
  Remove-Item $zip -Force
}

Remove-Item $tmp -Recurse -Force -ErrorAction SilentlyContinue
Write-Host '[done] 依赖已就位:' $Dest
Write-Host '       cmake -S . -B build -G "Visual Studio 18 2026" -A x64'
