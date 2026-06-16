# CRC-2 bzip2 100M Comparison: LRU vs SRRIP vs DRRIP vs SHIP

## Purpose

This file records the final CRC-2 `bzip2_259B` experiment using LRU, SRRIP, and DRRIP.

The 100M simulation run was performed after the 50M run showed useful LLC valid evictions. The goal was to obtain a stronger and more stable comparison over a longer measured region.

## Experiment Setup

| Item                     | Value                                        |
| ------------------------ | -------------------------------------------- |
| Trace                    | bzip2_259B.trace.xz                          |
| Trace Source             | CRC-2 trace set                              |
| Local Path               | traces_crc2/bzip2_259B.trace.xz              |
| Compared Policies        | LRU, SRRIP, DRRIP                            |
| Warmup Instructions      | 10,000,000                                   |
| Simulation Instructions  | 100,000,000                                  |
| Main Dead Block Metric   | Dead Block %                                 |
| Main Performance Metrics | IPC, CPI, L1D Misses, L2C Misses, LLC Misses |

## Why This Run Was Used

The 100M run was used because:

```text
1M simulation was too short and produced zero LLC valid evictions.
50M simulation produced useful LLC valid evictions.
100M simulation gives a longer measured window and stronger final data.
```

The same binaries and trace were used for all policies.

## Binaries Used

| Policy | Binary             |
| ------ | ------------------ |
| LRU    | bin/champsim_lru   |
| SRRIP  | bin/champsim_srrip |
| DRRIP  | bin/champsim_drrip |

## Run Commands

```bash
./bin/champsim_lru --warmup-instructions 10000000 --simulation-instructions 100000000 traces_crc2/bzip2_259B.trace.xz | tee run_crc2_bzip2_lru_100m.log
```

Runs LRU on the CRC-2 `bzip2_259B` trace and saves the full output to a local log file.

```bash
./bin/champsim_srrip --warmup-instructions 10000000 --simulation-instructions 100000000 traces_crc2/bzip2_259B.trace.xz | tee run_crc2_bzip2_srrip_100m.log
```

Runs SRRIP on the same trace and saves the full output to a local log file.

```bash
./bin/champsim_drrip --warmup-instructions 10000000 --simulation-instructions 100000000 traces_crc2/bzip2_259B.trace.xz | tee run_crc2_bzip2_drrip_100m.log
```

Runs DRRIP on the same trace and saves the full output to a local log file.

## Extraction Commands

```bash
grep -n "DEAD BLOCKS\|VALID EVICTIONS\|DEAD BLOCK %" <log_file>
```

Extracts dead block statistics from the full ChampSim output.

```bash
grep "CPU 0 cumulative IPC" <log_file>
```

Extracts IPC, instruction count, and cycle count.

```bash
grep "cpu0->cpu0_L1D TOTAL\|cpu0->cpu0_L2C TOTAL\|cpu0->LLC TOTAL" <log_file>
```

Extracts L1D, L2C, and LLC access, hit, and miss statistics.

## LRU Results

### LRU Dead Block Results

| Structure | Dead Blocks | Valid Evictions | Dead Block % |
| --------- | ----------: | --------------: | -----------: |
| STLB      |           0 |               0 |            - |
| L2C       |     395,303 |         701,676 |        56.34 |
| L1I       |           0 |               0 |            - |
| L1D       |     583,887 |       1,008,197 |        57.91 |
| ITLB      |           0 |               0 |            - |
| DTLB      |     504,771 |         665,532 |        75.84 |
| LLC       |      85,461 |         289,804 |        29.49 |

### LRU Performance and Miss Results

| Metric       |       Value |
| ------------ | ----------: |
| IPC          |       1.139 |
| Instructions | 100,000,000 |
| Cycles       |  87,783,116 |
| CPI          |       0.878 |
| L1D Misses   |   2,378,477 |
| L2C Misses   |     701,676 |
| LLC Misses   |     305,683 |

CPI calculation:

```text
CPI = cycles / instructions
CPI = 87783116 / 100000000
CPI ≈ 0.878
```

## SRRIP Results

### SRRIP Dead Block Results

| Structure | Dead Blocks | Valid Evictions | Dead Block % |
| --------- | ----------: | --------------: | -----------: |
| STLB      |           0 |               0 |            - |
| L2C       |     395,097 |         701,578 |        56.32 |
| L1I       |           0 |               0 |            - |
| L1D       |     570,188 |       1,008,201 |        56.55 |
| ITLB      |           0 |               0 |            - |
| DTLB      |     505,043 |         665,708 |        75.87 |
| LLC       |     305,265 |         318,287 |        95.91 |

### SRRIP Performance and Miss Results

| Metric       |       Value |
| ------------ | ----------: |
| IPC          |       1.208 |
| Instructions | 100,000,000 |
| Cycles       |  82,802,516 |
| CPI          |       0.828 |
| L1D Misses   |   2,335,372 |
| L2C Misses   |     701,578 |
| LLC Misses   |     334,166 |

CPI calculation:

```text
CPI = cycles / instructions
CPI = 82802516 / 100000000
CPI ≈ 0.828
```

## DRRIP Results

