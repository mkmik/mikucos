#include <ngulfs_allocation.h>


cn_t ngul_find_next_free_region(extent *ptr, int len){

  u_int32_t i = MAX_FREE_REGION;
  u_int32_t found = 0;
  cn_t first_free_block = 0;

  while(i-- && found < len) {
    found = ptr[i].len;
    first_free_block = ptr[i].lcn;
  }
  return(first_free_block);
}


/*cn_t ngul_expand_file(cn_t file_id,extent *new_blocks){
  file_record file;
  u_int32_t i,j,aux;
  read_cluster(vcn_to_lcn(MFT_VCN,file_id),&file);
  j=i=aux=0;
  aux=vcn_to_lcn(file_id,file.len)-1;
   while(file.ext_ls[j] != aux && j < MAX_EXT_NUM 
    while(new_blocks[i].len != 0) {
            file.ext
      } 

}*/
