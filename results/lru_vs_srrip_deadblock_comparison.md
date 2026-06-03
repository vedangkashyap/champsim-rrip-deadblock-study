# LRU vs SRRIP Dead Block Comparison

## Purpose

This file compares dead block behavior between LRU and SRRIP using the same ChampSim setup, same trace, same warmup length, and same simulation length.

The goal is to understand how changing the LLC replacement policy affects dead block behavior, cache misses, and overall performance.

This comparison is important because a higher dead block percentage does not automatically mean better overall performance. A replacement policy may be good at selecting dead blocks as victims, but it can still have more misses or lower IPC.

## Experiment Setup

| Item                     | Value                                        |
| ------------------------ | -------------------------------------------- |
| Simulator                | ChampSim                                     |
| Trace                    | 450.soplex-92B.champsimtrace.xz              |
| Warmup Instructions      | 1,000,000                                    |
| Simulation Instructions  | 1,000,000                                    |
| Number of Cores          | 1                                            |
| Compared Policies        | LRU, SRRIP                                   |
| Main Dead Block Metric   | Dead Block %                                 |
| Main Performance Metrics | IPC, CPI, L1D Misses, L2C Misses, LLC Misses |

The same trace and same instruction counts were used for both policies. The main variable changed between the two runs was the LLC replacement policy.

## Config Files and Binaries

| Policy | Config File        | Binary             |
| ------ | ------------------ | ------------------ |
| LRU    | configs_lru.json   | bin/champsim_lru   |
| SRRIP  | configs_srrip.json | bin/champsim_srrip |

Separate binaries were used so that the LRU and SRRIP builds did not overwrite each other.

## Config Verification Command

```bash
grep -n "executable_name\|replacement" configs_lru.json configs_srrip.json
```

Checks that each config file has the correct executable name and replacement policy.

Expected meaning:

```text
configs_lru.json   -> executable_name: champsim_lru, replacement: lru
configs_srrip.json -> executable_name: champsim_srrip, replacement: srrip
```

## LRU Build Commands

```bash
rm -rf .csconfig
```

Removes ChampSim’s generated build configuration directory so the LRU build starts from a clean generated state.

```bash
./config.sh configs_lru.json
```

Generates ChampSim build files using the LRU experiment configuration.

```bash
make -j1
```

Builds the LRU binary using one thread so that compiler output is easier to read.

```bash
ls -lh bin/champsim_lru
```

Checks that the LRU binary was generated successfully.

## LRU Run Command

```bash
./bin/champsim_lru --warmup-instructions 1000000 --simulation-instructions 1000000 traces/450.soplex-92B.champsimtrace.xz | tee run_soplex_lru.log
```

Runs the LRU binary on the `450.soplex` trace and saves the full terminal output into a local log file.

The `tee` command is used so that output is shown on the terminal and also written to `run_soplex_lru.log`.

## LRU Extraction Commands

```bash
grep -n "DEAD BLOCKS\|VALID EVICTIONS\|DEAD BLOCK %" run_soplex_lru.log
```

Extracts only the dead block statistic lines from the full LRU simulation output.

```bash
grep "CPU 0 cumulative IPC" run_soplex_lru.log
```

Extracts the IPC, instruction count, and cycle count for the LRU run.

```bash
grep "cpu0->cpu0_L1D TOTAL\|cpu0->cpu0_L2C TOTAL\|cpu0->LLC TOTAL" run_soplex_lru.log
```

Extracts the L1D, L2C, and LLC access, hit, and miss statistics for the LRU run.

## LRU Dead Block Results

| Structure | Dead Blocks | Valid Evictions | Dead Block % |
| --------- | ----------: | --------------: | -----------: |
| STLB      |         666 |             907 |        73.43 |
| L2C       |       22240 |           53997 |        41.19 |
| L1I       |           0 |               0 |            - |
| L1D       |       20754 |           65382 |        31.74 |
| ITLB      |           0 |               0 |            - |
| DTLB      |         295 |            6085 |        4.848 |
| LLC       |       19800 |           29184 |        67.85 |

## LRU Performance and Miss Results

| Metric       |     Value |
| ------------ | --------: |
| IPC          |    0.2692 |
| Instructions | 1,000,001 |
| Cycles       | 3,714,094 |
| CPI          |     3.714 |
| L1D Misses   |   163,876 |
| L2C Misses   |    53,999 |
| LLC Misses   |    40,995 |

CPI was calculated as:

```text
CPI = cycles / instructions
CPI = 3714094 / 1000001
CPI ≈ 3.714
```

## SRRIP Build Commands

```bash
rm -rf .csconfig
```

Removes ChampSim’s generated build configuration directory so the SRRIP build starts from a clean generated state.

```bash
./config.sh configs_srrip.json
```

Generates ChampSim build files using the SRRIP experiment configuration.

```bash
make -j1
```

Builds the SRRIP binary using one thread so that compiler output is easier to read.

```bash
ls -lh bin/champsim_srrip
```

Checks that the SRRIP binary was generated successfully.

## SRRIP Run Command

```bash
./bin/champsim_srrip --warmup-instructions 1000000 --simulation-instructions 1000000 traces/450.soplex-92B.champsimtrace.xz | tee run_soplex_srrip.log
```

