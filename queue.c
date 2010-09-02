#include <queue.h>
#include <debug.h>
#include <libc.h>

void char_enqueue(char_queue_t* queue, char ch) {
  spin_lock(&queue->lock);
  char_enqueue_nolock(queue, ch);
  spin_unlock(&queue->lock);
}

void char_enqueue_nolock(char_queue_t* queue, char ch) {
  int newlast;

  newlast = (queue->last + 1) % sizeof(queue->buffer);
  if(newlast == queue->first) {
    xprintf("queue %p OVERFLOW\n", queue);
    panicf("queue overflow\n");
  }
  
  queue->buffer[queue->last] = ch;
  queue->last = newlast;
}

char char_dequeue(char_queue_t* queue) {
  spin_lock(&queue->lock);
  char ch = char_dequeue_nolock(queue);
  spin_unlock(&queue->lock);
  return ch;
}

char char_dequeue_nolock(char_queue_t* queue) {
  int newfirst;
  char ch;
  if(char_queue_empty_nolock(queue))
    return 0;
  
  newfirst = (queue->first + 1) % sizeof(queue->buffer);
  ch = queue->buffer[queue->first];
  queue->first = newfirst;
  return ch;
}

int char_queue_empty(char_queue_t* queue) {
  spin_lock(&queue->lock);
  int res = char_queue_empty_nolock(queue);
  spin_unlock(&queue->lock);
  return res;
}

int char_queue_empty_nolock(char_queue_t* queue) {
  return queue->first == queue->last;
}

int char_queue_size(char_queue_t* queue) {
  spin_lock(&queue->lock);
  int res = char_queue_size_nolock(queue);
  spin_unlock(&queue->lock);
  return res;
}

int char_queue_size_nolock(char_queue_t* queue) {
  int s = queue->last - queue->first;
  if(s<0)
    s += sizeof(queue->buffer);
  return s;
}

int char_queue_free(char_queue_t* queue) {
  spin_lock(&queue->lock);
  int res = char_queue_free_nolock(queue);
  spin_unlock(&queue->lock);
  return res;
}
int char_queue_free_nolock(char_queue_t* queue) {
  return sizeof(queue->buffer) - char_queue_size_nolock(queue);
}

