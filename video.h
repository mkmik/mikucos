#ifndef __VIDEO_H__
#define __VIDEO_H__

#include <types.h>

void video_init();
void video_drawAA(char *s, int row, int col, size_t len);
void video_draw_char(char c, int row, int col);
void video_set_cursor_pos(int row, int col);
void video_set_screen_size(int r, int c);

void setAttr(char attr);
char getAttr();

/** scrolls region defined by col, row and w, h by 'lines' lines.
 * if lines < 0 direction is up **/
void video_scroll(int row, int col, int w, int h, int lines);

int video_get_rows();
int video_get_columns();
void video_cls();

#endif
