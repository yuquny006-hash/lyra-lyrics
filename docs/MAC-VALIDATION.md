# macOS 0.5.4 预览包验证

日期：2026-10-25。Universal 包包含 Intel x86_64 与 Apple Silicon arm64，最低目标 macOS 11。

| 检查 | Apple Silicon | Intel |
|---|---|---|
| 四个 bundle 的架构与临时签名 | 通过 | 通过 |
| 核心功能 56 项 | 通过 | 通过 |
| 集成功能 125 项，连续三轮 | 全部通过 | 全部通过 |
| KTV 渲染测试 | 通过 | 通过 |
| CLAP 工厂、状态、传输时间处理、音频透传 | 通过 | 通过 |
| AU 的 Apple auval | 通过 | 通过 |

[构建与三轮测试](https://github.com/yuquny006-hash/lyra-lyrics/actions/runs/40175064399)的 build 任务成功。首次安装 AU 的云端注册缓存导致该运行的后续验证失败；刷新临时测试机的注册进程后，[相同构建包的双 CPU 验证](https://github.com/yuquny006-hash/lyra-lyrics/actions/runs/40180383080)全部通过。没有更换二进制或跳过功能断言。

VST3 本次验证到编译、双架构、签名与共享处理逻辑测试，未在 Mac 商业宿主或 pluginval 中实测。AU 校验不等于真实 Logic 录音工程验收。所有 Mac 包均无 Apple Developer ID 公证；可能需要按系统提示允许首次使用。

[AAX 开发包构建](https://github.com/yuquny006-hash/lyra-lyrics/actions/runs/40172342285)成功，双架构检查通过；没有 Avid/PACE 发布签名，没有 Pro Tools 加载测试。

## 校验和 SHA256

- CueFlow-0.5.4-macOS-Universal.zip：`b2a8c36e8001b3a1fe42d7ac95ceab567b8acd17a7fc699aaad753fb2133e9ef`
- CueFlow-0.5.4-macOS-AAX-development-UNSIGNED.zip：`63607ea038e232d48e8d2bd4255a6e088943e225f8e6ce31df9d25afba98ed5c`

下载包附有各自对应源码及第三方许可证。普通插件构建源码提交 ef4de4d，AAX 构建源码提交 62bb513；后续提交只更新云端验证与说明。Mac 自动对齐运行环境、真实宿主控制回路与音频设备测试仍需实际 Mac 环境验收。
