#include "buffer_pool_free_list.hpp"

#include "bf_tree.h"

zero::buffer_pool::FreeList::FreeList(bf_tree_m *bufferpool, const sm_options &options) :
        bufferPool(bufferpool) {}

zero::buffer_pool::FreeListLowContention::FreeListLowContention(bf_tree_m *bufferpool, const sm_options &options) noexcept :
        FreeList(bufferpool, options) {
    for (bf_idx i = 1; i < bufferpool->get_block_cnt(); i++) {
        list.enqueue(i);
    }
}

void zero::buffer_pool::FreeListLowContention::addFreeBufferpoolFrame(bf_idx freeFrame) noexcept {
    list.enqueue(freeFrame);
}

bool zero::buffer_pool::FreeListLowContention::grabFreeBufferpoolFrame(bf_idx &freeFrame) noexcept {
    while (true) {
        if (list.dequeue(freeFrame)) {
            return true;
        } else {
            // There're no free frames left. -> The warmup is done!
            bufferPool->set_warmup_done();

            if (bufferPool->_async_eviction) {
                // Start the asynchronous eviction and block until a page was evicted:
                bufferPool->_evictioner->wakeup(true);
            } else {
                freeFrame = 0;
                bool success = false;
                while (!success) {
                    freeFrame = bufferPool->_evictioner->pickVictim();
                    w_assert0(freeFrame > 0);
                    success = bufferPool->_evictioner->evictOne(freeFrame);
                }
                return true;
            }
        }
    }
}

bf_idx zero::buffer_pool::FreeListLowContention::getCount() {
    return list.size();
};

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

void zero::buffer_pool::FreeListHighContention::addFreeBufferpoolFrame(bf_idx freeFrame) {
    bool pushSuccessful = list.try_push(std::move(freeFrame));
    throw1(!pushSuccessful, AddFreeBufferpoolFrameException(freeFrame));
    approximateListLength++;
};

bool zero::buffer_pool::FreeListHighContention::grabFreeBufferpoolFrame(bf_idx &freeFrame) {
    while (true) {
        if (list.try_pop(freeFrame)) {
            approximateListLength--;
            return true;
        } else {
            // There're no free frames left. -> The warmup is done!
            bufferPool->set_warmup_done();

            if (bufferPool->_async_eviction) {
                // Start the asynchronous eviction and block until a page was evicted:
                bufferPool->_evictioner->wakeup(true);
            } else {
                freeFrame = 0;
                bool success = false;
                while (!success) {
                    freeFrame = bufferPool->_evictioner->pickVictim();
                    w_assert0(freeFrame > 0);
                    success = bufferPool->_evictioner->evictOne(freeFrame);
                }
                return true;
            }
        }
    }
};

bf_idx zero::buffer_pool::FreeListHighContention::getCount() {
    return approximateListLength;
};
