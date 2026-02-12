/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2007 Robert N. M. Watson
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


/*
 * Sort processes first by pid and then tid.
 */
static int
kinfo_proc_compare(const void *a, const void *b)
{
#if defined(__APPLE__)
	pid_t pa;
	pid_t pb;

	pa = ((const struct kinfo_proc *)a)->kp_proc.p_pid;
	pb = ((const struct kinfo_proc *)b)->kp_proc.p_pid;
	if (pa < pb)
		return (-1);
	if (pa > pb)
		return (1);
	return (0);
#else
	int i;

	i = ((const struct kinfo_proc *)a)->ki_pid -
	    ((const struct kinfo_proc *)b)->ki_pid;
	if (i != 0)
		return (i);
	i = ((const struct kinfo_proc *)a)->ki_tid -
	    ((const struct kinfo_proc *)b)->ki_tid;
	return (i);
#endif
}

static void
kinfo_proc_sort(struct kinfo_proc *kipp, int count)
{

	qsort(kipp, count, sizeof(*kipp), kinfo_proc_compare);
}

struct kinfo_proc *
kinfo_getallproc(int *cntp)
{
	struct kinfo_proc *kipp;
	size_t len;
#if defined(__APPLE__)
	int mib[4];
#else
	int mib[3];
#endif

#if defined(__APPLE__)
	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	mib[2] = KERN_PROC_ALL;
	mib[3] = 0;

	for (;;) {
		len = 0;
		if (sysctl(mib, nitems(mib), NULL, &len, NULL, 0) < 0)
			return (NULL);

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

	if (len % sizeof(*kipp) != 0)
		goto bad;
	*cntp = len / sizeof(*kipp);
	kinfo_proc_sort(kipp, *cntp);
	return (kipp);
#else
	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	mib[2] = KERN_PROC_PROC;

	len = 0;
	if (sysctl(mib, nitems(mib), NULL, &len, NULL, 0) < 0)
		return (NULL);

	kipp = malloc(len);
	if (kipp == NULL)
		return (NULL);

	if (sysctl(mib, nitems(mib), kipp, &len, NULL, 0) < 0)
		goto bad;
	if (len % sizeof(*kipp) != 0)
		goto bad;
	if (kipp->ki_structsize != sizeof(*kipp))
		goto bad;
	*cntp = len / sizeof(*kipp);
	kinfo_proc_sort(kipp, len / sizeof(*kipp));
	return (kipp);
#endif
bad:
	*cntp = 0;
	free(kipp);
	return (NULL);
}
