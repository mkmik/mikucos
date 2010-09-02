#include <avl.h>

enum AVLSKEW {
  AVL_NONE,
  AVL_LEFT,
  AVL_RIGHT
};

void avl_rotateRight(avl_node_t **root);
void avl_rotateLeft(avl_node_t **root);
avl_res_t avl_grownLeft(avl_node_t **n);
avl_res_t avl_grownRight(avl_node_t **n);


avl_res_t _avl_add(avl_node_t* node, avl_node_t** root, 
		  avl_compare_t compare) {
  if(!*root) {
    *root = node;
    return AVL_BALANCE;
  }

  if(compare(node, *root) < 0) {
    if(_avl_add(node, &(*root)->left, compare) == AVL_BALANCE)
      return avl_grownLeft(root);
  } else if(compare(node, *root) > 0) {
    if(_avl_add(node, &(*root)->right, compare) == AVL_BALANCE)
      return avl_grownRight(root);
  } else {
    //    printf("already exists\n");
  }
  return AVL_OK;
}

void avl_rotateRight(avl_node_t **root) {
  avl_node_t *tmp = *root;

  *root = (*root)->left;
  tmp->left = (*root)->right;
  (*root)->right = tmp;
}

void avl_rotateLeft(avl_node_t **root) {
  avl_node_t *tmp = *root;

  *root = (*root)->right;
  tmp->right = (*root)->left;
  (*root)->left = tmp;
}

avl_res_t avl_grownLeft(avl_node_t **n) {
  switch ((*n)->skew) {
  case AVL_LEFT:
    if ((*n)->left->skew == AVL_LEFT) {
      (*n)->skew = (*n)->left->skew = AVL_NONE;
      avl_rotateRight(n);
    } else {
      switch ((*n)->left->right->skew) {
      case AVL_LEFT:
	(*n)->skew = AVL_RIGHT;
	(*n)->left->skew = AVL_NONE;
	break;
	
      case AVL_RIGHT:
	(*n)->skew = AVL_NONE;
	(*n)->left->skew = AVL_LEFT;
	break;
	
      default:
	(*n)->skew = AVL_NONE;
	(*n)->left->skew = AVL_NONE;
      }
      (*n)->left->right->skew = AVL_NONE;
      avl_rotateLeft(& (*n)->left);
      avl_rotateRight(n);
    }
    return AVL_OK;
    
  case AVL_RIGHT:
    (*n)->skew = AVL_NONE;
    return AVL_OK;
    
  default:
    (*n)->skew = AVL_LEFT;
    return AVL_BALANCE;
  }
}

avl_res_t avl_grownRight(avl_node_t **n) {
  switch ((*n)->skew) {
  case AVL_LEFT:
    (*n)->skew = AVL_NONE;
    return AVL_OK;

  case AVL_RIGHT:
    if ((*n)->right->skew == AVL_RIGHT) {
      (*n)->skew = (*n)->right->skew = AVL_NONE;
      avl_rotateLeft(n);
    } else {
      switch ((*n)->right->left->skew) {
      case AVL_RIGHT:
	(*n)->skew = AVL_LEFT;
	(*n)->right->skew = AVL_NONE;
	break;

      case AVL_LEFT:
	(*n)->skew = AVL_NONE;
	(*n)->right->skew = AVL_RIGHT;
	break;

      default:
	(*n)->skew = AVL_NONE;
	(*n)->right->skew = AVL_NONE;
      }
      (*n)->right->left->skew = AVL_NONE;
      avl_rotateRight(& (*n)->right);
      avl_rotateLeft(n);
    }
    return AVL_OK;

  default:
    (*n)->skew = AVL_RIGHT;
    return AVL_BALANCE;
  }
}
