# Git仓库配置信息

## 仓库信息

### GitHub

| 项目 | 值 |
|------|-----|
| **仓库地址 (SSH)** | `git@github.com:lerbao/ltest.git` |
| **仓库地址 (HTTPS)** | `https://github.com/lerbao/ltest.git` |
| **用户名** | `lerbao` |
| **邮箱** | `2080883448@qq.com` |
| **主分支** | `master` |

### Gitee

| 项目 | 值 |
|------|-----|
| **仓库地址 (SSH)** | `git@gitee.com:er_baocom/video-player.git` |
| **仓库地址 (HTTPS)** | `https://gitee.com/er_baocom/video-player.git` |
| **用户名** | `lerbao` (@er_baocom) |
| **邮箱** | `2080883448@qq.com` |
| **主分支** | `master` |

## SSH密钥配置

### 密钥信息

| 项目 | 值 |
|------|-----|
| **密钥算法** | ed25519 |
| **密钥路径** | `C:\Users\20808\.ssh\id_ed25519` |
| **公钥路径** | `C:\Users\20808\.ssh\id_ed25519.pub` |
| **公钥内容** | `ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIKqAy1h6d32LhFPkU7a8xyxlP8d18/rZnQExOq3FdGeA 2080883448@qq.com` |

### SSH配置文件 (`~/.ssh/config`)

```ssh-config
Host gitee.com
  IdentityFile C:\Users\20808\.ssh\id_ed25519
  StrictHostKeyChecking no

Host github.com
  IdentityFile C:\Users\20808\.ssh\id_ed25519
  StrictHostKeyChecking no
```

## Git全局配置

```bash
# 用户信息
git config --global user.name "lerbao"
git config --global user.email "2080883448@qq.com"

# SSH优先（自动将HTTPS转换为SSH）
git config --global url."git@github.com:".insteadOf "https://github.com/"
git config --global url."git@gitee.com:".insteadOf "https://gitee.com/"
```

## 推送命令

### 标准推送（推送到默认远程仓库）
```bash
git push origin master
```

### 推送到GitHub
```bash
git push git@github.com:lerbao/ltest.git HEAD:master
```

### 推送到Gitee
```bash
git push git@gitee.com:er_baocom/video-player.git HEAD:master
```

### 强制推送（覆盖远程分支）
```bash
git push git@github.com:lerbao/ltest.git HEAD:master --force
git push git@gitee.com:er_baocom/video-player.git HEAD:master --force
```

## 自动化脚本

### 自动配置SSH
```powershell
# 运行配置脚本
.\setup_ssh.ps1

# 或使用批处理脚本
.\git-push-ssh.cmd <远程仓库> <分支>
```

### Git钩子（自动配置）
- **文件位置**：`.git/hooks/pre-push`
- **作用**：每次推送前自动配置SSH环境

## 分支管理策略

| 分支 | 用途 |
|------|------|
| `master` | 主分支，稳定版本 |
| `main` | 备用分支（与master内容一致） |
| `feature/*` | 功能开发分支（按需创建） |

**下次提交默认推送到**：`master` 分支

## 同步状态

| 仓库 | 状态 | 最新提交 |
|------|------|----------|
| GitHub (`lerbao/ltest`) | ✅ 已同步 | `f38acfd` |
| Gitee (`er_baocom/video-player`) | ✅ 已同步 | `f38acfd` |

## 注意事项

1. **SSH代理**：每次新终端会话需要启动SSH代理并添加密钥
2. **密钥安全**：不要将私钥文件提交到仓库
3. **分支一致性**：保持GitHub和Gitee的`master`分支同步
4. **大文件处理**：使用Git LFS处理大文件（如DLL、EXE）
