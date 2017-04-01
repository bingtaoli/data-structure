#include <stdio.h>
#include <stdlib.h>

/* B树结点结构 */  
typedef struct _btree_node_t  
{  
    int num;                        /* 关键字个数 */  
    int *key;                       /* 关键字：所占空间为(max+1) - 多出来的1个空间用于交换空间使用 */  
    struct _btree_node_t **child;   /* 子结点：所占空间为（max+2）- 多出来的1个空间用于交换空间使用 */  
    struct _btree_node_t *parent;   /* 父结点 */  
}btree_node_t;  

/* B树结构 */  
typedef struct  
{  
    int max;                        /* 单个结点最大关键字个数 - 阶m=max+1 */  
    int min;                        /* 单个结点最小关键字个数 */  
    int sidx;                       /* 分裂索引 = (max+1)/2 */  
    btree_node_t *root;             /* B树根结点地址 */  
}btree_t;  

int btree_creat(btree_t **_btree, int m);
int btree_insert(btree_t *btree, int key);
static int btree_split(btree_t *btree, btree_node_t *node);
static int _btree_insert(btree_t *btree, btree_node_t *node, int key, int idx);
static btree_node_t *btree_creat_node(btree_t *btree);