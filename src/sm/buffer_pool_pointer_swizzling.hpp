#ifndef __ZERO_BUFFER_POOL_POINTER_SWIZZLING_HPP
#define __ZERO_BUFFER_POOL_POINTER_SWIZZLING_HPP

#include "basics.h"
#include "generic_page.h"

namespace zero::buffer_pool {

    class NoSwizzling {
        friend class BufferPool;
    public:
        static constexpr bool usesPointerSwizzling = false;

        static bool isSwizzledPointer(const PageID pid) noexcept {
            return false;
        };

        static PageID makeSwizzledPointer(const bf_idx index) noexcept {
            w_assert0(false);
        };

        static bf_idx makeBufferIndex(const PageID pid) noexcept {
            w_assert0(false);
        };

        static general_recordid_t findSwizzledPageIDSlot(generic_page* page, bf_idx index) {
            w_assert0(false);
        };

        /*!\fn      debugDumpPointer(std::ostream& o, PageID pid) const
         * \brief   Dumps the given page ID to an output stream
         * \details Dumps the given possibly swizzled page ID with additional information about swizzling etc. to an
         *          output stream.
         *
         * \pre     The stream manipulators of \c o are set as expected.
         *
         * @param o   The output stream to which the pointer is printed.
         * @param pid The pointer (page ID) to print.
         */
        static void debugDumpPointer(std::ostream& o, PageID pid);

    private:
        static bf_idx makeBufferIndexForFix(generic_page* parentPage, generic_page*& targetPage, PageID pid) {
            w_assert0(false);
        };

    };

    class SimpleSwizzling {
        friend class BufferPool;
    public:
        static constexpr bool usesPointerSwizzling = true;

        /*!\fn      isSwizzledPointer(const PageID pid) const noexcept
         * \brief   Checks whether a page ID is a swizzled buffer index
         * \details Whether the given page ID is actually swizzled and therefore a buffer pool index marked as such.
         *
         * @param pid A page ID that might be swizzled.
         * @return    Whether \c pid is a swizzled pointer.
         */
        static bool isSwizzledPointer(const PageID pid) noexcept {
            return (pid & swizzledPIDBit) != 0;
        };

        /*!\fn      addSwizzledPIDBit(const PageID pid) const noexcept
         * \brief   Transforms a buffer pool index to a swizzled page ID
         * \details Transforms the given buffer pool index of this buffer pool to a swizzled page ID by adding the
         *          marking as swizzled pointer.
         *
         * @param index A buffer pool index of this buffer pool.
         * @return      The swizzled page ID for the parent page.
         */
        static PageID makeSwizzledPointer(const bf_idx index) noexcept {
            return index | swizzledPIDBit;
        };

        /*!\fn      removeSwizzledPIDBit(const PageID pid) const noexcept
         * \brief   Transforms a swizzled page ID to a buffer pool index
         * \details Transforms the given page ID which must be swizzled to a buffer pool index of this buffer pool by
         *          removing the marking as swizzled pointer.
         *
         * @param pid A swizzled page ID of this buffer pool.
         * @return    The buffer pool index of this buffer pool where the page specified in \c pid can be found.
         */
        static bf_idx makeBufferIndex(const PageID pid) noexcept {
            return pid & ~swizzledPIDBit;
        };

        static general_recordid_t findSwizzledPageIDSlot(generic_page* page, bf_idx index);

        /*!\fn      debugDumpPointer(std::ostream& o, PageID pid) const
         * \brief   Dumps the given page ID to an output stream
         * \details Dumps the given possibly swizzled page ID with additional information about swizzling etc. to an
         *          output stream.
         *
         * \pre     The stream manipulators of \c o are set as expected.
         *
         * @param o   The output stream to which the pointer is printed.
         * @param pid The pointer (page ID) to print.
         */
        static void debugDumpPointer(std::ostream& o, PageID pid);

    private:
        static bf_idx makeBufferIndexForFix(generic_page* parentPage, generic_page*& targetPage, PageID pid);

        static constexpr PageID swizzledPIDBit = 0b10000000000000000000000000000000;

    };

    class LeanStoreSwizzling {
        friend class BufferPool;
    public:
        static constexpr bool usesPointerSwizzling = true;

        static bool isSwizzledPointer(const PageID pid) noexcept {
            return (pid & swizzledPIDBit) != 0;
        };

        static PageID makeSwizzledPointer(const bf_idx index) noexcept {
            return index | hotPIDBits;
        };

        static bf_idx makeBufferIndex(const PageID pid) noexcept {
            return pid & ~hotPIDBits;
        };

        static general_recordid_t findSwizzledPageIDSlot(generic_page* page, bf_idx index);

        /*!\fn      debugDumpPointer(std::ostream& o, PageID pid) const
         * \brief   Dumps the given page ID to an output stream
         * \details Dumps the given possibly swizzled page ID with additional information about swizzling etc. to an
         *          output stream.
         *
         * \pre     The stream manipulators of \c o are set as expected.
         *
         * @param o   The output stream to which the pointer is printed.
         * @param pid The pointer (page ID) to print.
         */
        static void debugDumpPointer(std::ostream& o, PageID pid);

    private:
        static bool isHotPointer(const PageID pid) noexcept {
            return (pid & hotPIDBit) != 0;
        };

        static bool isCoolingPointer(const PageID pid) noexcept {
            return isSwizzledPointer(pid) && !isHotPointer(pid);
        };

        static PageID makeCoolingPointerFromHotPointer(const PageID pid) noexcept {
            return pid ^ hotPIDBit;
        };

        static PageID makeCoolingPointerFromBufferIndex(const bf_idx index) noexcept {
            return index | coolingPIDBit;
        };

        static bf_idx makeBufferIndexForFix(generic_page* parentPage, generic_page*& targetPage, PageID pid);

        static constexpr PageID coolingPIDBit =  0b10000000000000000000000000000000;
        static constexpr PageID swizzledPIDBit = 0b10000000000000000000000000000000;

        static constexpr PageID hotPIDBit =      0b01000000000000000000000000000000;

        static constexpr PageID hotPIDBits =     0b11000000000000000000000000000000;

    };

} // zero::buffer_pool

#endif // __ZERO_BUFFER_POOL_POINTER_SWIZZLING_HPP
