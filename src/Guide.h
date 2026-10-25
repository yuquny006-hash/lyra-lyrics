#pragma once
#include "Lyrics.h"
namespace cue {
inline juce::String guide(int language){return U(language?R"GUIDE(Lyra / Amu music
Recording prompter · User guide

NEW · Click lyrics and multiple screens
Click a timed line on the recording workspace, independent reader or phone to locate it. In editing workspaces use double-click. Pure mode first tap restores controls. Preview only moves lyrics. Cubase audio seeking requires the Lyra MIDI Remote bridge: run loopMIDI with Lyra Control and Lyra Feedback ports, install integrations/cubase/AmuMusic/Lyra in Documents/Steinberg/Cubase/MIDI Remote/Driver Scripts/Local, and set Cubase primary or secondary time display to Seconds. Keep Follow host selected. Stop recording before seeking. Several devices can scan the same QR code; their playback position is shared but text size and layout are independent. See LYRIC-SEEK.md in the package for setup.

NEW · Player view
Player view shows up to five nearby lyric rows on each side, with compact surrounding text and the current timed line highlighted. Actual visible rows depend on screen height and text size. Choose Shared default to use the project context setting.

NEW · Recording workspace and singer screens
Record is the uncluttered default. Open Edit lyrics for text, singer labels and LRC export; Timing for automatic alignment, stamps, song origin and correction; Reading style for shared defaults. Settings contains language, help and projection management. Phone screen opens the QR code directly.
The independent reader and phone have Player view / Single / Current + next / Three lines, A− / A+, and Reset to shared defaults. Each screen keeps its own display options. Lyrics only hides controls, labels and progress; click/tap the lyrics or press Escape to restore. Use F11 in the native reader for fullscreen. Progress is lyric duration, not accompaniment length. In Follow host it is read-only; control playback in Cubase. Native preview mode allows seek and play, without playing audio.
Closing the main plug-in editor keeps the independent reader and phone connected. Removing the plug-in or closing the host/project ends them. Keep the effect inserted and active while recording. Alignment is prepared once per song/version, saved in .cueflow or host state, then reused for every take; repeated vocal analysis is unnecessary unless lyrics or performance timing change.


NEW · KTV automatic alignment
Choose KTV auto align, select matching reference vocals, and keep only the actually sung lyrics in their performed order. Run setup-alignment.ps1 once if the local engine is missing; dependencies and the model download need an internet connection. Audio analysis runs locally. Start alignment, review the line/word timings and warnings, then Apply. Cancel leaves the current project unchanged. Text mismatch and unreliable timing may fail; correct the sung text and retry. Some multi-character tokens use proportional timing inside the token; sustained notes may need correction. This is offline preparation, not live voice following.
Import a timed .cueflow/YRC and use Follow host in Cubase. Set Song origin to the accompaniment event start, then play in Cubase. Characters fill continuously across their timed duration; pause/seek/loop follow the host. Save .cueflow and the Cubase project to retain word timing. LRC export keeps only line timing.

NEW · Scan from a phone
In Settings, Start projection to show a QR code. Use Scan with phone to reopen it. Connect phone and computer to the same network, scan using the camera, then open in a browser. Select another network address if needed. Rotate the phone for landscape or portrait; disable the phone’s orientation lock if required. Landscape and portrait retain your selected layout. Use Lyrics only to hide phone controls; tap the lyrics to restore. Only lyrics are transmitted, not audio. Stop/restart changes the QR code.

NEW · Cubase setup, song origin and projection
For a TXT file, select Follow host, choose the first sung line, start the accompaniment in Cubase, then click Stamp line at each line onset. Selection advances automatically. Save both your .cueflow project and Cubase project. Song origin cannot invent timestamps. Remove title/chord-only lines before stamping or leave them unmarked (LRC export requires every line marked).
For a matching timed lyric file, enter the accompaniment event start in project seconds under Song origin and press Enter, or position the host cursor there and click Use host position while host time is available. A lyric at 10 seconds with an origin of 30 seconds appears at project time 40 seconds. Global delay is additional fine adjustment. LRC exports remain song-relative; origin, singer labels and colours are retained in .cueflow and host state only.
In Settings, Start projection and copy a LAN URL to a phone/tablet browser on the same network. Multiple adapters may produce multiple URLs; choose the address reachable from the device. 127.0.0.1 is only for this computer. Private-network firewall permission may be needed. This sends lyrics and timing only, not audio. With the Lyra MIDI bridge connected, lyric clicks locate Cubase; play/pause remains in Cubase. Stop projection revokes the URL; restarting creates a new URL. It remains active when the plugin editor closes, until stopped or the plugin unloads. It never starts automatically after reopening a project. Untimed/article scrolling is not broadcast.
Select one or more lyric rows, enter a singer/role, choose a colour and click Label selected lines. Clear the name and choose Theme colour to remove annotations. Text is not changed. These labels also appear on the projected screen. Annotations are per line, not per character.


1. Your first session
Import a file with Import or drag it into the workspace. Open samples/录音演示.yrc for word-timed lyrics. Select Timeline preview, then Play. For a plain TXT script, use Script scroll. CueFlow displays text: standalone does not record or play audio, and does not recognise your voice.

2. Connect Cubase, Nuendo or Studio One
Close your host, copy the entire CueFlow.vst3 folder to C:\Program Files\Common Files\VST3, then rescan plug-ins. Insert CueFlow as an audio effect on an active mono or stereo track. Select Follow host. The host supplies the play position; play, pause, seek and loop in the host to move the lyrics. CueFlow passes audio through unchanged. Its Play button is disabled in this mode: it cannot start the host. Enable plug-in processing if the display says it is waiting. Actual host versions may handle inactive tracks differently.

3. Formats and timing
LRC has line timestamps. KRC, QRC and YRC can include word timestamps and progressive highlighting. TXT is untimed. CueFlow uses timing supplied by the file; it does not invent word alignment for LRC. Supported QRC variants include local encrypted files, cloud hexadecimal data, XML and plain text. Unsupported or damaged files leave the current document intact. Lyrics have a 4 MB limit; CueFlow projects have a 16 MB limit.

4. Edit a script
Select a line, change its text or start time (seconds), and click Apply line. Editing text removes that line's word timing. Add line and Delete line change the document. Edit script opens a full-text editor. If the line count is unchanged, unedited lines retain their timing. Changing the line count converts the script to untimed text. Undo and Redo recover previous edits (up to 64 steps). Save your project regularly.

5. Adjust synchronisation
Global delay uses seconds: a positive value makes text appear later, a negative value earlier. Reset offset returns to zero. Shift-click to select a contiguous range, enter Shift and Timing scale, then Apply timing. Scaling is anchored to the first selected line. A scale of 1 keeps speed unchanged; 2 doubles the relative time intervals. This changes line and word timestamps together. Double-clicking a line previews its time and leaves Follow host mode.

6. Create timing manually
In preview or a valid host timeline, select the first line and click Mark line at its start. The selection advances to the next original line. Untouched lines remain untimed. For Mark word, select a line and click once at each character's start, then once more at the end of the last character. Until complete, the remaining text is a provisional segment. Reselect the line if time jumps backwards. Editing, importing or undoing resets the word cursor. These tools record your clicks, not speech recognition.

7. Read comfortably
Font size, line spacing, context lines and alignment affect the reader. Mirror horizontally is for reflective teleprompter hardware. Script scroll speed is in pixels per second; the mouse wheel and Page up/down move through the script. Pop-out reader opens a separate resizable window that can be moved to another monitor. Full screen opens it without its frame; F11 or Esc toggles full screen in the reader. Returning from a disconnected display moves the window back onto a visible screen.

8. Themes and language
Choose one of five full-interface themes here or in Reader controls. Theme colours also apply to the reading canvas. Chinese and English switch immediately without changing your script. Appearance and language are stored in .cueflow projects and host state. Standalone remembers your last appearance and language when you exit. A new plug-in instance starts with Titanium and Chinese; loading a project restores its own choices. Other open plug-in instances retain their own settings.

9. Save and export
Save project embeds text, line/word timing, reader settings, theme and language in a .cueflow file. Host projects also embed the plug-in state. Export LRC writes line timing only; retain your CueFlow project for word timing. Finish marking every line and remove negative timestamps before LRC export. On standalone exit, choose Save and quit, Discard or Cancel if there are unsaved changes.

10. Keyboard and troubleshooting
Workspace: Ctrl+S saves; Ctrl+Z undoes; Space plays/pauses preview or scrolling. Reader: Space plays/pauses, Home rewinds, Page up/down moves pages, F11/Esc toggles full screen. Text fields keep normal typing behaviour. Hosts may intercept shortcuts; use the on-screen controls if necessary.
No movement? Check the mode, host processing and transport. Whole script early/late? Adjust Global delay. No progressive highlight? The file may only contain line timing. Text too crowded? Reduce font size or context lines. Save a .cueflow project before experimenting with calibration.

Cooperation & support
WeChat: Amumusic1005
Use Copy WeChat ID below, then search for the ID in WeChat.
Version 0.5.4 · Desktop · Amu music
)GUIDE":R"GUIDE(Lyra / Amu music
录音提词器 · 产品使用说明

新增 · 点击歌词定位与多设备
录音页列表、阅读画布、独立阅读窗和手机均可点击已标时歌词定位；编辑/校准页用双击，单击仍用于选择。纯歌词第一次点击恢复按钮，再点歌词定位。预览模式只移动歌词，不播放伴奏。Cubase 伴奏定位需要 Lyra MIDI Remote 桥：运行 loopMIDI，建立 Lyra Control 和 Lyra Feedback 端口；将发行包 integrations/cubase/AmuMusic/Lyra 复制到“文档/Steinberg/Cubase/MIDI Remote/Driver Scripts/Local/AmuMusic/Lyra”；Cubase 主时间或辅助时间显示设为“秒”。保持跟随宿主，录音写入时先停止再定位。具体设置见发行包 LYRIC-SEEK.md。
多个手机/平板可扫描同一个二维码，共享播放位置，各自保留字号和排版。任意一端点击后其他端跟随；不会将伴奏音频传到手机。关闭投屏后旧码失效。

新版 · 录音工作台与歌手屏幕
默认“录音”页专注歌词。文字修改、角色标注和导出放在“歌词编辑”；自动对齐、逐句/逐字打点、歌曲起点和偏移放在“时间校准”；字号、行距、主题等共享默认放在“阅读样式”。语言与说明在“设置”。点击“手机投屏”直接显示二维码。
主界面、独立阅读窗和手机均可选择“播放器视图”，同时显示更多相邻歌词，当前句逐字高亮。各屏幕还可选择单行、当前＋下一句、上下文三行，使用 A− / A+ 调字号，恢复默认可重新使用共享阅读设置。各屏幕单独调整，互不影响。“纯歌词”隐藏按钮、角色标签、状态和进度；点击歌词区域或按 Esc 恢复。独立窗 F11 切换全屏。进度表示歌词范围，不代表伴奏总长度；跟随宿主时只读，请在 Cubase 控制播放。独立窗预览模式可播放和拖动进度，但不播放音频。
关闭插件主编辑窗口后，独立阅读窗和手机继续同步；删除插件、关闭宿主或工程会结束同步。录音时请保留已启用的插件。同一首歌、同一版本只需准备一次逐字时间并保存，以后每遍录音复用；歌词或演唱节奏变化后才需要重新校准。

新增 · KTV 自动逐字对齐
点击“KTV 自动对齐”，选择同版原唱或参考干声，在左侧只保留实际唱出的歌词，顺序和重复次数必须与音频一致。若提示未安装引擎，先运行发行包 setup-alignment.ps1；首次安装依赖和下载模型需要联网，音频分析在本机进行。开始对齐后检查逐句、逐字时间与提示，点击应用。取消不会修改当前文稿。歌词不符或时间不可靠时可能失败，请修正实唱歌词后重试。部分多字词内部按时长分配，拖音需要检查。这是提前制作时间，不是实时听麦克风跟唱。
导入带逐字时间的 .cueflow/YRC，在 Cubase 使用“跟随宿主”，将“歌曲起点”设为伴奏事件开头，再由 Cubase 播放。每个字按自身时长连续扫色，暂停、跳转和循环跟随宿主。保存 .cueflow 和 Cubase 工程可保留逐字时间；导出 LRC 只保留逐句时间。

新增 · 手机扫码
设置中开启投屏会显示二维码，点击“手机扫码”可再次查看。手机和电脑连接同一网络，用相机扫码后在浏览器打开；如无法连接，可切换网卡地址。转动手机自动适配横竖屏，必要时关闭手机的方向锁定。横竖屏均保留本屏选择的排版。手机端使用“纯歌词”隐藏控件，点击歌词恢复，不再提供全屏按钮。只投歌词，不传伴奏。关闭后旧码失效，重新开启请重新扫码。

新增 · Cubase 同步、歌曲起点与投屏
TXT 歌词：切到“跟随宿主”，选第一句要唱的歌词，在 Cubase 播放伴奏，每句开始点一次“逐句打点”，软件自动选下一句。完成后保存 .cueflow 和 Cubase 工程。标题/和弦说明可先删除；未打点行不参与同步，导出 LRC 前需全部行已标时。歌曲起点不能自动生成每句时间。
带时间歌词：在“歌曲起点”输入伴奏事件开始的工程秒数并按回车，或把宿主光标放在伴奏开头后点击“取宿主位置”（需要宿主更新时间）。歌词第10秒、歌曲起点30秒，对应工程第40秒。“整体延后”用于额外微调。LRC 导出保持歌曲相对时间，歌曲起点、歌手和颜色只保存在项目/宿主状态中。
设置中“开启投屏”，复制局域网地址，用同一网络的手机/平板浏览器打开。多个网卡会列出多个地址，请选择手机能访问的地址；127.0.0.1仅本机可用。可能需要允许 Windows 防火墙的专用网络访问。只发送歌词和时间，不发送伴奏；连接 Lyra MIDI 回控后可点击歌词定位；播放暂停仍由 Cubase 操作。关闭投屏后旧地址失效，重新开启生成新地址。关闭插件界面后投屏继续，直到手动关闭或卸载插件；重开工程不会自动启用。文章滚动和未标时文稿不投屏。
选择一行或多行，输入歌手/角色、选择颜色，点“标注所选行”。清空标签并选择默认颜色可清除。投屏也显示标注，本版按整行设置，不是逐字颜色编辑。

一、第一次使用
点击“导入文稿”，或将文件拖入工作台。可先打开 samples 文件夹中的“录音演示.yrc”，选择“时间轴预览”，点击“开始”查看逐字提词。普通 TXT 文稿请使用“文章滚动”。独立版负责显示文稿，不播放伴奏、不录音，也不会识别声音自动跟读。

二、连接 Cubase、Nuendo、Studio One
关闭宿主，将完整的 CueFlow.vst3 文件夹复制到 C:\Program Files\Common Files\VST3，重新扫描插件。在启用处理的单声道或立体声音轨上，将 CueFlow 插入为音频效果器，选择“跟随宿主”。宿主播放、暂停、跳转和循环时，歌词跟随宿主提供的位置移动。插件原样传递音频，不改变声音。此模式的“开始”按钮禁用，需要从宿主操作播放。显示等待时，请检查音轨及插件处理是否启用；不同宿主版本对闲置音轨的处理有所区别。

三、支持的文件与时间原理
LRC 提供逐行时间；KRC、QRC、YRC 可包含逐字时间，实现逐字高亮；TXT 为普通文章。软件使用文件已有的时间信息，不会为普通 LRC 自动生成字级时间。QRC 支持本地加密、云端十六进制、XML 和纯文本等形式。文件不支持或损坏时保留当前文稿。歌词文件最大 4 MB，CueFlow 项目最大 16 MB。

四、编辑文稿
在左侧选择一行，修改文字或开始秒数，点击“应用本句”。修改文字会清除这一行的字级时间。“新增行”“删除行”用于调整内容。“编辑全文”可粘贴或批量修改：行数不变时保留未修改行的时间；行数改变时转为普通文章。可通过撤销、重做恢复修改，最多保留 64 步。建议定期保存项目。

五、同步与时间校准
“整体延后”单位为秒：正数让歌词更晚出现，负数提前。“重置偏移”回到零。按住 Shift 选择连续多行，填写“平移秒数”和“所选时间比例”，点击“校准所选范围”。比例以第一条所选歌词为锚点，1 为不缩放，2 为相对时间间隔扩大一倍；行与字时间会一起调整。双击歌词会跳至其时间进行预览，同时退出跟随宿主模式。

六、手动打点
在预览模式或有效的宿主时间下，选中第一行，在该句开始时点击“逐句打点”，选择会前进到原来的下一行；未点击的行仍为未标时。“逐字打点”需要在每个字开始时点击一次，最后一个字结束后再点击一次记录结束时间。未完成的后半句只是临时时间段，请完成整句后使用。时间回跳时重新选择本句；编辑、导入、撤销会重置逐字游标。打点记录的是你的点击时间，不是语音识别。

七、阅读与双屏
可调整字号、行距、上下文行数、对齐。“水平镜像”适合反射式提词硬件。“文章滚动”的速度以像素/秒计，鼠标滚轮和上一页、下一页可移动文稿。“独立阅读窗”可调整大小并拖到第二块屏幕。“全屏阅读”隐藏阅读窗口边框；在阅读窗按 F11 或 Esc 切换全屏。显示器断开后，窗口会尝试回到可见屏幕。

八、主题和语言
可在本页或阅读控制区选择钛银、午夜蓝、松石、暮紫、黑曜五款主题，整个界面及提词画布同步换色。中文与 English 即时切换，不翻译或改动你的文稿。外观和语言随 .cueflow 项目、宿主状态保存；独立版退出时会记住最近的外观与语言；新插件实例默认使用钛银与中文，打开项目则恢复项目中的选择。不同插件实例可以保留各自的选择。

九、保存与导出
“保存项目”把文稿、行/字时间、阅读设置、主题和语言写入 .cueflow 文件，宿主工程也会嵌入插件状态。“导出 LRC”只保存逐行时间，字级信息请保留 CueFlow 项目。所有行完成打点且最终时间不为负数时才可导出 LRC。独立版退出时若有修改，可选择保存并退出、不保存或取消。

十、快捷键与常见问题
工作台：Ctrl+S 保存，Ctrl+Z 撤销，空格开始/暂停预览或滚动。阅读窗：空格开始/暂停，Home 回到开头，Page Up/Down 翻页，F11/Esc 切换全屏。编辑文字时保留正常输入行为；宿主可能拦截快捷键，遇到这种情况请使用界面按钮。
画面不动：检查模式、宿主处理状态和播放状态。整篇偏早或偏晚：调整整体延后。没有逐字高亮：检查源文件是否包含字级时间。文字太挤：减小字号或上下文行数。尝试校准前先保存一份 .cueflow 项目。

合作与咨询
微信：Amumusic1005
点击下方“复制微信号”，在微信中搜索并添加。
版本 0.5.4 · Desktop · Amu music
)GUIDE");}
}
