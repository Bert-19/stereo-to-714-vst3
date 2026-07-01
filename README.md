# Stereo to 7.1.4 VST3

立体声输入 → 7.1.4 环绕声输出的 VST3 插件（JUCE + STFT 自适应上混）。

## 声道布局 (7.1.4)

| 索引 | 声道 | 说明 |
|------|------|------|
| 0 | L | 左主声道 |
| 1 | R | 右主声道 |
| 2 | C | 中置 |
| 3 | LFE | 低频效果 |
| 4 | Ls | 左环绕 |
| 5 | Rs | 右环绕 |
| 6 | Ltf | 左前天空 |
| 7 | Rtf | 右前天空 |
| 8 | Ltr | 左后天空 |
| 9 | Rtr | 右后天空 |
| 10 | Lrs | 左后环绕 |
| 11 | Rrs | 右后环绕 |

## 参数

| 参数 | 默认 | 说明 |
|------|------|------|
| Height Mode | Conservative | 天空只做 ambience / Aggressive 更多高度包围 |
| Center Mode | Movie | 对白突出 / Music 少抽 center |
| Width | 100% | 环绕宽度 |
| Height Amount | 65% | 天空声量 |
| Surround Mix | 100% | 扩散环绕强度 |
| LFE Level | 70.7% | 超低音量 |
| Bypass | Off | 直通立体声 |

## 构建

### Windows

依赖：CMake ≥ 3.22、Git、Visual Studio 2022（C++ 桌面开发）

```powershell
cd stereo-to-714-vst3
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

产物：`build\StereoTo714_artefacts\Release\VST3\Stereo to 7.1.4.vst3`

### macOS（Reaper 等）

**Windows 版不能在 Mac 上使用，需重编译。** 详见 [docs/build-macos.md](docs/build-macos.md)。

```bash
cmake -B build -G Xcode -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
cmake --build build --config Release
```

安装：`~/Library/Audio/Plug-Ins/VST3/`

### GitHub Actions 云端编译

Push 到 GitHub 后，在 **Actions → Build VST3** 下载 Artifacts：

- `StereoTo714-vst3-macos-universal` — Mac 用
- `StereoTo714-vst3-windows` — Windows 用

## 项目结构

```
stereo-to-714-vst3/
├── CMakeLists.txt
├── Source/
│   ├── PluginProcessor.cpp/h
│   ├── PluginEditor.cpp/h
│   └── Upmix/              # STFT 上混引擎
├── docs/
│   ├── upmix-math-model.md
│   └── build-macos.md
└── .github/workflows/build.yml
```

## 算法文档

见 [docs/upmix-math-model.md](docs/upmix-math-model.md)。
