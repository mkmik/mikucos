#ifndef _NGUL_FS_LOW_API_H
#define _NGUL_FS_LOW_API_H

#include <types.h>
#include <ngulfs.h>
#include <ide.h>

typedef u_int8_t* cluster_t;
typedef u_int8_t cluster_buffer_t[4096];

#include <ngulfs_buffering.h>

u_int8_t mount(void);
u_int8_t read_cluster(u_int32_t lcn, cluster_t buff);
u_int8_t write_cluster(u_int32_t lcn, cluster_t buff);

cn_t read_file_record_fast (cn_t lcn,file_record *fd );
cn_t read_file_record (cn_t lcn,cn_t vcn, file_record *fd );
cn_t write_file_record (cn_t lcn, cn_t vcn, file_record *fd );
cn_t vcn_to_lcn(u_int32_t f_idx,u_int32_t vcn);
cn_t get_lcn(extent *list,u_int32_t len,cn_t vcn);

#endif
