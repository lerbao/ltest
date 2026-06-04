#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Trae智能自动配置系统
自动分析问题文档，在新目录自动完成所有配置

使用方式：
1. 在新目录克隆仓库后，Trae自动运行此脚本
2. 脚本自动分析问题文档，识别需要执行的操作
3. 自动执行所有必要的配置步骤
"""

import os
import sys
import subprocess
import shutil
import time

def read_problem_docs():
    """读取所有问题文档，提取需要执行的操作"""
    docs = []
    doc_files = [
        "SSH_PUSH_TROUBLESHOOTING.md",
        "PROBLEM_SUMMARY.md",
        "COMPILATION_GUIDE.md",
        "GIT_CONFIG.md"
    ]
    
    for doc in doc_files:
        if os.path.exists(doc):
            with open(doc, 'r', encoding='utf-8') as f:
                docs.append(f.read())
    
    return docs

def analyze_problems(docs):
    """分析问题文档，生成需要执行的操作列表"""
    actions = []
    
    for doc in docs:
        # 分析SSH相关问题
        if "SSH" in doc or "ssh" in doc.lower():
            if "代理" in doc or "agent" in doc.lower():
                actions.append(("ssh_agent", "启动SSH代理"))
            if "密钥" in doc or "key" in doc.lower():
                actions.append(("ssh_key", "添加SSH密钥"))
            if "配置" in doc or "config" in doc.lower():
                actions.append(("ssh_config", "配置SSH环境变量"))
        
        # 分析编译相关问题
        if "编译" in doc or "compile" in doc.lower():
            actions.append(("compile", "编译项目"))
        
        # 分析DLL相关问题
        if "DLL" in doc or "dll" in doc.lower():
            if "缺失" in doc or "missing" in doc.lower():
                actions.append(("dll_check", "检查DLL文件"))
        
        # 分析Git相关问题
        if "git" in doc.lower():
            if "push" in doc.lower():
                actions.append(("git_push", "配置Git推送"))
    
    return list(set(actions))

def execute_action(action_code, action_desc):
    """执行单个操作"""
    print(f"📋 执行操作: {action_desc}")
    
    try:
        if action_code == "ssh_config":
            # 配置SSH环境变量
            os.environ['PATH'] = "G:\\Git\\bin;G:\\Git\\usr\\bin;" + os.environ['PATH']
            os.environ['GIT_SSH'] = "G:\\Git\\usr\\bin\\ssh.exe"
            print("✅ SSH环境变量配置完成")
        
        elif action_code == "ssh_agent":
            # 启动SSH代理
            result = subprocess.run(
                ["ssh-agent", "-s"],
                capture_output=True,
                text=True
            )
            for line in result.stdout.split('\n'):
                if 'SSH_AUTH_SOCK=' in line:
                    os.environ['SSH_AUTH_SOCK'] = line.split('=')[1].strip(';')
                if 'SSH_AGENT_PID=' in line:
                    os.environ['SSH_AGENT_PID'] = line.split('=')[1].strip(';')
            print("✅ SSH代理启动完成")
        
        elif action_code == "ssh_key":
            # 添加SSH密钥
            subprocess.run(
                ["ssh-add", os.path.expanduser("~/.ssh/id_ed25519")],
                capture_output=True
            )
            print("✅ SSH密钥添加完成")
        
        elif action_code == "dll_check":
            # 检查DLL文件
            dll_paths = [
                "lib/lib_win7/libmpv-2.dll",
                "lib/lib_win10/libmpv-2.dll"
            ]
            for dll in dll_paths:
                if os.path.exists(dll):
                    print(f"✅ {dll} 存在")
                else:
                    print(f"❌ {dll} 缺失")
        
        elif action_code == "compile":
            # 编译项目
            subprocess.run(
                ["powershell", "-Command", ".\\compile.bat win7"],
                capture_output=True,
                text=True
            )
            if os.path.exists("bin/VideoPlayer.exe"):
                print("✅ 项目编译成功")
            else:
                print("❌ 项目编译失败")
        
        elif action_code == "git_push":
            # 配置Git推送
            subprocess.run(
                ["git", "config", "--global", "url.git@github.com:.insteadOf", "https://github.com/"],
                capture_output=True
            )
            subprocess.run(
                ["git", "config", "--global", "url.git@gitee.com:.insteadOf", "https://gitee.com/"],
                capture_output=True
            )
            print("✅ Git推送配置完成")
        
        return True
    except Exception as e:
        print(f"❌ 操作失败: {e}")
        return False

def auto_setup():
    """自动配置主函数"""
    print("🚀 Trae智能自动配置系统启动")
    print("=" * 50)
    
    # 1. 读取问题文档
    print("\n📚 正在分析问题文档...")
    docs = read_problem_docs()
    print(f"✅ 读取了 {len(docs)} 个文档")
    
    # 2. 分析需要执行的操作
    print("\n🔍 正在分析需要执行的操作...")
    actions = analyze_problems(docs)
    print(f"✅ 识别到 {len(actions)} 个需要执行的操作")
    
    # 3. 执行所有操作
    print("\n⚙️ 正在执行配置操作...")
    success_count = 0
    for action_code, action_desc in actions:
        if execute_action(action_code, action_desc):
            success_count += 1
        time.sleep(0.5)
    
    # 4. 输出结果
    print("\n" + "=" * 50)
    print(f"🎉 自动配置完成!")
    print(f"成功: {success_count}/{len(actions)}")
    
    if success_count == len(actions):
        print("\n✅ 所有配置已完成，可以开始使用项目")
        print("运行命令: .\\bin\\VideoPlayer.exe")
    else:
        print("\n⚠️ 部分配置未完成，请检查错误信息")
    
    return success_count == len(actions)

if __name__ == "__main__":
    success = auto_setup()
    sys.exit(0 if success else 1)
