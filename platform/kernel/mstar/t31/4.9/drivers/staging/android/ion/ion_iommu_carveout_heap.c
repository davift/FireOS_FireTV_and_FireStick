/* SPDX-License-Identifier: GPL-2.0-only OR BSD-3-Clause */
/******************************************************************************
 *
 * This file is provided under a dual license.  When you use or
 * distribute this software, you may choose to be licensed under
 * version 2 of the GNU General Public License ("GPLv2 License")
 * or BSD License.
 *
 * GPLv2 License
 *
 * Copyright(C) 2019 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 *
 * BSD LICENSE
 *
 * Copyright(C) 2019 MediaTek Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/

#define pr_fmt(fmt) "Ion: " fmt

#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/genalloc.h>
#include <linux/mm.h>
#include <linux/scatterlist.h>
#include <linux/vmalloc.h>
#include <linux/dma-mapping.h>
#include <linux/version.h>

#include "ion_priv.h"
#include "ion.h"
#include "ion_of.h"
#include "mdrv_types.h"

#include "mdrv_system.h"

static struct ion_of_heap iommu_carveout_heaps[] = {
	PLATFORM_HEAP("iommu_ion,carveout", ION_HEAP_TYPE_IOMMU_CARVEOUT, ION_HEAP_TYPE_IOMMU_CARVEOUT,
		      "ion_iommu_carveout"),
	{}
};

static unsigned int orders_iommu_carveout[] = {8, 4, 2, 1, 0};
#define NUM_ORDER ARRAY_SIZE(orders_iommu_carveout)

static gfp_t high_order_gfp_flags = (GFP_HIGHUSER | __GFP_NOWARN |
				     __GFP_NORETRY) & ~__GFP_RECLAIM;
static gfp_t low_order_gfp_flags  = (GFP_HIGHUSER);

static ion_phys_addr_t iommu_carveout_base = 0;
static ion_phys_addr_t iommu_carveout_size = 0;
static struct mutex pool_lock;

struct ion_iommu_carveout_heap {
	struct ion_heap heap;
	struct gen_pool *pool;
	ion_phys_addr_t base;
	size_t size;
};

struct iommu_carveout_cache_node{
	struct list_head list;
	struct page *page;
	unsigned long len;
};

#define IOMMU_CARVEOUT_ALIGN (1<<20)

static ion_phys_addr_t ion_iommu_carveout_pool_alloc(struct ion_heap *heap,
					     unsigned long size,
					     unsigned long align)
{
	struct ion_iommu_carveout_heap *carveout_heap =
		container_of(heap, struct ion_iommu_carveout_heap, heap);
	unsigned long offset = 0;

	mutex_lock(&pool_lock);
	offset = gen_pool_alloc(carveout_heap->pool, size);
	mutex_unlock(&pool_lock);
	if (!offset)
		return -1;

	return offset;
}

static void ion_iommu_carveout_pool_free(struct gen_pool *pool,
	unsigned long addr, size_t size)
{
	mutex_lock(&pool_lock);
	gen_pool_free(pool, addr, size);
	mutex_unlock(&pool_lock);
}
static inline int is_carveout_range(struct ion_iommu_carveout_heap *heap, struct page *page)
{
	unsigned long ba = (unsigned long)page_to_phys(page);
	return (ba >= heap->base && ba < (heap->base + heap->size));
}


static int ion_iommu_carveout_heap_allocate(struct ion_heap *heap,
				      struct ion_buffer *buffer,
				      unsigned long len, unsigned long align,
				      unsigned long flags)
{
	struct list_head head;
	int  size = 0, index = 0, ret = 0;
	unsigned int order;
	struct sg_table *table = NULL;
	ion_phys_addr_t paddr;
	struct scatterlist *sg = NULL;
	struct page *page = NULL;
	int i;
	struct iommu_carveout_cache_node *node,*tmp;
	struct ion_iommu_carveout_heap *carveout_heap =
		container_of(heap, struct ion_iommu_carveout_heap, heap);

	INIT_LIST_HEAD(&head);
	while(len)
	{
		if(len >= IOMMU_CARVEOUT_ALIGN)
			size = IOMMU_CARVEOUT_ALIGN;
		else
			break;

		paddr = ion_iommu_carveout_pool_alloc(heap, size, align);
		if (paddr == -1) {
			break;
		}
		if(paddr & ((1 << PAGE_SHIFT) - 1)){
			pr_err("%s::%s::%d, paddr not 4KB align!\n", __FILE__, __FUNCTION__, __LINE__);
			break;
		}
		struct iommu_carveout_cache_node *nnode;
		nnode = kmalloc(sizeof(struct iommu_carveout_cache_node), GFP_KERNEL);
		if(!nnode){
			pr_err("%s::%s::%d::malloc iommu carveout cache node faill\n", __FILE__, __FUNCTION__, __LINE__);
			goto ERROR;
		}
		nnode->len = size;
		nnode->page = phys_to_page(paddr);
		INIT_LIST_HEAD(&nnode->list);
		list_add_tail(&nnode->list, &head);
		len -=size;
		index++;
	}

	while(len){
		for(i = 0; i < NUM_ORDER; i++){
			if (len < (PAGE_SIZE<<orders_iommu_carveout[i])){
				continue;
			}
			order = orders_iommu_carveout[i];
			if(order > 0)
				page = alloc_pages(high_order_gfp_flags, order);
			else
				page = alloc_pages(low_order_gfp_flags, order);

			if(!page){
				continue;
			}
			break;
		}

		if(!page){
			pr_err("%s::%s::%d, alloc_pages fail!\n", __FILE__, __FUNCTION__, __LINE__);
			goto ERROR;
		}
		if(is_carveout_range(carveout_heap, page)){
			pr_err(":%s::%d, alloc_pages overlay!BA =0x%lx ,size=0x%x\n", __FUNCTION__, __LINE__,(unsigned long)page_to_phys(page),PAGE_SIZE<<order);
			goto ERROR;
		}
		struct iommu_carveout_cache_node *nnode;
		nnode = kmalloc(sizeof(struct iommu_carveout_cache_node), GFP_KERNEL);
		if(!nnode){
			pr_err("%s::%s::%d::malloc iommu carveout cache node faill\n", __FILE__, __FUNCTION__, __LINE__);
			goto ERROR;
		}
		nnode->len = (PAGE_SIZE<<order);
		nnode->page = page;
		INIT_LIST_HEAD(&nnode->list);
		list_add_tail(&nnode->list, &head);
		len -= nnode->len;
		index++;
	}

	table = kmalloc(sizeof(struct sg_table), GFP_KERNEL);
	if (!table)
		goto ERROR;

	if (sg_alloc_table(table, index, GFP_KERNEL))
		goto FREE;

	sg = table->sgl;
	list_for_each_entry_safe(node, tmp, &head, list){
		sg_set_page(sg, node->page, node->len, 0);
		sg = sg_next(sg);
		list_del(&node->list);
		kfree(node);
	}

	buffer->sg_table = table;
	return 0;

FREE:
	kfree(table);
ERROR:
	if(index){
		list_for_each_entry_safe(node, tmp, &head, list){
			if(is_carveout_range(carveout_heap, node->page))
				ion_iommu_carveout_pool_free(carveout_heap->pool, page_to_phys(node->page), node->len);
			else
				__free_pages(node->page, get_order(node->len));
			list_del(&node->list);
			kfree(node);
		}
	}
	return -ENOMEM;
}

static void ion_iommu_carveout_heap_free(struct ion_buffer *buffer)
{
	struct ion_heap *heap = buffer->heap;
	struct sg_table *table = buffer->sg_table;
	struct page *page;
	struct ion_iommu_carveout_heap *carveout_heap =
		container_of(heap, struct ion_iommu_carveout_heap, heap);
	struct scatterlist *sg;
	int i;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
	if (ion_buffer_cached(buffer))
		dma_sync_sg_for_device(NULL, table->sgl, table->nents,
					   DMA_BIDIRECTIONAL);

#endif

	for_each_sg(table->sgl, sg, table->nents, i){
		page = sg_page(sg);
		if(is_carveout_range(carveout_heap, page))
			ion_iommu_carveout_pool_free(carveout_heap->pool, page_to_phys(page), sg->length);
		else{
			__free_pages(page, get_order(sg->length));
		}
	}

	sg_free_table(table);
	kfree(table);
}

static struct ion_heap_ops iommu_carveout_heap_ops = {
	.allocate = ion_iommu_carveout_heap_allocate,
	.free = ion_iommu_carveout_heap_free,
	.map_user = ion_heap_map_user,
	.map_kernel = ion_heap_map_kernel,
	.unmap_kernel = ion_heap_unmap_kernel,
};

struct ion_heap *ion_iommu_carveout_heap_create(struct ion_platform_heap *heap_data)
{
	struct ion_iommu_carveout_heap *carveout_heap;
	int ret;

	struct page *page = NULL;
	size_t size;

	if(!iommu_carveout_base || !iommu_carveout_size){
		pr_err("IOMMU Reserved memory: invalid \n");
		return ERR_PTR(-ENOMEM);
	}

	if(!pfn_valid(PFN_DOWN(heap_data->base))){
		pr_err("\033[31mFunction = %s, Line = %d, pfn invalid\033[m\n", __PRETTY_FUNCTION__, __LINE__);
		return ERR_PTR(-ENOMEM);
	}
	page = phys_to_page(heap_data->base);
	size = heap_data->size;

	ret = ion_heap_pages_zero(page, size, pgprot_writecombine(PAGE_KERNEL));
	if (ret)
		return ERR_PTR(ret);

	carveout_heap = kzalloc(sizeof(*carveout_heap), GFP_KERNEL);
	if (!carveout_heap)
		return ERR_PTR(-ENOMEM);

	carveout_heap->pool = gen_pool_create(PAGE_SHIFT, -1);
	if (!carveout_heap->pool) {
		kfree(carveout_heap);
		return ERR_PTR(-ENOMEM);
	}

	carveout_heap->base = heap_data->base;
	carveout_heap->size = heap_data->size;
	ret = gen_pool_add(carveout_heap->pool, carveout_heap->base, heap_data->size,-1);
	if(ret != 0){
		pr_err("\033[31mFunction = %s, Line = %d,ret=x%x \033[m\n", __PRETTY_FUNCTION__, __LINE__,ret);
		kfree(carveout_heap->pool);
		kfree(carveout_heap);
		return ERR_PTR(-ENOMEM);
	}
	carveout_heap->heap.ops = &iommu_carveout_heap_ops;
	carveout_heap->heap.type = ION_HEAP_TYPE_IOMMU_CARVEOUT;

	return &carveout_heap->heap;
}

void ion_iommu_carveout_heap_destroy(struct ion_heap *heap)
{
	struct ion_iommu_carveout_heap *carveout_heap =
	     container_of(heap, struct  ion_iommu_carveout_heap, heap);

	gen_pool_destroy(carveout_heap->pool);
	kfree(carveout_heap);
	carveout_heap = NULL;
}

#ifdef CONFIG_OF_RESERVED_MEM
#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/of_reserved_mem.h>

static int rmem_carveout_device_init(struct reserved_mem *rmem, struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct ion_platform_heap *heap = pdev->dev.platform_data;

	heap->base = rmem->base;
	heap->base = rmem->size;
	pr_debug("%s: heap %s base %pa size %pa dev %p\n", __func__,
		 heap->name, &rmem->base, &rmem->size, dev);
	return 0;
}

static void rmem_carveout_device_release(struct reserved_mem *rmem,
				    struct device *dev)
{
	return;
}

static const struct reserved_mem_ops rmem_ops = {
	.device_init	= rmem_carveout_device_init,
	.device_release	= rmem_carveout_device_release,
};

static int __init rmem_carveout_setup(struct reserved_mem *rmem)
{
	pr_info("ion_iommu Reserved memory: at %pa, size %ld MiB\n",
		&rmem->base, (unsigned long)rmem->size / SZ_1M);

	rmem->ops = &rmem_ops;
	iommu_carveout_base = rmem->base;
	iommu_carveout_size = rmem->size;
	return 0;
}

RESERVEDMEM_OF_DECLARE(ion_iommu, "iommu_ion,carveout", rmem_carveout_setup);
#endif


int iommu_carveout_ion_init(struct ion_platform_heap *heap)
{
	if(!heap)
		return -1;

	if(!iommu_carveout_base || !iommu_carveout_size){
		pr_err("IOMMU Reserved memory: invalid \n");
		return -1;
	}
	mutex_init(&pool_lock);
	heap->id = iommu_carveout_heaps[0].heap_id;
	heap->type = iommu_carveout_heaps[0].type;
	heap->name = iommu_carveout_heaps[0].name;
	heap->align = iommu_carveout_heaps[0].align;
	heap->base = iommu_carveout_base;
	heap->size = iommu_carveout_size;
	heap->priv = NULL;

	return 0;
}
