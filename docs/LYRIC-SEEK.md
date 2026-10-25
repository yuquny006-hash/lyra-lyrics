# 点击歌词定位与多设备投屏

电脑录音页的歌词列表、阅读画布、独立阅读窗和手机都能点击已标时的歌词。编辑/校准页单击仍用于选择，双击用于定位。纯歌词模式第一次点击恢复工具栏，再点击定位；拖动不会定位。

预览模式只移动提词时间，不播放音频。要移动 Cubase 的伴奏，需要安装 MIDI Remote 桥接：

1. 运行 loopMIDI，建立 `Lyra Control` 和 `Lyra Feedback` 两个端口，并保持 loopMIDI 运行。本机已创建这两个端口。
2. 将发行包 `integrations/cubase/AmuMusic/Lyra` 目录复制到“文档/Steinberg/Cubase/MIDI Remote/Driver Scripts/Local/AmuMusic/Lyra”。本机已复制。重新启动 Cubase 后，MIDI Remote 应识别 AmuMusic / Lyra；必要时手动选择输入 Lyra Control、输出 Lyra Feedback。
3. 将 Cubase 的主时间显示或辅助时间显示设为“秒”（00:00:00.000）。主显示可以继续保留小节，辅助显示设秒即可。插件使用绝对时间定位，不依赖速度或拍号。
4. 插件保持“跟随宿主”。连接未就绪会说明原因；正在录音写入时禁止跳转，先停止录音。播放/暂停状态由 Cubase 保持。

**Windows 必做端口设置（本机已完成）：** 在 Cubase「工作室 → 工作室设置 → MIDI 端口设置」中，找到 **输入 Lyra Feedback**，同时取消「可见」和「在 All MIDI Inputs 中」两个勾选。只取消「可见」仍可能被 Cubase 占用，插件就无法接收回控反馈。保留 **输入 Lyra Control** 和 **输出 Lyra Feedback** 给 AmuMusic / Lyra MIDI Remote 使用，不要把它们一起关闭。这是一次性设置。若仍失败，点击歌词后可检查 `%APPDATA%/AmuMusic/CueFlow/seek-diagnostic.txt`。

同一局域网内，多部手机/平板可扫描同一个二维码，无需逐台重新开启投屏。所有设备共用宿主位置；字号、横竖屏和排版各自独立。多个设备连续点击时按服务器收到的顺序处理，最后接受的定位生效。投屏关闭后旧二维码失效。这里只传歌词与控制，不传伴奏音频。

本次回控桥针对 Cubase / Nuendo MIDI Remote；其他宿主不能因支持 VST3 就自动获得回控。其他宿主仍可只读跟随，后续需适配各自控制接口。

官方接口参考：https://steinbergmedia.github.io/midiremote_api_doc/codedoc_api_reference/
