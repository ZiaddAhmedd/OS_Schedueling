#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#define MEM_SIZE 1024
#define EMPTY 0
#define HAS_PROCESS 1
#define HAS_CHILD 2
#define ROOT 3

struct MemNode
{
  int size;
  int pid;
  int state;
  int mem_start_position;
  struct MemNode *left;
  struct MemNode *right;
  struct MemNode *parent;
};

struct MemNode *create_node(int size, struct MemNode *parent)
{
  struct MemNode *node = (struct MemNode *)malloc(sizeof(struct MemNode));
  node->size = size;
  node->pid = -1;
  node->left = NULL;
  node->right = NULL;
  node->parent = parent;
  return node;
}

struct memTree
{
  /* data */
  struct MemNode *root;
};

struct memTree *create_memTree()
{
  struct memTree *tree = (struct memTree *)malloc(sizeof(struct memTree));
  tree->root = create_node(MEM_SIZE, NULL);
  tree->root->state = ROOT;
  tree->root->mem_start_position = 0;
  return tree;
}
// 65 - 128
struct MemNode *find_size(struct MemNode *root, int size)
{
  if (root == NULL || root->size < size)
    return NULL;

  struct MemNode *left = find_size(root->left, size);
  struct MemNode *right = find_size(root->right, size);
  // get values
  int left_int = 20000, right_int = 20000;
  if (left != NULL && left->size >= size && left->state == EMPTY)
  {
    left_int = left->size;
  }
  if (right != NULL && right->size >= size && right->state == EMPTY)
  {
    right_int = right->size;
  }

  // get minimum of left and right and root->size
  if (left_int <= right_int && left_int < root->size)
    return left;
  else if (right_int < left_int && right_int < root->size)
    return right;
  else
    return root;
}

struct MemNode *get_process_node(struct MemNode *root, int pid)
{
  if (root == NULL)
    return NULL;
  if (root->pid == pid)
    return root;

  struct MemNode *found_node = get_process_node(root->left, pid);
  if (found_node != NULL)
    return found_node;

  found_node = get_process_node(root->right, pid);
  if (found_node != NULL)
    return found_node;

  return NULL;
}

struct MemNode *find_closest_size(struct memTree *tree, int size)
{
  // get log2 of size
  float log2_size = log2(size);
  int wanted_size = pow(2, ceil(log2_size));

  // call find_closest_node
  struct MemNode *found_node = find_size(tree->root, wanted_size);

  while (found_node->size != wanted_size)
  {

    struct MemNode *left_node = create_node(found_node->size / 2, found_node);
    struct MemNode *right_node = create_node(found_node->size / 2, found_node);

    left_node->mem_start_position = found_node->mem_start_position;
    right_node->mem_start_position = found_node->mem_start_position + found_node->size / 2;

    left_node->state = EMPTY;
    right_node->state = EMPTY;
    found_node->left = left_node;
    found_node->right = right_node;
    found_node->state = HAS_CHILD;

    found_node = find_size(tree->root, wanted_size);
    // print_tree(tree->root);
    printf("found_node : found : %d - wanted : %d - state : %d\n", found_node->size, wanted_size, found_node->state);
  }

  return found_node;
}

int allocateProcess(struct memTree *tree, int process_size, int pid)
{
  // find closest node
  struct MemNode *found_node = find_closest_size(tree, process_size);
  // printf("find_closest_size : %d \n" , found_node->size );
  // if found node is null
  if (found_node == NULL)
    return -1;

  // if node is found
  found_node->state = HAS_PROCESS;
  found_node->pid = pid;

  return found_node->mem_start_position;
}

void delete_children(struct MemNode *current_deleted_node)
{
  free(current_deleted_node->left);
  current_deleted_node->left = NULL;
  free(current_deleted_node->right);
  current_deleted_node->right = NULL;
  current_deleted_node->state = EMPTY;
}

void recombine_memory(struct MemNode *parent)
{
  struct MemNode *current_deleted_node = parent;

  while (current_deleted_node)
  {
    if (current_deleted_node->left->state == EMPTY &&
        current_deleted_node->right->state == EMPTY)
    {
      delete_children(current_deleted_node);
    }

    if (current_deleted_node->state != ROOT)
      current_deleted_node = current_deleted_node->parent;
  }
}

int deallocateProcess(struct memTree *tree, int pid)
{
  // find process node
  struct MemNode *found_node = get_process_node(tree->root, pid);

  // if found node is null
  if (found_node == NULL)
    return -1;

  printf("deallocate: %d\n", pid);

  found_node->state = EMPTY;
  found_node->pid = -1;

  struct MemNode *parent = found_node->parent;

  if (parent->left == found_node)
  {
    if (parent->right->state == EMPTY || parent->right->state == ROOT)
    {
      recombine_memory(parent);
    }
  }
  else if (parent->right == found_node)
  {
    if (parent->left->state == EMPTY || parent->right->state == ROOT)
    {
      recombine_memory(parent);
    }
  }
  return 1;
}

void print_tree(struct MemNode *root)
{
  if (root == NULL)
    return;

  print_tree(root->left);
  printf("%d %d %d - %d\n", root->size, root->pid, root->mem_start_position, root->mem_start_position + root->size);

  print_tree(root->right);
}