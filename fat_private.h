#ifndef _FAT_PRIVATE_H_
#define _FAT_PRIVATE_H_

#include <types.h>
#include <block.h>

struct fat_private {
  // readed from boot block
  int sectors_per_fat;
  int number_hidden_sectors;
  int reserved_sector;
  int number_of_fats;
  int root_dir_entries;

  // computed at mount 
  off_t fat_start;
  off_t rootdir_start;
  size_t rootdir_sectors;

  // caching

  /** block buffer for a portion of fat 
   * which is currently used */
  block_space_t page;
  /** -1 if page is invalid otherwise
   * the contains the index of the fat 'page'.
   *  A fat can contain 'sector_per_fat' blocks or 'pages'
   * the one currentyl cached in 'page' is pointed 
   * by this variable. */
  int current_page;

  block_space_t dir_page;
  /** this variable helps avoiding to rescan the whole directory
   * searching for item 'x' because of vfat slots.
   * this var saves the starting position of the page saved
   * in dir_page
   * */
  int dir_page_starting_position;
  /** together with the previous, it is used
   * to see if 'dir_page' is valid for a given directory
   * and position */
  int dir_page_file_id;
};

typedef struct fat_private fat_private_t; 

#define private(this) ((fat_private_t*)((this)->private))

#endif
