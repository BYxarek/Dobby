# Preserve existing memory mappings

This branch is based on `edb2af1216313cf6c0d6771be2b279c1db573faf`, the
Dobby revision used by `io.github.vvb2060.ndk:dobby:1.2` in BYgram.

`NearMemoryArena` used an inverted last-region bounds check and passed a stale
address to `OSMemory::Allocate`. With `MAP_FIXED`, that allocation could replace
an existing mapping, including an ART `PROT_NONE` reservation, causing a later
GC crash.

The fix selects the candidate after the last region, checks the upper bound,
and resets the candidate before scanning gaps. POSIX allocations no longer use
`MAP_FIXED`. If the kernel returns a different address, only that new mapping
is unmapped and the allocation fails. Existing mappings are preserved even if
another thread occupies the candidate between the maps scan and `mmap`.

## Regression check

From the repository root in PowerShell, using an installed Android NDK:

```powershell
New-Item -ItemType Directory -Force build | Out-Null
$clang = "$env:LOCALAPPDATA/Android/Sdk/ndk/30.0.16248370/toolchains/llvm/prebuilt/windows-x86_64/bin/clang++.exe"
& $clang --target=x86_64-linux-android21 -std=c++17 -UNDEBUG `
  -static-libstdc++ -ffunction-sections -fdata-sections '-Wl,--gc-sections' `
  -Isource/UserMode -Isource -Isource/include -Iexternal -Iexternal/logging `
  -include android/log.h tests/test_memory_allocation.cpp `
  source/UserMode/UnifiedInterface/platform-posix.cc external/logging/logging.c `
  -llog -o build/test_memory_allocation
adb -s emulator-5554 push build/test_memory_allocation /data/local/tmp/dobby-memory-allocation-probe
adb -s emulator-5554 shell chmod 755 /data/local/tmp/dobby-memory-allocation-probe
adb -s emulator-5554 shell /data/local/tmp/dobby-memory-allocation-probe
```

The forced Android log include works around the old logging source including
the NDK header inside a function. Assertions must remain enabled.

On 2026-11-01, this focused test compiled the actual POSIX implementation and
passed on x86_64 Android API 36 (4 KiB pages), API 35 and 37 (16 KiB pages).
It checks occupied RW and PROT_NONE mappings, sentinel bytes, and ordinary
allocation/free. This run did not rebuild the full library or BYgram APK.
