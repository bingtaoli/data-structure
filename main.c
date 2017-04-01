#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <btree.h>

int main(){
	btree = (btree_node_t *)(void *)malloc(sizeof(btree_node_t))
	int result = btree_creat(&btree, 3)
	if (0 != result){
		printf("btree init fail\n");
	}
	int i = 0;
	for (; i < 10; i++){
		result = btree_insert(btree, i)
		if (0 != result){
			printf("btree insert %i fail\n", i);
		}
	}
	// free this tree
	
	return 0;
}