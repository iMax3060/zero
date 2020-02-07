#ifndef __ZERO_BUFFER_POOL_POINTER_SWIZZLING_HPP
#define __ZERO_BUFFER_POOL_POINTER_SWIZZLING_HPP

#include "basics.h"
#include "generic_page.h"
#include "page_evictioner_typedefs.hpp"

namespace zero::buffer_pool {

    /*!\class   NoSwizzling
     * \brief   TODO
     * \details TODO
     */
    class NoSwizzling {
        friend class BufferPool;
    public:
        /*!\var     usesPointerSwizzling
         * \brief   TODO
         */
        static constexpr bool usesPointerSwizzling = false;

        /*!\fn      isSwizzledPointer(const PageID pid) noexcept
         * \brief   Checks whether a page ID is a swizzled buffer index
         * \details Because this does not use pointer swizzling, there will be no swizzled pointers.
         *
         * @param pid A page ID that is not swizzled.
         * @return    Whether \c pid is a swizzled pointer, which is always \c false .
         */
        static bool isSwizzledPointer(const PageID pid) noexcept {
            return false;
        };

        /*!\fn      makeSwizzledPointer(const bf_idx index) noexcept
         * \brief   Transforms a buffer pool index into a swizzled page ID
         * \details Because this does not use pointer swizzling, this cannot make swizzled pointers.
         *
         * @post    The program exits with a failed assertion.
         *
         * @param index A buffer pool index
         * @return      Nothing because the program crashes
         */
        static PageID makeSwizzledPointer(const bf_idx index) noexcept {
            w_assert0(false);
        };

        /*!\fn      makeBufferIndex(const PageID pid) noexcept
         * \brief   Transforms a swizzled page ID to a buffer pool index
         * \details Because this does not use pointer swizzling, this cannot make buffer indexes from swizzled
         *          pointers.
         *
         * @post    The program exits with a failed assertion.
         *
         * @param pid A page ID that is not swizzled.
         * @return    Nothing because the program crashes
         */
        static bf_idx makeBufferIndex(const PageID pid) noexcept {
            w_assert0(false);
        };

        /*!\fn      debugDumpPointer(std::ostream& o, PageID pid) const
         * \brief   Dumps the given page ID to an output stream
         * \details Dumps the given page ID with additional information about swizzling etc. to an output stream.
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

    /*!\class   SimpleSwizzling
     * \brief   TODO
     * \details TODO
     */
    class SimpleSwizzling {
        friend class BufferPool;
    public:
        /*!\var     usesPointerSwizzling
         * \brief   TODO
         */
        static constexpr bool usesPointerSwizzling = true;

        /*!\fn      isSwizzledPointer(const PageID pid) noexcept
         * \brief   Checks whether a page ID is a swizzled buffer index
         * \details Whether the given page ID is actually swizzled and therefore a buffer pool index marked as such.
         *
         * @param pid A page ID that might be swizzled.
         * @return    Whether \c pid is a swizzled pointer.
         */
        static bool isSwizzledPointer(const PageID pid) noexcept {
            return (pid & swizzledPIDBit) != 0;
        };

        /*!\fn      makeSwizzledPointer(const bf_idx index) noexcept
         * \brief   Transforms a buffer pool index into a swizzled page ID
         * \details Transforms the given buffer pool index into a swizzled page ID by setting the flag
         *          \link swizzledPIDBit \endlink to mark it as a swizzled pointer.
         *
         * @param index A buffer pool index
         * @return      The swizzled page ID to be used inside the parent page
         */
        static PageID makeSwizzledPointer(const bf_idx index) noexcept {
            return index | swizzledPIDBit;
        };

        /*!\fn      makeBufferIndex(const PageID pid) noexcept
         * \brief   Transforms a swizzled page ID to a buffer pool index
         * \details Transforms the given swizzled page ID into a buffer pool index by unsetting the flag
         *          \link swizzledPIDBit \endlink that marked the page ID as a swizzled pointer.
         *
         * @param pid A swizzled page ID
         * @return    The buffer pool index where the page specified in \c pid can be found.
         */
        static bf_idx makeBufferIndex(const PageID pid) noexcept {
            return pid & ~swizzledPIDBit;
        };

        /*!\fn      debugDumpPointer(std::ostream& o, PageID pid)
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
        /*!\var   swizzledPIDBit
         * \brief Bit which is set in swizzled pointers to mark those
         */
        static constexpr PageID swizzledPIDBit = 0b10000000000000000000000000000000;

    };

} // zero::buffer_pool

#endif // __ZERO_BUFFER_POOL_POINTER_SWIZZLING_HPP