### DRRIP Dead Block Results

| Structure | Dead Blocks | Valid Evictions | Dead Block % |
| --------- | ----------: | --------------: | -----------: |
| STLB      |           0 |               0 |            - |
| L2C       |     395,389 |         701,734 |        56.34 |
| L1I       |           0 |               0 |            - |
| L1D       |     569,674 |       1,008,192 |        56.50 |
| ITLB      |           0 |               0 |            - |
| DTLB      |     505,297 |         665,836 |        75.89 |
| LLC       |     305,632 |         320,215 |        95.45 |

### DRRIP Performance and Miss Results

| Metric       |       Value |
| ------------ | ----------: |
| IPC          |       1.200 |
| Instructions | 100,000,000 |
| Cycles       |  83,345,686 |
| CPI          |       0.833 |
| L1D Misses   |   2,333,669 |
| L2C Misses   |     701,734 |
| LLC Misses   |     336,094 |

CPI calculation:

```text
CPI = cycles / instructions
CPI = 83345686 / 100000000
CPI ≈ 0.833
```
## SHIP Results

### SHIP Dead Block Results

| Structure | Dead Blocks | Valid Evictions | Dead Block % |
|---|---:|---:|---:|
| STLB | 0 | 0 | - |
| L2C | 395,388 | 701,733 | 56.34 |
| L1I | 0 | 0 | - |
| L1D | 568,582 | 1,008,203 | 56.40 |
| ITLB | 0 | 0 | - |
| DTLB | 505,309 | 665,847 | 75.89 |
| LLC | 297,188 | 311,383 | 95.44 |

### SHIP Performance and Miss Results

| Metric | Value |
|---|---:|
| IPC | 1.213 |
| Instructions | 100,000,000 |
| Cycles | 82,422,901 |
| CPI | 0.824 |
| L1D Misses | 2,330,601 |
| L2C Misses | 701,733 |
| LLC Misses | 327,262 |

CPI calculation:

```text
CPI = cycles / instructions
CPI = 82422901 / 100000000
CPI ≈ 0.824
```

## Performance Comparison

| Policy | IPC | CPI | L1D Misses | L2C Misses | LLC Misses |
|---|---:|---:|---:|---:|---:|
| LRU | 1.139 | 0.878 | 2,378,477 | 701,676 | 305,683 |
| SRRIP | 1.208 | 0.828 | 2,335,372 | 701,578 | 334,166 |
| DRRIP | 1.200 | 0.833 | 2,333,669 | 701,734 | 336,094 |
| SHIP | 1.213 | 0.824 | 2,330,601 | 701,733 | 327,262 |

## LLC Dead Block Comparison

| Policy | LLC Dead Blocks | LLC Valid Evictions | LLC Dead Block % | LLC Misses | IPC |
|---|---:|---:|---:|---:|---:|
| LRU | 85,461 | 289,804 | 29.49 | 305,683 | 1.139 |
| SRRIP | 305,265 | 318,287 | 95.91 | 334,166 | 1.208 |
| DRRIP | 305,632 | 320,215 | 95.45 | 336,094 | 1.200 |
| SHIP | 297,188 | 311,383 | 95.44 | 327,262 | 1.213 |

## Main Observation

The 100M CRC-2 bzip2 run showed a strong difference in LLC dead block behavior.

```text
LRU   LLC dead block % = 29.49
SRRIP LLC dead block % = 95.91
DRRIP LLC dead block % = 95.45
```

SRRIP and DRRIP selected dead LLC blocks as eviction victims much more often than LRU.

## Performance Observation

SRRIP had the best IPC:

```text
LRU   IPC = 1.139
SRRIP IPC = 1.208
DRRIP IPC = 1.200
```

DRRIP also improved IPC compared with LRU.

This is an important difference from the earlier `450.soplex` run. In `450.soplex`, SRRIP and DRRIP selected dead blocks more aggressively but did not improve IPC. In this CRC-2 bzip2 100M run, RRIP-style policies improved both dead victim selection and performance.

## Important Interpretation

A high LLC dead block percentage means that most evicted LLC blocks had not received a hit after being filled.

It does not mean the LLC had no useful data. It means the replacement policy was choosing eviction victims that were mostly dead according to the implemented counter.

For this run, SRRIP and DRRIP were much better than LRU at selecting dead LLC victims.

## LLC Miss Count Note

SRRIP and DRRIP had more LLC misses than LRU:

```text
LRU   LLC misses = 305,683
SRRIP LLC misses = 334,166
DRRIP LLC misses = 336,094
```

However, SRRIP and DRRIP still had better IPC than LRU.

This shows that LLC miss count alone is not enough to judge the policy. IPC depends on timing, overlap, memory behavior, and how misses are distributed.

## Final Conclusion

For the CRC-2 `bzip2_259B` trace with 10M warmup and 100M simulation instructions:

```text
SHIP gave the best overall result.
SRRIP was the second-best by IPC.
DRRIP also improved over LRU.
SRRIP, DRRIP, and SHIP all selected dead LLC victims much more effectively than LRU.

This is the strongest final result so far because it shows RRIP-style policies improving both dead-block victim selection and overall performance.
```
