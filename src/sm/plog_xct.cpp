#include "w_defines.h"

#define SM_SOURCE
#define PLOG_XCT_C

#include "plog_xct.h"
#include "allocator.h"

const std::string plog_xct_t::IMPL_NAME = "plog";

DEFINE_SM_ALLOC(plog_xct_t);

plog_xct_t::plog_xct_t(
    sm_stats_info_t*             stats,  // allocated by caller
    timeout_in_ms                timeout,
    bool                         sys_xct,
    bool                         single_log_sys_xct,
    const lsn_t&                 last_lsn,
    const lsn_t&                 undo_nxt,
    bool                         loser_xct
)
    : xct_t(stats, timeout, sys_xct, single_log_sys_xct, tid_t::null,
            last_lsn, undo_nxt, loser_xct)
{
}

rc_t plog_xct_t::get_logbuf(logrec_t*& lr, int nbytes)
{
    //if (!curr_ext) {
        //curr_ext = ext_mgr->alloc_extent();
    //}
    //if (curr_ext->size >= NEW_EXT_THRESHOLD) {
        //link_new_ext();
    //}
    //w_assert3(curr_ext->size < NEW_EXT_THRESHOLD);
    //w_assert1(!curr_ext->committed);
    
    char* data = plog.get();

    // In the current milestone (M1), log records are replicated into
    // both the private log and the traditional ARIES log. To achieve that,
    // we simply use the logrec pointer in the current extent as the xct
    // logbuf in the traditional implementation.
    _log_buf = (logrec_t*) data;

    // The replication also means we need to invoke log reservations,
    // which is done in the traditional get_logbuf
    xct_t::get_logbuf(_log_buf, nbytes);

    return RCOK;
}

rc_t plog_xct_t::give_logbuf(logrec_t* lr, const fixable_page_h* p,
                    const fixable_page_h* p2)
{
    // If this ever happens, then memory exception should have been thrown
    // already anyway. This is the drawback of not knowing the logrec size
    // before insertion. The only solution is to use an extent size that
    // can accommodate the largest possible logrec.
    //w_assert1(curr_ext->space_available() >= lr->length());

    // replicate logic on traditional log, i.e., call log->insert and set LSN
    xct_t::give_logbuf(lr, p, p2);
    plog.give(lr);
    return RCOK;
}

//void plog_xct_t::free_extents()
//{
    //plog_ext_m::extent_t* prev = curr_ext;
    //while(curr_ext) {
        //prev = curr_ext->prev;
        //ext_mgr->free_extent(curr_ext);
        //curr_ext = prev;
    //}
//}

rc_t plog_xct_t::_abort()
{
    xct_t::_abort();
    plog.set_state(plog_t::ABORTED);
    return RCOK;
}

rc_t plog_xct_t::_commit(uint32_t flags, lsn_t* plastlsn)
{
    xct_t::_commit(flags, plastlsn);
    plog.set_state(plog_t::COMMITTED);
    return RCOK;
}

//void plog_xct_t::link_new_ext()
//{
    //plog_ext_m::extent_t* n = ext_mgr->alloc_extent();
    //w_assert1(curr_ext->next == NULL);
    //curr_ext->next = n;
    //curr_ext = n;
//}
