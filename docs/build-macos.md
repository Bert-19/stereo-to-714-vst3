# macOS 构建与 Reaper 使用指南

Windows 版 VST3 **不能**在 Mac 上使用，必须在 macOS 上重新编译（或使用 GitHub Actions 产物）。

---

## 1. 安装工具

### 必装

| 工具 | 说明 |
|------|------|
| **Xcode** | App Store 安装，首次打开并完成初始化 |
| **Xcode Command Line Tools** | 终端执行：`xcode-select --install` |
| **CMake** ≥ 3.22 | `brew install cmake` |
| **Git** | `brew install git`（Xcode CLT 通常已包含） |

### 安装 Homebrew（如尚未安装）

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

---

## 2. 本地编译

将 `stereo-to-714-vst3` 文件夹拷到 Mac，在终端执行：

```bash
cd /path/to/stereo-to-714-vst3

# 配置（Xcode 工程，Universal Binary：Apple Silicon + Intel）
cmake -B build -G Xcode \
  -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"

# 编译 Release
cmake --build build --config Release
```

仅 Apple Silicon（M 系列）可省略 Universal，只编 arm64：

```bash
cmake -B build -G Xcode -DCMAKE_OSX_ARCHITECTURES=arm64
cmake --build build --config Release
```

### 产物路径

```
build/StereoTo714_artefacts/Release/VST3/StereoTo714.vst3
```

---

## 3. 安装到系统

**当前用户（推荐）：**

```bash
mkdir -p ~/Library/Audio/Plug-Ins/VST3
cp -R "build/StereoTo714_artefacts/Release/VST3/StereoTo714.vst3" \
      ~/Library/Audio/Plug-Ins/VST3/
```

**所有用户：**

```bash
sudo cp -R "build/StereoTo714_artefacts/Release/VST3/StereoTo714.vst3" \
           /Library/Audio/Plug-Ins/VST3/
```

### 若 macOS 拦截未签名插件

```bash
xattr -cr ~/Library/Audio/Plug-Ins/VST3/StereoTo714.vst3
```

或在 **系统设置 → 隐私与安全性** 中允许打开。

---

## 4. Reaper 配置

1. 打开 **Reaper → Preferences → Plug-ins → VST**
2. 确认 **VST plug-in paths** 包含：
   - `~/Library/Audio/Plug-Ins/VST3`
3. 点击 **Re-scan**（或 Clear cache and re-scan）
4. 在轨道上：**FX → VST3 → StereoUpmix → Stereo to 7.1.4**
5. 轨道 **I/O** 输出设为 **7.1.4**（或与你监听系统一致的 12 声道布局）

### 延迟说明

插件 STFT 处理延迟约 **32 ms**（@48 kHz）。Reaper 通常会自动补偿；若声画不同步，可在轨道 FX 链中微调 **PDC / 延迟**。

---

## 5. 使用 GitHub Actions 自动编译（无需本地 Mac 编译环境）

若项目已推送到 GitHub，每次 push 或手动触发 workflow 会在云端 macOS 编译并上传 zip。

### 操作步骤

1. 将项目 push 到 GitHub 仓库
2. 打开仓库 **Actions** 页
3. 选择 **Build VST3** workflow → **Run workflow**
4. 完成后在 **Artifacts** 下载：
   - `StereoTo714-vst3-macos-universal`（Mac 用）
   - `StereoTo714-vst3-windows`（Windows 用）
5. 解压 zip，将 `.vst3` 复制到 `~/Library/Audio/Plug-Ins/VST3/`

---

## 6. 常见问题

| 问题 | 处理 |
|------|------|
| `cmake: command not found` | `brew install cmake` |
| JUCE 下载失败 | 检查网络；或设置 Git 代理 |
| Reaper 扫不到插件 | 确认路径、重新扫描、检查 `.vst3` 是否在 VST3 目录 |
| Apple Silicon 上 Reaper 用 Rosetta | 建议使用原生 ARM 版 Reaper + arm64 插件 |
| 只有 Intel Mac | 编译时 `-DCMAKE_OSX_ARCHITECTURES=x86_64` |
