# Mi 10 / 10 Pro KernelSU-Next + SUSFS 自建内核

基于 clcwpwqi/xiaomi10-kernelsu-susfs-kernel-build 的 workflow 改造，适配 2026 年的上游现状。

## 与原版的差异

1. **KSU-Next 集成重写**：原版写死 `rifsxd/KernelSU-Next` 的 `next-susfs` 分支（已删除）。现改为官方 setup.sh + `v3.4.0-legacy` tag（可用 `ksu_tag` 输入改）。选 legacy 线的原因：
   - KSU-Next v3.x 主线只支持 kprobes 挂钩（`depends on KPROBES`），且内核代码已无任何 SUSFS 集成；
   - legacy 线保留 `KSU_MANUAL_HOOK`（KPROBES 关闭时自动启用），与 susfs4ksu 官方"非 GKI 手动挂钩"路线一致；
   - 内核源码不含 hook 调用点，由本仓库 `patches/ksun-legacy-manual-hooks.patch` 写入（fs/exec.c 的 do_execveat_common 单点覆盖 execve 全家、fs/open.c faccessat、fs/stat.c newfstatat/fstatat64、kernel/reboot.c sys_reboot——最后一处是 v3.x Kbuild 的集成校验点）。旧 non-kprobes.patch 不可用：其调用的 `ksu_handle_vfs_read` 在 v3.x 已不存在，会导致链接失败。
2. **SUSFS 集成**：susfs4ksu `kernel-4.19` 分支 fs 侧补丁 + 本仓库 `patches/ksu-susfs-kconfig.fragment`（Kconfig 声明）+ `patches/susfs_ksu_compat.c`（胶水：该分支 fs 侧补丁引用的 3 个域检查/try_umount 函数在其 susfs.c 中缺失，此文件委托给 KSU-Next legacy 自带的 selinux 基础设施，编入 kernelsu 对象）。
3. **defconfig 显式启用 KSU/SUSFS**：n0kernel 的 cmi/umi defconfig 不含任何 `CONFIG_KSU*`（首跑因此静默编出无 KSU 空壳）。现于 defconfig 后用 `scripts/config` 启用全部选项 + `olddefconfig`，并 fail-fast 断言 `CONFIG_KSU=y / KSU_MANUAL_HOOK=y / KSU_SUSFS=y / SUS_MOUNT / TRY_UMOUNT / HAS_MAGIC_MOUNT`。
4. runner `ubuntu-22.04` → `ubuntu-24.04`；artifact 保留 7 天；型号选错显式失败。

## 用法

1. 本仓库 Actions → 选 `Build KSUN SUSFS Kernel` → Run workflow → 选「小米10Pro」（cmi）或「小米10」（umi）；`ksu_tag` 留默认即 legacy 线，`version` 可自定义内核版本后缀
2. 跑完在 Artifacts 下载 `SM8250-KernelSU-Next-SUSFS`（下载的 zip 即 AnyKernel3 卡刷包，可直接 TWRP 刷入）
3. 刷机前备份 boot！

## 已知依赖（构建时现拉的上游）

- 内核：clcwpwqi/kernel_xiaomi_sm8250_n0kernel（munch 分支，2025-06；上游 jhchong94 仍在活跃）
- KSU：KernelSU-Next/KernelSU-Next tag `v3.4.0-legacy`（2026-09-21）
- 补丁/AnyKernel3：clcwpwqi/xiaomi10-kernelsu-susfs-kernel-build（src）
- SUSFS：gitlab.com/simonpunk/susfs4ksu `kernel-4.19` 分支
- 工具链：ZyCromerZ Clang 19.0.0git-20240125

## 风险与预案

- 若 `patch -p1 < non-kprobes.patch` 失败：内核源码 fs/ 文件变化，需按新源码手工重写 hook 调用点。
- 若 fail-fast 断言失败（`CONFIG_KSU* 未生效`）：多为 KSU tag 换成了非 legacy 线（主线 KSU `depends on KPROBES`）或 Kconfig 片段与 susfs4ksu 新版不再对齐。
- SUS_SU 特性按 susfs4ksu 官方说明在非 GKI 上禁用（其 Kconfig 依赖 KPROBES，天然不生效）。
- 首跑（2026-09-27）教训：原版 workflow 从不启用 `CONFIG_KSU`，编出的 Image `KernelSU/susfs` 字符串数为 0——已由断言杜绝复发。
