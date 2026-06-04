# SSH配置自动设置脚本
# 用于确保Trae能够自动处理SSH推送

# 设置环境变量
$env:Path = "G:\Git\bin;G:\Git\usr\bin;" + $env:Path
$env:GIT_SSH = "G:\Git\usr\bin\ssh.exe"

# 检查SSH代理是否运行
$agentRunning = $false
try {
    $sock = Get-ChildItem -Path "/d/Cadence/Spb_data/.ssh/agent/" -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($sock) {
        $env:SSH_AUTH_SOCK = "/d/Cadence/Spb_data/.ssh/agent/" + $sock.Name
        $env:SSH_AGENT_PID = $sock.Name.Split('.')[4]
        $agentRunning = $true
        Write-Host "✅ SSH代理已运行: $env:SSH_AUTH_SOCK"
    }
} catch {
    $agentRunning = $false
}

if (-not $agentRunning) {
    Write-Host "🔄 启动SSH代理..."
    $agentOutput = ssh-agent -s
    foreach ($line in $agentOutput) {
        if ($line -match 'SSH_AUTH_SOCK=(.+?);') {
            $env:SSH_AUTH_SOCK = $matches[1]
        }
        if ($line -match 'SSH_AGENT_PID=(\d+);') {
            $env:SSH_AGENT_PID = $matches[1]
        }
    }
    Write-Host "✅ SSH代理已启动: $env:SSH_AUTH_SOCK"
}

# 添加SSH密钥
try {
    $keyPath = "$env:USERPROFILE\.ssh\id_ed25519"
    if (Test-Path $keyPath) {
        ssh-add $keyPath 2>&1 | Out-Null
        Write-Host "✅ SSH密钥已加载"
    } else {
        Write-Host "❌ SSH密钥不存在: $keyPath"
    }
} catch {
    Write-Host "⚠️ 添加密钥时出现警告: $_"
}

# 验证Gitee连接
try {
    $result = ssh -T git@gitee.com 2>&1
    if ($result -match "successfully authenticated") {
        Write-Host "✅ Gitee SSH连接验证成功"
    } else {
        Write-Host "❌ Gitee SSH连接验证失败: $result"
    }
} catch {
    Write-Host "❌ Gitee连接失败: $_"
}

# 验证GitHub连接
try {
    $result = ssh -T git@github.com 2>&1
    if ($result -match "successfully authenticated") {
        Write-Host "✅ GitHub SSH连接验证成功"
    } else {
        Write-Host "❌ GitHub SSH连接验证失败: $result"
    }
} catch {
    Write-Host "❌ GitHub连接失败: $_"
}

Write-Host "`n🎉 SSH配置完成！现在可以使用git push命令推送代码了。"
