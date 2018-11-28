#ifndef __ZERO_BF_TREE_HASHTABLE_HPP
#define __ZERO_BF_TREE_HASHTABLE_HPP

#include "basics.h"
#include <limits>
#include <memory>
#include "junction/junction/ConcurrentMap_Leapfrog.h"

namespace zero::buffer_pool {

    class Hashtable {
    public:
        Hashtable(bf_idx block_count) :
                _hashtable(std::make_unique<junction::ConcurrentMap_Leapfrog<PageID, bf_idx_pair*, HashtableKeyTraits>>(block_count)) {};

        ~Hashtable() {};

        void erase(const PageID& pid) {
            delete(_hashtable->erase(pid));
        };

        void erase(PageID&& pid) {
            delete(_hashtable->erase(pid));
        };

        bf_idx_pair* lookupPair(const PageID& pid) const {
            return _hashtable->get(pid);
        };

        bf_idx_pair* lookupPair(PageID&& pid) const {
            return _hashtable->get(pid);
        };

        bf_idx* lookup(const PageID& pid) const {
            bf_idx_pair* idx_pair = _hashtable->get(pid);
            if (idx_pair) {
                return &(idx_pair->first);
            } else {
                return nullptr;
            }
        };

        bf_idx* lookup(PageID&& pid) const {
            bf_idx_pair* idx_pair = _hashtable->get(pid);
            if (idx_pair) {
                return &(idx_pair->first);
            } else {
                return nullptr;
            }
        };

        bf_idx* lookupParent(const PageID& pid) const {
            bf_idx_pair* idx_pair = _hashtable->get(pid);
            if (idx_pair) {
                return &(idx_pair->second);
            } else {
                return nullptr;
            }
        };

        bf_idx* lookupParent(PageID&& pid) const {
            bf_idx_pair* idx_pair = _hashtable->get(pid);
            if (idx_pair) {
                return &(idx_pair->second);
            } else {
                return nullptr;
            }
        };

        bool tryInsert(const PageID& pid, bf_idx_pair* idx_pair) {
            bool inserted = false;
            auto mutator = _hashtable->insertOrFind(pid);
            bf_idx_pair* value = mutator.getValue();
            if (!value) {
                delete(mutator.exchangeValue(idx_pair));
                inserted = true;
            }
            return inserted;
        }

        bool tryInsert(PageID&& pid, bf_idx_pair* idx_pair) {
            bool inserted = false;
            auto mutator = _hashtable->insertOrFind(pid);
            bf_idx_pair* value = mutator.getValue();
            if (!value) {
                delete(mutator.exchangeValue(idx_pair));
                inserted = true;
            }
            return inserted;
        }

    private:
        struct HashtableKeyTraits {
            typedef PageID Key;
            typedef typename turf::util::BestFit<PageID>::Unsigned Hash;
            static const Key NullKey = Key(std::numeric_limits<PageID>::max());
            static const Hash NullHash = Hash(std::numeric_limits<PageID>::max());
            static Hash hash(PageID key) {
                return turf::util::avalanche(Hash(key));
            }
            static Key dehash(Hash hash) {
                return (PageID) turf::util::deavalanche(hash);
            }
        };

        std::unique_ptr<junction::ConcurrentMap_Leapfrog<PageID, bf_idx_pair*, HashtableKeyTraits>> _hashtable;
    };

}

#endif // __ZERO_BF_TREE_HASHTABLE_HPP
