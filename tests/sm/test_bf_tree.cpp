#include "btree_test_env.h"
#include "generic_page.h"
#include "btree.h"
#include "btree_page_h.h"
#include "btree_impl.h"
#include "log_core.h"
#include "w_error.h"

#include "bf_tree_cb.h"
#include "buffer_pool.hpp"
#include "sm_base.h"

#include <vector>
#include <set>

btree_test_env *test_env;
/**
 * Unit test for new bufferpool for B-tree pages (zero::buffer_pool::BufferPool).
 * This class (test_bf_tree) is a friend of zero::buffer_pool::BufferPool so that
 * we can test private methods in these test cases.
 */
class test_bf_tree {
public:
    static bf_idx get_bf_idx (zero::buffer_pool::BufferPool *bf, generic_page* page) {
        return bf->getIndex(page);
    }
    static bf_tree_cb_t& get_bf_control_block(zero::buffer_pool::BufferPool *bf, generic_page* page) {
        bf_idx idx = get_bf_idx(bf, page);
        return bf->getControlBlock(idx);
    }


    /** manually emulate the btree page layout */
    static void _add_child_pointer (btree_page *page, PageID child) {
        btree_page_h p;
        p.fix_nonbufferpool_page(reinterpret_cast<generic_page*>(page)); // <<<>>>
        if (!page->insert_item(p.nrecs()+1, false, 0, child, 0)) {
            w_assert1(false);
        }
        if (p.nrecs() == 0) {
            *p.page_pointer_address(-1) = child;
        }
    }
};


enum test_size_t {
    SMALL, NORMAL, LARGE
};

void run_bf_test(w_rc_t (*func)(ss_m*, test_volume_t*),
    test_size_t size, bool initially_enable_cleaners/*, bool enable_swizzling*/)
{
    size_t npages = (size == LARGE ? 10000 : (size == NORMAL ? 1024 : 256));
    // (some of) tests in this file needs REALLY big log.
    test_env->empty_logdata_dir();
    sm_options options;
    options.set_int_option("sm_logbufsize", 512 << 10);
    options.set_int_option("sm_logsize", 8192 << 10);
    options.set_int_option("sm_locktablesize", default_locktable_size);
    options.set_int_option("sm_bufpoolsize", (sizeof(generic_page) * npages) / 1048576);
    options.set_int_option("sm_num_page_writers", 1);
    options.set_int_option("sm_cleaner_interval_millisec_min",
        (size == LARGE ? 10000 : (size == NORMAL ? 1000 : 20)));
    options.set_int_option("sm_cleaner_interval_millisec_max", 10000);
    options.set_int_option("sm_cleaner_write_buffer_pages", 64);
    options.set_bool_option("sm_backgroundflush", initially_enable_cleaners);

    options.set_int_option("sm_rawlock_lockpool_initseg",
        (size == LARGE ? 100 : (size == NORMAL ? 50 : 20)));
    options.set_int_option("sm_rawlock_lockpool_segsize",
        (size == LARGE ? 1 << 14 : (size == NORMAL ? 1 << 12 : 1 << 10)));
    options.set_int_option("sm_rawlock_gc_generation_count",
        (size == LARGE ? 30 : (size == NORMAL ? 20 : 10)));
    options.set_int_option("sm_rawlock_gc_free_segment_count",
        (size == LARGE ? 50 : (size == NORMAL ? 20 : 10)));
    options.set_int_option("sm_rawlock_gc_max_segment_count",
        (size == LARGE ? 200 : (size == NORMAL ? 100 : 50)));

    EXPECT_EQ(test_env->runBtreeTest(func, false, options), 0);
}

TEST (TreeBufferpoolTest, AlignmentCheck) {
    cout << "sizeof(bf_tree_cb_t)=" << sizeof(bf_tree_cb_t) << endl;
    EXPECT_EQ ((uint)0, sizeof(bf_tree_cb_t) % 8);
}

