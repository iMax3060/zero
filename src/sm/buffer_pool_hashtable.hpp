#ifndef __SM_BUFFER_POOL_HASHTABLE_HPP
#define __SM_BUFFER_POOL_HASHTABLE_HPP

#include "basics.h"
#include <cmath>
#include <limits>
#include <memory>
#include "junction/junction/ConcurrentMap_Leapfrog.h"

namespace zero::buffer_pool {

    class Hashtable {
    public:
        Hashtable(bf_idx block_count) :
                _hashtable(std::make_unique<
                        junction::ConcurrentMap_Leapfrog<PageID, atomic_bf_idx_pair*, HashtableKeyTraits>>(
                        static_cast<size_t>(std::pow(2, std::ceil(std::log2(block_count)))))) {};

        ~Hashtable() {};

        void erase(const PageID& pid) {
            delete (_hashtable->erase(pid));
        };

        void erase(PageID&& pid) {
            delete (_hashtable->erase(pid));
        };

        atomic_bf_idx_pair* lookupPair(const PageID& pid) const {
            return _hashtable->get(pid);
        };

        atomic_bf_idx_pair* lookupPair(PageID&& pid) const {
            return _hashtable->get(pid);
        };

        atomic_bf_idx* lookup(const PageID& pid) const {
            atomic_bf_idx_pair* indexPair = _hashtable->get(pid);
            if (indexPair) {
                return &(indexPair->first);
            } else {
                return nullptr;
            }
        };

        atomic_bf_idx* lookup(PageID&& pid) const {
            atomic_bf_idx_pair* indexPair = _hashtable->get(pid);
            if (indexPair) {
                return &(indexPair->first);
            } else {
                return nullptr;
            }
        };

        atomic_bf_idx* lookupParent(const PageID& pid) const {
            atomic_bf_idx_pair* indexPair = _hashtable->get(pid);
            if (indexPair) {
                return &(indexPair->second);
            } else {
                return nullptr;
            }
        };

        atomic_bf_idx* lookupParent(PageID&& pid) const {
            atomic_bf_idx_pair* indexPair = _hashtable->get(pid);
            if (indexPair) {
                return &(indexPair->second);
            } else {
                return nullptr;
            }
        };

        bool tryInsert(const PageID& pid, atomic_bf_idx_pair* idx_pair) {
            bool inserted = false;
            auto mutator = _hashtable->insertOrFind(pid);
            atomic_bf_idx_pair* value = mutator.getValue();
            if (!value) {
                delete (mutator.exchangeValue(idx_pair));
                inserted = true;
            }
            return inserted;
        }

        bool tryInsert(PageID&& pid, atomic_bf_idx_pair* indexPair) {
            bool inserted = false;
            auto mutator = _hashtable->insertOrFind(pid);
            atomic_bf_idx_pair* value = mutator.getValue();
            if (!value) {
                delete (mutator.exchangeValue(indexPair));
                inserted = true;
            }
            return inserted;
        }

    private:
        struct HashtableKeyTraits {
            typedef PageID Key;

            typedef typename turf::util::BestFit<PageID>::Unsigned Hash;

            static const Key NullKey = Key(4294967295);

            static const Hash NullHash = Hash(2180083513);

            static Hash hash(PageID key) {
                return turf::util::avalanche(Hash(key));
            }

            static Key dehash(Hash hash) {
                return (PageID)turf::util::deavalanche(hash);
            }
        };

        std::unique_ptr<junction::ConcurrentMap_Leapfrog<PageID, atomic_bf_idx_pair*, HashtableKeyTraits>> _hashtable;
    };
}

#endif // __SM_BUFFER_POOL_HASHTABLE_HPP
