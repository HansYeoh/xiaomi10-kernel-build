# Mi 10 / 10 Pro KernelSU-Next + SUSFS 自建内核

基于 clcwpwqi/xiaomi10-kernelsu-susfs-kernel-build 的 workflow 改造，修好了 2026 年已失效的上游引用。

## 与原版的差异

1. **第 3 步 KSU-Next 集成重写**：原版写死 `rifsxd/KernelSU-Next` 的 `next-susfs` 分支，该分支已被删除（SUSFS 支持已并入 KSU-Next 主线）。改为从 `KernelSU-Next/KernelSU-Next` 的 `dev` 分支取官方 setup.sh：
   - `ksu_tag` 留空 → 每次构建自动 checkout 最新 release tag（这就是"同步上游"，无需任何维护）
   - `ksu_tag` 填如 `v3.4.0` → 固定版本，构建可复现
2. runner `ubuntu-22.04` → `ubuntu-24.04`（22.04 已进入淘汰周期；clang 工具链自带，不受影响）
3. 新增 `ksu_tag` 手动输入参数；型号输入错误时显式 `exit 1`（原版会带着空 DEFCONFIG 继续跑）
4. 构建并行度 `-j8` → `-j$(nproc)`；artifact 保留 3 天 → 7 天

## 用法

1. 新建一个 GitHub 公开仓库，把 `.github/workflows/build_ksun_susfs.yml` 原样推上去（不需要 fork 原仓库——所有源码都是构建时从上游 URL 现拉的）
2. Actions 页 → 选这个 workflow → Run workflow → 选「小米10Pro」（cmi，即你的测试机），可选填版本名和 KSU 版本
3. 跑完在 Artifacts 里下载 `SM8250-KernelSU-Next-SUSFS`（AnyKernel3 卡刷包）

## 风险与预案

- 第 4 步的预修复补丁是仓库作者 2025 年针对旧版 KSU-Next 写的，对 v3.x 可能冲突。首跑挂了优先看第 4/6 步日志：删掉对应补丁再试，或把 `ksu_tag` 回退到能编过的旧版（v3.0.x / 带 `-legacy` 后缀的变体）。
- SUSFS 内核侧补丁每次构建从 susfs4ksu `kernel-4.19` 分支现拉（已验证该分支存活），也是自动同步。
- 内核 base 是 clcwpwqi 的 fork（2025-06 后未更新），上游 jhchong94/kernel_xiaomi_sm8250_n0kernel 仍在活跃（2026-06）。想要上游新特性需自己 rebase fork，不影响当前可用性。
- 刷机前备份 boot！不开机可刷回原 boot 恢复。
