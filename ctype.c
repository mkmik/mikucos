#include <ctype.h>

int isalnum(int c) {
  return isalpha(c) || isdigit(c);
}
int isalpha(int c) {
  return islower(c) || isupper(c);
}
int isascii(int c);
int isblank(int c);
int iscntrl(int c);
int isdigit(int c) {
  return c >= '0' && c <= '9';
}
int isgraph(int c);
int islower(int c) {
  return c >= 'a' && c <= 'z';
}
int isprint(int c) {
  return isalnum(c) || ispunct(c);
}
int ispunct(int c) {
  const char *punct = "!@#$%^&*()_+{}[];':\",./<>?`|";
  while(*punct)
    if(c == *punct++)
      return 1;
  return 0;
}
int isspace(int c) {
  return c==' ' || c=='\n' || c=='\t';
}
int isupper(int c) {
  return c >= 'A' && c <= 'Z';
}
int isxdigit(int c);
