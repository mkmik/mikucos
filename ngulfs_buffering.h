#ifndef _NGUL_FS_BUFFERING_H
#define _NGUL_FS_BUFFERING_H

#include <types.h>
#include <ngulfs.h>
#include <ngulfs_low_api.h>
#include <ide.h>

#define MAX_ASSOCIATIVE_BUFF  5

typedef struct cluster_associative_buff {
  cn_t lcn[MAX_ASSOCIATIVE_BUFF];
  cluster_buffer_t  cls[MAX_ASSOCIATIVE_BUFF];
  int touched[MAX_ASSOCIATIVE_BUFF];

  u_int8_t idx;
} cluster_associative_buff;

cluster_associative_buff tabula;

fs_error buffera(cn_t lcn, u_int8_t *ptr);
u_int32_t verify_lcn_buffed(cn_t lcn);

#endif
