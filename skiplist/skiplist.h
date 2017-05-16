/**
 *  Steal from yodb: https://github.com/kedebug/yodb/blob/master/tree/skiplist.h
 *  author is kedebug, thanks
 */
#ifndef _YODB_SKIPLIST_H_
#define _YODB_SKIPLIST_H_

#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include <vector>
#include <boost/noncopyable.hpp>

#include "arena.h"

namespace yodb {

class Arena;

template<class Key, class Comparator>
class SkipList : boost::noncopyable {
private:
    struct Node;
public:
    explicit SkipList(Comparator cmp);
    
    void insert(const Key& key);
    bool contains(const Key& key) const;
    void erase(const Key& key);
    void resize(size_t size);
    void clear();

    size_t count() const { return count_; }
    size_t memory_usage() const { return arena_.usage(); }

    class Iterator {
    public:
        explicit Iterator(const SkipList* list);

        bool valid() const;
        const Key& key() const;
        void next();
        void prev();

        void seek(const Key& target);
        void seek_to_first();
        void seek_to_middle();
        void seek_to_last();

    private:
        const SkipList* list_;
        Node* node_;
    };

private:
    enum { kMaxHeight = 17 };
    
    Arena arena_;
    Node* head_;
    size_t max_height_;
    size_t count_;
    Comparator compare_;

    int seed_;

    int random_height();
    bool equal(const Key& a, const Key& b) const;

    Node* new_node(const Key& key, size_t height);
    Node* find_greater_or_equal(const Key& key, Node** prev) const;
    Node* find_less_than(const Key& key) const;
};

/**
 * (bt)定义node类型，struct一般表示数据类型
 */
template<class Key, class Comparator>
struct SkipList<Key, Comparator>::Node {
    explicit Node(const Key& k) : key(k) { } 

    Key key;

