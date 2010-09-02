#ifndef _NGUL_ALLOCATION_H_
#define _NGUL_ALLOCATION_H_

#include <ngulfs.h>
#include <types.h>
#include <avl.h>
#include <ngulfs_format.h>

extent *ngul_free_tabula_blocks;

cn_t ngul_find_next_free_region(extent *ptr, int len);
cn_t ngul_expand_file(cn_t file_id,extent *new_blocks);
cn_t ngul_creat_file(void);



#endif
