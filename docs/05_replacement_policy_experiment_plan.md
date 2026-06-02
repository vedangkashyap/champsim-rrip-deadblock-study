# Replacement Policy Experiment Plan

## Goal

Compare dead block behavior across different cache replacement policies using the same trace and same simulation length.

## Trace
Warmup instructions: 1,000,000
Simulation instructions: 1,000,000

Metrics to Compare

For each policy, record:

DEAD BLOCKS
VALID EVICTIONS
DEAD BLOCK %
LLC misses
L2C misses
L1D misses

Policies to Test

Initial policies:

LRU
SRRIP
DRRIP
SHIP

```text
450.soplex-92B.champsimtrace.xz
