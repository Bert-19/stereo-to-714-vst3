# Stereo → 7.1.4 Upmix 数学模型

> 目标：建立可对标 **Dolby Surround / Atmos Music upmix** 的算法框架。  
> 本文档为纯数学与信号处理模型，不涉及具体 C++ 实现细节。

---

## 0. 符号约定

| 符号 | 含义 |
|------|------|
| \(t\) | 连续时间（秒） |
| \(n\) | 帧索引（STFT 帧） |
| \(k\) | 频点索引 |
| \(f_k\) | 第 \(k\) 个频点中心频率（Hz） |
| \(\mathbf{x}(t)\) | 输入立体声 \([L(t),\, R(t)]^\mathsf{T}\) |
| \(\mathbf{y}(t)\) | 输出 12 声道向量 |
| \(X_L(n,k), X_R(n,k)\) | L/R 的 STFT 复系数 |
| \(j\) | 虚数单位 |
| \(\|\cdot\|\) | 复数模 / 向量 2-范数 |
| \((\cdot)^*\) | 复共轭 |
| \(\Re\{\cdot\}, \Im\{\cdot\}\) | 实部 / 虚部 |

输出声道索引（与 JUCE `create7point1point4()` 一致）：

\[
\mathbf{y} = [y_\mathrm{L},\, y_\mathrm{R},\, y_\mathrm{C},\, y_\mathrm{LFE},\, y_\mathrm{Ls},\, y_\mathrm{Rs},\, y_\mathrm{Lrs},\, y_\mathrm{Rrs},\, y_\mathrm{Ltf},\, y_\mathrm{Rtf},\, y_\mathrm{Ltr},\, y_\mathrm{Rtr}]^\mathsf{T}
\]

---

## 1. 问题陈述

### 1.1 欠定逆问题

立体声可视为 12 声道空间在 2 维子空间上的投影：

\[
\mathbf{x}(t) = \mathbf{P}\,\mathbf{s}(t), \qquad \mathbf{P} \in \mathbb{R}^{2 \times 12}
\]

其中 \(\mathbf{s}(t)\) 为「真实」12 声道场，\(\mathbf{P}\) 为下混矩阵。**Upmix 即求 \(\hat{\mathbf{s}}(t)\)**，使

\[
\hat{\mathbf{y}}(t) \approx \hat{\mathbf{s}}(t), \qquad \mathbf{P}\,\hat{\mathbf{s}}(t) \approx \mathbf{x}(t)
\]

该问题 **严重欠定**：无穷多 \(\hat{\mathbf{s}}\) 满足下混一致性。Dolby 类算法的核心不是「唯一还原」，而是在约束下 **最大化空间一致性与感知自然度**。

### 1.2 优化目标（全局形式）

在每一时频单元 \((n,k)\) 上，求分解与路由，使

\[
\min_{\hat{\mathbf{S}}(n,k)} \;
\mathcal{J} = \underbrace{\|\mathbf{X}(n,k) - \mathbf{P}\,\hat{\mathbf{S}}(n,k)\|_2^2}_{\text{下混一致性}}
+ \lambda_1 \underbrace{\Phi_\mathrm{spat}(\hat{\mathbf{S}})}_{\text{空间平滑}}
+ \lambda_2 \underbrace{\Phi_\mathrm{diff}(\hat{\mathbf{S}})}_{\text{扩散自然度}}
+ \lambda_3 \underbrace{\Phi_\mathrm{enr}(\hat{\mathbf{S}})}_{\text{能量守恒}}
\]

其中 \(\mathbf{X}(n,k) = [X_L(n,k),\, X_R(n,k)]^\mathsf{T}\)，\(\hat{\mathbf{S}}(n,k) \in \mathbb{C}^{12}\) 为各输出声道的 STFT 估计。

**Dolby 思路对应关系**：
- 下混一致性 → 不破坏原混音意图
- 空间平滑 → 避免频谱「拉丝」与声道跳变
- 扩散自然度 → 环绕/天空声道的去相关与宽扩散
- 能量守恒 → 防止 12 路叠加过载

---

## 2. 扬声器几何与坐标系

### 2.1 球坐标

