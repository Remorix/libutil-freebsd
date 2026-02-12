#include <sys/cdefs.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/user.h>
#if defined(__APPLE__)
#include <libproc.h>
#include <mach/vm_prot.h>
#include <sys/proc_info.h>
#endif
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "libutil.h"

#ifndef nitems
#define	nitems(x)	(sizeof((x)) / sizeof((x)[0]))
#endif

#if defined(__APPLE__)
/*
 * macOS does not expose FreeBSD's struct kinfo_vmentry in <sys/user.h>.
 * Use a compatible local layout and return it through the public pointer type.
 */
struct kinfo_vmentry_compat {
	int	 kve_structsize;
	int	 kve_type;
	uint64_t kve_start;
	uint64_t kve_end;
	uint64_t kve_offset;
	uint64_t kve_vn_fileid;
	uint32_t kve_vn_fsid_freebsd11;
	int	 kve_flags;
	int	 kve_resident;
	int	 kve_private_resident;
	int	 kve_protection;
	int	 kve_ref_count;
	int	 kve_shadow_count;
	int	 kve_vn_type;
	uint64_t kve_vn_size;
	uint32_t kve_vn_rdev_freebsd11;
	uint16_t kve_vn_mode;
	uint16_t kve_status;
	union {
		uint64_t _kve_vn_fsid;
		uint64_t _kve_obj;
	} kve_type_spec;
	uint64_t kve_vn_rdev;
	int	 _kve_ispare[8];
	char	 kve_path[PATH_MAX];
};
#define	kve_vn_fsid	kve_type_spec._kve_vn_fsid
#define	kve_obj		kve_type_spec._kve_obj

#ifndef KVME_TYPE_NONE
#define	KVME_TYPE_NONE		0
#define	KVME_TYPE_DEFAULT	1
#define	KVME_TYPE_VNODE		2
#define	KVME_TYPE_SWAP		3
#define	KVME_TYPE_DEVICE	4
#define	KVME_TYPE_PHYS		5
#define	KVME_TYPE_DEAD		6
#define	KVME_TYPE_SG		7
#define	KVME_TYPE_MGTDEVICE	8
#define	KVME_TYPE_GUARD		9
#define	KVME_TYPE_UNKNOWN	255
#endif

#ifndef KVME_PROT_READ
#define	KVME_PROT_READ		0x00000001
#define	KVME_PROT_WRITE		0x00000002
#define	KVME_PROT_EXEC		0x00000004
#define	KVME_MAX_PROT_READ	0x00010000
#define	KVME_MAX_PROT_WRITE	0x00020000
#define	KVME_MAX_PROT_EXEC	0x00040000
#endif

#ifndef KVME_FLAG_COW
#define	KVME_FLAG_COW		0x00000001
#define	KVME_FLAG_USER_WIRED	0x00000040
#endif
#endif

