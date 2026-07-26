# Per-Core LLC Dead-Block Stats

## Purpose

This document explains **why** the original dead-block counter (global-only)
was extended to also track dead blocks **per core**, and walks through the
exact change. It's the canonical reference that `02_dead_block_counter_design.md`
and `03_implementation_log.md` now point to.

## The problem this fixes

The original dead-block counter (see `02_dead_block_counter_design.md`) added
two global scalars — `dead_block_count`, `total_valid_evictions` — that live
inside each cache level's `cache_stats` struct. That's correct and sufficient
for **private** structures: L1I, L1D, L2C, and the TLBs each get their own
independent `CACHE` object per core, so a "global" number for one of those is
already, implicitly, a per-core number — there's only one core touching it.

The LLC is different. In this project's dual-core configuration, the LLC is a
**single, shared** `CACHE` object that both cores' misses funnel into (see
`08_dual_core_setup.md` for how that's confirmed). That means there is only
**one** `cache_stats` instance for the whole LLC — and the original global
counters blend both cores' dead-block activity into one number.

This became a real problem once DAAIP was implemented and needed evaluating.
DAAIP's entire premise (see `07_daaip_design.md`) is that a cache-friendly
application and a streaming application, sharing the same LLC, should be
protected differently — but the original stats couldn't show whether that was
actually happening, because both cores' dead blocks were counted together.
Concretely, before this change, the plain-text output showed the exact same
number twice:

```text
cpu0->LLC DEAD BLOCKS:    2484569 VALID EVICTIONS:    2964961 DEAD BLOCK %: 83.8
cpu1->LLC DEAD BLOCKS:    2484569 VALID EVICTIONS:    2964961 DEAD BLOCK %: 83.8
```

Same numbers on both lines — the printer was looping over each core and
printing the same global field every time, because that field had no
per-core dimension to draw from.

## The fix

### 1. Attribute each block to the core that installed it

`inc/block.h` gained a new field:

```cpp
uint32_t owner_cpu = 0;
```

Set once, at fill time, in `CACHE::fill_block` (`src/cache.cc`):

```cpp
to_fill.owner_cpu = mshr.cpu;
```

This is the only place the field is written. It answers: *which core's miss
caused this exact block to be installed?*

### 2. Split the stats into global + per-core

`inc/cache_stats.h` was changed from two scalars to two scalars (renamed) plus
two new per-core counters:

```cpp
// global — cache-wide, blends all cores together
uint64_t dead_block_count_global = 0;
uint64_t total_valid_evictions_global = 0;

// per-core — meaningful for shared structures like the LLC
champsim::stats::event_counter<std::remove_cv_t<decltype(NUM_CPUS)>> dead_block_count_percpu = {};
champsim::stats::event_counter<std::remove_cv_t<decltype(NUM_CPUS)>> total_valid_evictions_percpu = {};
```

The per-core fields use `champsim::stats::event_counter<Key>` — the exact
same class this codebase already uses for `hits`/`misses`/`mshr_merge`/
`mshr_return`, just keyed by core index instead of `(access_type, core)`.
This wasn't a new mechanism invented for this change; it's the existing house
convention for "a stat that needs a per-core breakdown," applied to a stat
that previously didn't have one.

### 3. Count both, at eviction time

`src/cache.cc`, in the eviction path (runs just before the victim block is
overwritten):

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

`way->owner_cpu` — set back when this block was filled — tells us which
core's counter to increment. The global counters are still updated exactly as
before; nothing about the global tracking changed, it only gained a sibling.

### 4. Propagate through subtraction, ROI copy, and printing

- `src/cache_stats.cc` — `operator-` extended to subtract the two new
  `event_counter` fields (they already support `operator-`, matching how
  `hits`/`misses` are subtracted just above).
- `src/cache.cc` — the warmup→ROI stat copy extended to copy both new fields.
- `src/plain_printer.cc` — prints one **global** line per structure (using
  `_global`), and one **per-core** line per core per structure (using
  `_percpu.value_or(cpu, 0L)`).

## What the output looks like now

Private structure (L1D) — per-core and global numbers match, since only one
core ever touches it. This is expected, not a bug:

```text
cpu0->cpu0_L1D DEAD BLOCKS (CORE):    1356866 VALID EVICTIONS (CORE):    2307601 DEAD BLOCK % (CORE): 58.8
cpu0_L1D DEADBLOCK GLOBAL:    1356866 VALID EVICTIONS GLOBAL:    2307601 DEAD BLOCK % GLOBAL: 58.8
```

Shared structure (LLC) — per-core numbers now genuinely differ:

```text
cpu0->LLC DEAD BLOCKS (CORE):     728154 VALID EVICTIONS (CORE):    1039113 DEAD BLOCK % (CORE): 70.07
cpu1->LLC DEAD BLOCKS (CORE):    1756415 VALID EVICTIONS (CORE):    1925848 DEAD BLOCK % (CORE): 91.2
LLC DEADBLOCK GLOBAL:    2484569 VALID EVICTIONS GLOBAL:    2964961 DEAD BLOCK % GLOBAL: 83.8
```

## Sanity check: does it add up?

Per-core numbers should always sum exactly to the global line, since every
eviction is counted in exactly one core's bucket and also in the global
total:

```text
728154 + 1756415  = 2484569   (matches DEADBLOCK GLOBAL)
1039113 + 1925848 = 2964961   (matches VALID EVICTIONS GLOBAL)
```

This held exactly in every run performed during this project — confirming no
eviction was double-counted or dropped by the per-core split.

## Files changed

```text
inc/block.h            — added owner_cpu
inc/cache_stats.h       — renamed dead_block_count/total_valid_evictions to
                          *_global, added dead_block_count_percpu /
                          total_valid_evictions_percpu
src/cache_stats.cc      — extended operator- for the new/renamed fields
src/cache.cc            — set owner_cpu at fill, count global+percpu at
                          eviction, copy both to roi_stats
src/plain_printer.cc    — print one GLOBAL line + one (CORE) line per core
```

No replacement policy file needed any changes for this — same
policy-independence principle as the original dead-block counter design.

## Why this mattered for evaluating DAAIP

With this change in place, DAAIP's effect could be measured directly at the
mechanism level, not just inferred from aggregate IPC. See
`09_results_bzip2_libquantum.md` for the actual comparison this enabled:
under SRRIP, the cache-friendly application's LLC dead-block rate was
indistinguishable from the streaming application's (99.89% vs 99.94%) — under
DAAIP, the cache-friendly application's rate dropped to 70.07%, while the
streaming application's stayed high (91.2%), exactly matching what the per-core
split was built to reveal.