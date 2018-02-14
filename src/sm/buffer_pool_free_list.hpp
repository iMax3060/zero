#ifndef ZERO_SM_BUFFER_POOL_FREE_LIST_HPP
#define ZERO_SM_BUFFER_POOL_FREE_LIST_HPP

#include <atomic>
#include <queue>

#include "sm_options.h"
#include "bf_hashtable.h"
#include "exception.hpp"

#include "MPMCQueue/MPMCQueue.h"

class bf_tree_m;

namespace zero::buffer_pool {

    class FreeList {
    public:
        FreeList(bf_tree_m* bufferpool, const sm_options& options);

        virtual void addFreeBufferpoolFrame(bf_idx freeFrame) = 0;

        virtual bool grabFreeBufferpoolFrame(bf_idx& freeFrame) = 0;

        virtual bf_idx getCount() = 0;

        class RunTimeException : public zero::RuntimeException {
        public:
            RunTimeException(std::string const& message) :
                    zero::RuntimeException(message) {};
        };

        class AddFreeBufferpoolFrameException : public RunTimeException {
        public:
            AddFreeBufferpoolFrameException(bf_idx const triedFrame) :
                    RunTimeException("Failed to add buffer frame " + std::to_string(triedFrame)
                                   + " to the free list of the buffer pool.") {};
        };

    private:
        bf_tree_m* bufferPool;

    };

    class FreeListHighContention : public FreeList {
    public:
        FreeListHighContention(bf_tree_m* bufferpool, const sm_options& options);

        virtual void    addFreeBufferpoolFrame(bf_idx freeFrame) final {
            bool pushSuccessful = list.try_push(std::move(freeFrame));
            throw1(!pushSuccessful, AddFreeBufferpoolFrameException(freeFrame));
            approximateListLength++;
        };

        virtual bool    grabFreeBufferpoolFrame(bf_idx& freeFrame) final {
            if (list.try_pop(freeFrame)) {
                approximateListLength--;
                return true;
            } else {
                freeFrame = 0;
                return false;
            }
        };

        virtual bf_idx  getCount() final {
            return approximateListLength;
        };

    private:
        rigtorp::MPMCQueue<bf_idx>  list;

        mutable std::atomic<bf_idx> approximateListLength;

    };

}

#endif //ZERO_SM_BUFFER_POOL_FREE_LIST_HPP
