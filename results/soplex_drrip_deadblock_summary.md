# DRRIP Dead Block Summary: 450.soplex

## Purpose

This file records the DRRIP run for the `450.soplex` trace.

The goal of this run is to observe how DRRIP behaves with the implemented dead block counter and compare its high-level behavior against the earlier LRU and SRRIP runs.

This file is kept separate from the LRU vs SRRIP comparison so that DRRIP results can be documented clearly without making the first comparison file too crowded.

## Experiment Setup

| Item                    | Value                           |
| ----------------------- | ------------------------------- |
| Simulator               | ChampSim                        |
| Trace                   | 450.soplex-92B.champsimtrace.xz |
| Replacement Policy      | DRRIP                           |
| Config File             | configs_drrip.json              |
| Binary                  | bin/champsim_drrip              |
| Warmup Instructions     | 1,000,000                       |
| Simulation Instructions | 1,000,000                       |
| Number of Cores         | 1                               |

The trace, warmup length, and simulation length are kept the same as the LRU and SRRIP runs so that the results remain comparable.

## DRRIP Config Setup

```bash
cp champsim_config.json configs_drrip.json
```

Creates a separate DRRIP config file from the baseline ChampSim config.

```bash
sed -i 's/"replacement": "lru"/"replacement": "drrip"/' configs_drrip.json
```

Changes only the copied DRRIP config so that the LLC replacement policy becomes `drrip`.

```bash
sed -i 's/"executable_name": "champsim"/"executable_name": "champsim_drrip"/' configs_drrip.json
```

Changes the output binary name to `bin/champsim_drrip` so it does not overwrite the LRU or SRRIP binaries.

## Config Verification Command

```bash
grep -n "executable_name\|replacement" configs_drrip.json
```

Checks that the DRRIP config has the correct executable name and replacement policy.

Expected meaning:

```text
executable_name -> champsim_drrip
replacement     -> drrip
```

## Build Commands

```bash
rm -rf .csconfig
```

Removes ChampSim’s generated build configuration directory so the DRRIP build starts from a clean generated state.

```bash
./config.sh configs_drrip.json
```

Generates ChampSim build files using the DRRIP experiment configuration.

```bash
make -j1
```

Builds the DRRIP binary using one thread so build output remains easier to read.

```bash
ls -lh bin/champsim_drrip
```

Checks that the DRRIP binary was generated successfully.

## Build Result

The DRRIP binary was successfully generated:

```text
bin/champsim_drrip
```

During the build, some `catch.hpp` messages appeared from test dependency generation, but they did not stop the main ChampSim binary from being built.

A non-blocking warning also appeared in `src/plain_printer.cc` about converting `uint64_t` to `double` during dead block percentage printing. The binary was still generated successfully.

## Run Command

```bash
./bin/champsim_drrip --warmup-instructions 1000000 --simulation-instructions 1000000 traces/450.soplex-92B.champsimtrace.xz | tee run_soplex_drrip.log
```

Runs the DRRIP binary on the `450.soplex` trace and saves the full terminal output into a local log file.

The `tee` command is used so that the output is shown on the terminal and also written to `run_soplex_drrip.log`.

## Dead Block Extraction Command

```bash
grep -n "DEAD BLOCKS\|VALID EVICTIONS\|DEAD BLOCK %" run_soplex_drrip.log
```

Extracts only the dead block statistic lines from the full DRRIP simulation output.

## Performance Extraction Commands

```bash
grep "CPU 0 cumulative IPC" run_soplex_drrip.log
```

Extracts IPC, instruction count, and cycle count for the DRRIP run.

```bash
grep "cpu0->cpu0_L1D TOTAL\|cpu0->cpu0_L2C TOTAL\|cpu0->LLC TOTAL" run_soplex_drrip.log
```

Extracts the L1D, L2C, and LLC access, hit, and miss statistics for the DRRIP run.

## DRRIP Dead Block Output