struct kinfo_vmentry *
kinfo_getvmmap(pid_t pid, int *cntp)
{
#if defined(__APPLE__)
	struct proc_regionwithpathinfo rwpi;
	struct kinfo_vmentry_compat *kiv, *tmp, *kv;
	uint64_t addr, next;
	int cnt, cap, len;

	if (cntp == NULL) {
		errno = EINVAL;
		return (NULL);
	}

	*cntp = 0;
	kiv = NULL;
	cap = 0;
	cnt = 0;
	addr = 0;

	for (;;) {
		memset(&rwpi, 0, sizeof(rwpi));
		len = proc_pidinfo(pid, PROC_PIDREGIONPATHINFO, addr, &rwpi,
		    sizeof(rwpi));
		if (len <= 0) {
			if (len == 0 && (errno == EINVAL || errno == ESRCH))
				break;
			free(kiv);
			return (NULL);
		}
		if (len < (int)sizeof(rwpi)) {
			free(kiv);
			errno = EINVAL;
			return (NULL);
		}

		if (cnt == cap) {
			cap = cap == 0 ? 64 : cap * 2;
			tmp = reallocf(kiv, cap * sizeof(*kiv));
			if (tmp == NULL)
				return (NULL);
			kiv = tmp;
		}

		kv = &kiv[cnt];
		memset(kv, 0, sizeof(*kv));
		kv->kve_structsize = sizeof(*kv);
		kv->kve_start = rwpi.prp_prinfo.pri_address;
		if (rwpi.prp_prinfo.pri_size > UINT64_MAX - kv->kve_start)
			kv->kve_end = UINT64_MAX;
		else
			kv->kve_end = kv->kve_start + rwpi.prp_prinfo.pri_size;
		kv->kve_offset = rwpi.prp_prinfo.pri_offset;
		kv->kve_flags = 0;
		if (rwpi.prp_prinfo.pri_share_mode == SM_COW)
			kv->kve_flags |= KVME_FLAG_COW;
		if (rwpi.prp_prinfo.pri_user_wired_count != 0)
			kv->kve_flags |= KVME_FLAG_USER_WIRED;
		kv->kve_resident = (int)rwpi.prp_prinfo.pri_pages_resident;
		kv->kve_private_resident =
		    (int)rwpi.prp_prinfo.pri_private_pages_resident;
		kv->kve_ref_count = (int)rwpi.prp_prinfo.pri_ref_count;
		kv->kve_shadow_count = (int)rwpi.prp_prinfo.pri_shadow_depth;
		kv->kve_protection = 0;
		if ((rwpi.prp_prinfo.pri_protection & VM_PROT_READ) != 0)
			kv->kve_protection |= KVME_PROT_READ;
		if ((rwpi.prp_prinfo.pri_protection & VM_PROT_WRITE) != 0)
			kv->kve_protection |= KVME_PROT_WRITE;
		if ((rwpi.prp_prinfo.pri_protection & VM_PROT_EXECUTE) != 0)
			kv->kve_protection |= KVME_PROT_EXEC;
		if ((rwpi.prp_prinfo.pri_max_protection & VM_PROT_READ) != 0)
			kv->kve_protection |= KVME_MAX_PROT_READ;
		if ((rwpi.prp_prinfo.pri_max_protection & VM_PROT_WRITE) != 0)
			kv->kve_protection |= KVME_MAX_PROT_WRITE;
		if ((rwpi.prp_prinfo.pri_max_protection & VM_PROT_EXECUTE) != 0)
			kv->kve_protection |= KVME_MAX_PROT_EXEC;
		kv->kve_status = (uint16_t)rwpi.prp_prinfo.pri_flags;
		kv->kve_obj = (uint64_t)rwpi.prp_prinfo.pri_obj_id;

		if (rwpi.prp_vip.vip_path[0] != '\0') {
			kv->kve_type = KVME_TYPE_VNODE;
			kv->kve_vn_fileid = rwpi.prp_vip.vip_vi.vi_stat.vst_ino;
			kv->kve_vn_fsid = rwpi.prp_vip.vip_vi.vi_stat.vst_dev;
			kv->kve_vn_rdev = rwpi.prp_vip.vip_vi.vi_stat.vst_rdev;
			kv->kve_vn_mode = rwpi.prp_vip.vip_vi.vi_stat.vst_mode;
			kv->kve_vn_type = rwpi.prp_vip.vip_vi.vi_type;
			kv->kve_vn_size = rwpi.prp_vip.vip_vi.vi_stat.vst_size;
			(void)strlcpy(kv->kve_path, rwpi.prp_vip.vip_path,
			    sizeof(kv->kve_path));
		} else {
			kv->kve_type = KVME_TYPE_DEFAULT;
		}

		cnt++;
		if (rwpi.prp_prinfo.pri_size >
		    UINT64_MAX - rwpi.prp_prinfo.pri_address)
			break;
		next = rwpi.prp_prinfo.pri_address + rwpi.prp_prinfo.pri_size;
		if (next <= addr)
			break;
		addr = next;
	}

	if (cnt == 0) {
		free(kiv);
		if (errno == 0)
			errno = ESRCH;
		return (NULL);
	}

	*cntp = cnt;
	return ((struct kinfo_vmentry *)(void *)kiv);
#else
	int mib[4];
	int error;
	int cnt;
	size_t len;
	char *buf, *bp, *eb;
	struct kinfo_vmentry *kiv, *kp, *kv;

	*cntp = 0;
	len = 0;
	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	mib[2] = KERN_PROC_VMMAP;
	mib[3] = pid;

	error = sysctl(mib, nitems(mib), NULL, &len, NULL, 0);
	if (error)
		return (NULL);
	len = len * 4 / 3;
	buf = malloc(len);
	if (buf == NULL)
		return (NULL);
	error = sysctl(mib, nitems(mib), buf, &len, NULL, 0);
	if (error) {
		free(buf);
		return (NULL);
	}
	/* Pass 1: count items */
	cnt = 0;
	bp = buf;
	eb = buf + len;
	while (bp < eb) {
		kv = (struct kinfo_vmentry *)(uintptr_t)bp;
		if (kv->kve_structsize == 0)
			break;
		bp += kv->kve_structsize;
		cnt++;
	}

	kiv = calloc(cnt, sizeof(*kiv));
	if (kiv == NULL) {
		free(buf);
		return (NULL);
	}
	bp = buf;
	eb = buf + len;
	kp = kiv;
	/* Pass 2: unpack */
	while (bp < eb) {
		kv = (struct kinfo_vmentry *)(uintptr_t)bp;
		if (kv->kve_structsize == 0)
			break;
		/* Copy/expand into pre-zeroed buffer */
		memcpy(kp, kv, kv->kve_structsize);
		/* Advance to next packed record */
		bp += kv->kve_structsize;
		/* Set field size to fixed length, advance */
		kp->kve_structsize = sizeof(*kp);
		kp++;
	}
	free(buf);
	*cntp = cnt;
	return (kiv);	/* Caller must free() return value */
#endif
}
