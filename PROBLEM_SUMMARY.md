# VideoPlayer 项目问题记录与解决方案

## 目录
1. [DLL 依赖问题](#1-dll-依赖问题)
2. [符号导入问题](#2-符号导入问题)
3. [路径计算问题](#3-路径计算问题)
4. [编译脚本问题](#4-编译脚本问题)
5. [Git 配置问题](#5-git-配置问题)
6. [SSH 连接问题](#6-ssh-连接问题)
7. [多线程竞态条件](#7-多线程竞态条件)
8. [Git LFS 配置](#8-git-lfs-配置)
9. [仓库推送问题](#9-仓库推送问题)

---

## 1. DLL 依赖问题

### 问题描述
Win7 上运行 `videoplay.exe` 报错："libmpv-2.dll没有被指定在Windows上运行"

### 原因分析
- `E:\0601\bin\libmpv-2.dll` 是 0 字节空文件
- 头文件中 `MPV_EXPORT` 宏定义为 `__declspec(dllexport)`（用于导出符号），与实际需要导入符号不符

### 解决方案
1. 删除空的 `libmpv-2.dll` 文件
2. 修改 `E:\0601\include\mpv\client.h` 中 `MPV_EXPORT` 为 `__declspec(dllimport)`
3. 确保使用正确版本的 DLL（Win7 需要特定版本）

### 预防措施
- 定期检查 DLL 文件完整性
- 使用 Git LFS 管理二进制文件，避免文件损坏
- 在编译脚本中添加 DLL 校验步骤

---

## 2. 符号导入问题

### 问题描述
"VideoPlayer.exe已停止工作"，运行时无法正确加载 mpv 库

### 原因分析
- DLL 依赖缺失
- 符号解析失败（头文件宏定义错误）
- 目录结构不匹配

### 解决方案
1. 修复符号定义（`dllexport` → `dllimport`）
2. 确保 DLL 路径正确
3. 复制完整依赖到执行目录

### 预防措施
- 构建脚本自动复制依赖文件
- 添加运行前依赖检查

---

## 3. 路径计算问题

### 问题描述
"NO video files found in video folder!"

### 原因分析
视频扫描路径计算错误，代码中误删一次 `PathRemoveFileSpecW` 调用，导致路径为 `E:\0601\bin\video\*`（不存在）

### 解决方案
恢复两次 `PathRemoveFileSpecW` 调用，确保视频路径正确指向 `E:\0601\video\*`

### 预防措施
- 添加路径调试日志
- 使用绝对路径配置
- 单元测试路径计算逻辑

---

## 4. 编译脚本问题

### 问题描述
编译脚本工作目录错误，生成的 exe 在错误位置

### 原因分析
`compile.bat` 中硬编码切换到 `D:\workspace\trae\day12` 目录

### 解决方案
移除错误的目录切换命令，在当前目录编译

### 预防措施
- 使用相对路径或动态获取当前目录
- 在脚本开头保存原始工作目录

---

## 5. Git 配置问题

### 问题描述
"Author identity unknown" (fatal: unable to auto-detect email address)

### 原因分析
未配置 Git 用户信息

### 解决方案
```bash
git config user.email "2080883448@qq.com"
git config user.name "lerbao"
```

### 预防措施
- 在项目初始化脚本中配置用户信息
- 使用全局 Git 配置

---

## 6. SSH 连接问题

### 问题描述
"Host key verification failed" / "Permission denied (publickey)"

### 原因分析
- SSH 连接时主机密钥验证失败
- SSH 密钥格式不正确（使用了高版本 OpenSSH 格式）
- 公钥未添加到 Gitee/GitHub 账户

### 解决方案
1. 使用 `ssh-keyscan gitee.com >> ~/.ssh/known_hosts` 添加主机密钥
2. 使用 ed25519 算法重新生成密钥：`ssh-keygen -t ed25519 -C "2080883448@qq.com"`
3. 将公钥添加到 Gitee/GitHub 账户
4. 配置 `GIT_SSH` 环境变量指向 Git 安装目录下的 ssh.exe

### 预防措施
- 使用标准 SSH 密钥格式
- 定期检查 SSH 配置
- 使用 HTTPS 协议作为备选方案

---

## 7. 多线程竞态条件

### 问题描述
执行 `videoplayer.exe` 大约运行 20 分钟后黑屏

### 原因分析
`EventThread` 和 `TimerThread` 同时操作 MPV 实例，导致使用已销毁的 mpv 句柄

### 解决方案
1. 添加原子变量 `g_isResetting`，在 `ResetMPV` 时设置标志
2. `EventThread` 检测到标志后等待
3. 使用临界区（CRITICAL_SECTION）保护共享资源访问

### 预防措施
- 使用线程安全的数据结构
- 添加线程同步机制
- 定期进行多线程测试

---

## 8. Git LFS 配置

### 问题描述
无法上传大文件（DLL, EXE）到远程仓库

### 原因分析
GitHub/Gitee 对大文件有限制（通常 100MB）

### 解决方案
1. 安装 Git LFS：`git lfs install`
2. 配置 `.gitattributes` 跟踪大文件：
   ```
   *.dll filter=lfs diff=lfs merge=lfs -text
   *.exe filter=lfs diff=lfs merge=lfs -text
   ```
3. 使用 `git add` 添加文件（自动使用 LFS）

### 预防措施
- 在项目初始化时配置 Git LFS
- 定期检查 `.gitattributes` 配置

---

## 9. 仓库推送问题

### 问题描述
推送代码到远程仓库失败或内容不完整

### 原因分析
- `.gitignore` 配置错误，排除了需要上传的文件
- 网络连接不稳定
- 分支保护设置阻止推送

### 解决方案
1. 检查并修复 `.gitignore`，确保 bin/ 和 lib/ 目录被正确包含
2. 使用 `git push -f` 强制推送（谨慎使用）
3. 分段推送大文件
4. 在网络稳定时进行推送

### 预防措施
- 定期检查 `.gitignore` 配置
- 使用可靠的网络连接
- 定期备份本地仓库

---

## 检查清单（编译前必读）

| 检查项 | 状态 | 说明 |
|--------|------|------|
| Visual Studio 2022 已安装 | ✅ | 需安装 C++ 工作负载 |
| Windows SDK 已安装 | ✅ | 版本 >= 10.0.19041.0 |
| Git LFS 已安装 | ✅ | `git lfs install` |
| DLL 文件完整 | ✅ | 检查 bin/ 目录 |
| 头文件宏定义正确 | ✅ | `MPV_EXPORT` 应为 `dllimport` |
| Git 用户信息已配置 | ✅ | user.email, user.name |
| 网络连接正常 | ✅ | 可访问 GitHub/Gitee |

---

*最后更新: 2026-06-03*
*文档版本: 1.0*