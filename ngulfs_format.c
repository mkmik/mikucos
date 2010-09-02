#include <ngulfs_format.h>
#include <ngulfs.h>
file_record mft = {
  .type=FR_TYPE_MFT,
  .date=0,
  .uid=0,
  .gid=0,
  .perm=0,
  .len=10,
  .ext_ls[0].lcn=MFT_LCN,
  .ext_ls[0].len=10,
}

,free_f ={
  .type=FR_TYPE_FREE_FILE,
  .date=0,
  .uid=0,
  .gid=0,
  .perm=0,
  .len=2,
  .ext_ls[0].lcn=12,
  .ext_ls[0].len=2,
}
,root_dir={
  .type=FR_TYPE_FREE_FILE,
  .date=0,
  .uid=0,
  .gid=0,
  .perm=0,
  .len=1,
  .ext_ls[0].lcn=20,
  .ext_ls[0].len=1,
    
};

u_int8_t root_dir_buff[CLUST_LEN]={ROOT_VCN,0,0,0,'.',' ',' ',' ',0,1,0,0,0,'$','M','F','T',0,0,0,0,0,'$','M','B','R',0};

extent free_region[MAX_FREE_REGION]={{0,},};



fs_error simple_format(void){
  
  free_region[0].lcn = 21;
  free_region[0].len = 9979;
  write_cluster(MFT_LCN, (cluster_t)&mft);
  mount();
  write_cluster(vcn_to_lcn(MFT_VCN, 5), (cluster_t)&free_f);
  write_cluster(vcn_to_lcn(MFT_VCN, 3), (cluster_t)&root_dir);
  write_cluster(vcn_to_lcn(ROOT_VCN,0),(cluster_t)&root_dir_buff);
  write_cluster(vcn_to_lcn(FREE_FILE_VCN,0),(cluster_t)&free_region);
}

