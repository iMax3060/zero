#ifndef __PAGE_CLEANER_H
#define __PAGE_CLEANER_H

#include "smthread.h"
#include "sm_options.h"
#include "lsn.h"
#include "allocator.h"
#include "generic_page.h"

#include "worker_thread.h"

namespace zero::buffer_pool {
    class BufferPool;
}
class generic_page;

class page_cleaner_base : public worker_thread_t {
public:
    page_cleaner_base(const sm_options& _options);
    virtual ~page_cleaner_base();

    virtual void notify_archived_lsn(lsn_t) {}

protected:
    void write_pages(size_t from, size_t to);
    void mark_pages_clean(size_t from, size_t to);

    /** the buffer pool this cleaner deals with. */
    zero::buffer_pool::BufferPool* _bufferpool;

    /** in-transit buffer for written pages */
    vector<generic_page, memalign_allocator<generic_page>> _workspace;
    size_t _workspace_size;

    vector<bf_idx> _workspace_cb_indexes;

    lsn_t _clean_lsn;

    bool _write_elision;
};

#endif // __PAGE_CLEANER_H
