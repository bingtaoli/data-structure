摘自http://blog.csdn.net/v_july_v/article/details/6735293

感谢原作者！

## 概述

B树是一种较为复杂的数据结构。删除某个key的时候，涉及到合并merge；插入某个元素的时候，涉及到拆分split，作者在博客中做了详细的介绍。

## 使用

1、得到动态库

```shell
g++ -fPIC btree.cpp -shared -o libbtree.so
```

2、测试程序

```shell
g++ -L. -lbtree test.cpp -o main
```


