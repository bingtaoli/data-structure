#include <stdio.h>
#include <string.h>
#include <errno.h> 
#include "btree.h"

/**
* 摘自博客：http://blog.csdn.net/qifengzou/article/details/21079325
* with too much appreciation
* steal from: 作    者: # Qifeng.zou # 2014.03.12 # 
*/

/****************************************************************************** 
 **函数名称: btree_creat 
 **功    能: 创建B树 
 **输入参数:  
 **     _btree: B树 
 **     m: 阶 - 取值范围m>=3 
 **输出参数: NONE 
 **返    回: 0:成功 -1:失败 
 **实现描述:  
 **注意事项:  
 **     注意：参数max的值不能小于2. 
 **作    者: # Qifeng.zou # 2014.03.12 # 
 ******************************************************************************/  
int btree_creat(btree_t **_btree, int m)  
{  
    btree_t *btree = NULL;  
  
    if(m < 3) {  
        fprintf(stderr, "[%s][%d] Parameter 'max' must geater than 2.\n", __FILE__, __LINE__);  
        return -1;  
    }  
  
    btree = (btree_t *)calloc(1, sizeof(btree_t));  
    if(NULL == btree) {  
        fprintf(stderr, "[%s][%d] errmsg:[%d] %s!\n", __FILE__, __LINE__, errno, strerror(errno));  
        return -1;  
    }  
  
    btree->max= m - 1;  
    btree->min = m/2;  
    if(0 != m%2) {  
        btree->min++;  
    }  
    btree->min--;  
    btree->sidx = m / 2;  
    btree->root = NULL; /* 空树 */  
  
    *_btree = btree;  
    return 0;  
}  

/****************************************************************************** 
 **函数名称: btree_insert 
 **功    能: 插入关键字（对外接口） 
 **输入参数:  
 **     btree: B树 
 **     key: 被插入的关键字 
 **输出参数: NONE 
 **返    回: 0:成功 -1:失败 
 **实现描述:  
 **注意事项:  
 **作    者: # Qifeng.zou # 2014.03.12 # 
 ******************************************************************************/  
int btree_insert(btree_t *btree, int key)  
{  
    int idx = 0;  
    btree_node_t *node = btree->root;  
  
    /* 1. 构建第一个结点 */  
    if(NULL == node) {  
        node = btree_creat_node(btree);  
        if(NULL == node) {  
            fprintf(stderr, "[%s][%d] Create node failed!\n", __FILE__, __LINE__);  
            return -1;  
        }  
  
        node->num = 1;   
        node->key[0] = key;  
        node->parent = NULL;  
  
        btree->root = node;  
        return 0;  
    }  
  
    /* 2. 查找插入位置：在此当然也可以采用二分查找算法，有兴趣的可以自己去优化 */  
    while(NULL != node) {  
        for(idx=0; idx<node->num; idx++) {  
            if(key == node->key[idx]) {  
                fprintf(stderr, "[%s][%d] The node is exist!\n", __FILE__, __LINE__);  
                return 0;  
            }  
            else if(key < node->key[idx]) {  
                break;  
            }  
        }  
  
        if(NULL != node->child[idx]) {  
            node = node->child[idx];  
        }  
        else {  
            break;  
        }  
    }  
  
    /* 3. 执行插入操作 */  
    return _btree_insert(btree, node, key, idx);  
}  

/****************************************************************************** 
 **函数名称: _btree_insert 
 **功    能: 插入关键字到指定结点 
 **输入参数:  
 **     btree: B树 
 **     node: 指定结点 
 **     key: 被插入的关键字 
 **     idx: 指定位置 
 **输出参数: NONE 
 **返    回: 0:成功 -1:失败 
 **实现描述:  
 **注意事项:  
 **作    者: # Qifeng.zou # 2014.03.12 # 
 ******************************************************************************/  
static int _btree_insert(btree_t *btree, btree_node_t *node, int key, int idx)  
{  
    int i = 0;  
  
    /* 1. 移动关键字:首先在最底层的某个非终端结点上插入一个关键字,因此该结点无孩子结点，故不涉及孩子指针的移动操作 */  
    for(i=node->num; i>idx; i--) {  
        node->key[i] = node->key[i-1];  
    }  
  
    node->key[idx] = key; /* 插入 */  
    node->num++;  
  
    /* 2. 分裂处理 */  
    if(node->num > btree->max) {  
        return btree_split(btree, node);  
    }  
  
    return 0;  
}  