    Node* next(size_t n) { return next_[n]; }
    void set_next(size_t n, Node* node) { next_[n] = node; }
    void set_key(const Key& k) { key = k; }

private:
    /**
     * 每一个node的next是一个数组，分别是该node在每一层的下一个节点，因为每一层都有某个node的指针。
     * 比如next_[1]就是该node在第一层的下一个node; next_[2]就是该node在第二层的下一个node
     */
    Node* next_[1];
};

template<class Key, class Comparator>
typename SkipList<Key, Comparator>::Node* 
SkipList<Key, Comparator>::new_node(const Key& key, size_t height)
{
    size_t size = sizeof(Node) + sizeof(Node*) * (height - 1);
    char* alloc_ptr = arena_.alloc_aligned(size);
    // (bt) 强转?
    return new (alloc_ptr) Node(key);
}

template<class Key, class Comparator>
inline SkipList<Key, Comparator>::Iterator::Iterator(const SkipList* list)
{
    list_ = list;
    node_ = NULL;
}

template<class Key, class Comparator>
inline bool SkipList<Key, Comparator>::Iterator::valid() const 
{
    return node_ != NULL;
}

template<class Key, class Comparator>
inline const Key& SkipList<Key, Comparator>::Iterator::key() const
{
    assert(valid());
    return node_->key;
}

template<class Key, class Comparator>
inline void SkipList<Key, Comparator>::Iterator::next()
{
    assert(valid());
    node_ = node_->next(0);
}

template<class Key, class Comparator>
inline void SkipList<Key, Comparator>::Iterator::prev()
{
    assert(valid());

    node_ = list_->find_less_than(node_->key);
    if (node_ == list_->head_)
        node_ = NULL;
}

template<class Key, class Comparator>
inline void SkipList<Key, Comparator>::Iterator::seek(const Key& target)
{
    node_ = list_->find_greater_or_equal(target, NULL);
    if (node_ == list_->head_)
        node_ = NULL;
}

// (bt) 定位到头节点最低高度的 node 即可
template<class Key, class Comparator>
inline void SkipList<Key, Comparator>::Iterator::seek_to_first()
{
    node_ = list_->head_->next(0);
}

template<class Key, class Comparator>
inline void SkipList<Key, Comparator>::Iterator::seek_to_middle()
{
    int middle = list_->count_ / 2; 

    seek_to_first();
    
    for (int i = 0; i < middle; i++)
        node_ = node_->next(0);
}

// (bt) 从头节点的最高开始，依次前进，知道达到链表尾。
template<class Key, class Comparator>
inline void SkipList<Key, Comparator>::Iterator::seek_to_last()
{
    Node* curr = list_->head_;
    size_t level = list_->max_height_ - 1;

    while (true) {
        Node* next = curr->next(level);

        if (next == NULL) {
            if (level == 0)
                break;
            else 
                level--;
        } else {
            curr = next;
        }
    }

    node_ = curr;
    if (node_ == list_->head_)
        node_ = NULL; 
}

template<class Key, class Comparator>
bool SkipList<Key, Comparator>::equal(const Key& a, const Key& b) const
{
    return compare_(a, b) == 0;
}

template<class Key, class Comparator>
int SkipList<Key, Comparator>::random_height()
{
    static const size_t kBranching = 4;
    int height = 1;

    while (height < kMaxHeight && (rand() % kBranching) == 0)
        height++;

    return height;
}

/**
 * prev: 保留每一层最后一个比key小的数的index
 */
template<class Key, class Comparator>
typename SkipList<Key, Comparator>::Node* 
SkipList<Key, Comparator>::find_greater_or_equal(const Key& key, Node** prev) const
{
    Node* curr = head_;
    size_t level = max_height_ - 1;

    while (true) {
        Node* next = curr->next(level);

        if (next != NULL && compare_(next->key, key) < 0) {
            curr = next;
        } else {
            // (bt) next为NULL了，表示没有找到，到下一层去，保留上一层的位置到prev数组中
            if (prev != NULL)
                prev[level] = curr;

            if (level == 0)
                return next;
            else
                level--;
        }
    } 
}

template<class Key, class Comparator>
typename SkipList<Key, Comparator>::Node* 
SkipList<Key, Comparator>::find_less_than(const Key& key) const
{
    Node* curr = head_;
    size_t level = max_height_ - 1;
    // (bt) 从最高层往下查找，找到就返回；没找到返回NULL
    while (true) {
        Node* next = curr->next(level);

        if (next == NULL || compare_(next->key, key) >= 0) {
            if (level == 0)
                return curr;
            else
                level--;
        } else {
            curr = next;
        }
    }
}

/**
 * (bt) SkipList的构造函数
 */
template<class Key, class Comparator>
SkipList<Key, Comparator>::SkipList(Comparator cmp)
    : arena_(), head_(new_node(Key(), kMaxHeight)),
      max_height_(1), count_(0), 
      compare_(cmp), seed_(time(NULL))
{
    srand(seed_);
    // (bt) 初始化每一层
    for (int i = 0; i < kMaxHeight; i++)
        head_->set_next(i, NULL);
}

template<class Key, class Comparator>
void SkipList<Key, Comparator>::insert(const Key& key)
{
    Node* prev[kMaxHeight];
    Node* next = find_greater_or_equal(key, prev);

    size_t height = random_height();

    if (height > max_height_) {
        for (size_t i = max_height_; i < height; i++)
            prev[i] = head_;

        max_height_ = height;
    }

    if (next && equal(next->key, key)) {
        next->set_key(key);
    } else {
        // (bt) 表中没有key，根据prev保留的位置，为每一层插入新的node，每一层拥有的只是指针，这样方便删除
        Node* curr = new_node(key, height);

        for (size_t i = 0; i < height; i++) {
            curr->set_next(i, prev[i]->next(i));
            prev[i]->set_next(i, curr);
        }

        count_++;
    }
}

template<class Key, class Comparator>
bool SkipList<Key, Comparator>::contains(const Key& key) const
{
    Node* x = find_greater_or_equal(key, NULL);
    // (bt) 可以认为最上面的每一层都是下一层的索引，如果包含的话，最底层一定存在
    if (x != NULL && equal(x->key, key))
        return true;
    else
        return false;
}

/**
 * 删除某个key，找到具有key的最高level，每一层都删除
 */
template<class Key, class Comparator>
void SkipList<Key, Comparator>::erase(const Key& key)
{
    Node* prev[kMaxHeight];
    Node* curr = find_greater_or_equal(key, prev);

    assert(curr != NULL);
    assert(equal(curr->key, key));

    for (size_t i = 0; i < max_height_; i++) {
        if (prev[i]->next(i) == curr)
            // (bt) 把指针指向其他地方
            prev[i]->set_next(i, curr->next(i));
    }

    count_--;
}

template<class Key, class Comparator>
void SkipList<Key, Comparator>::resize(size_t size)
{
    assert(size <= count_);
    
    std::vector<Key> keys;
    keys.reserve(size);

    Iterator iter(this);
    iter.seek_to_first();

    for (size_t i = 0; i < size; i++) {
        assert(iter.valid());
        keys.push_back(iter.key());
        iter.next();
    }

    clear();

    for (size_t i = 0; i < keys.size(); i++) 
        insert(keys[i]);

    count_ = size;
}

template<class Key, class Comparator>
void SkipList<Key, Comparator>::clear()
{
    arena_.clear();
    head_ = new_node(Key(), kMaxHeight);

    for (int i = 0; i < kMaxHeight; i++)
        head_->set_next(i, NULL);

    count_ = 0;
    max_height_ = 1;
}

} // namespace yodb

#endif // _YODB_SKIPLIST_H_
