#include "arena.h"
#include <assert.h>

using namespace yodb;

Arena::Arena()
{
    blocks_size_ = 0;
    alloc_ptr_ = NULL;
    remaining_ = 0;
}

Arena::~Arena()
{
    clear();
}

/**
 * Arena 每次按 kBlockSize(4096)单位向系统申请内存，提供地址对齐的内存，记录内存使用。
 * 当 memtable 申请内存时，如果 size 不大于 kBlockSize 的四分之一，就在当前空闲的内存 block 中 分配，
 * 否则，直接向系统申请(malloc)。这个策略是为了能更好的服务小内存的申请，避免个别大 内存使用影响。
 * @param
 * @return
 */
char* Arena::alloc(size_t bytes)
{
    assert(bytes > 0);

    ScopedMutex lock(mutex_);

    if (bytes <= remaining_) {
        char* result = alloc_ptr_;

        alloc_ptr_ += bytes;
        remaining_ -= bytes;
        
        return result;
    }

    return alloc_fallback(bytes);
}

char* Arena::alloc_aligned(size_t bytes)
{
    ScopedMutex lock(mutex_);

    size_t cut = reinterpret_cast<uintptr_t>(alloc_ptr_) & (kAlignedSize - 1); 
    size_t slop = (cut == 0 ? 0 : kAlignedSize - cut);
    size_t fixed_size = bytes + slop;
    char* result;

    if (fixed_size <= remaining_) {
        result = alloc_ptr_ + slop;
        alloc_ptr_ += fixed_size;
        remaining_ -= fixed_size;
    } else {
        result = alloc_fallback(bytes);
    }

    assert((reinterpret_cast<uintptr_t>(result) & (kAlignedSize - 1)) == 0);

    return result;
}

/**
 * 当alloc某个size不够时，向系统申请新的内存；如果bytes>kBlockSize / 4直接向系统申请，否则申请一个block，再在block中申请
 * @param
 * @return
 */
char* Arena::alloc_fallback(size_t bytes)
{
    if (bytes > kBlockSize / 4)
        return alloc_new_block(bytes);

    alloc_ptr_ = alloc_new_block(kBlockSize);
    remaining_ = kBlockSize;

    char* result = alloc_ptr_;

    alloc_ptr_ += bytes;
    remaining_ -= bytes;

    return result;
}

char* Arena::alloc_new_block(size_t block_bytes)
{
    char* result = new char[block_bytes];

    blocks_size_ += block_bytes;
    blocks_.push_back(result);

    return result;
}

size_t Arena::usage() const 
{
    ScopedMutex lock(mutex_);
    return blocks_size_ + blocks_.capacity() * (sizeof(char*));
}

void Arena::clear()
{
    ScopedMutex lock(mutex_);

    for (size_t i = 0; i < blocks_.size(); i++)
        delete[] blocks_[i];

    blocks_size_ = 0;
    alloc_ptr_ = NULL;
    remaining_ = 0;

    blocks_.clear();
}
