# Replacement Policy Experiment Plan

## Purpose

This document defines the experiment plan for comparing dead block behavior across cache replacement policies.

The first comparison is between LRU and SRRIP. Both policies are tested using the same trace, same warmup length, same simulation length, and same dead block counter implementation.

The goal is to observe whether changing the LLC replacement policy changes the number and percentage of dead blocks.

## Policies Compared First

The initial comparison uses:

```text
LRU
SRRIP
```

LRU is used as the baseline replacement policy.

SRRIP is tested because it is an RRIP-style policy that predicts re-reference behavior using re-reference prediction values.

## Fixed Experiment Conditions

The following conditions are kept the same across runs:

| Item                    | Value                           |
| ----------------------- | ------------------------------- |
| Trace                   | 450.soplex-92B.champsimtrace.xz |
| Warmup Instructions     | 1,000,000                       |
| Simulation Instructions | 1,000,000                       |
| Number of Cores         | 1                               |
| Dead Block Counter Code | Same implementation             |
| Compared Variable       | LLC replacement policy          |

Keeping these fixed ensures that differences in results are mainly due to the replacement policy change.

## Metrics to Record

For each policy, the main metrics are:

```text
DEAD BLOCKS
VALID EVICTIONS
DEAD BLOCK %
```

The most important structures to compare first are:

```text
L1D
L2C
LLC
```

LLC is especially important because the replacement policy in the current config is specified for LLC.

## Config Files

Separate config files are used so each policy can produce a separate binary.

| Policy | Config File        | Binary             |
| ------ | ------------------ | ------------------ |
| LRU    | configs_lru.json   | bin/champsim_lru   |
| SRRIP  | configs_srrip.json | bin/champsim_srrip |

## Creating Config Copies

```bash
cp champsim_config.json configs_lru.json
```

Creates a separate LRU config file from the baseline ChampSim config.

```bash
cp champsim_config.json configs_srrip.json
```

Creates a separate SRRIP config file from the baseline ChampSim config.

## Setting SRRIP Replacement

```bash
sed -i 's/"replacement": "lru"/"replacement": "srrip"/' configs_srrip.json
```

Changes only the copied SRRIP config so that its replacement policy becomes `srrip`.

The original `champsim_config.json` remains unchanged.

## Setting Separate Executable Names

```bash
sed -i 's/"executable_name": "champsim"/"executable_name": "champsim_lru"/' configs_lru.json
```

Changes the LRU config so it builds the binary as `bin/champsim_lru`.

```bash
sed -i 's/"executable_name": "champsim"/"executable_name": "champsim_srrip"/' configs_srrip.json
```

Changes the SRRIP config so it builds the binary as `bin/champsim_srrip`.

This prevents one build from overwriting the other binary.

## Verifying Config Files

```bash
grep -n "executable_name\|replacement" configs_lru.json configs_srrip.json
```

Checks that each config has the correct executable name and replacement policy.

Expected result:

```text
configs_lru.json: "executable_name": "champsim_lru"
configs_lru.json: "replacement": "lru"
configs_srrip.json: "executable_name": "champsim_srrip"
configs_srrip.json: "replacement": "srrip"
```

## Building LRU

```bash
rm -rf .csconfig
```

Removes previously generated ChampSim build files so the LRU build starts from a clean generated state.

```bash
./config.sh configs_lru.json
```

Generates ChampSim build files using the LRU experiment config.

```bash
make -j1
```

Builds the LRU binary using one thread so build errors remain easy to read.

```bash
ls -lh bin/champsim_lru
```

Checks that the LRU binary was generated.

## Running LRU

```bash
./bin/champsim_lru --warmup-instructions 1000000 --simulation-instructions 1000000 traces/450.soplex-92B.champsimtrace.xz | tee run_soplex_lru.log
```

Runs the LRU binary on the `450.soplex` trace and stores the output in a local log file.

```bash
grep -n "DEAD BLOCKS\|VALID EVICTIONS\|DEAD BLOCK %" run_soplex_lru.log
```

Extracts only the dead block statistic lines from the LRU run log.

## Building SRRIP

```bash
rm -rf .csconfig
```

Removes generated build files from the previous build before switching to the SRRIP config.

```bash
./config.sh configs_srrip.json
```

Generates ChampSim build files using the SRRIP experiment config.

```bash
make -j1
```

Builds the SRRIP binary using one thread.

```bash
ls -lh bin/champsim_srrip
```

Checks that the SRRIP binary was generated.

## Running SRRIP

```bash
./bin/champsim_srrip --warmup-instructions 1000000 --simulation-instructions 1000000 traces/450.soplex-92B.champsimtrace.xz | tee run_soplex_srrip.log
```

Runs the SRRIP binary on the same `450.soplex` trace and stores the output in a local log file.

```bash
grep -n "DEAD BLOCKS\|VALID EVICTIONS\|DEAD BLOCK %" run_soplex_srrip.log
```

Extracts only the dead block statistic lines from the SRRIP run log.

## Comparison Method

After both runs are complete, the results should be summarized in a Markdown table.

The comparison should include:

| Structure | LRU Dead Block % | SRRIP Dead Block % | Observation |
| --------- | ---------------: | -----------------: | ----------- |
| L1D       |                  |                    |             |
| L2C       |                  |                    |             |
| LLC       |                  |                    |             |

The observation column should describe whether SRRIP increases or decreases the dead block percentage compared with LRU.

