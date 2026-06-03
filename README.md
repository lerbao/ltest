# VideoPlayer - Windows 视频播放器

基于 mpv 库的高性能 Windows 视频播放器，支持 Windows 7 和 Windows 10。

## ✨ 功能特性

- ✅ 支持多种视频格式（MP4, MKV, AVI 等）
- ✅ 硬件解码加速（DXVA2/D3D11VA）
- ✅ 音视频同步播放
- ✅ Windows 7/10 双平台兼容
- ✅ 自动扫描 video 目录中的视频文件
- ✅ 简洁易用的界面

## 📁 项目结构

```
0601/
├── src/                    # 源代码
│   ├── VideoPlayer.cpp     # 播放器核心实现
│   └── ...
├── include/                # 头文件
│   └── mpv/                # mpv 库头文件
├── lib/                    # 依赖库（未提交）
│   ├── lib_win7/           # Windows 7 兼容库
│   └── lib_win10/          # Windows 10 库
├── bin/                    # 编译产物（未提交）
├── video/                  # 测试视频（未提交）
├── compile.bat             # 编译脚本
└── .gitignore              # Git 忽略配置
```

## 🚀 快速开始

### 环境要求

- Windows 7 或 Windows 10
- Visual Studio 2022
- Git（用于获取源码）

### 编译步骤

1. 打开命令提示符或 PowerShell
2. 进入项目目录：`cd 0601`
3. 运行编译脚本：`.\compile.bat win7`（Windows 7）或 `.\compile.bat win10`（Windows 10）
4. 编译产物在 `bin/` 目录中

### 运行播放器

```bash
# 将视频文件放入 video 目录
copy your_video.mp4 video/

# 运行播放器
bin\VideoPlayer.exe
```

## 🎮 操作说明

| 按键/操作 | 功能 |
|-----------|------|
| 鼠标点击 | 选择视频文件播放 |
| ESC | 退出程序 |

## 📝 技术栈

- **C/C++**: 核心播放器实现
- **mpv**: 多媒体播放库
- **Direct3D 11**: 视频渲染
- **Win32 API**: 窗口管理

## 📄 许可证

MIT License

---

*基于 mpv 库开发*