每个扬声器 \(i\) 用方位角 \(\theta_i\)（水平，0° 为正前方，左为正）和仰角 \(\phi_i\)（水平面为 0°，向上为正）描述：

\[
\mathbf{u}_i = \begin{bmatrix}
\cos\phi_i \cos\theta_i \\
\cos\phi_i \sin\theta_i \\
\sin\phi_i
\end{bmatrix}
\]

### 2.2 7.1.4 参考布局（Dolby / ITU 兼容）

| 声道 | \(\theta_i\) | \(\phi_i\) |
|------|-------------|-----------|
| L | +30° | 0° |
| R | −30° | 0° |
| C | 0° | 0° |
| Ls | +110° | 0° |
| Rs | −110° | 0° |
| Lrs | +150° | 0° |
| Rrs | −150° | 0° |
| Ltf | +45° | +45° |
| Rtf | −45° | +45° |
| Ltr | +135° | +45° |
| Rtr | −135° | +45° |

LFE 不参与 VBAP 二维/三维定位，单独处理。

---

## 3. 时频分析层

### 3.1 STFT

窗函数 \(w[m]\)（推荐 Hann），帧移 \(H\)，FFT 长度 \(N\)：

\[
X_L(n,k) = \sum_{m=0}^{N-1} L[m + nH]\, w[m]\, e^{-j2\pi km/N}
\]

\(X_R(n,k)\) 同理。综合向量 \(\mathbf{X}(n,k) = [X_L,\, X_R]^\mathsf{T}\)。

### 3.2 功率与交叉谱

\[
P_L(n,k) = |X_L(n,k)|^2, \quad
P_R(n,k) = |X_R(n,k)|^2
\]

\[
P_{LR}(n,k) = X_L(n,k)\, X_R^*(n,k)
\]

平滑（时间/频率 EMA，增益 \( \alpha_t, \alpha_f \)）：

\[
\tilde{P}_\cdot(n,k) = \alpha \tilde{P}_\cdot(n-1,k) + (1-\alpha) P_\cdot(n,k)
\]

后续公式中的 \(P\) 均指平滑后的 \(\tilde{P}\)，略去波浪号。

### 3.3 双耳线索（Binaural Cues）

**强度差 ILD**（dB）：

\[
\mathrm{ILD}(n,k) = 10 \log_{10} \frac{P_L(n,k) + \epsilon}{P_R(n,k) + \epsilon}
\]

**相位差 IPD**：

\[
\mathrm{IPD}(n,k) = \angle P_{LR}(n,k) = \angle X_L(n,k) - \angle X_R(n,k)
\]

**归一化互相关（频域相干度）**：

\[
\gamma(n,k) = \frac{|P_{LR}(n,k)|}{\sqrt{P_L(n,k)\, P_R(n,k) + \epsilon}} \in [0,1]
\]

\(\gamma \to 1\)：高度相关 → 点源 / 同相内容（中心或前方）。  
\(\gamma \to 0\)：不相关 → 扩散 / 混响 / 反相宽声场。

---

## 4. 空间参数估计

### 4.1 水平方位角 \(\hat\theta(n,k)\)

由 ILD 与 IPD 联合估计。简化线性模型（正前方为 0°，左为正）：

\[
\hat\theta_\mathrm{ILD}(n,k) = \mathrm{clip}\bigl(K_\mathrm{ILD}\,\mathrm{ILD}(n,k),\,-\theta_\max,\,+\theta_\max\bigr)
\]

\[
\hat\theta_\mathrm{IPD}(n,k) = \mathrm{clip}\bigl(K_\mathrm{IPD}(f_k)\,\mathrm{IPD}(n,k),\,-\theta_\max,\,+\theta_\max\bigr)
\]

\(K_\mathrm{IPD}(f_k)\) 随频率变化（高频 IPD 更可靠）。融合：

\[
\hat\theta(n,k) = w_\mathrm{ILD}(f_k)\,\hat\theta_\mathrm{ILD} + w_\mathrm{IPD}(f_k)\,\hat\theta_\mathrm{IPD}, \qquad w_\mathrm{ILD} + w_\mathrm{IPD} = 1
\]

