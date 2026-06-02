# Dead Block Counter Design

## Goal

The goal is to count dead cache blocks in ChampSim in a policy-independent way.

A cache block is considered dead if it is inserted into the cache and then evicted without receiving any hit after insertion.

## Core Idea

Each cache block needs one extra metadata bit:
Important Design Rule

The dead block counter should be implemented in common cache logic, not inside LRU, SRRIP, DRRIP, or any individual replacement policy.

Replacement policy chooses the victim. Cache logic observes whether the victim was dead.

```cpp
used_after_fill
