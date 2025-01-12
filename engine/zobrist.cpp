#include "zobrist.h"
#include <random>
#include <cstdint>

uint64_t Zobrist::secureRandomUnsigned64() {
    std::random_device rd; // Non-deterministic random number generator
    uint64_t random_number = (static_cast<uint64_t>(rd()) << 32) | rd();
    return random_number;
}