/****************************************************************************** 
 **函数名称: btree_split 
 **功    能: 结点分裂处理 
 **输入参数:  
 **     btree: B树 
 **     node: 需要被分裂处理的结点 
 **输出参数: NONE 
 **返    回: 0:成功 -1:失败 
 **实现描述:  
 **注意事项:  
 **作    者: # Qifeng.zou # 2014.03.12 # 
 ******************************************************************************/  
static int btree_split(btree_t *btree, btree_node_t *node)  
{  
    int idx = 0, total = 0, sidx = btree->sidx;  
    btree_node_t *parent = NULL, *node2 = NULL;   
  
  
    while(node->num > btree->max) {  
        /* Split node */   
        total = node->num;  
  
        node2 = btree_creat_node(btree);  
        if(NULL == node2) {         
            fprintf(stderr, "[%s][%d] Create node failed!\n", __FILE__, __LINE__);  
            return -1;  
        }  
  
        /* Copy data */   
        // void *memcpy(void*dest, const void *src, size_t n);
        // 1、key数组分裂
        memcpy(node2->key, node->key + sidx + 1, (total-sidx-1) * sizeof(int));  
        // 2、子节点指针数组分裂
        memcpy(node2->child, node->child + sidx+1, (total-sidx) * sizeof(btree_node_t *));  
  
        node2->num = (total - sidx - 1);  
        node2->parent  = node->parent;  
  
        node->num = sidx;   
        /* Insert into parent */  
        parent  = node->parent;  
        if(NULL == parent)  {     
        	// (bt) 如果没有root节点，生成root节点   
            /* Split root node */   
            parent = btree_creat_node(btree);  
            if(NULL == parent) {         
                fprintf(stderr, "[%s][%d] Create root failed!", __FILE__, __LINE__);  
                return -1;  
            }         
  
            btree->root = parent;   
            parent->child[0] = node;   
            node->parent = parent;   
            node2->parent = parent;   
  
            parent->key[0] = node->key[sidx];  
            parent->child[1] = node2;  
            parent->num++;  
        }         
        else {         
            /* Insert into parent node */   
            for(idx=parent->num; idx>0; idx--) {         
                if(node->key[sidx] < parent->key[idx-1]) {         
                    parent->key[idx] = parent->key[idx-1];  
                    parent->child[idx+1] = parent->child[idx];  
                    continue;  
                }  
                break;  
            }         
  
            parent->key[idx] = node->key[sidx];  
            parent->child[idx+1] = node2;  
            node2->parent = parent;   
            parent->num++;  
        }         
  
        memset(node->key+sidx, 0, (total - sidx) * sizeof(int));  
        memset(node->child+sidx+1, 0, (total - sidx) * sizeof(btree_node_t *));  
  
        /* Change node2's child->parent */  
        for(idx=0; idx<=node2->num; idx++) {  
            if(NULL != node2->child[idx]) {         
                node2->child[idx]->parent = node2;  
            }         
        }         
        node = parent;   
    }  
  
    return 0;  
}  

/****************************************************************************** 
 **函数名称: btree_creat_node 
 **功    能: 新建结点 
 **输入参数:  
 **     btree: B树 
 **输出参数: NONE 
 **返    回: 节点地址 
 **实现描述:  
 **注意事项:  
 **作    者: # Qifeng.zou # 2014.03.12 # 
 ******************************************************************************/  
static btree_node_t *btree_creat_node(btree_t *btree)  
{  
    btree_node_t *node = NULL;  
  
  
    node = (btree_node_t *)calloc(1, sizeof(btree_node_t));  
    if(NULL == node) {  
        fprintf(stderr, "[%s][%d] errmsg:[%d] %s\n", __FILE__, __LINE__, errno, strerror(errno));  
        return NULL;  
    }  
  
    node->num = 0;  
  
    /* More than (max) is for move */  
    node->key = (int *)calloc(btree->max+1, sizeof(int));  
    if(NULL == node->key) {  
        free(node), node=NULL;  
        fprintf(stderr, "[%s][%d] errmsg:[%d] %s\n", __FILE__, __LINE__, errno, strerror(errno));  
        return NULL;  
    }  
  
    /* More than (max+1) is for move */  
    node->child = (btree_node_t **)calloc(btree->max+2, sizeof(btree_node_t *));  
    if(NULL == node->child) {  
        free(node->key);  
        free(node), node=NULL;  
        fprintf(stderr, "[%s][%d] errmsg:[%d] %s\n", __FILE__, __LINE__, errno, strerror(errno));  
        return NULL;  
    }  
  
    return node;  
}

// TODO destroy function