低频（\(f < f_\mathrm{low}\)）ILD 权重大；中高频 IPD 权重增加。

### 4.2 仰角 \(\hat\phi(n,k)\)（高度推断）

立体声不含真实高度信息，需从 **频谱与相干特征推断**。定义高度激活函数：

\[
\Psi(n,k) = \underbrace{\frac{P_L + P_R}{P_\mathrm{tot}(n) + \epsilon}}_{\text{总能量占比}}
\cdot \underbrace{(1 - \gamma(n,k))^\eta}_{\text{扩散度}}
\cdot \underbrace{\sigma\!\left(\frac{f_k - f_\mathrm{hf}}{B_\mathrm{hf}}\right)}_{\text{高频权重}}
\]

其中 \(\sigma(\cdot)\) 为 sigmoid，\(f_\mathrm{hf}\) 约 4–6 kHz，\(\eta \in [0.5, 2]\)。

仰角估计：

\[
\hat\phi(n,k) = \phi_\mathrm{max} \cdot \Psi(n,k) \cdot \bigl(1 - |\hat\theta(n,k)| / \theta_\max\bigr)^\beta
\]

物理含义：宽、 diffuse、偏高频的内容 → 更多分配到天空层；强方向性内容 → 保持水平面。

---

## 5. 信号分解（核心：Direct / Diffuse / Center / LFE）

Dolby 类 upmix 的关键不是单一矩阵，而是 **按感知属性分解后再路由**。

### 5.1 频域分解模型

将 \((n,k)\) 处的 L/R 能量分解为四个正交（近似）成分：

\[
\begin{aligned}
X_L &= C + D_L + A_L + B_L \\
X_R &= C + D_R + A_R + B_R
\end{aligned}
\]

| 成分 | 含义 | 路由目标 |
|------|------|---------|
| \(C\) | 中心（单声道同相） | C, 少量 L/R |
| \(D_L, D_R\) | 方向性/direct | VBAP 按 \(\hat\theta, \hat\phi\) |
| \(A_L, A_R\) | 扩散/ambient | 环绕 + 天空 + 去相关 |
| \(B_L, B_R\) | 超低频 | LFE |

### 5.2 中心分量提取

**相干度法**（Pro Logic 思想的频域推广）：

\[
C(n,k) = \frac{P_{LR}(n,k)}{|P_{LR}(n,k)| + \epsilon} \cdot \min\bigl(|X_L|, |X_R|\bigr) \cdot \gamma(n,k)^\mu
\]

或 **Mid 投影**：

\[
C(n,k) = \alpha_C(n,k) \cdot \frac{X_L(n,k) + X_R(n,k)}{2}, \qquad
\alpha_C = \mathrm{clip}(\gamma(n,k)^\mu,\, 0,\, 1)
\]

### 5.3 扩散分量提取

去除中心后的残差：

\[
R_L = X_L - C, \quad R_R = X_R - C
\]

扩散权重：

\[
\alpha_A(n,k) = (1 - \gamma(n,k))^\eta
\]

扩散分量（保留宽声场）：

\[
A_L = \alpha_A \cdot R_L, \quad A_R = \alpha_A \cdot R_R
\]

### 5.4 方向性/direct 分量

\[
D_L = (1 - \alpha_A) \cdot R_L, \quad D_R = (1 - \alpha_A) \cdot R_R
\]

合成 direct 复包络（用于 VBAP 增益计算）：

\[
D(n,k) = \frac{D_L + D_R}{2} + j \cdot \kappa \cdot \frac{D_L - D_R}{2}
\]

\(\kappa\) 控制立体声相位信息保留程度。

### 5.5 LFE 分量

一阶 Linkwitz-Riley 或 Butterworth 低通，截止 \(f_c\)（通常 80–120 Hz）：

\[
B_L(n,k) = H_\mathrm{LP}(f_k)\, X_L(n,k), \quad
B_R(n,k) = H_\mathrm{LP}(f_k)\, X_R(n,k)
\]

\[
Y_\mathrm{LFE}(n,k) = G_\mathrm{LFE} \cdot \bigl(B_L(n,k) + B_R(n,k)\bigr)
\]

