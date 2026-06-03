# Dead Block Counter Implementation Log

## Purpose

This document records the actual implementation changes made to add a policy-independent dead block counter in ChampSim.

The implementation follows the design rule that dead block tracking should be done in common cache logic, not inside individual replacement policies.

## Implementation Summary

The dead block counter was implemented using one per-block metadata field and two cache statistics counters.

The basic logic is:

```text
On fill:      mark block as not reused
On hit:       mark block as reused
On eviction:  if valid block was never reused, count it as dead
```

## Files Modified

The implementation affects the following source files:

```text
inc/block.h
inc/cache_stats.h
src/cache_stats.cc
src/cache.cc
src/plain_printer.cc
```

No replacement policy file was modified for dead block counting.

## 1. Per-Block Metadata

File modified:

```text
inc/block.h
```

Added field:

```cpp
bool used_after_fill = false;
```

Purpose:

This field records whether a cache block received a hit after it was inserted into the cache.

Meaning:

```text
false -> block was filled but has not been reused yet
true  -> block received at least one hit after fill
```

## 2. Cache Statistics Fields

File modified:

```text
inc/cache_stats.h
```

Added fields:

```cpp
uint64_t dead_block_count = 0;
uint64_t total_valid_evictions = 0;
```

Purpose:

`dead_block_count` stores how many valid evicted blocks were never reused after fill.

`total_valid_evictions` stores how many valid cache blocks were evicted in total.

## 3. Statistics Subtraction

File modified:

```text
src/cache_stats.cc
```

Added subtraction support for:

```cpp
result.dead_block_count = lhs.dead_block_count - rhs.dead_block_count;
result.total_valid_evictions = lhs.total_valid_evictions - rhs.total_valid_evictions;
```

Purpose:

ChampSim uses statistics subtraction to separate phases such as warmup and region-of-interest execution. These lines make sure the new dead block counters behave like the existing statistics.

## 4. Fill Logic

File modified:

```text
src/cache.cc
```

Added inside the cache block fill path:

```cpp
to_fill.used_after_fill = false;
```

Purpose:

Every newly inserted block starts as not reused. It should only become reused if it later receives a hit.

## 5. Eviction Counting Logic

File modified:

```text
src/cache.cc
```

Added before the victim block is overwritten:

```cpp
if (way->valid) {
  ++sim_stats.total_valid_evictions;

  if (!way->used_after_fill) {
    ++sim_stats.dead_block_count;
  }
}
```

Purpose:

If the victim block is valid, it is counted as a valid eviction. If it was never reused after fill, it is counted as a dead block.

This logic must execute before:

```cpp
*way = fill_block(fill_mshr, metadata_thru);
```

because that line overwrites the old victim block.

## 6. Hit Logic

File modified:

```text
src/cache.cc
```

Added inside the cache hit block:

```cpp
way->used_after_fill = true;
```

Purpose:

When a block receives a hit, it is marked as reused. Such a block should not be counted as dead when it is later evicted.

## 7. ROI Statistics Copy

File modified:

```text
src/cache.cc
```

Added:

```cpp
roi_stats.dead_block_count = sim_stats.dead_block_count;
roi_stats.total_valid_evictions = sim_stats.total_valid_evictions;
```

Purpose:

These lines copy the new dead block counters into ROI statistics, matching how other cache statistics are handled.

## 8. Output Printing

File modified:

```text
src/plain_printer.cc
```

Added printing for:

```text
DEAD BLOCKS
VALID EVICTIONS
DEAD BLOCK %
```

Example output format:

```text
cpu0->LLC DEAD BLOCKS: 19800 VALID EVICTIONS: 29184 DEAD BLOCK %: 67.85
```

Purpose:

This makes the new counter visible in normal ChampSim output.

## Important Verification Commands

```bash
grep -R "dead_block_count\|total_valid_evictions\|used_after_fill" -n inc src
```

Checks that the dead block fields and logic appear in the expected source files.

```bash
make -j1
```

Builds ChampSim using one thread, making compiler output easier to inspect.

```bash
ls -lh bin/champsim
```

Checks that the ChampSim binary was successfully generated.

```bash
grep -n "DEAD BLOCKS\|VALID EVICTIONS\|DEAD BLOCK %" run_soplex_deadblock.log
```

Extracts the dead block statistic lines from the simulation output log.

## Build Status

The modified ChampSim binary builds successfully.

The build produced a warning in `src/plain_printer.cc` about converting `uint64_t` to `double` in the percentage calculation, but the binary was generated successfully.

## Validation Status

The dead block counter was validated using the `450.soplex-92B.champsimtrace.xz` trace.

The output successfully showed dead block statistics for multiple structures, including L2C, L1D, DTLB, STLB, and LLC.

