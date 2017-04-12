SkipList

关于skiplist的原理一篇博客已经写得很好了，[http://www.spongeliu.com/63.html](http://www.spongeliu.com/63.html)

下面的介绍都是从这篇博客中摘录的。

### 查找

```c
p=top
While(1){
    while (p->next->key < x ) p=p->next;
    If (p->down == NULL ) return p->next
    p=p->down ;
}
```

### 插入

假设要插入一个元素“119”，我们设定需要插入该元素的层数为“k”（即我们需要在所有的[1,k]范围内的层里都插入元素。k的值我们会在下文中叙述）。

```c
int insert(val x){
    int i;
    int j = n; //n是当前表所拥有的level数
    cell *p[k]; //指针数组，用来保存每一层要插入元素的前驱
    cell *p1;
    p1 = top->next;
    while(p1){
        while(p1->next->val < x) p1=p1->next;
        if(j <= k){
            p[j-1] = p1; //保存每一层的指针
            p1 = p1->down; //指向下一层
            j--;
        }
    }
    //下面的代码是将x插入到各层
    for (i = 0; i<k; i++){
        if(p[i]==NULL){ //k>n的情况，需要创建一个层
            //创建层的第一个元素，并将top指向它
            cell *elementhead = (cell *) malloc(sizeof(cell));
            element->val = -1;
            element->down = top;
            top = elementhead; 
            //创建最后一个元素
            cell *elementtail = (cell *) malloc(sizeof(cell));
            elementtail->val = 1;
            elementtail->next = elementtail->down = NULL;
            //在该层中创建并插入x
            cell *element = (cell *) malloc(sizeof(cell));
            element->val = x;
            elementhead->next = element;
            element->next = elementtail;
            element->down = p[i-1]->next;
        }
        //正常插入一个元素
        cell *element = (cell *) malloc(sizeof(cell));
        element->val = x;
        element->next = p[i]->next;
        element->down = (i=0?NULL:(p[i-1]->next));
        p[i]->next = element;
    }
    return 0;
}
```

最后，让我们来阐述哪些元素应该在上一层保留，以及插入操作时确定插入元素的层数k。 哪些元素应该在高层保留，是随机决定的。具体算法如下：
我们假定一个函数rand()，随机返回1或者0
假设元素i最多在第k层保留
k的值由程式“ while(rand()) k++;”来决定

### Redis的skiplist

Redis的实现其实不是特别通用化，没有方便的存储API，比如insert某个key和value，它的rank更加有作用。
所以**不建议把Redis的SkipList作为学习的对象**。


### 本目录的代码

本目录的代码摘自`yodb`的实现，感谢作者`kedebug`的工作。由于需要yodb的skiplist运行起来依赖于yodb项目的其他东西，比如arena，mutex等，所以本目录还摘下了很多其他的文件。

### 运行

```sh
g++ test_skiplist.cpp arena.cc thread.cc -lpthread
```

上面会得到a.out，运行就OK了，这段测试程序还计算了内存使用，整个逻辑不复杂，适合学习。