并从 \(D, A\) 路径中减去 \(B\) 的贡献，避免低频重复。

---

## 6. 路由层：VBAP 与扩散 spreading

### 6.1 VBAP（Vector Base Amplitude Panning）

对 direct 分量 \(D(n,k)\)，给定目标方向 \(\mathbf{u}(\hat\theta, \hat\phi)\)，在扬声器三角网格上求非负增益 \(\mathbf{g} = [g_1,\ldots,g_{11}]^\mathsf{T}\)（不含 LFE）：

\[
\mathbf{u}(\hat\theta, \hat\phi) = \sum_i g_i\, \mathbf{u}_i, \qquad g_i \ge 0, \quad \sum_i g_i = 1
\]

三维 VBAP：选取包含 \(\mathbf{u}\) 的三个扬声器构成基 \(\mathbf{U}_3 = [\mathbf{u}_a\ \mathbf{u}_b\ \mathbf{u}_c]\)：

\[
\begin{bmatrix} g_a \\ g_b \\ g_c \end{bmatrix}
= \mathbf{U}_3^{-1}\, \mathbf{u}(\hat\theta, \hat\phi)
\]

若出现负增益，退到 **二维水平 VBAP**（\(\phi=0\) 平面三角网格），高度分量单独处理（见 6.3）。

各输出声道 direct 贡献：

\[
Y_{\mathrm{dir},i}(n,k) = g_i(\hat\theta, \hat\phi)\, D(n,k)
\]

### 6.2 扩散 spreading 矩阵

扩散分量 \([A_L, A_R]^\mathsf{T}\) 不应简单拷贝到所有环绕声道。定义 **扩散 spreading 矩阵** \(\mathbf{M}_A \in \mathbb{R}^{12 \times 2}\)：

\[
\mathbf{Y}_A(n,k) = \mathbf{M}_A \cdot \begin{bmatrix} A_L(n,k) \\ A_R(n,k) \end{bmatrix}
\]

\(\mathbf{M}_A\) 结构（示意）：

\[
\mathbf{M}_A =
\begin{bmatrix}
0 & 0 \\
0 & 0 \\
0 & 0 \\
0 & 0 \\
m_\mathrm{Ls} & 0 \\
0 & m_\mathrm{Rs} \\
m_\mathrm{Lrs} & 0 \\
0 & m_\mathrm{Rrs} \\
m_\mathrm{Ltf} & 0 \\
0 & m_\mathrm{Rtf} \\
m_\mathrm{Ltr} & 0 \\
0 & m_\mathrm{Rtr}
\end{bmatrix}
\]

系数 \(m\) 由 \(\hat\phi\) 调制：\(\phi\) 越大，天空行权重越高：

\[
m_\mathrm{Ltf} = G_\mathrm{height}\, w_\mathrm{tf}(\hat\phi), \quad \text{etc.}
\]

### 6.3 高度层专用路由

天空声道接收两部分：
1. **Direct 高度**：\(\hat\phi > \phi_\mathrm{thr}\) 时，VBAP 在 {Ltf, Rtf, Ltr, Rtr} 子集上二次 panning
2. **Ambient 高度**：\(A_L, A_R\) 经 \(\mathbf{M}_A\) 的天空行 + 去相关

\[
Y_{\mathrm{height},i}(n,k) = g_i^\mathrm{height}(\hat\theta, \hat\phi)\, D(n,k) + Y_{A,i}(n,k)
\]

---

## 7. 去相关（Decorrelation）

### 7.1 问题

若 \(Y_{\mathrm{Ls}} = Y_{\mathrm{Rs}} = A\)，则物理上为单点源，无包围感。Dolby 对 ambient 通道施加 **全通扩散链**。

### 7.2 全通滤波器模型

对每个 ambient 声道 \(i\)，独立全通：

\[
H_i(z) = \frac{a_i + z^{-1}}{1 + a_i z^{-1}}, \qquad |H_i(e^{j\omega})| = 1
\]

级联 \(Q\) 节，各节 \(a_i^{(q)}\) 不同（如 0.3–0.7 伪随机但固定），群延迟 \(\tau_i(f)\) 随频率变化。

