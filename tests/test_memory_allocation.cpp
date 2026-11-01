#include "UnifiedInterface/platform.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <sys/mman.h>

// An occupied address hint must preserve its mapping; build with assertions enabled.
int main() {
    const int size = OSMemory::PageSize();
    void* occupied = mmap(nullptr, size, PROT_READ | PROT_WRITE,
                          MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    assert(occupied != MAP_FAILED);
    std::memset(occupied, 0x5a, size);
    assert(OSMemory::Allocate(size, kReadWrite, occupied) == nullptr);
    for (int i = 0; i < size; ++i)
        assert(static_cast<unsigned char*>(occupied)[i] == 0x5a);
    assert(mprotect(occupied, size, PROT_NONE) == 0);
    assert(OSMemory::Allocate(size, kReadWrite, occupied) == nullptr);
    assert(mprotect(occupied, size, PROT_READ) == 0);
    for (int i = 0; i < size; ++i)
        assert(static_cast<unsigned char*>(occupied)[i] == 0x5a);
    assert(munmap(occupied, size) == 0);
    void* allocated = OSMemory::Allocate(size, kReadWrite);
    assert(allocated != nullptr);
    assert(OSMemory::Free(allocated, size));
    std::printf("PASS: occupied RW/PROT_NONE hints preserve mappings, null allocation/free; page=%d\n", size);
}
