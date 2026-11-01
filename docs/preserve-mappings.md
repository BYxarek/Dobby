# Android near-allocation fix

Based on LSPosed/Dobby `c1da0315d7a2069bde2ab432e45d4ab6df91237a`,
the upstream master checked on 2026-11-01.

POSIX `OSMemory::Allocate` must not replace occupied mappings, including ART's
PROT_NONE reservations. Use mmap's address hint without MAP_FIXED; reject and
unmap a newly returned mapping if its address differs from the requested one.
This also handles another thread occupying a candidate after the maps scan.

The old Dobby 1.2 NearMemoryArena patch is not applied: upstream has replaced
that allocator. AliuHook must use the new function-pointer DobbyHook API.

## Regression

Build a static Dobby with Android NDK 30.0.16248370, CMake 4.1.2,
`-DDOBBY_GENERATE_SHARED=OFF -DCMAKE_POLICY_VERSION_MINIMUM=3.5`.
Then compile the regression against that archive (assertions enabled):

```powershell
$clang = "$env:LOCALAPPDATA/Android/Sdk/ndk/30.0.16248370/toolchains/llvm/prebuilt/windows-x86_64/bin/clang++.exe"
& $clang --target=x86_64-linux-android21 -std=c++17 -UNDEBUG -static-libstdc++ `
  -Isource/Backend/UserMode -Isource -Isource/include -Iexternal -Iexternal/logging `
  tests/test_memory_allocation.cpp build/latest-x86_64/libdobby.a -llog `
  -o build/test_memory_allocation_latest
adb -s emulator-5554 push build/test_memory_allocation_latest /data/local/tmp/dobby-latest-memory-probe
adb -s emulator-5554 shell chmod 755 /data/local/tmp/dobby-latest-memory-probe
adb -s emulator-5554 shell /data/local/tmp/dobby-latest-memory-probe
```

Passed on 2026-11-01: x86_64 API 36 (4 KiB), API 35 and 37 (16 KiB).
Checks: occupied RW and PROT_NONE allocations fail, sentinel bytes survive,
and unrestricted allocation/free succeeds.
