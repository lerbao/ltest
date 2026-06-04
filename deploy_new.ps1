# 一键部署脚本 - 在新目录快速部署项目

param(
    [string]$TargetDir = "E:\VideoPlayer_New"
)

Write-Host "🚀 开始部署项目到: $TargetDir"

# 1. 创建目录
New-Item -ItemType Directory -Path $TargetDir -Force | Out-Null
Set-Location $TargetDir

# 2. 配置SSH环境
Write-Host "`n📦 配置SSH环境..."
$env:Path = "G:\Git\bin;G:\Git\usr\bin;" + $env:Path
$env:GIT_SSH = "G:\Git\usr\bin\ssh.exe"

# 启动SSH代理
try {
    $sock = Get-ChildItem -Path "/d/Cadence/Spb_data/.ssh/agent/" -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($sock) {
        $env:SSH_AUTH_SOCK = "/d/Cadence/Spb_data/.ssh/agent/" + $sock.Name
        Write-Host "✅ SSH代理已运行"
    } else {
        $agentOutput = ssh-agent -s
        foreach ($line in $agentOutput) {
            if ($line -match 'SSH_AUTH_SOCK=(.+?);') { $env:SSH_AUTH_SOCK = $matches[1] }
            if ($line -match 'SSH_AGENT_PID=(\d+);') { $env:SSH_AGENT_PID = $matches[1] }
        }
        Write-Host "✅ 启动SSH代理"
    }
    ssh-add "$env:USERPROFILE\.ssh\id_ed25519" 2>&1 | Out-Null
    Write-Host "✅ 加载SSH密钥"
} catch {
    Write-Host "⚠️ SSH配置警告: $_"
}

# 3. 克隆仓库
Write-Host "`n📥 克隆仓库..."
if (Test-Path ".git") {
    Write-Host "⚠️ 目录已存在，执行pull更新"
    git pull origin master
} else {
    git clone git@github.com:lerbao/ltest.git .
}

# 4. 编译项目
Write-Host "`n🔧 编译项目..."
& "G:\vs\2022_community\VC\Auxiliary\Build\vcvarsall.bat" x64
.\compile.bat win7

# 5. 验证编译结果
Write-Host "`n✅ 验证部署结果..."
if (Test-Path "bin/VideoPlayer.exe") {
    Write-Host "✅ VideoPlayer.exe 编译成功"
} else {
    Write-Host "❌ 编译失败"
    exit 1
}

if (Test-Path "lib/lib_win7/libmpv-2.dll") {
    Write-Host "✅ Win7 DLL 已存在"
} else {
    Write-Host "❌ Win7 DLL 缺失"
    exit 1
}

Write-Host "`n🎉 部署完成！项目已准备就绪"
Write-Host "运行命令: .\bin\VideoPlayer.exe"
