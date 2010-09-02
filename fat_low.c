#include <fat_low.h>
#include <fat_private.h>
#include <filesystem.h>

#include <stdio.h>

u_int16_t fat16_read_entry(physical_mounted_filesystem_t* this, off_t cluster) {
  int page = cluster / private(this)->sectors_per_fat;

  if(private(this)->current_page != page)
    fat_read_page(this, page);

  off_t offset = cluster % (private(this)->sectors_per_fat);

  u_int16_t * fat = (u_int16_t*)private(this)->page;
  return fat[offset];
}

void fat_read_page(struct physical_mounted_filesystem* this, int page) {
  filesystem_read_blocks(this, private(this)->page, private(this)->fat_start + page, 1);
  private(this)->current_page = page;
}
