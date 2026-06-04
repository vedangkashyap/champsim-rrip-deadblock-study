# CRC-2 bzip2 50M Comparison: LRU vs SRRIP vs DRRIP

## Purpose

This file records the CRC-2 `bzip2_259B` experiment using LRU, SRRIP, and DRRIP.

This experiment was run after short 1M runs failed to produce useful LLC replacement behavior. The 50M simulation run was used to create enough LLC valid evictions for dead block comparison.

## Experiment Setup

| Item                     | Value                                        |
| ------------------------ | -------------------------------------------- |
| Trace                    | bzip2_259B.trace.xz                          |
| Trace Source             | CRC-2 trace set                              |
| Local Path               | traces_crc2/bzip2_259B.trace.xz              |
| Compared Policies        | LRU, SRRIP, DRRIP                            |
| Warmup Instructions      | 10,000,000                                   |
| Simulation Instructions  | 50,000,000                                   |
| Main Dead Block Metric   | Dead Block %                                 |
| Main Performance Metrics | IPC, CPI, L1D Misses, L2C Misses, LLC Misses |

## Why This Run Was Needed

The first CRC-2 bzip2 run used only 1M simulation instructions and produced:

```text
LLC valid evictions = 0
```

That meant LLC replacement policies could not be meaningfully compared.

The 50M simulation run was used to give the LLC enough activity to produce valid evictions.

## Binaries Used

| Policy | Binary             |
| ------ | ------------------ |
| LRU    | bin/champsim_lru   |
| SRRIP  | bin/champsim_srrip |
| DRRIP  | bin/champsim_drrip |

The same trace and instruction counts were used for all policies.

## Run Commands

```bash
./bin/champsim_lru --warmup-instructions 10000000 --simulation-instructions 50000000 traces_crc2/bzip2_259B.trace.xz | tee run_crc2_bzip2_lru_50m.log
```

Runs LRU on the CRC-2 `bzip2_259B` trace and saves the full output to a local log file.

```bash
./bin/champsim_srrip --warmup-instructions 10000000 --simulation-instructions 50000000 traces_crc2/bzip2_259B.trace.xz | tee run_crc2_bzip2_srrip_50m.log
```

Runs SRRIP on the same trace and saves the full output to a local log file.

```bash
./bin/champsim_drrip --warmup-instructions 10000000 --simulation-instructions 50000000 traces_crc2/bzip2_259B.trace.xz | tee run_crc2_bzip2_drrip_50m.log
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
| L2C       |     134,590 |         265,017 |        50.79 |
| L1I       |           0 |               0 |            - |
| L1D       |     197,628 |         403,455 |        48.98 |
| ITLB      |           0 |               0 |            - |
| DTLB      |     194,764 |         269,481 |        72.27 |
| LLC       |      37,349 |         119,903 |        31.15 |

### LRU Performance and Miss Results

| Metric       |      Value |
| ------------ | ---------: |
| IPC          |      1.220 |
| Instructions | 50,000,003 |
| Cycles       | 40,973,741 |
| CPI          |      0.819 |
| L1D Misses   |  1,088,892 |
| L2C Misses   |    265,017 |
| LLC Misses   |    135,749 |

CPI calculation:

```text
CPI = cycles / instructions
CPI = 40973741 / 50000003
CPI ≈ 0.819
```

## SRRIP Results

### SRRIP Dead Block Results

| Structure | Dead Blocks | Valid Evictions | Dead Block % |
| --------- | ----------: | --------------: | -----------: |
| STLB      |           0 |               0 |            - |
| L2C       |     134,432 |         264,930 |        50.74 |
| L1I       |           0 |               0 |            - |
| L1D       |     194,973 |         403,456 |        48.33 |
| ITLB      |           0 |               0 |            - |
| DTLB      |     195,181 |         269,736 |        72.36 |
| LLC       |     130,399 |         134,790 |        96.74 |

### SRRIP Performance and Miss Results

| Metric       |      Value |
| ------------ | ---------: |
| IPC          |      1.307 |
| Instructions | 50,000,003 |
| Cycles       | 38,255,209 |
| CPI          |      0.765 |
| L1D Misses   |  1,076,284 |
| L2C Misses   |    264,930 |
| LLC Misses   |    150,636 |

CPI calculation:

```text
CPI = cycles / instructions
CPI = 38255209 / 50000003
CPI ≈ 0.765
```

## DRRIP Results

### DRRIP Dead Block Results

| Structure | Dead Blocks | Valid Evictions | Dead Block % |
| --------- | ----------: | --------------: | -----------: |
| STLB      |           0 |               0 |            - |
| L2C       |     134,540 |         264,970 |        50.78 |
| L1I       |           0 |               0 |            - |
| L1D       |     195,294 |         403,452 |        48.41 |
| ITLB      |           0 |               0 |            - |
| DTLB      |     195,345 |         269,833 |        72.39 |
| LLC       |     134,316 |         140,299 |        95.74 |

### DRRIP Performance and Miss Results

| Metric       |      Value |
| ------------ | ---------: |
| IPC          |      1.284 |
| Instructions | 50,000,003 |
| Cycles       | 38,952,489 |
| CPI          |      0.779 |
| L1D Misses   |  1,078,321 |
| L2C Misses   |    264,970 |
| LLC Misses   |    156,145 |

CPI calculation:

```text
CPI = cycles / instructions
CPI = 38952489 / 50000003
CPI ≈ 0.779
```

## Performance Comparison

| Policy |   IPC |   CPI | L1D Misses | L2C Misses | LLC Misses |
| ------ | ----: | ----: | ---------: | ---------: | ---------: |
| LRU    | 1.220 | 0.819 |  1,088,892 |    265,017 |    135,749 |
| SRRIP  | 1.307 | 0.765 |  1,076,284 |    264,930 |    150,636 |
| DRRIP  | 1.284 | 0.779 |  1,078,321 |    264,970 |    156,145 |

## LLC Dead Block Comparison

| Policy | LLC Dead Blocks | LLC Valid Evictions | LLC Dead Block % | LLC Misses |   IPC |
| ------ | --------------: | ------------------: | ---------------: | ---------: | ----: |
| LRU    |          37,349 |             119,903 |            31.15 |    135,749 | 1.220 |
| SRRIP  |         130,399 |             134,790 |            96.74 |    150,636 | 1.307 |
| DRRIP  |         134,316 |             140,299 |            95.74 |    156,145 | 1.284 |

## Main Observation

The 50M CRC-2 bzip2 run produced meaningful LLC valid evictions for all three policies.

SRRIP and DRRIP selected dead LLC blocks as victims much more often than LRU.

```text
LRU   LLC dead block % = 31.15
SRRIP LLC dead block % = 96.74
DRRIP LLC dead block % = 95.74
```

This means that the RRIP-style policies were much more focused on evicting LLC blocks that were not reused after insertion.

## Performance Observation

SRRIP had the best IPC in this run:

```text
LRU   IPC = 1.220
SRRIP IPC = 1.307
DRRIP IPC = 1.284
```

SRRIP also had the lowest CPI.

However, SRRIP and DRRIP had more LLC misses than LRU. This shows that LLC miss count alone does not fully explain performance. Timing behavior, memory-level behavior, and the distribution of misses also matter.

## Conclusion

For the CRC-2 `bzip2_259B` trace with 10M warmup and 50M simulation instructions:

```text
SRRIP gave the best overall performance.
DRRIP also improved IPC compared with LRU.
Both SRRIP and DRRIP selected dead LLC victims much more effectively than LRU.
```

This result supports the usefulness of RRIP-style replacement policies for this trace.

