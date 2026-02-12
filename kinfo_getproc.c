/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2009 Ulf Lilleengen
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/user.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "libutil.h"

#ifndef nitems
#define	nitems(x)	(sizeof((x)) / sizeof((x)[0]))
#endif

struct kinfo_proc *
kinfo_getproc(pid_t pid)
{
	struct kinfo_proc *kipp;
	size_t len;
#if defined(__APPLE__)
	int mib[4];

	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	mib[2] = KERN_PROC_PID;
	mib[3] = pid;

	for (;;) {
		len = 0;
		if (sysctl(mib, nitems(mib), NULL, &len, NULL, 0) < 0)
			return (NULL);
		if (len == 0) {
			errno = ESRCH;
			return (NULL);
		}

		kipp = malloc(len);
		if (kipp == NULL)
			return (NULL);

		if (sysctl(mib, nitems(mib), kipp, &len, NULL, 0) < 0) {
			if (errno == ENOMEM) {
				free(kipp);
				continue;
			}
			goto bad;
		}
		break;
	}

	if (len < sizeof(*kipp))
		goto bad;
	if (kipp->kp_proc.p_pid != pid)
		goto bad;
	return (kipp);
#else
	int mib[4];

	len = sizeof(*kipp);
	kipp = malloc(len);
	if (kipp == NULL)
		return (NULL);

	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	mib[2] = KERN_PROC_PID;
	mib[3] = pid;

	if (sysctl(mib, nitems(mib), kipp, &len, NULL, 0) < 0)
		goto bad;
	if (len != sizeof(*kipp))
		goto bad;
	if (kipp->ki_structsize != sizeof(*kipp))
		goto bad;
	if (kipp->ki_pid != pid)
		goto bad;
	return (kipp);
#endif
bad:
	free(kipp);
	return (NULL);
}