w_rc_t test_bf_init(ss_m* /*ssm*/, test_volume_t */*test_volume*/) {
    ::usleep(200000); // CS TODO WHY?????????
    zero::buffer_pool::BufferPool&pool(*smlevel_0::bf);
    pool.debugDump(std::cout);
    return RCOK;
}
//TEST (TreeBufferpoolTest, Init) {
//    run_bf_test(test_bf_init, SMALL, true, true);
//}
w_rc_t test_bf_fix_virgin_root(ss_m* /*ssm*/, test_volume_t *test_volume) {
    lsn_t thelsn = smlevel_0::log->curr_lsn();
    zero::buffer_pool::BufferPool&pool(*smlevel_0::bf);
    for (size_t i = 1; i < 4; ++i) {
        generic_page *page = nullptr;
        PageID pid = i+10;
        W_DO(pool.fixRootOldStyleExceptions(page, i, LATCH_SH, false, true));
        EXPECT_TRUE (page != nullptr);
        if (page != nullptr) {
            ::memset(page, 0, sizeof(generic_page));
            btree_page *bp = reinterpret_cast<btree_page*>(page);
            page->pid      = pid;
            page->store    = i;
            bp->lsn        = thelsn;
            page->tag      = t_btree_p;
            bp->btree_level = 1;
            bp->init_items();
            pool.unfix(page);
        }

        // fix again
        page = nullptr;
        W_DO(pool.fixRootOldStyleExceptions(page, i, LATCH_SH, false, false));
        EXPECT_TRUE (page != nullptr);
        if (page != nullptr) {
            btree_page *bp = reinterpret_cast<btree_page*>(page);
            EXPECT_EQ(i + 10, page->pid);
            EXPECT_EQ(i, page->store);
            EXPECT_EQ(thelsn, bp->lsn);
            EXPECT_EQ(1, bp->btree_level);
            pool.unfix(page);
        }
    }
    pool.debugDump(std::cout);
    return RCOK;
}
//TEST (TreeBufferpoolTest, FixVirginRoot) {
//    run_bf_test(test_bf_fix_virgin_root, SMALL, true, true);
//}

w_rc_t test_bf_fix_virgin_child(ss_m* /*ssm*/, test_volume_t *test_volume) {
    zero::buffer_pool::BufferPool&pool(*smlevel_0::bf);
    lsn_t thelsn = smlevel_0::log->curr_lsn();
    PageID root_pid = 11;
    StoreID stid = 1;

    generic_page *root_page = nullptr;
    W_DO(pool.fixRootOldStyleExceptions(root_page, stid, LATCH_EX, false, true));
    EXPECT_TRUE (root_page != nullptr);
    ::memset(root_page, 0, sizeof(generic_page));
    btree_page *rbp = reinterpret_cast<btree_page*>(root_page);
    root_page->pid    = root_pid;
    rbp->lsn          = thelsn;
    root_page->tag    = t_btree_p;
    rbp->btree_level  = 2;
    rbp->btree_foster = 0;
    rbp->init_items();
    for (size_t i = 0; i < 3; ++i) {
        generic_page *page = nullptr;
        PageID pid = root_pid + 1 + i;
        test_bf_tree::_add_child_pointer (rbp, pid);

        W_DO(pool.fixNonRootOldStyleExceptions(page, root_page, pid, LATCH_EX, false, true));
        EXPECT_TRUE (page != nullptr);
        if (page != nullptr) {
            ::memset(page, 0, sizeof(generic_page));
            btree_page *bp = reinterpret_cast<btree_page*>(page);
            page->pid = pid;
            bp->lsn = thelsn;
            page->tag = t_btree_p;
            bp->btree_level = 1;
            bp->init_items();
            pool.unfix(page);
        }

        // fix again
        page = nullptr;
        W_DO(pool.fixNonRootOldStyleExceptions(page, root_page, pid, LATCH_SH, false, false));
        EXPECT_TRUE (page != nullptr);
        if (page != nullptr) {
            btree_page *bp = reinterpret_cast<btree_page*>(page);
            EXPECT_EQ(pid, page->pid);
            EXPECT_EQ(thelsn, bp->lsn);
            EXPECT_EQ(t_btree_p, page->tag);
            EXPECT_EQ(1, bp->btree_level);
            pool.unfix(page);
        }
    }

    pool.unfix(root_page);
    pool.debugDump(std::cout);
    return RCOK;
}
//TEST (TreeBufferpoolTest, FixVirginChild) {
//    run_bf_test(test_bf_fix_virgin_child, SMALL, true, true);
//}

