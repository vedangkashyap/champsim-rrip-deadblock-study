# Dead Block Counter Design

> **Update note (see `06_percpu_deadblock_stats.md`):** the original design
> below used two *global* scalar counters (`dead_block_count`,
> `total_valid_evictions`). Once dual-core experiments began, it became clear
> a shared structure like the LLC needs a **per-core** breakdown too, since a
> single blended number can't distinguish which core's blocks were actually
> dying. The fields were renamed and extended accordingly:
>
> | Original name             | Current name                  |
> |----------------------------|-------------------------------|
> | `dead_block_count`         | `dead_block_count_global`     |
> | `total_valid_evictions`    | `total_valid_evictions_global`|
> | *(did not exist)*          | `dead_block_count_percpu`     |
> | *(did not exist)*          | `total_valid_evictions_percpu`|
>
> This document has been updated in place to reflect the current names.
> See `06_percpu_deadblock_stats.md` for the full rationale, the new
> `owner_cpu` field, and the exact diff.

## Purpose

This document explains the design of the dead block counter added to ChampSim.

The goal is to count cache blocks that are filled into the cache but evicted before receiving any hit. The implementation is designed to be policy-independent, meaning it should work with LRU, SRRIP, DRRIP, SHIP, or any other replacement policy without modifying each replacement policy separately.

## Dead Block Definition

In this project, a cache block is considered dead if:

```text
The block is inserted into the cache and later evicted without receiving any hit after insertion.
```

This means the block occupied space in the cache but did not provide reuse benefit before eviction.

## Why This Should Not Be Inside Replacement Policy Code

Replacement policies are responsible for choosing the victim block. For example:

* LRU chooses based on recency.
* SRRIP chooses based on re-reference prediction value.
* DRRIP dynamically chooses between insertion strategies.
* SHIP uses signature-based prediction.

Dead block counting is not a replacement decision. It is an observation about what happened to the victim block before eviction.

Therefore, the counter should be placed in the common cache logic, where all policies eventually pass through the same fill and eviction path.

## Metadata Added Per Cache Block

Each cache block needs two extra metadata fields:

```cpp
bool used_after_fill = false;
uint32_t owner_cpu = 0;
```

`used_after_fill` records whether the block received a hit after being filled. This field is unchanged from the original design.

`owner_cpu` was added later (see `06_percpu_deadblock_stats.md`) to record which core installed the block. It is needed so that a **shared** structure — the LLC, which serves multiple cores — can attribute an eviction to the correct core's counter. Private per-core structures (L1I, L1D, L2C, TLBs) don't strictly need it, but it's set uniformly for every fill for consistency.

Meaning:

```text
used_after_fill:
  false -> the block has been filled but has not been reused yet
  true  -> the block received at least one hit after fill

owner_cpu:
  the core index that triggered the fill which installed this block
```

## Fill Logic

When a new block is inserted into the cache, it should start as not reused, and should record which core installed it.

Code idea:

```cpp
to_fill.used_after_fill = false;
to_fill.owner_cpu = mshr.cpu;
```

This is added inside the cache block fill creation path (`CACHE::fill_block`).

## Hit Logic

When the cache access hits on a block, the block should be marked as reused.

Code idea:

```cpp
way->used_after_fill = true;
```

This must be inside the `if (hit)` block. It should not be done on misses, because on a miss the selected `way` is not a hit block.

## Eviction Logic

Before a valid victim block is overwritten, the cache checks whether it was ever reused, and counts the eviction both **globally** and against the **core that owned the block**.

Code idea:

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

The check must happen before this overwrite:

```cpp
*way = fill_block(fill_mshr, metadata_thru);
```

because that line overwrites the old victim block.

## Statistics Tracked

Four counters are tracked (two global scalars, two per-core event counters):

```cpp
uint64_t dead_block_count_global = 0;
uint64_t total_valid_evictions_global = 0;

champsim::stats::event_counter<std::remove_cv_t<decltype(NUM_CPUS)>> dead_block_count_percpu = {};
champsim::stats::event_counter<std::remove_cv_t<decltype(NUM_CPUS)>> total_valid_evictions_percpu = {};
```

Meaning:

```text
dead_block_count_global       -> number of valid evicted blocks that were
                                  never reused, across ALL cores combined
total_valid_evictions_global  -> number of valid blocks evicted,
                                  across ALL cores combined

dead_block_count_percpu       -> same idea, but keyed by core index.
                                  Meaningful for shared structures (the LLC),
                                  where different cores' blocks compete for
                                  the same space.
total_valid_evictions_percpu  -> same, per core.
```

The dead block percentage (global or per-core) is:

```text
dead_block_percentage = dead_block_count / total_valid_evictions * 100
```

The percentage is meaningful only when the corresponding `total_valid_evictions` value is greater than 0.

If no valid blocks were evicted, the output uses `-` instead of a percentage. In that case, the result does not mean there were no dead blocks in the cache. It only means no valid block reached eviction during the measured region.

## Output Format

The plain-text ChampSim output includes **one global line per structure**, plus **one per-core line per structure per core**:

```text
GLOBAL LINE:
<structure> DEADBLOCK GLOBAL: <n> VALID EVICTIONS GLOBAL: <n> DEAD BLOCK % GLOBAL: <pct>

PER-CORE LINE (one per core):
cpu<N>-><structure> DEAD BLOCKS (CORE): <n> VALID EVICTIONS (CORE): <n> DEAD BLOCK % (CORE): <pct>
```

Example output (private structure — L1D — both numbers happen to match since only one core ever touches it):

```text
cpu0->cpu0_L1D DEAD BLOCKS (CORE):    1356866 VALID EVICTIONS (CORE):    2307601 DEAD BLOCK % (CORE): 58.8
cpu0_L1D DEADBLOCK GLOBAL:    1356866 VALID EVICTIONS GLOBAL:    2307601 DEAD BLOCK % GLOBAL: 58.8
```

Example output (shared structure — LLC — per-core numbers genuinely differ, and sum to the global line):

```text
cpu0->LLC DEAD BLOCKS (CORE):     728154 VALID EVICTIONS (CORE):    1039113 DEAD BLOCK % (CORE): 70.07
cpu1->LLC DEAD BLOCKS (CORE):    1756415 VALID EVICTIONS (CORE):    1925848 DEAD BLOCK % (CORE): 91.2
LLC DEADBLOCK GLOBAL:    2484569 VALID EVICTIONS GLOBAL:    2964961 DEAD BLOCK % GLOBAL: 83.8
```

Sanity check: `728154 + 1756415 = 2484569` and `1039113 + 1925848 = 2964961` — per-core numbers always sum exactly to the global line, since every eviction is counted in exactly one place globally and attributed to exactly one core.

## Important Commands Used During Verification

```bash
grep -R "dead_block_count\|total_valid_evictions\|used_after_fill\|owner_cpu" -n inc src
```

Checks that all dead block counter fields and logic are present in the expected source files.

```bash
grep -n "DEAD BLOCKS (CORE)\|DEADBLOCK GLOBAL" run_soplex_deadblock.log
```

Extracts the dead block statistic lines from a simulation output log.

## Expected Source Locations

The implementation affects these files:

```text
inc/block.h
inc/cache_stats.h
src/cache_stats.cc
src/cache.cc
src/plain_printer.cc
```

The replacement policy files (SRRIP, DRRIP, DAAIP, etc.) do not need dead block counter code — the counter is entirely policy-independent, as designed.