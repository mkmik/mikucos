#ifndef _NGUL_FSAPI_H_
#define _NGUL_FSAPI_H_

#include <file.h>

fs_error ngul_read_data(file_unitN_t file,u_int8_t *buff,u_int32_t start_B, u_int32_t len );
fs_error ngul_readb_cluster(cn_t lcn, u_int32_t addr_byte, u_int8_t *buff, u_int32_t len);

// API OF NGUL-FS
u_int32_t pizza_read(u_int16_t fd, u_int8_t *buff, u_int32_t len);
off_t pizza_lseek(int fildes,off_t offset,int whence);
/** read next dir entry from fd_dir, putting name in 'name' and its'
 * lcn in 'lcn'. It returns 0 if no more entries can be found */
int ngul_read_dir(u_int16_t fd_dir, char *name, u_int32_t *lcn);
fs_error ngul_read_dir_name(u_int16_t fd_dir ,u_int32_t num_file, char *name,u_int32_t *addr);


#endif    