```text
cpu0->cpu0_STLB DEAD BLOCKS: 666   VALID EVICTIONS: 907    DEAD BLOCK %: 73.43
cpu0->cpu0_L2C  DEAD BLOCKS: 22200 VALID EVICTIONS: 53961  DEAD BLOCK %: 41.14
cpu0->cpu0_L1I  DEAD BLOCKS: 0     VALID EVICTIONS: 0      DEAD BLOCK %: -
cpu0->cpu0_L1D  DEAD BLOCKS: 20700 VALID EVICTIONS: 65377  DEAD BLOCK %: 31.66
cpu0->cpu0_ITLB DEAD BLOCKS: 0     VALID EVICTIONS: 0      DEAD BLOCK %: -
cpu0->cpu0_DTLB DEAD BLOCKS: 288   VALID EVICTIONS: 6080   DEAD BLOCK %: 4.737
cpu0->LLC       DEAD BLOCKS: 46250 VALID EVICTIONS: 48466  DEAD BLOCK %: 95.43
```

## DRRIP Dead Block Summary Table

| Structure | Dead Blocks | Valid Evictions | Dead Block % |
| --------- | ----------: | --------------: | -----------: |
| STLB      |         666 |             907 |        73.43 |
| L2C       |       22200 |           53961 |        41.14 |
| L1I       |           0 |               0 |            - |
| L1D       |       20700 |           65377 |        31.66 |
| ITLB      |           0 |               0 |            - |
| DTLB      |         288 |            6080 |        4.737 |
| LLC       |       46250 |           48466 |        95.43 |

## DRRIP Performance and Miss Summary

| Metric       |     Value |
| ------------ | --------: |
| IPC          |    0.2565 |
| Instructions | 1,000,003 |
| Cycles       | 3,898,240 |
| CPI          |     3.898 |
| L1D Misses   |   164,265 |
| L2C Misses   |    53,962 |
| LLC Misses   |    60,275 |

CPI was calculated as:

```text
CPI = cycles / instructions
CPI = 3898240 / 1000003
CPI ≈ 3.898
```

## LLC Observation

The LLC result for DRRIP was:

```text
cpu0->LLC DEAD BLOCKS: 46250 VALID EVICTIONS: 48466 DEAD BLOCK %: 95.43
```

This means that during the measured simulation region:

* 48,466 valid LLC blocks were evicted.
* 46,250 of those evicted LLC blocks had not received a hit after being filled.
* The LLC dead block percentage was 95.43%.

This shows that DRRIP selected dead LLC blocks as victims very frequently in this run.

## Comparison Context

For the same `450.soplex` trace and same run length, the earlier LLC results were:

| Policy | LLC Dead Blocks | LLC Valid Evictions | LLC Dead Block % | LLC Misses |    IPC |
| ------ | --------------: | ------------------: | ---------------: | ---------: | -----: |
| LRU    |          19,800 |              29,184 |            67.85 |     40,995 | 0.2692 |
| SRRIP  |          47,952 |              47,952 |              100 |     59,762 | 0.2598 |
| DRRIP  |          46,250 |              48,466 |            95.43 |     60,275 | 0.2565 |

DRRIP is closer to SRRIP than LRU in LLC dead block behavior. It selected dead blocks as victims much more often than LRU, but it also had more LLC misses and lower IPC than LRU in this short run.

## Interpretation

For this trace and simulation length, DRRIP was strong at selecting dead LLC blocks as eviction victims. Its LLC dead block percentage was 95.43%, which is much higher than LRU’s 67.85%.

However, DRRIP did not improve overall performance for this run. Compared with LRU, it had:

* lower IPC
* higher CPI
* more LLC misses

Therefore, the correct conclusion is:

```text
For 450.soplex, DRRIP selected dead LLC victims more aggressively than LRU, but LRU still performed better overall in this short run.
```

## Conclusion

This DRRIP result supports the idea that dead block percentage should not be interpreted alone.

A high dead block percentage can mean that the replacement policy is good at choosing dead victims, but overall performance also depends on miss count, IPC, CPI, and workload behavior.

More traces and longer simulations are needed before making a general conclusion about DRRIP compared with LRU and SRRIP.

