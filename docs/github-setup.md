# GitHub 推送指南（方案 B：云端编译 Mac 版）

本地代码已提交到 Git 。你只需完成 **一次 GitHub 登录**，然后运行推送脚本即可。

---

## 第一步：注册 / 登录 GitHub

1. 打开 https://github.com 注册或登录
2. 记住你的 **用户名**（例如 `daihao`）

---

## 第二步：在本机登录 GitHub CLI（只需一次）

在 **PowerShell** 或 **Cursor 终端** 中执行：

```powershell
gh auth login
```

按提示选择：

| 提示 | 选择 |
|------|------|
| What account? | GitHub.com |
| Preferred protocol | HTTPS |
| Authenticate | Login with a web browser |
| Paste an authentication token? | 否（选浏览器） |

终端会显示 **8 位验证码**，浏览器打开 github.com/login/device 输入验证码并授权。

验证成功：

```powershell
gh auth status
```

应显示 `Logged in to github.com`.

---

## 第三步：推送仓库

在 PowerShell 中执行（**把 `你的用户名` 换成真实 GitHub 用户名**）：

```powershell
cd d:\AI\stereo-to-714-vst3
.\scripts\push-to-github.ps1 -GitHubUser 你的用户名
```

脚本会：

1. 在 GitHub 创建仓库 `stereo-to-714-vst3`（私有或公开，默认公开）
2. 推送代码
3. 自动触发 **Build VST3** 编译

---

## 第四步：下载 Mac 版插件

1. 浏览器打开 `https://github.com/你的用户名/stereo-to-714-vst3`
2. 点击 **Actions** 标签
3. 等待 **Build VST3** 显示绿色 ✓（约 5–15 分钟）
4. 进入该次运行 → 底部 **Artifacts** → 下载 **StereoTo714-vst3-macos-universal**
5. 解压 zip，将 `Stereo to 7.1.4.vst3` 复制到 Mac 的：

   ```
   ~/Library/Audio/Plug-Ins/VST3/
   ```

6. Reaper → Preferences → VST → Re-scan → 加载插件

---

## 常见问题

**Q: 仓库想设为私有？**

```powershell
.\scripts\push-to-github.ps1 -GitHubUser 你的用户名 -Private
```

**Q: 推送时提示 repository already exists？**

脚本会尝试推送到已有仓库；若你手动建过同名仓库，直接推送即可：

```powershell
git remote add origin https://github.com/你的用户名/stereo-to-714-vst3.git
git switch main
git push -u origin main
```

**Q: Actions 没有运行？**

仓库第一次 push 后应自动触发。也可在 Actions 页手动 **Run workflow**。

**Q: 以后改代码怎么更新？**

```powershell
cd d:\AI\stereo-to-714-vst3
git add .
git commit -m "描述你的修改"
git push
```

Actions 会再次编译，重新下载 Artifacts 即可。
