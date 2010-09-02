#ifndef _NGUL_FS_H_
#define _NGUL_FS_H_

#include<types.h>

#define MAX_EXT_NUM  679 // max number fragmentation need to complete 4096 byte cluster 
#define FR_TYPE_FILE      (1<<0)
#define FR_TYPE_DIRECTORY (1<<1)
#define FR_TYPE_MFT       (1<<2)
#define FR_TYPE_ROOT_DIR  (1<<3)
#define FR_TYPE_FREE_FILE    (1<<4)
#define FR_TYPE_OTHER     (1<<5)
#define MAX_FILE_NAME     256
// these are lcn values
#define BOOT_SECTOR_LCN      0
#define MFT_LCN        1  
// this is vcn value of mft in mft 
#define MFT_VCN            0
#define ROOT_VCN           3
#define FREE_FILE_VCN      5
#define CLUST_LEN  4096

//----API define
#define SEEK_SET     0
#define SEEK_CUR     1
#define SEEK_END     2


//----end API define


typedef u_int32_t cn_t;   //cluster number type
typedef cn_t file_unitN_t; //file cluster number type

typedef struct extent {
  cn_t lcn;
  u_int16_t len;
} extent; //extent is a beautiful ngul idea :) ngul to m$ and vms  


typedef struct file_record {
  u_int8_t type;
  u_int16_t date;
  u_int16_t uid;
  u_int16_t gid;
  u_int16_t perm;
  u_int32_t len; // extent list len or file len ? porcatroia che cazzo di nome!
  cn_t my;
  u_int8_t  unused;
  cn_t next_fr;
  extent ext_ls[MAX_EXT_NUM];
} file_record ;
//this struct starts an high layer

typedef struct ngulfs_file {
  u_int32_t  cursor;  
  cn_t file_id;
  u_int32_t len;
  u_int8_t type;
} file;

typedef u_int8_t fs_error;
// all this is a brute idea of ngulfs 
// but just now it works, it will be better :)

extern file  *file_desc[];

#endif