Runs the SRRIP binary on the same `450.soplex` trace and saves the full terminal output into a local log file.

The trace, warmup length, and simulation length are kept the same as the LRU run.

## SRRIP Extraction Commands

```bash
grep -n "DEAD BLOCKS\|VALID EVICTIONS\|DEAD BLOCK %" run_soplex_srrip.log
```

Extracts only the dead block statistic lines from the full SRRIP simulation output.

```bash
grep "CPU 0 cumulative IPC" run_soplex_srrip.log
```

Extracts the IPC, instruction count, and cycle count for the SRRIP run.

```bash
grep "cpu0->cpu0_L1D TOTAL\|cpu0->cpu0_L2C TOTAL\|cpu0->LLC TOTAL" run_soplex_srrip.log
```

Extracts the L1D, L2C, and LLC access, hit, and miss statistics for the SRRIP run.

## SRRIP Dead Block Results

| Structure | Dead Blocks | Valid Evictions | Dead Block % |
| --------- | ----------: | --------------: | -----------: |
| STLB      |         666 |             907 |        73.43 |
| L2C       |       22164 |           54034 |        41.02 |
| L1I       |           0 |               0 |            - |
| L1D       |       20750 |           65383 |        31.74 |
| ITLB      |           0 |               0 |            - |
| DTLB      |         297 |            6090 |        4.877 |
| LLC       |       47952 |           47952 |          100 |

## SRRIP Performance and Miss Results

| Metric       |     Value |
| ------------ | --------: |
| IPC          |    0.2598 |
| Instructions | 1,000,003 |
| Cycles       | 3,848,440 |
| CPI          |     3.848 |
| L1D Misses   |   164,859 |
| L2C Misses   |    54,035 |
| LLC Misses   |    59,762 |

CPI was calculated as:

```text
CPI = cycles / instructions
CPI = 3848440 / 1000003
CPI ≈ 3.848
```

## Dead Block Comparison

| Structure | LRU Dead Block % | SRRIP Dead Block % | Observation                                        |
| --------- | ---------------: | -----------------: | -------------------------------------------------- |
| STLB      |            73.43 |              73.43 | No change                                          |
| L2C       |            41.19 |              41.02 | Very small decrease with SRRIP                     |
| L1I       |                - |                  - | No valid evictions                                 |
| L1D       |            31.74 |              31.74 | Nearly unchanged                                   |
| ITLB      |                - |                  - | No valid evictions                                 |
| DTLB      |            4.848 |              4.877 | Very small increase with SRRIP                     |
| LLC       |            67.85 |                100 | Large increase in dead victim selection with SRRIP |

## Performance and Miss Comparison

| Policy |    IPC |   CPI | L1D Misses | L2C Misses | LLC Misses |
| ------ | -----: | ----: | ---------: | ---------: | ---------: |
| LRU    | 0.2692 | 3.714 |    163,876 |     53,999 |     40,995 |
| SRRIP  | 0.2598 | 3.848 |    164,859 |     54,035 |     59,762 |

## LLC-Specific Comparison

| Policy | LLC Dead Blocks | LLC Valid Evictions | LLC Dead Block % | LLC Misses |
| ------ | --------------: | ------------------: | ---------------: | ---------: |
| LRU    |          19,800 |              29,184 |            67.85 |     40,995 |
| SRRIP  |          47,952 |              47,952 |              100 |     59,762 |

## Main Observation

The largest difference appears at the LLC.

For LRU:

```text
cpu0->LLC DEAD BLOCKS: 19800 VALID EVICTIONS: 29184 DEAD BLOCK %: 67.85
```

For SRRIP:

```text
cpu0->LLC DEAD BLOCKS: 47952 VALID EVICTIONS: 47952 DEAD BLOCK %: 100
```

This means that, in the SRRIP run, every valid LLC eviction counted by the counter was a dead block.

This does not mean the LLC had no useful blocks. The SRRIP run still had LLC hits. It means that the LLC blocks selected as victims during the measured region had not received a hit after being filled.

## Interpretation

For this trace and simulation length, SRRIP was better than LRU at selecting dead LLC blocks as eviction victims. Its LLC dead block percentage was 100%, meaning every valid LLC eviction was classified as dead by the counter.

However, SRRIP was not better overall in this run. It had:

* lower IPC than LRU
* higher CPI than LRU
* more L1D misses than LRU
* slightly more L2C misses than LRU
* many more LLC misses than LRU

Therefore, the correct conclusion is not that SRRIP is generally better than LRU. The better conclusion is:

```text
For this trace and run length, SRRIP selected dead LLC blocks more cleanly than LRU, but it also produced worse overall performance metrics in this run.
```

Dead block percentage is useful for understanding victim quality, but it should be interpreted together with misses, IPC, CPI, and results from more traces.

## Final Conclusion

This experiment confirms that the dead block counter can reveal replacement-policy behavior that is not obvious from misses alone.

In this run:

* LRU had a lower LLC dead block percentage but fewer LLC misses and better IPC.
* SRRIP had a perfect LLC dead block percentage among evicted blocks, but more LLC misses and lower IPC.

This suggests that SRRIP’s victim selection was strongly focused on blocks that had not been reused, but the overall cache behavior for this trace was not better than LRU.

More traces and longer simulations are needed before making a general conclusion about LRU vs SRRIP.

