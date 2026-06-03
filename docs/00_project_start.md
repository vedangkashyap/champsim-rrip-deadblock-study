# ChampSim RRIP / Dead Block Counter Study

## Overview

This repository documents my work on understanding ChampSim, cache replacement policies, RRIP-style policies, and dead block behavior in caches. The main goal is to study how different replacement policies affect cache blocks that are inserted into the cache but never reused before eviction.

The project starts from a clean ChampSim setup and gradually adds a policy-independent dead block counter. The counter is implemented in common cache logic, not inside individual replacement policy files. This makes the same dead block counter usable with LRU, SRRIP, DRRIP, SHIP, or any other replacement policy.

## Motivation

A cache block is useful only if it is reused before it is evicted. If a block is brought into the cache and later evicted without receiving any hit, it occupied cache space without giving reuse benefit. In this project, such a block is treated as a dead block.

Counting dead blocks helps answer questions such as:

* How many cache blocks are evicted without reuse?
* Which cache levels show high dead block percentages?
* Does changing the replacement policy reduce or increase dead blocks?
* Can dead block behavior be measured without modifying every replacement policy separately?

## Main Technical Goal

The main technical goal is to implement a dead block counter inside ChampSim’s common cache logic.

The counter tracks:

* total valid evictions
* dead block evictions
* dead block percentage

The counter should work across replacement policies without being implemented separately inside each replacement policy.

## Important Design Rule

Dead block counting must be implemented in common cache code, not inside LRU, SRRIP, DRRIP, SHIP, or any individual replacement policy file.

Replacement policies should only choose which block is evicted. The common cache logic should observe the victim block and decide whether it was dead.

## Documentation Style

This repository documents both the code changes and the important commands used during the project. Routine Git commands are not explained in detail, but important build, debugging, configuration, and experiment commands are recorded with a short explanation of what each command does.

Example:

```bash
rm -rf .csconfig
```

Removes ChampSim’s generated configuration/build folder so the next build starts from a clean generated state.

```bash
./config.sh champsim_config.json
```

Generates ChampSim build files using the selected configuration file.

```bash
make -j1
```

Builds ChampSim using one thread, which makes compiler errors easier to read in order.

```bash
grep -n "DEAD BLOCKS\|VALID EVICTIONS\|DEAD BLOCK %" run_soplex_lru.log
```

Extracts only the dead block statistic lines from a full simulation output log.

## Planned Workflow

1. Set up and build ChampSim cleanly.
2. Understand the cache fill, hit, and eviction flow.
3. Add per-block reuse tracking metadata.
4. Add dead block statistics.
5. Print dead block statistics in ChampSim output.
6. Validate the implementation using a trace.
7. Compare dead block behavior between LRU and SRRIP first.
8. Extend the comparison to other policies such as DRRIP and SHIP later.

## Current Status

The dead block counter has been implemented and validated using the `450.soplex` trace. The current experiment stage is comparing LRU and SRRIP using the same trace and simulation length.

