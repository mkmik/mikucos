#ifndef _AVL_H_
#define _AVL_H_

/* This is an implementation of AVL trees.
 * implementation based on http://www.purists.org/georg/avltree 
 * Data genericity modeled after linux linked list implementation (list.h)
 *
 * usage:
 * if you want to put a xyz structure in a AVL tree 
 * declare a member avl_node_t inside the struct (like list_head_t)
 * then (possibly in the header file next to the struct decl)
 * use the macro "avl_make_compare(type, member, key)"
 * where "type" is the type of the struct, "member" is
 * the name of the avl_node_t member, and "key" is the
 * name of a numeric member wich will be the ordering key.
 *  You can provide custom ordering functions. Please see
 * read and understand this header file for details.
 */

typedef struct avl_node avl_node_t;
typedef enum AVLRES avl_res_t;
enum AVLRES {
  AVL_ERROR = 0,
  AVL_OK,
  AVL_BALANCE,
};


struct avl_node {
  avl_node_t *left;
  avl_node_t *right;
  int skew;
};

#define null_avl_node ((avl_node_t){0,0,0})

typedef int (*avl_compare_t)(avl_node_t* a, avl_node_t* b);

#define avl_add(node, root, type) _avl_add(node, root, type ## _avlcompare)
avl_res_t _avl_add(avl_node_t* node, avl_node_t** root, 
		  avl_compare_t compare);
#define avl_entry(ptr, type, member) \
        ((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))
#define avl_find(key, root, type, member) \
({ \
  avl_node_t *node = root; \
  type* entry; \
  int match; \
  while(node) { \
    entry = avl_entry(node, type, member); \
    match = type ## _avlmatch(key, entry); \
    if(match == 0) \
      break; \
    if(match < 0) \
      node = node->left; \
    if(match > 0) \
      node = node->right; \
  } \
  entry = node ? avl_entry(node, type, member) : 0; \
}) 

#define avl_make_compare(type, member, key) \
static inline int type ## _avlcompare(avl_node_t *_a, avl_node_t *_b) {   \
  type *a = avl_entry(_a, type, member); \
  type *b = avl_entry(_b, type, member); \
  return a->key < b->key ? -1 : (a->key == b->key ? 0 : 1); \
}
#define avl_make_match(type, key) \
static inline int type ## _avlmatch(typeof(((type*)0)->key) k, type *b) {   \
  return k < b->key ? -1 : (k == b->key ? 0 : 1); \
}

#define avl_make_string_compare(type, member, key) \
static inline int type ## _avlcompare(avl_node_t *_a, avl_node_t *_b) {   \
  type *a = avl_entry(_a, type, member); \
  type *b = avl_entry(_b, type, member); \
  return strcmp(a->key, b->key); \
}
#define avl_make_string_match(type, key) \
static inline int type ## _avlmatch(typeof(((type*)0)->key) k, type *b) {   \
  return strcmp(k, b->key); \
}

#endif
