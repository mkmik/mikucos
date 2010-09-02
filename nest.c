#include <stdio.h>
#include <stdlib.h>
#include "list.h"

typedef void (*func_t)();

typedef struct {
  list_head_t link;
  func_t handler;
} handler_t;

list_head_t handlers = LIST_HEAD_INIT(handlers);

void addExceptionHandler(func_t h) {
  handler_t *hs = malloc(sizeof(handler_t));
  hs->handler = h;
  list_add(&hs->link, &handlers);
};

void raiseException() {
  list_head_t *it;
  list_for_each(it, &handlers) {
    list_entry(it, handler_t, link)->handler();
  }
}

#define TRY { __label__ exception_label; \
 void exhandler() { goto exception_label; } \
 addExceptionHandler(exhandler); 

#define CATCH  goto finally_label; \
exception_label: \
if(!list_empty(&handlers))  \
  list_del(handlers.next); 

#define ENDTRY finally_label: ; }

void func2() {
  printf("running func2\n");
  raiseException();
  printf("finishing func2, should never display this\n");
}

void func() {
  printf("running func\n");

  TRY {
    func2();
  } CATCH {
    printf("got exception in func, raising again\n");
    raiseException();
  } ENDTRY;
  
  printf("finishing func\n");
}

int main() {

  TRY {
    func();
  } CATCH {
    printf("got exception\n");
  } ENDTRY;

  return 0;
}
