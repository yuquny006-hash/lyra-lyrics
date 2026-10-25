# Lyra 多宿主与插件格式

格式兼容、跟随宿主时间、点击歌词回控是三个不同能力。支持某种格式不代表所有宿主版本均已实测。

| 平台/宿主 | 选择格式 | 本次状态 |
|---|---|---|
| Windows Cubase / Nuendo | VST3 | Cubase 15 已实测跟随和 MIDI Remote 回控；Nuendo 待实测 |
| Windows Studio One / Studio Pro、Ableton Live 10.1+、FL Studio、REAPER、Bitwig 等支持 VST3 的版本 | VST3 | 提供 Windows x64 二进制，需按具体宿主版本验收 |
| 支持 CLAP 的 REAPER、Bitwig 等版本 | CLAP | 提供 Windows x64 二进制；使用独立 CLAP 测试宿主验证，商业宿主待实测 |
| Mac Logic Pro / MainStage / GarageBand | AU component | 已提供 Universal 预览包，Apple Silicon 与 Intel 的 auval 均通过；真实 Logic 等宿主仍待验收 |
| Mac 支持 VST3 / CLAP 的宿主 | VST3 / CLAP | 已提供 Universal 预览包；CLAP 双 CPU 加载与处理测试通过，VST3 本次验证到编译/架构/签名层，真实宿主待实测 |
| Windows / Mac Pro Tools | AAX Native | Mac 已提供 Universal 未签名开发包，Windows 提供构建脚本；正式 Pro Tools 加载仍需 Avid/PACE 签名及实机验收 |

目前点击歌词回控只适配 Cubase / Nuendo MIDI Remote。其他宿主可以通过插件读取的播放位置显示歌词，但点击歌词不会自动取得控制宿主的能力；需要独立适配并实测。手机投屏与独立阅读窗复用相同的宿主时间。关闭插件编辑窗口不影响投屏，卸载插件/关闭工程会停止。

## Windows 安装

关闭宿主，在发行包运行 `./install-plugins.ps1`。已有安装使用 `./install-plugins.ps1 -Update`，会备份旧文件并沿用其路径；拒绝两处重复 VST3 安装。首次默认当前用户目录：
- VST3：`%LOCALAPPDATA%/Programs/Common/VST3`
- CLAP：`%LOCALAPPDATA%/Programs/Common/CLAP`

在宿主添加该目录并重新扫描。若宿主不支持用户插件目录，可将完整 VST3 包/CLAP 文件安装到相应 `C:/Program Files/Common Files` 标准目录。每条轨道只插入一个格式的实例。保留 CueFlow 文件名和插件 ID，避免已有工程丢失插件。

Cubase 反馈端口设置见 LYRIC-SEEK.md。其他宿主应在有持续音频处理的音轨/总线上加载插件，避免旁路、冻结或节能挂起导致时间停止。

## Mac 构建与验收

在装有 Xcode、CMake、Git 的 Mac 上展开 Source，执行 `bash scripts/build-macos.sh`。脚本自动取得固定版本 JUCE 与 CLAP；若发行包已包含依赖源码则直接使用。构建目标为 macOS 11+、arm64+x86_64。然后 `bash scripts/install-macos.sh` 安装到用户的 Audio/Plug-Ins 目录。安装脚本保留已有插件，遇到重复会停止。也可使用公开仓库 Actions 的 macOS plugins 工作流取得构建包，安装说明见 MAC-INSTALL.md。

运行 `auval -v aufx Cfp1 Cufl`，在 Logic 插件管理器中验证并做播放、暂停、回跳、循环、保存重开、投屏测试。分发给其他 Mac 还应完成 Developer ID 签名与公证；脚本不会关闭 Gatekeeper 或伪造签名。

## Pro Tools / AAX

Windows：运行 `scripts/build-aax-development.ps1`，在独立的 `build/aax-development` 构建目录生成 `CueFlowPlugin_AAX`。Mac：`bash scripts/build-macos.sh --aax-development`。JUCE 包含的 AAX 源码保留其许可。开发产物与面向用户的 VST3/CLAP 包分开放置。

AAX 未签名文件不能承诺在普通 Pro Tools 中加载。完成 Avid 开发者和 PACE 签名条件后，在 Pro Tools 实机验收才算正式交付。Mac 编译已通过 GitHub 标准云端运行器完成；仍无真实 Pro Tools 环境与可用发布签名凭证。结果见 MAC-VALIDATION.md。

## 官方参考

- JUCE 格式支持：https://juce.com/features/
- Ableton 插件格式：https://help.ableton.com/hc/en-us/articles/5937501570460-Supported-Plug-in-Formats
- Logic AU：https://support.apple.com/en-ie/guide/logicpro/lgcp22a0dab0/mac
- AAX SDK 与签名：https://developer.avid.com/aax/
- CLAP wrapper：https://github.com/free-audio/clap-juce-extensions
