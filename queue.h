#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <types.h>
#include <spin_lock.h>

#define CHAR_QUEUE_SIZE (1024)

struct char_queue {
  u_int8_t buffer[CHAR_QUEUE_SIZE];
  int first;
  int last;
  spin_lock_t lock;
};
typedef struct char_queue char_queue_t;

#define null_char_queue ((char_queue_t){{0,}, 0, 0, SPIN_LOCK_INITIALIZER})

extern void char_enqueue(char_queue_t* queue, char ch);
extern char char_dequeue(char_queue_t* queue);
extern int  char_queue_empty(char_queue_t*);
extern int char_queue_size(char_queue_t* queue);
extern int char_queue_free(char_queue_t* queue);

extern void char_enqueue_nolock(char_queue_t* queue, char ch);
extern char char_dequeue_nolock(char_queue_t* queue);
extern int  char_queue_empty_nolock(char_queue_t*);
extern int char_queue_size_nolock(char_queue_t* queue);
extern int char_queue_free_nolock(char_queue_t* queue);

#endif
