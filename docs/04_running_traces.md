# Running ChampSim Traces

## Purpose

This document explains how traces are handled and run in this project.

ChampSim is a trace-based simulator. This means the compiled binary does not directly run a normal program. Instead, it reads an already generated instruction trace and simulates how the processor, caches, TLBs, and memory system behave for that trace.

For this project, traces are needed to validate that the dead block counter works during actual simulation runs.

## Why Trace Files Are Needed

The dead block counter only produces meaningful results when cache blocks are filled, hit, and evicted during simulation.

Without a trace, ChampSim has no workload to simulate, so there is no useful cache activity to measure.

A trace run allows the counter to report:

* how many valid blocks were evicted
* how many of those blocks were dead
* the dead block percentage for each cache or TLB structure

## Local Trace Storage

Trace files are stored locally and are not committed to GitHub.

Normal ChampSim/DPC-style traces are stored in:

```text
traces/
```

CRC-2 traces are stored separately in:

```text
traces_crc2/
```

The CRC-2 folder is kept separate because CRC-2 traces are specifically related to cache replacement policy experiments.

The `.gitignore` file includes rules for local trace files and trace folders such as:

```text
traces/
traces_crc2/
*.trace
*.trace.gz
*.trace.xz
*.xz
*.gz
```

These rules keep the repository clean while still allowing traces to be used locally for experiments.

## Traces Used

The first validation trace was:

```text
450.soplex-92B.champsimtrace.xz
```

This trace was stored locally as:

```text
traces/450.soplex-92B.champsimtrace.xz
```

It was useful because it produced LLC valid evictions even with a short 1M simulation run.

The later CRC-2 trace used for stronger replacement-policy comparison was:

```text
bzip2_259B.trace.xz
```

This trace was stored locally as:

```text
traces_crc2/bzip2_259B.trace.xz
```

The CRC-2 bzip2 trace required longer simulation lengths before it produced meaningful LLC valid evictions.

## Checking Whether Traces Exist

```bash
find . -name "*.xz" | head
```

Searches the current repository for `.xz` compressed trace files.

This was used to confirm that the clean ChampSim repository did not originally include traces.

```bash
ls -lh traces
```

Lists locally downloaded normal ChampSim/DPC-style traces with file sizes.

```bash
ls -lh traces_crc2
```

Lists locally downloaded CRC-2 traces with file sizes.

This is useful for confirming that the required trace exists before running ChampSim.

## Basic ChampSim Run Format

```bash
./bin/champsim --warmup-instructions 1000000 --simulation-instructions 1000000 <trace_file>
```

Runs ChampSim using a selected trace file.

`--warmup-instructions` controls how many instructions are used to warm up simulator state before detailed measurement.

`--simulation-instructions` controls how many instructions are used for the measured region of interest.

## First Validation Run Command

```bash
./bin/champsim --warmup-instructions 1000000 --simulation-instructions 1000000 traces/450.soplex-92B.champsimtrace.xz | tee run_soplex_deadblock.log
```

Runs ChampSim on the `450.soplex` trace and saves the terminal output into a local log file.

`tee` is used so the output appears on the terminal and is also written to a log file.

The log file is local only and ignored by Git.

## CRC-2 bzip2 Run Commands

The CRC-2 `bzip2_259B` trace was first tested with short 1M runs, but those runs produced zero LLC valid evictions.

Longer runs were then used to create meaningful LLC replacement behavior.

Example 50M run format:

```bash
./bin/champsim_lru --warmup-instructions 10000000 --simulation-instructions 50000000 traces_crc2/bzip2_259B.trace.xz | tee run_crc2_bzip2_lru_50m.log
```

Runs the LRU binary on the CRC-2 `bzip2_259B` trace with 10M warmup and 50M simulation instructions.

Example 100M run format:

```bash
./bin/champsim_lru --warmup-instructions 10000000 --simulation-instructions 100000000 traces_crc2/bzip2_259B.trace.xz | tee run_crc2_bzip2_lru_100m.log
```

Runs the LRU binary on the CRC-2 `bzip2_259B` trace with 10M warmup and 100M simulation instructions.

The same run format was repeated for SRRIP and DRRIP using their separate binaries.

## Extracting Dead Block Output

```bash
grep -n "DEAD BLOCKS\|VALID EVICTIONS\|DEAD BLOCK %" <log_file>
```

Extracts only the dead block statistic lines from the full simulation output log.

This is useful because the complete ChampSim output is long, while the dead block lines are the specific validation target for this project.

## Extracting IPC and Miss Output

```bash
grep "CPU 0 cumulative IPC" <log_file>
```

Extracts IPC, instruction count, and cycle count from the simulation log.

```bash
grep "cpu0->cpu0_L1D TOTAL\|cpu0->cpu0_L2C TOTAL\|cpu0->LLC TOTAL" <log_file>
```

Extracts L1D, L2C, and LLC access, hit, and miss statistics from the simulation log.

These values are used along with dead block percentage to compare replacement policies.

## Expected Dead Block Output

A successful run should show lines containing:

```text
DEAD BLOCKS
VALID EVICTIONS
DEAD BLOCK %
```

Example output:

```text
cpu0->LLC DEAD BLOCKS: 19800 VALID EVICTIONS: 29184 DEAD BLOCK %: 67.85
```

This confirms that the dead block counter is connected to the normal ChampSim output path.

## Interpreting Zero Valid Evictions

The dead block percentage is meaningful only when valid evictions occur.

If the output shows:

```text
VALID EVICTIONS: 0
DEAD BLOCK %: -
```

then the replacement policy did not get a chance to choose valid eviction victims for that structure during the measured region.

This does not mean the cache had no useful or dead blocks. It only means no valid block reached eviction during that run.

For replacement policy comparison, the most important requirement is:

```text
LLC valid evictions > 0
```

If LLC valid evictions are zero, then LRU, SRRIP, and DRRIP cannot be meaningfully compared at the LLC for that run length.

## Validation Result

The first validation run on `450.soplex` was successful. Dead block statistics were printed for multiple structures, including:

* STLB
* L2C
* L1D
* DTLB
* LLC

The LLC result from the first validation run was:

```text
cpu0->LLC DEAD BLOCKS: 19800 VALID EVICTIONS: 29184 DEAD BLOCK %: 67.85
```

This confirmed that the implemented counter was active during simulation and that the output formatting worked correctly.

Later CRC-2 `bzip2_259B` experiments showed why run length matters. With 1M simulation instructions, the LLC had zero valid evictions. With longer 50M and 100M simulation runs, the LLC produced meaningful valid evictions and clear policy differences between LRU, SRRIP, and DRRIP.

## Repository Rule

Raw trace files and raw run logs should not be committed.

Only summarized results should be saved in the `results/` directory.