频域近似（STFT 域逐帧）：

\[
\tilde{Y}_{A,i}(n,k) = H_i(k) \cdot Y_{A,i}(n,k)
\]

\(H_i(k)\) 为全通链在 bin \(k\) 的复响应（离线预计算）。

### 7.3 可选微延迟

环绕声道附加样本延迟 \(d_i\)（5–25 ms 量级，各声道不同）：

\[
\tilde{Y}_{A,i}(n,k) \leftarrow \tilde{Y}_{A,i}(n,k)\, e^{-j2\pi k d_i / N}
\]

---

## 8. 合成方程（完整 STFT 域）

综合所有路径：

\[
\boxed{
\hat{Y}_i(n,k) =
\underbrace{g_i(\hat\theta, \hat\phi)\, D(n,k)}_{\text{direct VBAP}}
+ \underbrace{(\mathbf{M}_A \mathbf{A})_i(n,k)}_{\text{ambient spread}}
+ \underbrace{\delta_{i,C}\, C(n,k) + \delta_{i,L}\, D_L + \delta_{i,R}\, D_R}_{\text{center / 主声道}}
+ \underbrace{\delta_{i,\mathrm{LFE}}\, Y_\mathrm{LFE}(n,k)}_{\text{LFE}}
}
\]

经去相关：

\[
Y_i(n,k) = \hat{Y}_i(n,k) \cdot \mathbb{1}_{i \notin \mathrm{ambient}} + H_i(k)\, \hat{Y}_i(n,k) \cdot \mathbb{1}_{i \in \mathrm{ambient}}
\]

逆 STFT 合成时域输出 \(\mathbf{y}(t)\)。

---

## 9. 时域平滑与下混一致性约束

### 9.1 参数时间平滑

\(\hat\theta, \hat\phi, \alpha_A, \alpha_C\) 在帧间 EMA 平滑，防止声像抖动：

\[
\hat\theta(n) = \alpha_t \hat\theta(n-1) + (1-\alpha_t)\,\hat\theta_\mathrm{raw}(n)
\]

### 9.2 下混一致性投影（可选 refinement）

Dolby 接收器常保证 upmix 后再 downmix 接近原立体声。定义下混矩阵 \(\mathbf{P} \in \mathbb{R}^{2 \times 12}\)（ITU 标准系数）：

\[
\mathbf{P}\,\hat{\mathbf{Y}}(n,k) \approx \mathbf{X}(n,k)
\]

若残差 \(\mathbf{R} = \mathbf{X} - \mathbf{P}\hat{\mathbf{Y}}\) 能量超过阈值，将 \(\mathbf{R}\) 按比例加回 L/R 主声道：

\[
Y_L \leftarrow Y_L + r_L\, R_L, \quad Y_R \leftarrow Y_R + r_R\, R_R
\]

\(r_L, r_R\) 由最小二乘或固定系数确定。

### 9.3 能量归一化

防止 12 路叠加过载：

\[
G(n,k) = \min\left(1,\; \frac{P_L + P_R}{\sum_i |\hat{Y}_i|^2 + \epsilon}\right)
\]

\[
Y_i(n,k) \leftarrow G(n,k)\, Y_i(n,k)
\]

---

## 10. 用户参数化（映射到数学量）

| UI 参数 | 数学映射 |
|---------|---------|
| Width \(w\) | \(\alpha_A \leftarrow \alpha_A \cdot w\) |
| Height \(h\) | \(G_\mathrm{height} \leftarrow h\)，\(\phi_\max \leftarrow h \cdot \phi_\mathrm{max,0}\) |
| Center \(c\) | \(\alpha_C \leftarrow c \cdot \gamma^\mu\) |
| LFE Level | \(G_\mathrm{LFE}\) |
| LFE Crossover | \(f_c\) |
| Surround Mix | \(\mathbf{M}_A\) 中环绕行标量 |
| Direct/Ambient Bias | \(\eta\) in \((1-\gamma)^\eta\) |

---

## 11. 与 Dolby Surround 的对标要点

