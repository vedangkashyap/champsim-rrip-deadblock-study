# Dead Block Counter Implementation Log

> **Update note (see `06_percpu_deadblock_stats.md`):** this log originally
> described the fields as `dead_block_count` / `total_valid_evictions`
> (global scalars only). Those were later renamed to `*_global` and paired
> with new `*_percpu` event counters, plus a new `owner_cpu` field on
> `cache_block`. The sections below have been updated in place to reflect
> the current code — search for "**[UPDATED]**" to find every changed
> section quickly.

## Purpose

This document records the actual implementation changes made to add a policy-independent dead block counter in ChampSim.

The implementation follows the design rule that dead block tracking should be done in common cache logic, not inside individual replacement policies.

## Implementation Summary

The dead block counter was implemented using per-block metadata fields and cache statistics counters.

The basic logic is:

```text
On fill:      mark block as not reused, and record which core installed it
On hit:       mark block as reused
On eviction:  if valid block was never reused, count it as dead
              (both globally, and against the core that installed it)
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

## 1. Per-Block Metadata **[UPDATED]**

File modified:

```text
inc/block.h
```

Added fields:

```cpp
bool used_after_fill = false;
uint32_t owner_cpu = 0;
```

Purpose:

`used_after_fill` records whether a cache block received a hit after it was inserted into the cache. This was part of the original design.

`owner_cpu` was added later, once dual-core experiments began. It records which core's fill request installed this block. It is required so that a **shared** structure (the LLC, accessed by multiple cores) can attribute an eviction to the correct core when computing per-core dead-block stats — see the "Statistics Fields" section below.

Meaning:

```text
used_after_fill:
  false -> block was filled but has not been reused yet
  true  -> block received at least one hit after fill

owner_cpu:
  the core index whose fill request installed this block
```

## 2. Cache Statistics Fields **[UPDATED]**

File modified:

```text
inc/cache_stats.h
```

Original fields (now renamed):

```cpp
uint64_t dead_block_count = 0;          // renamed -> dead_block_count_global
uint64_t total_valid_evictions = 0;     // renamed -> total_valid_evictions_global
```

Current fields:

```cpp
uint64_t dead_block_count_global = 0;
uint64_t total_valid_evictions_global = 0;

