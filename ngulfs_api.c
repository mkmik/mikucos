#include<ngulfs.h>
#include<ngulfs_api.h>
#include<uart.h>
#include<libc.h>
#include<ngulfs_low_api.h>
#include<ngulfs_error.h>
#include<hal.h>
static u_int8_t data_buff[4096];
file  *file_desc[256];
//this func read  "n" bytes from a cluster so u have to pay atention in len and addr_byte however this is a kernel layer
// so if there will be an error we have a kernel panic  

u_int8_t ngul_readb_cluster(cn_t lcn,u_int32_t addr_byte,u_int8_t *buff,u_int32_t len){
  fs_error err;
  
  if((err=read_cluster(lcn,data_buff)))
    panic(err); 

  //printf("addr=%d  len=%d\n",addr_byte,len);
  
  if(addr_byte+len>=CLUST_LEN)
    panic(ER_READ_DT_CLST);
  memcpy(buff, data_buff+addr_byte, len);
 
}
  


//this func reads from file, "len" Bytes from "start_B" byte 

fs_error ngul_read_data(file_unitN_t file, u_int8_t *buff, u_int32_t start_B, u_int32_t len){
 
  if(len==0)                                 // oviamente se len e' zero non faccio un cazzo 
    return 0;
                                             // dichiaro le mie variabili
  file_unitN_t idx_start;                      
  u_int16_t  ncluster, nbytes, start_by, next_len=len;
  u_int16_t count = 0, delta = 0;
  u_int16_t control = 0;
 
  idx_start = start_B/CLUST_LEN;             // posiziono idx_start  al cluster di partenza  
  start_by  = start_B%CLUST_LEN;             // posiziono start_by  al byte di partenza all'interno del cluster
  ncluster  = len/CLUST_LEN;                 // numero di cluster da leggere
  nbytes    = len%CLUST_LEN;                 // numero di byte da leggere
  control   = start_B+len;
  //printf("read_data: prima dell'if di allineamento SB=%d\n",start_by );   // degug
  
  if(start_by != 0) {
                      // nel caso il file abbia una "testa non allineata la si legge fino al cluster sucessivo 
                      // se cluster non ve ne sono leggo solo i byte necessari
    if(ncluster==0 && idx_start==control/CLUST_LEN) {
      ngul_readb_cluster(vcn_to_lcn(file,idx_start),start_by,buff,len);
      return 0;
    }
    
    delta = CLUST_LEN - start_by -1;

    //printf("delta=%d\nstart_by=%d\n",delta,start_by);
    
    ngul_readb_cluster(vcn_to_lcn(file,idx_start),start_by,buff,delta);
    next_len-=delta;
    ncluster = next_len/CLUST_LEN;
    nbytes = next_len%CLUST_LEN;
    idx_start++;
    start_by = 0;
    //printf("read_data: nell'IF di allign prima della lettura dell offset\n");
    if (!ngul_read_data(idx_start,buff+delta,start_by,next_len))
      return 0;
    else  
      panic(1);
  }
  
                         // leggo i cluster necessari e la coda di byte
  
  if(ncluster>0){
    //printf("read_data: prima del ciclo\n");
    while(count<ncluster) {
      //printf("read_data: ciclo lettura cluster %d\n",count);
      read_cluster(vcn_to_lcn(file,count+idx_start),buff+(count*CLUST_LEN));
      count++;
    }
    //printf("read_data: dopo il ciclo\n");
    if(nbytes>0){
      //printf("read_data: prima della lettura coda\n");
      ngul_readb_cluster(vcn_to_lcn(file,idx_start+count),0,buff+(count*CLUST_LEN),nbytes);
      return 0;
    }
  } else {
    ngul_readb_cluster(vcn_to_lcn(file,idx_start),0,buff,nbytes);
    return 0;
  }
  return 1;
}


//---------directory enumerator rivisitata --------------
int ngul_read_dir(u_int16_t fd_dir, char *name,u_int32_t *lcn) {
  if(!pizza_read(fd_dir, (u_int8_t*)lcn, sizeof(*lcn)))
    return 0;
  do { 
    pizza_read(fd_dir, name, 1);
  } while(*name++);
  return 1;
}

//---------directory scassa  cazz --------------
// this func read the name of a "num_dir_file" in the directory pointed from dir_file
// lo so questa funzione fara' schifo il caching lo implemento poi ...
fs_error  ngul_read_dir_name(u_int16_t fd_dir, u_int32_t num_file, char *name,u_int32_t *addr){
  
  int j=0;
  u_int8_t control='a',helper[4];
  // leggo j file fino al voluto 
  while(j<num_file){
    //leggo i primi 4 byte che sono il vcn del file 
    pizza_read(fd_dir,helper,4);     
    //inizializzo control se no e' un casino 
    control=1;

    while(control!=0){
      pizza_read(fd_dir,&control,1);
    } 
    j++;
  }
  // se il file j e' quello ricercato lo leggo e metto
  // il nome in name e l'addr in addr :) 
  if(j == num_file){
    pizza_read(fd_dir,(u_int8_t*)addr,4);
    control=1;
    while(control!=0){
      // printf("cur:%d\n",file_desc[1]->cursor); //Debug
      pizza_read(fd_dir,&control,1);
      
      *name=control;
      //printf("control:%d\n",*name); //Debug
      name++;
    }
    //  printf("num_file:%d j:%d fd_dir:%d addr:%d",num_file,j,fd_dir,*addr);
    return 0;
  }
  return 1;
}
      

			


/*-------------------API NGUL_FS------------------------*/

u_int32_t pizza_read(u_int16_t fd, u_int8_t *buff, u_int32_t len){
  file_unitN_t file;
  u_int32_t cur=0,len_file=0;
  int err;
  
  file = file_desc[fd]->file_id;
  cur  = file_desc[fd]->cursor;
  len_file = file_desc[fd]->len;
  if(len>=len_file - cur)
    len=len_file - cur;
  
  if((err=ngul_read_data(file,buff,cur,len))!=0 )
    panic(err);
  else {
    file_desc[fd]->cursor=cur+len;
    return len;
  }
}

off_t pizza_lseek(int fildes,off_t offset,int whence){
  int len;
  len = file_desc[fildes]->len;
 
  switch(whence){
  
  case SEEK_SET:
    file_desc[fildes]->cursor=offset;
    return offset;
  case SEEK_CUR:
    file_desc[fildes]->cursor+=offset;
    return file_desc[fildes]->cursor;
  case SEEK_END:
    file_desc[fildes]->cursor=len+offset;
    return file_desc[fildes]->cursor;

  }
  return 1;
}




