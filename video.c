#include <video.h>
#include <libc.h>
#include <io.h>

#define PORT 0x3d4
#define framebuffer ((char*)0xB8000)
#define COLUMNS 80
#define ROWS 25

static int columns = COLUMNS, rows = ROWS;
static int attr = 0x07;

void video_init() {
}

void video_set_cursor_pos(int row, int col) {
  int off = col + row * columns;
  int port = 0x3d4;

  outb_p(0xe, port + 0);
  outb_p(off >> 8, port + 1);
  outb_p(0xf, port + 0);
  outb_p(off & 0xFF, port + 1);
}

void video_set_screen_size(int r, int c) {
  rows = r;
  columns = c;
}

void video_drawAA(char *s, int row, int col, size_t len) {
  char *p=s;
    
  while(len--) {
    if(*p == '\n') {
      row++;
      col=0;
    } else 
      if(col < columns && row < rows) {
//	if(*p != ' ') 
	  video_draw_char(*p, row, col++);
//	else col++;
      }
    p++;
  }
}

void video_draw_char(char c, int row, int col) {
  *(framebuffer + (col + row * columns) * 2) = (char)(c & 0xFF);
  *(framebuffer + (col + row * columns) * 2 + 1) = attr;   
}

void setAttr(char a) {
  attr = a;
}

char getAttr() {
  return attr;
}

void video_cls() {
  int i;
  for(i=0; i<rows*columns * 2 ; i++) 
    framebuffer[i] = 0;
}

int video_get_rows() {
  return rows;
}

int video_get_columns() {
  return columns;
}

static u_int16_t video_get_complete_char(int row, int col) {
  return *(framebuffer + (col + row * columns) * 2) |
    *(framebuffer + (col + row * columns) * 2 + 1) << 8;
}

static void video_set_complete_char(u_int16_t c, int row, int col) {
  *(framebuffer + (col + row * columns) * 2) = (char)(c & 0xFF);
  *(framebuffer + (col + row * columns) * 2 + 1) = (char)(c >> 8);
}

void video_scroll(int row, int col, int w, int h, int lines) {
  int i,j;
  u_int16_t ch;

  if(lines < 0) {
    row -= lines;
    
    for(i=row; i<(row+h); i++)
      for(j=col; j<(col+w); j++) {
	ch = video_get_complete_char(i, j);
	video_set_complete_char(ch, i+lines, j);
      }
    for(i=row+h+lines; i<(row+h); i++)
      for(j=col; j<(col+w); j++) {
	video_set_complete_char(0, i, j);
      }
    
  } else {
    panicf("not implemented yet\n");
  }
}
