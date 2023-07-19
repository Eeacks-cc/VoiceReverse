# VoiceReverse

通过按键控制“实时”倒放麦克风录制的声音，实现倒放聊天。

# 构建要求

* C++ 17
* Visual Studio 2022

# 操作系统

* Windows

# 已知问题

* 有时候waveInAddBuffer会因为尝试播放正在使用中的音频数据而导致异常

# 计划清单

* 保存设置，不需要每次都初始化再配置

# 使用指南

1. 从Release中下载最新的版本
2. 安装 VB Cable 驱动 -> [VB-Audio](https://vb-audio.com/Cable/)
3. 运行本软件
4. 选择输入设备，一般为电脑的麦克风设备
5. 选择输出设备，一般设置为VB Cable的输入
6. 设置倒放操作按键绑定
7. 在需要使用的软件中调用VB Cable的输出端作为音频输入
8. 在正常使用麦克风的过程中，麦克风输入通过本软件传输到VB Cable播放设备，并通过VB Cable重定向到输入设备，透传至需要使用麦克风的软件中
9. 按住倒放按键时，进行录音，松开按键时以倒放形式输出录制的声音片段
10. 开启loopback功能时，说话的同时会在本地实时环回麦克风的声音，但录制时loopback将禁用

#混音使用

可使用如[VoiceMeeter](https://vb-audio.com/Voicemeeter/banana.htm)的banana版本或者Potato版本，拥有两路或以上虚拟输入输出及混音能力的软件进行同时loopback及录制
思路：
1. 系统默认音频播放设备设置为VoiceMeeter虚拟输入线路1，将本软件的输出设备指定成虚拟输入线路2
2. VoiceMeeter中虚拟输入线路1的音频仅连接至物理输出设备（实际的播放设备）
3. VoiceMeeter中虚拟输入线路2的音频（本软件输出）同时连接到物理输出设备及虚拟输出设备
4. VoiceMeeter中物理输入线路的音频（实际麦克风—）仅连接到虚拟输出设备
5. 本软件的loopback关闭
6. 需要使用麦克风的软件调用VoiceMeeter的虚拟输出作为输入设备
7. 此时本软件待机及录制时不输出，倒放时通过VoiceMeeter仅loopback倒放的输出，且实际麦克风的语音保持输出，可实现正放倒放混说
