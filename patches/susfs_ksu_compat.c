/* susfs_ksu_compat.c
 *
 * Glue for the susfs4ksu kernel-4.19 branch inconsistency: its
 * 50_add_susfs_in_kernel-4.19.patch references three ksu/zygote domain
 * helpers that this branch's fs/susfs.c does not define (only newer susfs
 * versions / the KSU-side integration provide them). This file supplies
 * minimal, faithful implementations by delegating to KernelSU-Next's own
 * selinux infrastructure, and routes susfs_try_umount_all() to the susfs
 * try-umount list processing (KSU-Next keeps doing its own try_umount via
 * its setuid LSM hook).
 */
#include <linux/types.h>
#include <linux/cred.h>
#include "selinux/selinux.h"

extern void susfs_try_umount(uid_t uid);

extern bool is_ksu_domain(void);
extern bool is_zygote(const struct cred *cred);

#ifdef CONFIG_KSU_SUSFS

bool susfs_is_current_ksu_domain(void)
{
	return is_ksu_domain();
}

bool susfs_is_current_zygote_domain(void)
{
	return is_zygote(current_cred());
}

void susfs_try_umount_all(uid_t uid)
{
	susfs_try_umount(uid);
}

#endif /* CONFIG_KSU_SUSFS */
