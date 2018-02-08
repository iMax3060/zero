#ifndef __LOCAL_RANDOM_H
#define __LOCAL_RANDOM_H
#include <cstdint>
// thread-local-random. it's also deterministic.
struct tlr_t {
    tlr_t (uint64_t seed) : _seed(seed) {}
    void moveOn () {
        _seed = _seed * 0xD04C3175 + 0x53DA9022;
    }
    uint32_t nextInt32() {
        moveOn();
        return (_seed >> 32) ^ (_seed & 0xFFFFFFFF);
    }
    uint64_t _seed;
};
#endif // __LOCAL_RANDOM_H
