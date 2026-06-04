# Dead Block Counter Design

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

Each cache block needs one extra metadata bit:

```cpp
bool used_after_fill = false;
```

This bit records whether the block received a hit after being filled.

Meaning:

```text
false -> the block has been filled but has not been reused yet
true  -> the block received at least one hit after fill
```

## Fill Logic

When a new block is inserted into the cache, it should start as not reused.

Code idea:

```cpp
to_fill.used_after_fill = false;
```

This is added inside the block fill creation path.

## Hit Logic

When the cache access hits on a block, the block should be marked as reused.

Code idea:

```cpp
way->used_after_fill = true;
```

This must be inside the `if (hit)` block. It should not be done on misses, because on a miss the selected `way` is not a hit block.

## Eviction Logic

Before a valid victim block is overwritten, the cache should check whether it was ever reused.

Code idea:

```cpp
if (way->valid) {
  ++sim_stats.total_valid_evictions;

  if (!way->used_after_fill) {
    ++sim_stats.dead_block_count;
  }
}
```

The check must happen before this overwrite:

```cpp
*way = fill_block(fill_mshr, metadata_thru);
```

After the overwrite, the old victim block information is lost.

## Statistics Tracked

Two counters are added:

```cpp
uint64_t dead_block_count = 0;
uint64_t total_valid_evictions = 0;
```

Meaning:

```text
dead_block_count       -> number of valid evicted blocks that were never reused
total_valid_evictions  -> number of valid blocks evicted
```

The dead block percentage is:

```text
dead_block_percentage = dead_block_count / total_valid_evictions * 100

The percentage is meaningful only when `total_valid_evictions > 0`.

If no valid blocks were evicted, the output should use `-` instead of a percentage. In that case, the result does not mean there were no dead blocks in the cache. It only means no valid block reached eviction during the measured region.

```

## Output Format

The plain-text ChampSim output should include:

```text
DEAD BLOCKS
VALID EVICTIONS
DEAD BLOCK %
```

Example output format:

```text
cpu0->LLC DEAD BLOCKS: 19800 VALID EVICTIONS: 29184 DEAD BLOCK %: 67.85
```

## Important Commands Used During Verification

```bash
grep -R "dead_block_count\|total_valid_evictions\|used_after_fill" -n inc src
```

Checks that all dead block counter fields and logic are present in the expected source files.

```bash
grep -n "DEAD BLOCKS\|VALID EVICTIONS\|DEAD BLOCK %" run_soplex_deadblock.log
```

Extracts the dead block statistic lines from a simulation output log.

## Expected Source Locations

The implementation is expected to affect these files:

```text
inc/block.h
inc/cache_stats.h
src/cache_stats.cc
src/cache.cc
src/plain_printer.cc
```

The replacement policy files should not need dead block counter code.

