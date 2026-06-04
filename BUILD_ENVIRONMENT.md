# VideoPlayer - 编译环境与依赖文档

## 目录

1. [编译工具链](#1-编译工具链)
2. [编译器配置](#2-编译器配置)
3. [编译依赖](#3-编译依赖)
4. [编译流程](#4-编译流程)
5. [路径配置](#5-路径配置)
6. [故障排除](#6-故障排除)

---

## 1. 编译工具链

### 1.1 编译器信息

| 项目 | 说明 |
|------|------|
| **编译器** | Microsoft Visual C++ (MSVC) |
| **版本** | 19.51.36243 (Visual Studio 2026) |
| **架构** | x64 (64-bit) |
| **命令行工具** | `cl.exe` |

### 1.2 工具链路径

| 路径 | 说明 |
|------|------|
| `G:\vs\2022_community\VC\Auxiliary\Build\vcvarsall.bat` | MSVC 环境变量配置脚本 |
| `G:\vs\2022_community\VC\Tools\MSVC\14.51.36243\bin\Hostx64\x64\cl.exe` | x64 编译器 |
| `G:\vs\2022_community\VC\Tools\MSVC\14.51.36243\include` | C/C++ 标准库头文件 |
| `G:\vs\2022_community\VC\Tools\MSVC\14.51.36243\lib\x64` | 链接库 |

---

## 2. 编译器配置

### 2.1 环境变量设置

编译前需运行 Visual Studio 的环境配置脚本：

```batch
call "G:\vs\2022_community\VC\Auxiliary\Build\vcvarsall.bat" x64
```

此脚本会设置以下关键环境变量：

| 环境变量 | 说明 |
|----------|------|
| `PATH` | 添加编译器、链接器路径 |
| `INCLUDE` | 添加头文件搜索路径 |
| `LIB` | 添加库文件搜索路径 |
| `CL` | 编译器默认选项 |

### 2.2 编译命令

```batch
cl.exe /EHsc /Fe:bin\VideoPlayer.exe src\VideoPlayer.cpp ^
    /I"include" ^
    /link lib\%LIB_DIR%\libmpv.dll.a Shlwapi.lib user32.lib delayimp.lib ^
    /DELAYLOAD:libmpv-2.dll /SUBSYSTEM:WINDOWS
```

**编译参数说明**：

| 参数 | 说明 |
|------|------|
| `/EHsc` | 启用 C++ 异常处理 |
| `/Fe:bin\VideoPlayer.exe` | 指定输出文件路径 |
| `/I"include"` | 添加头文件搜索路径 |
| `/link` | 链接器选项 |
| `/DELAYLOAD:libmpv-2.dll` | 延迟加载 mpv DLL |
| `/SUBSYSTEM:WINDOWS` | 构建 Windows 应用程序 |

---

## 3. 编译依赖

### 3.1 头文件依赖

| 路径 | 说明 | 来源 |
|------|------|------|
| `include/mpv/client.h` | mpv 客户端 API 头文件 | mpv 库 |
| `include/mpv/render.h` | mpv 渲染 API 头文件 | mpv 库 |
| `include/mpv/render_gl.h` | OpenGL 渲染头文件 | mpv 库 |

### 3.2 链接库依赖

| 库文件 | 说明 | 路径 |
|--------|------|------|
| `libmpv.dll.a` | mpv 导入库 | `lib/lib_win7/` 或 `lib/lib_win10/` |
| `Shlwapi.lib` | Windows Shell API | MSVC 标准库 |
| `user32.lib` | Windows 用户界面 API | MSVC 标准库 |
| `delayimp.lib` | 延迟加载支持 | MSVC 标准库 |

### 3.3 运行时 DLL 依赖

**Windows 7 版本** (`lib/lib_win7/`)：

| DLL 文件 | 说明 | 大小 |
|----------|------|------|
| `libmpv-2.dll` | mpv 主库 (Win7 兼容版) | ~15 MB |
| `mpv-2.dll` | mpv 客户端库 | ~2 MB |
| `avcodec-62.dll` | FFmpeg 编解码 | ~30 MB |
| `avformat-62.dll` | FFmpeg 格式处理 | ~10 MB |
| `avutil-60.dll` | FFmpeg 工具库 | ~2 MB |
| `swscale-9.dll` | FFmpeg 图像缩放 | ~2 MB |
| `swresample-6.dll` | FFmpeg 音频重采样 | ~1 MB |
| `libass-9.dll` | 字幕渲染 | ~1 MB |
| `libfreetype-6.dll` | 字体渲染 | ~1 MB |
| `libharfbuzz-0.dll` | 文本排版 | ~2 MB |
| `libdav1d.dll` | AV1 解码 | ~5 MB |
| `libplacebo-358.dll` | 着色器处理 | ~3 MB |
| `vulkan-1.dll` | Vulkan 图形 API | ~1 MB |

**Windows 10+ 版本** (`lib/lib_win10/`)：

| DLL 文件 | 说明 | 大小 |
|----------|------|------|
| `libmpv-2.dll` | mpv 主库 (Win10+ 优化版) | ~18 MB |
| `mpv-2.dll` | mpv 客户端库 | ~2 MB |
| `avcodec-62.dll` | FFmpeg 编解码 (优化版) | ~35 MB |
| `avformat-62.dll` | FFmpeg 格式处理 | ~12 MB |
| `avutil-60.dll` | FFmpeg 工具库 | ~2 MB |
| `swscale-9.dll` | FFmpeg 图像缩放 | ~2 MB |
| `swresample-6.dll` | FFmpeg 音频重采样 | ~1 MB |
| `libass-9.dll` | 字幕渲染 | ~1 MB |
| `libfreetype-6.dll` | 字体渲染 | ~1 MB |
| `libharfbuzz-0.dll` | 文本排版 | ~2 MB |
| `libdav1d.dll` | AV1 解码 | ~6 MB |
| `libplacebo-358.dll` | 着色器处理 | ~4 MB |
| `vulkan-1.dll` | Vulkan 图形 API | ~1 MB |

---

## 4. 编译流程

### 4.1 完整编译步骤

```
┌─────────────────────────────────────────────────────────┐
│                    编译流程                             │
├─────────────────────────────────────────────────────────┤
│                                                        │
│  1. 运行 vcvarsall.bat                                 │
│     │                                                  │
│     ▼                                                  │
│  2. 设置环境变量 (PATH, INCLUDE, LIB)                  │
│     │                                                  │
│     ▼                                                  │
│  3. 编译 src/VideoPlayer.cpp                           │
│     │                                                  │
│     ▼                                                  │
│  4. 链接 libmpv.dll.a + 系统库                         │
│     │                                                  │
│     ▼                                                  │
│  5. 生成 bin/VideoPlayer.exe                           │
│                                                        │
└─────────────────────────────────────────────────────────┘
```

### 4.2 脚本执行

**默认编译（Win10+）**：
```batch
compile.bat
```

**Win7 兼容编译**：
```batch
compile.bat win7
```

---

## 5. 路径配置

### 5.1 项目路径结构

```
ltest/
├── src/                    # 源代码目录
│   └── VideoPlayer.cpp     # 主程序源文件
├── include/                # 头文件目录
│   └── mpv/               # mpv 头文件
├── lib/                   # 库文件目录
│   ├── lib_win7/          # Win7 兼容库
│   │   └── libmpv.dll.a   # Win7 导入库
│   └── lib_win10/         # Win10+ 优化库
│       └── libmpv.dll.a   # Win10+ 导入库
├── bin/                   # 输出目录
│   ├── VideoPlayer.exe    # 编译产物
│   └── *.dll              # 运行时依赖
└── compile.bat            # 编译脚本
```

### 5.2 路径变量说明

| 变量 | 默认值 | 说明 |
|------|--------|------|
| `LIB_DIR` | `lib_win10` | 库目录选择 |
| `INCLUDE` | `include` | 头文件路径 |
| `OUTPUT` | `bin/VideoPlayer.exe` | 输出文件 |

---

## 6. 故障排除

### 6.1 常见问题

| 错误 | 原因 | 解决方案 |
|------|------|----------|
| `'cl' is not recognized` | 未运行 vcvarsall.bat | 运行环境配置脚本 |
| `cannot open include file: 'mpv/client.h'` | 头文件路径错误 | 检查 /I"include" 参数 |
| `cannot open input file 'libmpv.dll.a'` | 库文件缺失 | 使用 Git LFS 拉取 |
| `LINK : fatal error LNK1104` | 链接器找不到库 | 检查 LIB 环境变量 |
| `libmpv-2.dll missing` | 运行时 DLL 缺失 | 复制 DLL 到 bin/ 目录 |

### 6.2 环境检查命令

```batch
# 检查编译器版本
cl.exe /?

# 检查环境变量
echo %INCLUDE%
echo %LIB%
echo %PATH%

# 检查 vcvarsall.bat 路径
dir "G:\vs\2022_community\VC\Auxiliary\Build\vcvarsall.bat"
```

---

## 附录

### A. 编译脚本完整内容

```batch
@echo off
if "%1"=="win7" (
    set "LIB_DIR=lib_win7"
) else (
    set "LIB_DIR=lib_win10"
)

call "G:\vs\2022_community\VC\Auxiliary\Build\vcvarsall.bat" x64
cl.exe /EHsc /Fe:bin\VideoPlayer.exe src\VideoPlayer.cpp ^
    /I"include" ^
    /link lib\%LIB_DIR%\libmpv.dll.a Shlwapi.lib user32.lib delayimp.lib ^
    /DELAYLOAD:libmpv-2.dll /SUBSYSTEM:WINDOWS
echo Build completed!
```

### B. 系统要求

| 要求 | 说明 |
|------|------|
| **操作系统** | Windows 10/11 (开发) / Windows 7 (运行) |
| **Visual Studio** | 2022 或更高版本 |
| **工作负载** | Desktop development with C++ |
| **Windows SDK** | 10.0.19041.0 或更高 |
| **Git LFS** | 用于下载二进制文件 |

---

*文档版本: 1.0*  
*最后更新: 2026-06-04*  
*项目: VideoPlayer*
