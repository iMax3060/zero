#include "buffer_pool_pointer_swizzling.hpp"

#include "buffer_pool.hpp"
#include "btree_page_h.h"

using namespace zero::buffer_pool;

void NoSwizzling::debugDumpPointer(std::ostream& o, PageID pid) {
    o << "normal(page("
      << pid
      << "))";
}

void SimpleSwizzling::debugDumpPointer(std::ostream& o, PageID pid) {
    if (isSwizzledPointer(pid)) {
        bf_idx index = makeBufferIndex(pid);
        o << "swizzled(bf_idx("
          << index;
        o << "), page("
          << smlevel_0::bf->getControlBlock(index)._pid
          << "))";
    } else {
        NoSwizzling::debugDumpPointer(o, pid);
    }
}
