#include <ngulfs_buffering.h>
#include <libc.h>

u_int32_t verify_lcn_buffed(cn_t lcn){
  int i;
  for(i=0; i<MAX_ASSOCIATIVE_BUFF; i++) {
    if(tabula.lcn[i] == lcn) {
      tabula.touched[i]++;
      //      printf("tabula %d touched %d times\n", i, tabula.touched[i]++);
      return i;
    }
  }
  return 7;
}

fs_error buffera(cn_t lcn, u_int8_t *ptr){
#if ALEATORY_CACHE == 1
  tabula.lcn[tabula.idx] = lcn;
  memcpy(tabula.cls[tabula.idx], ptr, sizeof(*tabula.cls));
  if(tabula.idx == (MAX_ASSOCIATIVE_BUFF-1)))
    tabula.idx = 0;
  else
    tabula.idx++;
#else
  int min = 0;
  int i;
  for(i=0; i<MAX_ASSOCIATIVE_BUFF; i++) {
    if(tabula.touched[i] < tabula.touched[min])
      min = i;
  }
// printf("replaced ngulfs cache entry %d touched %d times\n", min, tabula.touched[min]);
  tabula.lcn[min] = lcn;
  tabula.touched[min] = 1;
  memcpy(tabula.cls[min], ptr, sizeof(*tabula.cls));
  
#endif  
  return 0;
}
  
