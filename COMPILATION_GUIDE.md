# VideoPlayer 编译依赖说明

## 编译环境要求

### 1. 操作系统
- Windows 10 / Windows 11 (开发环境)
- Windows 7 (运行环境，需使用 Win7 兼容的 DLL)

### 2. 开发工具
- **Visual Studio 2022** (Community 或更高版本)
  - 安装工作负载："Desktop development with C++"
  - 确保安装了 Windows SDK (推荐版本 10.0.19041.0 或更高)

### 3. 编译步骤

#### 方法一：使用 Visual Studio
1. 打开 `VideoPlayer.sln`
2. 选择目标平台 (建议 x64)
3. 点击 "生成" -> "生成解决方案"

#### 方法二：使用批处理脚本
```batch
# 确保已配置 Visual Studio 环境变量
# 运行 compile.bat
compile.bat
```

### 4. 运行依赖

#### 运行时 DLL 依赖 (已包含在 bin/ 目录)
- mpv-2.dll - 主媒体播放库
- libmpv-2.dll - mpv 客户端库
- avcodec-62.dll - FFmpeg 编解码库
- avformat-62.dll - FFmpeg 格式处理库
- avutil-60.dll - FFmpeg 工具库
- avfilter-11.dll - FFmpeg 滤镜库
- avdevice-62.dll - FFmpeg 设备库
- swscale-9.dll - FFmpeg 缩放库
- swresample-6.dll - FFmpeg 重采样库
- libass-9.dll - 字幕渲染库
- libfreetype-6.dll - 字体渲染库
- libharfbuzz-0.dll - 文本排版库
- libfribidi-0.dll - 双向文本处理库
- libdav1d.dll - AV1 解码库
- libplacebo-358.dll - 着色器处理库
- libshaderc_shared.dll - GLSL 编译器
- libspirv-cross-c-shared.dll - SPIR-V 转换库
- liblcms2.dll - 颜色管理库
- libiconv-2.dll - 字符编码转换
- libgcc_s_seh-1.dll - GCC 运行时
- libstdc++-6.dll - C++ 标准库
- libwinpthread-1.dll - POSIX 线程库
- libssp-0.dll - 栈保护库
- vulkan-1.dll - Vulkan 图形 API
- zlib1.dll - 压缩库

### 5. 视频文件
- 将视频文件放入 `video/` 目录
- 支持格式：MP4, MKV, AVI, MOV 等 (mpv 支持的所有格式)

### 6. 注意事项

#### Windows 7 兼容性
- 使用 `lib/lib_win7/` 目录下的 DLL 替换 `bin/` 目录中的对应文件
- Win7 不支持某些高级特性，可能需要降低视频分辨率

#### Git LFS
- 大文件 (DLL, EXE) 使用 Git LFS 管理
- 克隆仓库时需安装 Git LFS：`git lfs install`

### 7. 故障排除

**问题：无法找到 mpv-2.dll**
- 确保 `bin/` 目录与可执行文件同目录
- 或添加 `bin/` 目录到系统 PATH

**问题：编译时找不到 mpv 头文件**
- 确保 include 目录路径正确配置
- 检查项目属性中的附加包含目录

**问题：运行时黑屏**
- 检查视频文件路径是否正确
- 确保视频文件格式被支持
- 尝试使用软件解码模式

---

*最后更新: 2026-06-03*