| Dolby 特性 | 本模型对应模块 |
|-----------|---------------|
| 自适应矩阵解码 | §5 相干度分解 \(C, D, A\) |
| 多声道 panning | §6 VBAP |
| Surround / Height 合成 | §4.2 \(\hat\phi\)，§6.3 高度路由 |
| 非相关环绕 | §7 全通去相关 |
| 对话清晰（Center） | §5.2 \(\alpha_C(\gamma)\) |
| 低频管理 | §5.5 LFE 独立路径 |
| 下混兼容 | §9.2 一致性投影 |

**差距与后续增强**（Phase 2+）：
- Dolby 使用专有 **transient / object** 检测 → 可加 §12 瞬态模型
- 部分场景用 ML 分类 direct/ambient → 可替换 \(\alpha_A\) 为学习分类器
- 内容自适应（Movie / Music profile）→ 调整 \(\lambda_i, \eta, f_\mathrm{hf}\)

---

## 12. 瞬态检测扩展（Phase 2）

定义频谱通量：

\[
\Phi(n,k) = \max\bigl(0,\; \log P(n,k) - \log P(n-1,k)\bigr)
\]

瞬态掩码：

\[
T(n,k) = \sigma\left(\frac{\sum_k \Phi(n,k) - \tau_\mathrm{tr}}{\Delta}\right)
\]

瞬态帧：**减小** \(\alpha_A\)，**增大** direct VBAP 权重，使打击乐保留在前方 L/R/C，避免「糊到环绕」。

\[
\alpha_A(n,k) \leftarrow \alpha_A(n,k)\,(1 - T(n))
\]

---

## 13. 处理流程总览

```
x(t) = [L, R]
    │
    ▼ STFT ─────────────────────────────────────────┐
    │                                                │
    ├─→ P_L, P_R, P_LR, γ, ILD, IPD                 │
    │         │                                      │
    │         ├─→ θ̂, φ̂                            │
    │         │                                      │
    ├─→ 分解: C, D_L/R, A_L/R, B_L/R               │
    │         │                                      │
    │         ├─→ VBAP( D, θ̂, φ̂ ) → Y_dir          │
    │         ├─→ M_A · [A_L, A_R] → Y_amb           │
    │         ├─→ C → Y_C, Y_L/R                     │
    │         └─→ B → Y_LFE                          │
    │                   │                            │
    │                   ▼                            │
    │         合成 Ŷ_i + 去相关 H_i                   │
    │                   │                            │
    │                   ▼                            │
    │         下混一致性 + 能量归一化                  │
    │                   │                            │
    ◄─────── iSTFT ◄────┘                            │
    │
    ▼
y(t) ∈ R^12
```

---

## 14. 推荐默认超参数（初调起点）

| 参数 | 建议值 |
|------|--------|
| STFT \(N\) | 2048 |
| Hop \(H\) | 512（75% overlap） |
| \(f_c\) (LFE) | 100 Hz |
| \(f_\mathrm{hf}\) | 5000 Hz |
| \(\theta_\max\) | 150° |
| \(\phi_\max\) | 45° |
| \(\mu\) (center) | 1.5 |
| \(\eta\) (ambient) | 1.0 |
| \(\alpha_t\) (时间平滑) | 0.85 |
| \(G_\mathrm{LFE}\) | 0.707 |
| 全通级数 \(Q\) | 3 |

---

## 15. 实现模块映射（`Source/Upmix/`）

| 模块 | 实现文档章节 |
|------|-------------|
| `StftEngine` | §3 |
| `SpatialEstimator` | §4 |
| `SignalDecomposer` | §5 |
| `VbapPanner714` | §6.1 |
| `AmbientSpreader` | §6.2–6.3 |
| `Decorrelator` | §7 |
| `UpmixEngine` | §8–9 |
| `TransientDetector` | §12（Phase 2） |

---

## 16. 下一步

1. 确认本数学模型是否符合你的预期（尤其是 §5 分解 + §6 VBAP 框架）
2. 定稿超参数与用户参数表
3. 在 `Source/Upmix/` 按 §15 实现 Phase 1（STFT + 分解 + VBAP + 去相关）
4. AB 测试：下混一致性、宽度、高度、对话清晰度
