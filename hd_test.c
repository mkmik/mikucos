#include <uart.h>
#include <ide.h>
#include <ngulfs.h>
#include <ngulfs_low_api.h>
#include <io.h>
#include <libc.h>
#include <hal.h>
#include <ngulfs_api.h> 

#include <malloc.h>
#include <process.h>
#include <timer.h>
#include <ngulfs_format.h>
//static int use_lba = 1;

void hd_test_init(){
  
}

file_record mfted;
char file_buff[4096];
char file_buff_dummy[50];

void hd_test_run(){
  hal_enable_interrupts();
  cn_t  lcn;
  int i;
  extent regionf;  
  xprintf("Formating... the disk ....\n");
  simple_format();
  xprintf("DISK FORMATED\n");
  //print_status();
  read_cluster(MFT_LCN, (cluster_t)&mfted);
  
  //xprintf("ext_lcn=:%d\n",mfted.ext_ls[0].lcn);
  //xprintf("ext_len=:%d\n",mfted.ext_ls[0].len);
  xprintf("READ MFT TYPE IS:%d\n",mfted.type);
  xprintf("VCN  1 of mft point to ->lcn:%d\n",vcn_to_lcn(MFT_VCN,1));
  xprintf("LCN of MFT is %d\n",vcn_to_lcn(MFT_VCN,0));
  
  
  file_desc[1]->file_id=ROOT_VCN;
  file_desc[1]->len=5000;
  file_desc[1]->cursor=0;
  lcn=0;
  xprintf("ls\n"); 
    for (i=0;i<3;i++){
      file_desc[1]->cursor=0;
      ngul_read_dir_name(1,i,file_buff_dummy,&lcn);
      xprintf("%s  %d\n", file_buff_dummy, lcn);
    }
    read_cluster(vcn_to_lcn(FREE_FILE_VCN,0),(cluster_t)file_buff);
  
    regionf=*(extent*)file_buff;
    xprintf("free space is:%d KB\n",(regionf.len*4096)/1024);
    xprintf("First extent free is :%d \n",regionf.lcn);
}



 

// old func
/*  for (i=290;i<=300;i++){
    char buf2[20];
    itoa(buf2, 10, i);
    sector_BUFF[21]=' ';
    strcpy(sector_BUFF+21, buf2);
    write_sector(sector_BUFF,0,i,1);
    xprintf("%d,",i);
     delay(100); 
    //xprintf(";");
  }
  xprintf("Try to read 10 sectors\n");
  i=9;
  delay(200);
  for (i=290;i<=310;i++){
    read_sector(sector_BUFF,0,i,1);
    xprintf("STRINGA:%s\n",sector_BUFF);
    delay(200);
  }
  print_status();
  i=3;
  while(i--)print_status();*/

