/*
 * Renesas Proprietary Information.
 * The information contained herein is confidential property of
 * Renesas Electronics Corporation
 *
 * Copyright (C) Renesas Electronics Corporation 2013 All rights reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "mmngr_user_public.h"
#include "mmngr_user_private.h"

static int mm_alloc_kh_in_user(MMNGR_ID *pid, unsigned long size,
				unsigned long *pphy_addr,
				unsigned long *phard_addr,
				unsigned long *puser_virt_addr,
				unsigned long flag)
{
	int		fd = -1;
	int		ret;
	void		*virt_addr;
	struct MM_PARAM	p;

	fd = open(DEVFILE, O_RDWR);
	if (fd == -1) {
		perror("MMI open");
		ret = R_MM_FATAL;
		goto exit;
	}

	memset(&p, 0, sizeof(p));
	p.flag = flag;
	p.size = size;
	ret = ioctl(fd, MM_IOC_ALLOC, &p);
	if (ret) {
		if (errno == ENOMEM)
			ret = R_MM_NOMEM;
		else
			ret = R_MM_FATAL;
		perror("MMI ALLOC");
		goto exit;
	}

	virt_addr = mmap((void *)0, size, PROT_READ | PROT_WRITE,
			MAP_SHARED, fd, 0);
	if (virt_addr == MAP_FAILED) {
		perror("MMI mmap()");
		ret = R_MM_FATAL;
		goto exit;
	}

	p.user_virt_addr = (unsigned long)virt_addr;
	ret = ioctl(fd, MM_IOC_SET, &p);
	if (ret) {
		perror("MMI SET");
		ret = R_MM_FATAL;
		goto exit;
	}

	ret = ioctl(fd, MM_IOC_GET, &p);
	if (ret) {
		perror("MMI GET");
		ret = R_MM_FATAL;
		goto exit;
	}

	*puser_virt_addr = p.user_virt_addr;
	*pphy_addr = p.phy_addr >> 12;
	*phard_addr = p.hard_addr;
	*((MMNGR_ID *)pid) = fd;

	return R_MM_OK;

exit:
	if (fd != -1) {
		if (close(fd))
			perror("MMI close");
	}
	return ret;
}

static int mm_free_kh_in_user(MMNGR_ID id)
{
	int		ret;
	struct MM_PARAM	p;

	ret = ioctl(id, MM_IOC_GET, &p);
	if (ret) {
		perror("MMI GET");
		ret = R_MM_FATAL;
		goto exit;
	}

	ret = munmap((void *)p.user_virt_addr, p.size);
	if (ret) {
		perror("MMI munmap");
		ret = R_MM_FATAL;
		goto exit;
	}

	ret = ioctl(id, MM_IOC_FREE, 0);
	if (ret) {
		perror("MMI FREE");
		ret = R_MM_FATAL;
		goto exit;
	}

	ret = close(id);
	if (ret) {
		perror("MMI close");
		ret = R_MM_FATAL;
		goto exit;
	}

	return R_MM_OK;

exit:
	return ret;
}

static int mm_alloc_co_in_user(MMNGR_ID *pid, unsigned long size,
				unsigned long *pphy_addr,
				unsigned long *phard_addr,
				unsigned long *puser_virt_addr,
				unsigned long flag)
{
	int		fd = -1;
	int		ret;
	struct MM_PARAM	p;

	fd = open(DEVFILE, O_RDWR);
	if (fd == -1) {
		perror("MMI open");
		ret = R_MM_FATAL;
		goto exit;
	}

	memset(&p, 0, sizeof(p));
	p.flag = flag;
	p.size = size;
	ret = ioctl(fd, MM_IOC_ALLOC_CO, &p);
	if (ret) {
		if (errno == ENOMEM)
			ret = R_MM_NOMEM;
		else if (errno == EINVAL)
			ret =  R_MM_PARE;
		else
			ret = R_MM_FATAL;
		perror("MMI ALLOC");
		goto exit;
	}

	ret = ioctl(fd, MM_IOC_GET, &p);
	if (ret) {
		perror("MMI GET");
		ret = R_MM_FATAL;
		goto exit;
	}

	*pphy_addr = p.phy_addr >> 12;
	*phard_addr = p.hard_addr;
	*puser_virt_addr = 0;
	*((MMNGR_ID *)pid) = fd;

	return R_MM_OK;

exit:
	if (fd != -1) {
		if (close(fd))
			perror("MMI close");
	}
	return ret;
}

static int mm_free_co_in_user(MMNGR_ID id)
{
	int		ret;

	ret = ioctl(id, MM_IOC_FREE_CO, 0);
	if (ret) {
		perror("MMI FREE");
		ret = R_MM_FATAL;
		goto exit;
	}

	ret = close(id);
	if (ret) {
		perror("MMI close");
		ret = R_MM_FATAL;
		goto exit;
	}

	return R_MM_OK;
exit:
	return ret;
}

int mmngr_alloc_in_user(MMNGR_ID *pid, unsigned long size,
			unsigned long *pphy_addr,
			unsigned long *phard_addr,
			unsigned long *puser_virt_addr,
			unsigned long flag)
{
	int ret;

	if ((pid == NULL) || (pphy_addr == NULL)
	|| (phard_addr == NULL) || (puser_virt_addr == NULL) || (size == 0)) {
		ret = R_MM_PARE;
		goto exit;
	}

	if ((flag != MM_KERNELHEAP) && (flag != MM_CARVEOUT)
	&& (flag != MM_CARVEOUT_SSP) && (flag != MM_CARVEOUT_MV)) {
		ret = R_MM_PARE;
		goto exit;
	}

	if (flag == MM_KERNELHEAP) {
		ret = mm_alloc_kh_in_user(pid, size, pphy_addr, phard_addr,
					puser_virt_addr, flag);
		if (ret)
			goto exit;
	} else if (flag == MM_CARVEOUT) {
		ret = mm_alloc_co_in_user(pid, size, pphy_addr, phard_addr,
					puser_virt_addr, flag);
		if (ret)
			goto exit;
	} else if (flag == MM_CARVEOUT_SSP) {
		ret = mm_alloc_co_in_user(pid, size, pphy_addr, phard_addr,
					puser_virt_addr, flag);
		if (ret)
			goto exit;
	} else if (flag == MM_CARVEOUT_MV) {
		ret = mm_alloc_co_in_user(pid, size, pphy_addr, phard_addr,
					puser_virt_addr, flag);
		if (ret)
			goto exit;
	}

	return R_MM_OK;
exit:
	return ret;
}

int mmngr_free_in_user(MMNGR_ID id)
{
	int		ret;
	struct MM_PARAM	p;

	ret = ioctl(id, MM_IOC_GET, &p);
	if (ret) {
		if (errno == EBADF)
			ret = R_MM_SEQE;
		else
			ret = R_MM_FATAL;
		perror("MMI GET");
		goto exit;
	}

	if (p.flag == MM_KERNELHEAP) {
		ret = mm_free_kh_in_user(id);
		if (ret)
			goto exit;
	} else if (p.flag == MM_CARVEOUT) {
		ret = mm_free_co_in_user(id);
		if (ret)
			goto exit;
	} else if (p.flag == MM_CARVEOUT_SSP) {
		ret = mm_free_co_in_user(id);
		if (ret)
			goto exit;
	} else if (p.flag == MM_CARVEOUT_MV) {
		ret = mm_free_co_in_user(id);
		if (ret)
			goto exit;
	}  else {
		ret = R_MM_PARE;
		goto exit;
	}

	return R_MM_OK;
exit:
	return ret;
}

int mmngr_share_in_user(MMNGR_ID *pid,
			unsigned long size,
			unsigned long hard_addr,
			unsigned long *puser_virt_addr)
{
	int		ret;
	int		fd = -1;
	unsigned char	*mp;
	struct MM_PARAM	p;

	if ((pid == NULL) || (puser_virt_addr == NULL)) {
		ret = R_MM_PARE;
		goto exit;
	}

	if (size == 0) {
		ret = R_MM_PARE;
		goto exit;
	}

	fd = open(DEVFILE, O_RDWR);
	if (fd == -1) {
		perror("MMI share");
		ret = R_MM_FATAL;
		goto exit;
	}

	p.size = size;
	p.phy_addr = hard_addr;
	ret = ioctl(fd, MM_IOC_SHARE, &p);
	if (ret) {
		ret = R_MM_FATAL;
		goto exit;
	}

	mp = mmap((void *)0, size, PROT_READ | PROT_WRITE,
		MAP_SHARED, fd, 0);
	if (mp == MAP_FAILED) {
		ret = R_MM_FATAL;
		goto exit;
	}

	*puser_virt_addr = (unsigned long)mp;
	p.user_virt_addr = (unsigned long)mp;
	ret = ioctl(fd, MM_IOC_SET, &p);
	if (ret) {
		ret = R_MM_FATAL;
		goto exit;
	}

	*((MMNGR_ID *)pid) = fd;

	return R_MM_OK;
exit:
	if (fd != -1) {
		if (close(fd))
			perror("MMI close");
	}
	return ret;
}

int mmngr_release_in_user(MMNGR_ID id)
{
	int		ret;
	unsigned char	*mp;
	struct MM_PARAM	p;

	ret = ioctl(id, MM_IOC_GET, &p);
	if (ret) {
		if (errno == EBADF)
			ret = R_MM_SEQE;
		else
			ret = R_MM_FATAL;
		perror("MMI GET");
		goto exit;
	}

	mp = (unsigned char *)p.user_virt_addr;

	ret = munmap(mp, p.size);
	if (ret) {
		ret = R_MM_FATAL;
		goto exit;
	}

	ret = close(id);
	if (ret) {
		perror("MMI close");
		ret = R_MM_FATAL;
		goto exit;
	}

	return R_MM_OK;
exit:
	return ret;
}

