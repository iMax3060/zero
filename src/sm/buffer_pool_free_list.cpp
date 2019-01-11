#include "buffer_pool_free_list.hpp"

#include "buffer_pool.hpp"
#include "page_evictioner.hpp"
#include "page_evictioner_select_and_filter.hpp"
#include "page_evictioner_other.hpp"

using namespace zero::buffer_pool;

FreeList::FreeList(BufferPool* bufferPool, const sm_options& options) :
        bufferPool(bufferPool) {}

FreeListLowContention::FreeListLowContention(BufferPool* bufferPool, const sm_options& options) noexcept :
        FreeList(bufferPool, options) {
    for (bf_idx i = 1; i < bufferPool->getBlockCount(); i++) {
        list.enqueue(i);
    }
}

void FreeListLowContention::addFreeBufferpoolFrame(bf_idx freeFrame) noexcept {
    list.enqueue(freeFrame);
}

bool FreeListLowContention::grabFreeBufferpoolFrame(bf_idx& freeFrame) noexcept {
    while (true) {
        if (list.dequeue(freeFrame)) {
            return true;
        } else {
            // There're no free frames left. -> The warmup is done!
            bufferPool->_setWarmupDone();

            if (bufferPool->_asyncEviction) {
                // Start the asynchronous eviction and block until a page was evicted:
                bufferPool->getPageEvictioner()->wakeup(true);
            } else {
                freeFrame = 0;
                bool success = false;
                while (!success) {
                    freeFrame = bufferPool->getPageEvictioner()->pickVictim();
                    w_assert0(freeFrame > 0);
                    success = bufferPool->getPageEvictioner()->evictOne(freeFrame);
                }
                return true;
            }
        }
    }
}

bf_idx FreeListLowContention::getCount() {
    return list.size();
};

FreeListHighContention::FreeListHighContention(BufferPool* bufferPool, const sm_options& options) :
        FreeList(bufferPool, options),
        list(bufferPool->getBlockCount()) {
    bf_idx pushSuccessful = 0;
    for (bf_idx i = 1; i < bufferPool->getBlockCount(); i++) {
        pushSuccessful += list.try_push(std::move(i));
        throw1(pushSuccessful != i + 1, AddFreeBufferpoolFrameException(i));
    }
    approximateListLength = bufferPool->getBlockCount() - 1;
}

void FreeListHighContention::addFreeBufferpoolFrame(bf_idx freeFrame) {
    bool pushSuccessful = list.try_push(std::move(freeFrame));
    throw1(!pushSuccessful, AddFreeBufferpoolFrameException(freeFrame));
    approximateListLength++;
};

bool FreeListHighContention::grabFreeBufferpoolFrame(bf_idx &freeFrame) {
    while (true) {
        if (list.try_pop(freeFrame)) {
            approximateListLength--;
            return true;
        } else {
            // There're no free frames left. -> The warmup is done!
            bufferPool->_setWarmupDone();

            if (bufferPool->_asyncEviction) {
                // Start the asynchronous eviction and block until a page was evicted:
                bufferPool->getPageEvictioner()->wakeup(true);
            } else {
                freeFrame = 0;
                while (freeFrame == 0) {
                    freeFrame = bufferPool->getPageEvictioner()->pickVictim();
                    w_assert0(freeFrame > 0);
                    addFreeBufferpoolFrame(freeFrame);
                }
                return true;
            }
        }
    }
};

bf_idx FreeListHighContention::getCount() {
    return approximateListLength;
};
