@echo off
setlocal

:: 设置Git路径
set PATH=G:\Git\bin;G:\Git\usr\bin;%PATH%
set GIT_SSH=G:\Git\usr\bin\ssh.exe

:: 检查SSH代理
set AGENT_RUNNING=0
for /f "delims=" %%a in ('dir /b /a-d "D:\Cadence\Spb_data\.ssh\agent\" 2^>nul') do (
    set AGENT_SOCK=D:\Cadence\Spb_data\.ssh\agent\%%a
    set AGENT_RUNNING=1
    goto :agent_found
)
:agent_found

:: 如果代理未运行，启动代理
if %AGENT_RUNNING% equ 0 (
    for /f "delims=" %%a in ('ssh-agent -s') do (
        echo %%a | findstr "SSH_AUTH_SOCK" >nul && set "%%a"
        echo %%a | findstr "SSH_AGENT_PID" >nul && set "%%a"
    )
    echo ✅ 启动SSH代理
) else (
    set SSH_AUTH_SOCK=%AGENT_SOCK%
    echo ✅ SSH代理已运行
)

:: 添加SSH密钥
ssh-add "%USERPROFILE%\.ssh\id_ed25519" >nul 2>&1
echo ✅ 加载SSH密钥

:: 执行git push命令
git push %*

endlocal
