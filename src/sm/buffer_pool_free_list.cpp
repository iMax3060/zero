#include "buffer_pool_free_list.hpp"

#include "bf_tree.h"

zero::buffer_pool::FreeList::FreeList(bf_tree_m *bufferpool, const sm_options &options) :
        bufferPool(bufferpool) {}

zero::buffer_pool::FreeListHighContention::FreeListHighContention(bf_tree_m *bufferpool, const sm_options &options) :
        FreeList(bufferpool, options),
        list(bufferpool->get_block_cnt()) {
    bf_idx pushSuccessful = 0;
    for (bf_idx i = 1; i < bufferpool->get_block_cnt(); i++) {
        pushSuccessful += list.try_push(std::move(i));
        throw1(pushSuccessful != i + 1, AddFreeBufferpoolFrameException(i));
    }
    approximateListLength = bufferpool->get_block_cnt() - 1;
}