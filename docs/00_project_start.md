# ChampSim RRIP / Dead Block Counter Study

## Goal

This repository documents my study of ChampSim, cache replacement policies, RRIP, and policy-independent dead block counting.

## Current Plan

1. Build a clean ChampSim setup.
2. Understand the cache access and fill flow.
3. Study LRU, SRRIP, BRRIP, and DRRIP.
4. Implement a dead block counter at cache level.
5. Compare dead block statistics across replacement policies.

## Important Rule

Dead block counting should be implemented in common cache logic, not inside individual replacement policy files.