// make big enough database for tests
w_rc_t prepare_test(ss_m* ssm, test_volume_t *test_volume, StoreID &stid, PageID &root_pid) {
    W_DO(x_btree_create_index(ssm, test_volume, stid, root_pid));

    const int recsize = SM_PAGESIZE / 6;
    char datastr[recsize];
    ::memset (datastr, 'a', recsize);
    vec_t data;
    data.set(datastr, recsize);

    // create at least 30 pages.
    // commit each time so that pages can be evicted
    w_keystr_t key;
    char keystr[6] = "";
    ::memset(keystr, '\0', 6);
    keystr[0] = 'k';
    keystr[1] = 'e';
    keystr[2] = 'y';
    for (int i = 0; i < 180; ++i) {
        keystr[3] = ('0' + ((i / 100) % 10));
        keystr[4] = ('0' + ((i / 10) % 10));
        keystr[5] = ('0' + ((i / 1) % 10));
        key.construct_regularkey(keystr, 6);
        W_DO(ssm->begin_xct());
        test_env->set_xct_query_lock();
        W_DO(ssm->create_assoc(stid, key, data));
        W_DO(ssm->commit_xct());
    }
    W_DO(x_btree_verify(ssm, stid));
    smlevel_0::bf->getPageCleaner()->wakeup(true);
    return RCOK;
}

w_rc_t test_bf_evict(ss_m* ssm, test_volume_t *test_volume) {
    StoreID stid;
    PageID root_pid;
    W_DO (prepare_test(ssm, test_volume, stid, root_pid));

    btree_page_h root_p;
    W_DO(root_p.fix_root(stid, LATCH_SH));
    EXPECT_TRUE (root_p.is_node());
    EXPECT_TRUE (root_p.nrecs() > 30);

    W_DO(ssm->begin_xct());

    btree_page_h keep_latch_p;
    const size_t keep_latch_i = 23; // this page will be kept latched
    std::map<size_t, lsn_t> dirty_lsns;
    for (size_t i = 0; i < 30; ++i) {
        PageID pid = root_p.child(i);
        if (i == keep_latch_i) {
            W_DO(keep_latch_p.fix_nonroot(root_p, pid, LATCH_SH));
            continue;
        }
        btree_page_h child_p;
        W_DO(child_p.fix_nonroot(root_p, pid, i % 5 == 0 ? LATCH_EX : LATCH_SH));
        if (i % 5 == 0) {
            // do some operation to make this page dirty
            w_keystr_t key;
            child_p.get_key(0, key);
            EXPECT_FALSE(child_p.is_dirty());
            W_DO(ssm->destroy_assoc(stid, key));
            EXPECT_TRUE(child_p.is_dirty());
            dirty_lsns.insert(std::pair<size_t, lsn_t>(i, child_p.lsn()));
        }
        child_p.unfix();
    }

    // fix again
    for (size_t i = 0; i < 30; ++i) {
        if (i == keep_latch_i) {
            EXPECT_FALSE(keep_latch_p.is_dirty());
        } else {
            PageID pid = root_p.child(i);
            btree_page_h child_p;
            W_DO(child_p.fix_nonroot(root_p, pid, LATCH_SH));
            EXPECT_EQ(pid, child_p.pid()) << "i" << i;
            EXPECT_EQ(1, child_p.level());
            if (i % 5 == 0) {
                std::map<size_t, lsn_t>::const_iterator di = dirty_lsns.find(i);
                EXPECT_NE(dirty_lsns.end(), di);
                if (di != dirty_lsns.end()) {
                    EXPECT_EQ(di->second, child_p.lsn());
                }
                EXPECT_TRUE(child_p.is_dirty()) << "i" << i;
            } else {
                EXPECT_FALSE(child_p.is_dirty()) << "i" << i;
            }
            child_p.unfix();
        }
    }
    keep_latch_p.unfix();
    root_p.unfix();

    W_DO(ssm->commit_xct());

    return RCOK;
}
TEST (TreeBufferpoolTest, EvictNoSwizzle) {
    run_bf_test(test_bf_evict, NORMAL, false/*, false*/);
}
//TEST (TreeBufferpoolTest, EvictSwizzle) {
//    run_bf_test(test_bf_evict, NORMAL, false, true);
//}

