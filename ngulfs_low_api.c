#include <ngulfs_low_api.h>
#include <ngulfs.h>
#include <libc.h>
#include <timer.h> // delay

#define N_SECTOR  1  // hardcoded  bad 
#define CLUSTER_DIM  8
#define DRIVE  0   // to implement

file_record cached_mft;
file_record frbuff;

//this func caches the mft file_record
u_int8_t mount(void) {
  read_cluster(MFT_LCN, (cluster_t)&cached_mft);
  return 0;
}

// this func  reads a cluster of 4096 bytes 8 sector of 512 bytes 
u_int8_t read_cluster(u_int32_t lcn, cluster_t buff) {
  
  u_int32_t sector_num = lcn*CLUSTER_DIM;
  u_int8_t i;
  u_int32_t ver;
  ver=verify_lcn_buffed(lcn);
  if(ver <= 4) {
    memcpy(buff, tabula.cls[ver], 4096);
  } else {
    
    for(i=0; i<CLUSTER_DIM; i++) {
      // printf("reading sector\n");
      read_sector(buff+(i*512), DRIVE, sector_num+i, N_SECTOR);// read  cluster of 4096 bytes
    }
    buffera(lcn, buff);
  }
  return 0;
} 

// this func writes a cluster of 4096 bytes 8 sector of 512 bytes
u_int8_t write_cluster(u_int32_t lcn, cluster_t buff) {

  u_int32_t sector_num = lcn*CLUSTER_DIM;
  u_int8_t i;
  for(i=0; i<CLUSTER_DIM; i++) {
    //printf("writing sector %d\n", i);
    write_sector(buff+(i*512), DRIVE, sector_num+i, N_SECTOR);// read  cluster of 4096 bytes
    delay(10);
  }
  return 0;
}

// this func  converts virtual_cn to logical_cn
cn_t vcn_to_lcn(u_int32_t f_idx, u_int32_t vcn) {
  cn_t lcn;

  lcn = get_lcn(cached_mft.ext_ls, cached_mft.len, f_idx);
  read_file_record_fast(lcn, &frbuff);  //hardcoded bad need change
  return get_lcn(frbuff.ext_ls, frbuff.len, vcn);

}


// this func gets lcn from an extent list 
u_int32_t get_lcn(extent *list, u_int32_t len, cn_t vcn) {
  cn_t current_vcn = 0;
  int i;
  for(i=0; i<len; i++) {
    current_vcn += list[i].len;
    if(current_vcn > vcn)
      return list[i].lcn + vcn - (current_vcn - list[i].len);
  }
   
  printf("ERROR, no such lcn for vcn %d\n", vcn);
  return -1;
}
   
u_int32_t get_ext_lcn(extent *pointer){
  return(pointer->lcn);
}
u_int16_t get_ext_len(extent *pointer){
  return(pointer->len);
}

cn_t read_file_record_fast (cn_t lcn, file_record *fd){
  
  read_cluster(lcn, (u_int8_t*)fd);
  return lcn;
}
cn_t read_file_record (cn_t lcn,cn_t vcn, file_record *fd ){
 cn_t lcn_writen;
 
 if(lcn>MFT_VCN){
   lcn_writen=vcn_to_lcn(lcn, vcn);
   read_cluster(lcn_writen, (u_int8_t*)fd);
   return lcn_writen;
 }

 lcn_writen=vcn_to_lcn(MFT_VCN,vcn);
 read_cluster(lcn_writen, (u_int8_t*)fd);
 return lcn_writen ;
}


cn_t write_file_record (cn_t lcn, cn_t vcn, file_record* fd ){
  cn_t lcn_written;
  if(lcn>MFT_VCN){
    lcn_written = vcn_to_lcn(lcn,vcn);
    write_cluster(lcn_written, (u_int8_t*)fd);
    return lcn_written;
  }
  lcn_written = vcn_to_lcn(MFT_VCN, vcn);
  write_cluster(lcn_written, (u_int8_t*)fd);
  return lcn_written;

}
