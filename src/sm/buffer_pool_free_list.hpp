#ifndef __SM_BUFFER_POOL_FREE_LIST_HPP
#define __SM_BUFFER_POOL_FREE_LIST_HPP

#include <atomic>
#include <queue>

#include "sm_options.h"
#include "basics.h"
#include "exception.hpp"

#include "cds/container/fcqueue.h"
#include "MPMCQueue/MPMCQueue.h"

namespace zero::buffer_pool {
    class BufferPool;

    class FreeList {
    public:
        FreeList(BufferPool* bufferpool, const sm_options& options);

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
        BufferPool* bufferPool;

    };

    class FreeListLowContention : public FreeList {
    public:
        FreeListLowContention(BufferPool* bufferpool, const sm_options& options) noexcept;

        virtual void    addFreeBufferpoolFrame(bf_idx freeFrame) noexcept final;

        virtual bool    grabFreeBufferpoolFrame(bf_idx& freeFrame) noexcept final;

        virtual bf_idx  getCount() final;

    private:
        cds::container::FCQueue<bf_idx> list;

    };

    class FreeListHighContention : public FreeList {
    public:
        FreeListHighContention(BufferPool* bufferpool, const sm_options& options);

        virtual void    addFreeBufferpoolFrame(bf_idx freeFrame) final;

        virtual bool    grabFreeBufferpoolFrame(bf_idx& freeFrame) final;

        virtual bf_idx  getCount() final;

    private:
        rigtorp::MPMCQueue<bf_idx> list;

        mutable atomic_bf_idx      approximateListLength;

    };

} // zero::buffer_pool

#endif // __SM_BUFFER_POOL_FREE_LIST_HPP