w_rc_t _test_bf_swizzle(ss_m* /*ssm*/, test_volume_t *test_volume, bool enable_swizzle) {
    zero::buffer_pool::BufferPool &pool(*smlevel_0::bf);
    PageID root_pid = 3;

    generic_page *root_page = nullptr;
    StoreID stid = 1;
    W_DO(pool.fixRootOldStyleExceptions(root_page, stid, LATCH_EX, false, true));
    EXPECT_TRUE (root_page != nullptr);
    ::memset(root_page, 0, sizeof(generic_page));
    btree_page *rbp = reinterpret_cast<btree_page*>(root_page);
    root_page->pid = root_pid;
    root_page->tag = t_btree_p;
    rbp->btree_level = 2;
    rbp->btree_foster = 0;
    rbp->init_items();
    bf_tree_cb_t& root_cb = test_bf_tree::get_bf_control_block(&pool, root_page);
    // EXPECT_EQ(1, root_cb._pin_cnt); // root page is always swizzled by volume descriptor, so pin_cnt is 1.
    if (enable_swizzle) {
        EXPECT_TRUE (pool.getControlBlock(root_page)._swizzled);
    }

    pool.debugDumpPagePointers(std::cout, root_page);
    for (size_t i = 0; i < 20; ++i) {
        generic_page *page = nullptr;
        PageID pid = root_pid + 1 + i;
        test_bf_tree::_add_child_pointer (rbp, pid);

        if (enable_swizzle) {
#ifdef BP_MAINTAIN_PARENT_PTR
            // EXPECT_EQ ((int) (1 + i), root_cb._pin_cnt);
#else // BP_MAINTAIN_PARENT_PTR
            // EXPECT_EQ ((int) (1), root_cb._pin_cnt);
#endif // BP_MAINTAIN_PARENT_PTR
        } else {
            // EXPECT_EQ ((int) (1), root_cb._pin_cnt);
        }
        W_DO(pool.fixNonRootOldStyleExceptions(page, root_page, pid, LATCH_EX, false, true));
        EXPECT_TRUE (page != nullptr);
        if (page != nullptr) {
            bf_tree_cb_t& cb = test_bf_tree::get_bf_control_block(&pool, page);
            if (enable_swizzle) {
                // EXPECT_EQ (1, cb._pin_cnt); // because it's swizzled, pin_cnt is 1
                EXPECT_TRUE (pool.getControlBlock(page)._swizzled);
#ifdef BP_MAINTAIN_PARENT_PTR
                // EXPECT_EQ ((int) (2 + i), root_cb._pin_cnt); // parent's pin_cnt is added
#else // BP_MAINTAIN_PARENT_PTR
                // EXPECT_EQ ((int) 1, root_cb._pin_cnt);
#endif // BP_MAINTAIN_PARENT_PTR
            } else {
                // EXPECT_EQ (0, cb._pin_cnt); // otherwise, it's 0 after fix()
                // EXPECT_EQ ((int) (1), root_cb._pin_cnt);
            }
            ::memset(page, 0, sizeof(generic_page));
            btree_page *bp = reinterpret_cast<btree_page*>(page);
            page->pid = pid;
            page->tag = t_btree_p;
            bp->btree_level = 1;
            bp->init_items();
            pool.unfix(page);
            //  same after unfix too.
            if (enable_swizzle) {
                // EXPECT_EQ (1, cb._pin_cnt);
                EXPECT_TRUE (pool.getControlBlock(page)._swizzled);
            } else {
                // EXPECT_EQ (0, cb._pin_cnt);
            }
        }
    }
    pool.debugDumpPagePointers(std::cout, root_page);
    if (enable_swizzle) {
        // fix again
        for (size_t i = 0; i < 20; ++i) {
            generic_page *page = nullptr;
            PageID pid = root_pid + 1 + i;
            W_DO(pool.fixNonRootOldStyleExceptions(page, root_page, pid, LATCH_SH, false, false));
            EXPECT_TRUE (page != nullptr);
            if (page != nullptr) {
                btree_page *bp = reinterpret_cast<btree_page*>(page);
                bf_tree_cb_t& cb = test_bf_tree::get_bf_control_block(&pool, page);
                EXPECT_EQ(pid, page->pid);
                EXPECT_EQ(1, bp->btree_level);
                if (enable_swizzle) {
                    // EXPECT_EQ (1, cb._pin_cnt);
                    EXPECT_TRUE (pool.getControlBlock(page)._swizzled);
                } else {
                    // EXPECT_EQ (0, cb._pin_cnt);
                }
                pool.unfix(page);
                if (enable_swizzle) {
                    // EXPECT_EQ (1, cb._pin_cnt);
                    EXPECT_TRUE (pool.getControlBlock(page)._swizzled);
                } else {
                    // EXPECT_EQ (0, cb._pin_cnt);
                }
            }
        }
    if (enable_swizzle) {
#ifdef BP_MAINTAIN_PARENT_PTR
        // EXPECT_EQ (1 + 20, root_cb._pin_cnt);
#else // BP_MAINTAIN_PARENT_PTR
        // EXPECT_EQ (1, root_cb._pin_cnt);
#endif // BP_MAINTAIN_PARENT_PTR
    } else {
        // EXPECT_EQ (1, root_cb._pin_cnt);
    }
    pool.unfix(root_page);
    if (enable_swizzle) {
#ifdef BP_MAINTAIN_PARENT_PTR
        // EXPECT_EQ (1 + 20, root_cb._pin_cnt);
#else // BP_MAINTAIN_PARENT_PTR
        // EXPECT_EQ (1, root_cb._pin_cnt);
#endif // BP_MAINTAIN_PARENT_PTR
    } else {
        // EXPECT_EQ (1, root_cb._pin_cnt);
    }

    pool.debugDumpPagePointers(std::cout, root_page);
    pool.debugDump(std::cout);
    return RCOK;
}
//w_rc_t test_bf_swizzle(ss_m* ssm, test_volume_t *test_volume) {
//    return _test_bf_swizzle(ssm, test_volume, true);
//}
w_rc_t test_bf_noswizzle(ss_m* ssm, test_volume_t *test_volume) {
    return _test_bf_swizzle(ssm, test_volume/*, false*/);
}
//TEST (TreeBufferpoolTest, Swizzle) {
//    // disable background cleaner because we test pin_cnt
//    run_bf_test(test_bf_swizzle, LARGE, false, true);
//}
TEST (TreeBufferpoolTest, NoSwizzle) {
    run_bf_test(test_bf_noswizzle, LARGE, false/*, false*/);
}

#ifdef BP_MAINTAIN_PARENT_PTR
// w_rc_t test_bf_switch_parent(ss_m* /*ssm*/, test_volume_t *test_volume) {

//     zero::buffer_pool::BufferPool&pool(*smlevel_0::bf);
//     PageID root_pid(test_volume->_vid, 3);

//     btree_page *root_page = nullptr;
//     W_DO(pool.fix_virgin_root(root_page, test_volume->_vid, 1, root_pid.page));
//     EXPECT_TRUE (root_page != nullptr);
//     ::memset(root_page, 0, sizeof(generic_page));
//     root_page->pid = root_pid;
//     root_page->tag = t_btree_p;
//     root_page->btree_level = 2;
//     root_page->btree_foster = 0;
//     root_page->init_items();
//     bf_tree_cb_t &root_cb (*test_bf_tree::get_bf_control_block(&pool, root_page));
//     EXPECT_EQ(1, root_cb._pin_cnt); // root page is always swizzled, so pin_cnt is 1.
//     EXPECT_TRUE (pool.is_swizzled(root_page));

//     PageID child_pid(test_volume->_vid, 1, root_pid.page + 1);
//     test_bf_tree::_add_child_pointer (root_page, child_pid.page);
//     btree_page *child_page = nullptr;
//     W_DO(pool.fix_nonroot(child_page, root_page, child_pid.vol(), child_pid.page, LATCH_EX, true, true));
//     EXPECT_TRUE (child_page != nullptr);
//     ::memset(child_page, 0, sizeof(generic_page));
//     child_page->pid = child_pid;
//     child_page->tag = t_btree_p;
//     child_page->btree_level = 1;
//     child_page->btree_foster = 0;
//     child_page->init_items();
//     bf_tree_cb_t &child_cb (*test_bf_tree::get_bf_control_block(&pool, child_page));
//     EXPECT_EQ(1, child_cb._pin_cnt);
//     EXPECT_EQ(2, root_cb._pin_cnt); // added as child
//     EXPECT_TRUE (pool.is_swizzled(child_page));

//     PageID sibling_pid(test_volume->_vid, 1, root_pid.page + 2);
//     child_page->btree_foster = sibling_pid.page; // add as a foster child
//     generic_page *sibling_page = nullptr;
//     W_DO(pool.fix_nonroot(sibling_page, child_page, sibling_pid.vol(), sibling_pid.page, LATCH_EX, true, true));
//     EXPECT_TRUE (sibling_page != nullptr);
//     ::memset(sibling_page, 0, sizeof(generic_page));
//     sibling_page->pid = sibling_pid;
//     sibling_page->tag = t_btree_p;
//     sibling_page->btree_level = 1;
//     sibling_page->btree_foster = 0;
//     sibling_page->init_items();
//     bf_tree_cb_t &sibling_cb (*test_bf_tree::get_bf_control_block(&pool, sibling_page));
//     EXPECT_EQ(1, sibling_cb._pin_cnt);
//     EXPECT_EQ(2, child_cb._pin_cnt); // added as child
//     EXPECT_EQ(2, root_cb._pin_cnt);
//     EXPECT_TRUE (pool.is_swizzled(sibling_page));

//     pool.debug_dump(std::cout);

//     // then, adopt the sibling to real parent
//     test_bf_tree::_add_child_pointer (root_page, sibling_pid.page);
//     child_page->btree_foster = 0;
//     pool.switch_parent(sibling_page, root_page);

//     // moved the pin count to real parent
//     EXPECT_EQ(1, sibling_cb._pin_cnt);
//     EXPECT_EQ(1, child_cb._pin_cnt);
//     EXPECT_EQ(3, root_cb._pin_cnt);

//     pool.debug_dump(std::cout);

//     pool.unfix(sibling_page);
//     pool.unfix(child_page);
//     pool.unfix(root_page);

//     pool.debug_dump(std::cout);
//     return RCOK;
// }
// TEST (TreeBufferpoolTest, SwitchParent) {
//     test_env->empty_logdata_dir();
//     EXPECT_EQ(test_env->runBtreeTest(test_bf_switch_parent,
//         false, default_locktable_size, default_quota_in_pages, 10,
//         1, 10000, 10000, 64, false, true
//     ), 0);
// }
#endif // BP_MAINTAIN_PARENT_PTR

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    test_env = new btree_test_env();
    ::testing::AddGlobalTestEnvironment(test_env);
    return RUN_ALL_TESTS();
}
