# Gomoku-AI: 基于 Alpha-Beta 剪枝的智能五子棋

![C++](https://img.shields.io/badge/Language-C++-blue.svg)
![EasyX](https://img.shields.io/badge/Library-EasyX-orange.svg)
![License](https://img.shields.io/badge/License-MIT-green.svg)

这是一个基于 **C++** 和 **EasyX 图形库** 开发的五子棋项目。本项目是作为大一学期的设计作业开发的，重点实现了 **Alpha-Beta 剪枝算法**，使 AI 具备了较高的对弈水平和决策速度。

---

##项目亮点

- **智能 AI 决策**: 采用经典的博弈树搜索算法，支持多层深度迭代，寻找最优落子点。
- **性能优化**: 引入 **Alpha-Beta 剪枝** 技术过滤无效搜索分支，配合启发式评估函数，保证了流畅的对弈体验。
- **可视化搜索过程**: 支持“思考虚影”功能，AI 尝试过的候选点会以半透明圈形式实时展示，直观展示 AI 的思考路径。
- **多种模式**: 支持人机对战（支持选择 AI 先/后手）以及机机对战模式（用于观察 AI 算法互搏）。
- **友好交互**: 基于 EasyX 打造的清爽木质棋盘 UI，支持实时显示 AI 耗时、当前得分及落子坐标。

---

## 编译环境

由于项目使用了 Windows 平台专用的 **EasyX** 图形库，请确保你的开发环境满足以下条件：

- **操作系统**: Windows 10/11
- **开发工具**: Visual Studio 2019 / 2022 (推荐)
- **图形库**: [EasyX Graphics Library](https://easyx.cn/) (请先下载并安装到对应的 Visual Studio 版本中)

---