champsim::stats::event_counter<std::remove_cv_t<decltype(NUM_CPUS)>> dead_block_count_percpu = {};
champsim::stats::event_counter<std::remove_cv_t<decltype(NUM_CPUS)>> total_valid_evictions_percpu = {};
```

Purpose:

`dead_block_count_global` / `total_valid_evictions_global` store the same cache-wide totals the original design tracked, just renamed for clarity now that a per-core version also exists.

`dead_block_count_percpu` / `total_valid_evictions_percpu` are new. They use the same `event_counter` pattern already used by `hits`/`misses`/`mshr_merge`/`mshr_return` in this struct, keyed by core index. This was necessary because a **shared** structure like the LLC previously reported the exact same blended number for every core — there was no way to tell, from the stats alone, whether one core's blocks were dying more than another's while sharing the same cache.

## 3. Statistics Subtraction **[UPDATED]**

File modified:

```text
src/cache_stats.cc
```

Current subtraction logic:

```cpp
result.dead_block_count_global = lhs.dead_block_count_global - rhs.dead_block_count_global;
result.total_valid_evictions_global = lhs.total_valid_evictions_global - rhs.total_valid_evictions_global;
result.dead_block_count_percpu = lhs.dead_block_count_percpu - rhs.dead_block_count_percpu;
result.total_valid_evictions_percpu = lhs.total_valid_evictions_percpu - rhs.total_valid_evictions_percpu;
```

Purpose:

ChampSim uses statistics subtraction to separate phases such as warmup and region-of-interest execution. These lines make sure both the renamed global counters and the new per-core counters behave correctly under that subtraction. `event_counter` already implements `operator-`, so the per-core lines follow the same pattern as the existing `hits`/`misses` subtraction just above them.

## 4. Fill Logic **[UPDATED]**

File modified:

```text
src/cache.cc
```

Added inside the cache block fill path (`CACHE::fill_block`):

```cpp
to_fill.used_after_fill = false;
to_fill.owner_cpu = mshr.cpu;
```

Purpose:

Every newly inserted block starts as not reused (original design, unchanged). The new `owner_cpu` line records which core's fill request this is, so that a later eviction of this exact block can be attributed to the correct core.

## 5. Eviction Counting Logic **[UPDATED]**

File modified:

```text
src/cache.cc
```

Added before the victim block is overwritten:

```cpp
if (way->valid) {
  ++sim_stats.total_valid_evictions_global;
  sim_stats.total_valid_evictions_percpu.increment(way->owner_cpu);

  if (!way->used_after_fill) {
    ++sim_stats.dead_block_count_global;
    sim_stats.dead_block_count_percpu.increment(way->owner_cpu);
  }
}
```

Purpose:

If the victim block is valid, it is counted as a valid eviction — both in the global total (original design) and against the core recorded in `way->owner_cpu` (new). If it was never reused after fill, it is counted as a dead block in both places as well.

This logic must execute before:

```cpp
*way = fill_block(fill_mshr, metadata_thru);
```

because that line overwrites the old victim block, including its `owner_cpu`.

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

When a block receives a hit, it is marked as reused. Such a block should not be counted as dead when it is later evicted. Unchanged from the original design.

## 7. ROI Statistics Copy **[UPDATED]**

File modified:

```text
src/cache.cc
```

Added:

```cpp
roi_stats.dead_block_count_global = sim_stats.dead_block_count_global;
roi_stats.total_valid_evictions_global = sim_stats.total_valid_evictions_global;
roi_stats.dead_block_count_percpu = sim_stats.dead_block_count_percpu;
roi_stats.total_valid_evictions_percpu = sim_stats.total_valid_evictions_percpu;
```

Purpose:

These lines copy the dead block counters (both global and per-core) into ROI statistics, matching how other cache statistics are handled.

## 8. Output Printing **[UPDATED]**

File modified:

```text
src/plain_printer.cc
```

Added printing for:

```text
<structure> DEADBLOCK GLOBAL: ... VALID EVICTIONS GLOBAL: ... DEAD BLOCK % GLOBAL: ...
cpu<N>-><structure> DEAD BLOCKS (CORE): ... VALID EVICTIONS (CORE): ... DEAD BLOCK % (CORE): ...
```

One global line is printed once per structure. One per-core line is printed for each core that structure serves.

Example output (private structure, e.g. L1D — both lines show the same numbers, since only one core ever touches a private structure):

```text
cpu0->cpu0_L1D DEAD BLOCKS (CORE):    1356866 VALID EVICTIONS (CORE):    2307601 DEAD BLOCK % (CORE): 58.8
cpu0_L1D DEADBLOCK GLOBAL:    1356866 VALID EVICTIONS GLOBAL:    2307601 DEAD BLOCK % GLOBAL: 58.8
```

Example output (shared structure, the LLC — per-core numbers genuinely differ):

```text
cpu0->LLC DEAD BLOCKS (CORE):     728154 VALID EVICTIONS (CORE):    1039113 DEAD BLOCK % (CORE): 70.07
cpu1->LLC DEAD BLOCKS (CORE):    1756415 VALID EVICTIONS (CORE):    1925848 DEAD BLOCK % (CORE): 91.2
LLC DEADBLOCK GLOBAL:    2484569 VALID EVICTIONS GLOBAL:    2964961 DEAD BLOCK % GLOBAL: 83.8
```

Purpose:

This makes the new per-core counter visible in normal ChampSim output, without losing the original global view.

## Important Verification Commands

```bash
grep -R "dead_block_count\|total_valid_evictions\|used_after_fill\|owner_cpu" -n inc src
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
grep -n "DEAD BLOCKS (CORE)\|DEADBLOCK GLOBAL" run_soplex_deadblock.log
```

Extracts the dead block statistic lines from the simulation output log.

## Build Status

The modified ChampSim binary builds successfully.

Two `-Wconversion` warnings appear in `src/plain_printer.cc` (`uint64_t` to `double` conversion in the percentage calculations — one for the global line, one for the per-core line). These are pre-existing, harmless, and do not affect correctness at the magnitudes involved. They do not block the build.

## Validation Status

The dead block counter was first validated using the `450.soplex-92B.champsimtrace.xz` trace (single-core). The output successfully showed dead block statistics for multiple structures, including:

- STLB
- L2C
- L1D
- DTLB
- LLC

The implementation was later tested with separate LRU, SRRIP, and DRRIP binaries, confirming the counter works across replacement policies without adding dead block logic inside individual replacement policy files.

**[UPDATED]** Once dual-core support was added (see `08_dual_core_setup.md`), the per-core split was validated directly: for a shared structure (the LLC), the two per-core numbers summed exactly to the global number in every run, confirming no eviction was double-counted or missed. For private structures (L1D, L2C, TLBs), the per-core and global numbers were identical, as expected, since only one core ever touches a private structure.