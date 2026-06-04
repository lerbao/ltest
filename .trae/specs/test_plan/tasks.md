# Windows媒体播放器项目 - 测试计划

## [x] Task 1: 检查项目构建状态
- **Priority**: P0
- **Depends On**: None
- **Description**: 
  - 检查可执行文件是否存在
  - 验证MPV库是否已正确配置
  - 检查Python依赖是否已安装
- **Acceptance Criteria Addressed**: AC-1
- **Test Requirements**:
  - `programmatic` TR-1.1: 验证dist/VideoPlayer.exe存在 ✓
  - `programmatic` TR-1.2: 验证lib目录下存在mpv-2.dll ✓
  - `human-judgement` TR-1.3: 检查requirements.txt依赖状态 ✓ (PyQt5, python-mpv已安装)
- **Notes**: MPV运行时库已正确配置，可正常工作

## [x] Task 2: 测试Python版本播放器启动
- **Priority**: P0
- **Depends On**: Task 1
- **Description**: 
  - 运行Python版本播放器
  - 验证启动是否成功
  - 检查界面布局是否正常
- **Acceptance Criteria Addressed**: AC-1
- **Test Requirements**:
  - `human-judgement` TR-2.1: 播放器窗口应在5秒内显示 ✓
  - `human-judgement` TR-2.2: UI元素（播放按钮、进度条、音量控制）应正常显示 ✓
  - `human-judgement` TR-2.3: 菜单功能应正常可用 ✓
- **Notes**: MPV库正常工作，播放器启动成功

## [x] Task 3: 测试已构建的可执行文件
- **Priority**: P0
- **Depends On**: Task 1
- **Description**: 
  - 运行dist/VideoPlayer.exe
  - 验证启动是否成功
  - 检查界面布局是否正常
- **Acceptance Criteria Addressed**: AC-1
- **Test Requirements**:
  - `human-judgement` TR-3.1: 播放器窗口应在5秒内显示 ✓
  - `human-judgement` TR-3.2: UI元素应正常显示 ✓
  - `human-judgement` TR-3.3: 菜单功能应正常可用 ✓
- **Notes**: 可执行文件成功启动

## [x] Task 4: 测试播放控制功能
- **Priority**: P1
- **Depends On**: Task 2 或 Task 3
- **Description**: 
  - 打开一个测试视频文件
  - 测试播放/暂停/停止功能
  - 测试快进/快退功能
- **Acceptance Criteria Addressed**: AC-2, AC-3, AC-6
- **Test Requirements**:
  - `human-judgement` TR-4.1: 点击播放按钮视频应开始播放 ✓
  - `human-judgement` TR-4.2: 点击暂停按钮视频应暂停 ✓
  - `human-judgement` TR-4.3: 点击停止按钮视频应停止并重置 ✓
  - `human-judgement` TR-4.4: 点击快进按钮应前进10秒 ✓
  - `human-judgement` TR-4.5: 点击快退按钮应后退10秒 ✓

## [x] Task 5: 测试进度和音量控制
- **Priority**: P1
- **Depends On**: Task 4
- **Description**: 
  - 测试进度条拖动功能
  - 测试时间显示更新
  - 测试音量控制
- **Acceptance Criteria Addressed**: AC-4, AC-5
- **Test Requirements**:
  - `human-judgement` TR-5.1: 拖动进度条视频应跳转到对应时间点 ✓
  - `human-judgement` TR-5.2: 时间显示应实时更新 ✓
  - `human-judgement` TR-5.3: 调整音量滑块音量应变化 ✓

## [x] Task 6: 测试播放完成和错误处理
- **Priority**: P1
- **Depends On**: Task 4
- **Description**: 
  - 测试视频播放完成后的行为
  - 测试打开无效文件的错误处理
- **Acceptance Criteria Addressed**: AC-7, AC-8
- **Test Requirements**:
  - `human-judgement` TR-6.1: 视频播放完成后应停止并重置状态 ✓
  - `human-judgement` TR-6.2: 打开无效文件应显示错误提示 ✓

## [x] Task 7: 测试键盘快捷键
- **Priority**: P2
- **Depends On**: Task 2 或 Task 3
- **Description**: 
  - 测试Ctrl+O打开文件
  - 测试Ctrl+P播放
  - 测试Ctrl+S暂停
  - 测试Ctrl+X停止
  - 测试Ctrl+Q退出
- **Acceptance Criteria Addressed**: AC-2
- **Test Requirements**:
  - `human-judgement` TR-7.1: Ctrl+O应打开文件选择对话框 ✓
  - `human-judgement` TR-7.2: Ctrl+P应开始播放 ✓
  - `human-judgement` TR-7.3: Ctrl+S应暂停播放 ✓
  - `human-judgement` TR-7.4: Ctrl+X应停止播放 ✓
  - `human-judgement` TR-7.5: Ctrl+Q应退出程序 ✓
