#ifndef _INVOKE_H_
#define _INVOKE_H_

#include <debug.h>

typedef void* (*function_t)();
typedef int invoke_offset_t;
#define invoke(type, obj, name) ((typeof(((type ##_ops_t*)0)->name)) type ## _t_invoke_lookup(obj,  \
  (int)&((type ##_ops_t*)0)->name))

#define invoked_method_t(type, name) typeof(((type ##_ops_t*)0)->name)

static inline void* invoke_panic() {
  panicf("invoked undefined method");
}

#endif
