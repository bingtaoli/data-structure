#ifndef _YODB_ARENA_H_
#define _YODB_ARENA_H_

#include "mutex.h"

#include <cstddef>
#include <stdlib.h>
#include <stdint.h>
#include <vector>
#include <boost/noncopyable.hpp>

namespace yodb {

const int kBlockSize = 4096;
const int kAlignedSize = 8;

class Arena : boost::noncopyable {
public:
    Arena();
    ~Arena();

    // Return a newly allocated memory block.
    char* alloc(size_t bytes);

    // Return a newly allocated memory block with address alligned by kAlignedSize.
    char* alloc_aligned(size_t bytes);
    
    // Estimate of the total memory usage of data allocated by arena.
    size_t usage() const;

    // Clear all the memory allocated, reset all the class members.
    void clear();

private:
    char* alloc_fallback(size_t bytes);
    char* alloc_new_block(size_t block_bytes);

    char* alloc_ptr_;  //(bt) 当前空闲内存 block 内的可用地址
    size_t remaining_;  // (bt) 当前空闲内存 block 内的可用大小

    mutable Mutex mutex_;

    std::vector<char*> blocks_;  // 已经申请的内存 block
    size_t blocks_size_;  ///累计分配的内存大小, memtable 内的数据量就用这个值表示
};

} // namespace yodb

#endif // _YODB_ARENA_H_
