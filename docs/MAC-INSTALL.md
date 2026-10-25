# Lyra · 录音歌词跟随系统 — Mac 测试版

适用于 macOS 11 或以上，Universal 包含 Apple Silicon 与 Intel。

## 安装

1. 解压完整 ZIP，关闭音频宿主。
2. 在终端输入 `bash `（含一个空格），把解压文件夹中的 `install-macos.sh` 拖入终端，回车。
3. 脚本安装 AU、VST3、CLAP 到当前用户的 `~/Library/Audio/Plug-Ins` 对应文件夹。遇到已有同名插件会保留并停止，请先备份旧插件、移走旧版后再安装。
4. Logic 使用 AU（CueFlow）；Cubase 使用 VST3；支持 CLAP 的宿主可选择 CLAP。同一轨道使用一个格式。
5. 独立阅读可打开 `CueFlow.app`；独立版不直接读取宿主时间。

首次安装后如果 Logic 找不到 CueFlow，请先重启 Mac，再检查插件管理器；若已列出但不可用，可选择该插件重新扫描。参见 [Apple 的新装插件排查说明](https://support.apple.com/en-gb/122179)。云端验证会刷新一次临时测试机的 AU 注册进程，安装器不会结束你电脑上的任何进程。

这些包使用本机构建的临时签名，尚无 Apple Developer ID 签名或公证。macOS 可能阻止首次打开或扫描；仅在确认来源后使用系统“隐私与安全性”允许该应用，或从 Source 自行编译。安装脚本不会关闭系统安全保护。

## 验证范围

GitHub Actions 编译 Universal 包并在 Apple Silicon/Intel 运行自动测试、CLAP 加载测试及 AU 的 auval 检查。以对应 Actions 运行实际结果为准；这不能替代 Logic、Cubase、Pro Tools 的真实录音测试。

插件显示名称保留 CueFlow，以兼容现有工程；产品界面使用 Lyra 品牌。手机多设备投屏与歌词显示沿用现有功能。Mac 自动对齐引擎需要另行配置，当前未提供 Mac Python 一键安装。歌词点击回控目前只提供 Cubase/Nuendo MIDI Remote 适配，Mac MIDI 端口连接仍需实机验证。

AAX 不包含在此安装器中。可选 AAX-development 产物没有 Avid/PACE 发布签名，不能作为普通 Pro Tools 的正式可加载插件使用。

源码与第三方许可证位于 Source 和 LICENSES。合作咨询微信：Amumusic1005。
