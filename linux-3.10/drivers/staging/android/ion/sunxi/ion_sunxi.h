/*
 * drivers/staging/android/ion/sunxi/ion_sunxi.h
 *
 * Copyright(c) 2013-2015 Allwinnertech Co., Ltd.
 *      http://www.allwinnertech.com
 *
 * Author: liugang <liugang@allwinnertech.com>
 *
 * sunxi ion header file
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __ION_SUNXI_H
#define __ION_SUNXI_H

/**
 * ion_client_create() -  allocate a client and returns it
 * @name:		used for debugging
 */
struct ion_client *sunxi_ion_client_create(const char *name);


/**
 * sunxi_buf_alloc - alloc phys contigous memory in SUNXI platform.
 * @size: size in bytes to allocate.
 * @paddr: store the start phys address allocated.
 *
 * return the start virtual address, or 0 if failed.
 */
void *sunxi_buf_alloc(unsigned int size, unsigned int *paddr);
/**
 * sunxi_buf_free - free buffer allocated by sunxi_buf_alloc.
 * @vaddr: the kernel virt addr of the area.
 * @paddr: the start phys addr of the area.
 * @size: size in bytes of the area.
 */
void sunxi_buf_free(void *vaddr, unsigned int paddr, unsigned int size);
/**
 * sunxi_alloc_phys - alloc phys contigous memory in SUNXI platform.
 * @size: size in bytes to allocate.
 *
 * return the start phys addr, or 0 if failed.
 */
u32 sunxi_alloc_phys(size_t size);
/**
 * sunxi_free_phys - free phys contigous memory allocted by sunxi_alloc_phys.
 * @paddr: the start phys addr of the area.
 * @size: size in bytes of the area.
 */
void sunxi_free_phys(u32 paddr, size_t size);
/**
 * sunxi_map_kernel - map phys contigous memory to kernel virtual space.
 * @paddr: the start phys addr of the area.
 * @size: size in bytes of the area.
 *
 * return the start virt addr which is in vmalloc space, or NULL if failed.
 */
void *sunxi_map_kernel(unsigned int paddr, unsigned int size);
/**
 * sunxi_unmap_kernel - unmap phys contigous memory from kernel space.
 * @vaddr: the kernel virt addr of the area.
 * @paddr: the start phys addr of the area.
 * @size: size in bytes of the area.
 */
void sunxi_unmap_kernel(void *vaddr, unsigned int paddr, unsigned int size);


#endif
