#ifndef ZERO_SM_BUFFER_POOL_FREE_LIST_HPP
#define ZERO_SM_BUFFER_POOL_FREE_LIST_HPP

#include <atomic>
#include <queue>

#include "sm_options.h"
#include "bf_hashtable.h"
#include "exception.hpp"

#include "cds/container/fcqueue.h"
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

    protected:
        bf_tree_m* bufferPool;

    };

    class FreeListLowContention : public FreeList {
    public:
        FreeListLowContention(bf_tree_m* bufferpool, const sm_options& options) noexcept;

        virtual void    addFreeBufferpoolFrame(bf_idx freeFrame) noexcept final;

        virtual bool    grabFreeBufferpoolFrame(bf_idx& freeFrame) noexcept final;

        virtual bf_idx  getCount() final;

    private:
        cds::container::FCQueue<bf_idx> list;

    };

    class FreeListHighContention : public FreeList {
    public:
        FreeListHighContention(bf_tree_m* bufferpool, const sm_options& options);

        virtual void    addFreeBufferpoolFrame(bf_idx freeFrame) final;

        virtual bool    grabFreeBufferpoolFrame(bf_idx& freeFrame) final;

        virtual bf_idx  getCount() final;

    private:
        rigtorp::MPMCQueue<bf_idx>  list;

        mutable std::atomic<bf_idx> approximateListLength;

    };

} // zero::buffer_pool

#endif //ZERO_SM_BUFFER_POOL_FREE_LIST_HPP
