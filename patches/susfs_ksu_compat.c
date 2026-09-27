/* susfs_ksu_compat.c
 *
 * Glue for the susfs4ksu kernel-4.19 branch inconsistency: its
 * 50_add_susfs_in_kernel-4.19.patch and fs/susfs.c reference ksu/zygote
 * domain helpers and a 4-arg ksu_try_umount() that this branch's susfs.c
 * does not define (newer susfs versions and the old next-susfs KSU tree
 * provided them). This file supplies minimal, faithful implementations:
 * domain checks delegate to KernelSU-Next's own selinux infrastructure,
 * and ksu_try_umount() umounts via the kernel's own path_umount() the same
 * way upstream KSU does (never inside ksu-domain processes; with check_mnt
 * only mounts whose device name marks them as KSU mounts).
 */
#include <linux/types.h>
#include <linux/cred.h>
#include <linux/namei.h>
#include <linux/path.h>
#include <linux/printk.h>
#include "selinux/selinux.h"

extern bool is_ksu_domain(void);
extern bool is_zygote(const struct cred *cred);
extern void susfs_try_umount(uid_t uid);
extern bool susfs_is_mnt_devname_ksu(struct path *path);
extern int path_umount(struct path *path, int flags);

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

/* path_umount() consumes the path reference on all return paths. */
void ksu_try_umount(const char *mnt, bool check_mnt, int flags, uid_t uid)
{
	struct path path;
	int err;

	if (susfs_is_current_ksu_domain())
		return;

	err = kern_path(mnt, 0, &path);
	if (err)
		return;

	if (check_mnt && !susfs_is_mnt_devname_ksu(&path)) {
		path_put(&path);
		return;
	}

	err = path_umount(&path, flags);
	if (err)
		pr_warn("susfs_ksu_compat: umount '%s' for uid %u failed: %d\n",
			mnt, uid, err);
}

#endif /* CONFIG_KSU_SUSFS */
