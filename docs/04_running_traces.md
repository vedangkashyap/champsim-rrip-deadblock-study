# Running ChampSim Traces

## Purpose

This document explains how traces are handled and run in this project.

ChampSim is a trace-based simulator. This means the compiled binary does not directly run a normal program. Instead, it reads an already generated instruction trace and simulates how the processor, caches, TLBs, and memory system behave for that trace.

For this project, traces are needed to validate that the dead block counter works during an actual simulation run.

## Why Trace Files Are Needed

The dead block counter only produces meaningful results when cache blocks are filled, hit, and evicted during simulation. Without a trace, ChampSim has no workload to simulate, so there is no useful cache activity to measure.

A trace run allows the counter to report:

* how many valid blocks were evicted
* how many of those blocks were dead
* the dead block percentage for each cache or TLB structure

## Local Trace Storage

Trace files are stored locally inside:

```text
traces/
```

This folder is ignored by Git because trace files can be large and should not be pushed to GitHub.

The `.gitignore` file includes rules for local trace files such as:

```text
traces/
*.trace
*.trace.gz
*.trace.xz
*.xz
*.gz
```

These rules keep the repository clean while still allowing traces to be used locally for experiments.

## Trace Used

The trace used for the first validation run was:

```text
450.soplex-92B.champsimtrace.xz
```

This trace was stored locally as:

```text
traces/450.soplex-92B.champsimtrace.xz
```

## Checking Whether Traces Exist

```bash
find . -name "*.xz" | head
```

Searches the current repository for `.xz` compressed trace files.

This was used to confirm that the clean ChampSim repository did not originally include traces.

```bash
ls -lh traces
```

Lists locally downloaded traces with file sizes.

This is useful for confirming that the trace exists before running ChampSim.

## Basic ChampSim Run Format

```bash
./bin/champsim --warmup-instructions 1000000 --simulation-instructions 1000000 <trace_file>
```

Runs ChampSim using a selected trace file.

`--warmup-instructions` controls how many instructions are used to warm up simulator state before detailed measurement.

`--simulation-instructions` controls how many instructions are used for the measured region of interest.

## Validation Run Command

```bash
./bin/champsim --warmup-instructions 1000000 --simulation-instructions 1000000 traces/450.soplex-92B.champsimtrace.xz | tee run_soplex_deadblock.log
```

Runs ChampSim on the `450.soplex` trace and saves the terminal output into a local log file.

`tee` is used so the output appears on the terminal and is also written to a log file.

The log file is local only and ignored by Git.

## Extracting Dead Block Output

```bash
grep -n "DEAD BLOCKS\|VALID EVICTIONS\|DEAD BLOCK %" run_soplex_deadblock.log
```

Extracts only the dead block statistic lines from the full simulation output log.

This is useful because the complete ChampSim output is long, while the dead block lines are the specific validation target for this project.

## Expected Output

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

## Validation Result

The first validation run was successful. Dead block statistics were printed for multiple structures, including:

* STLB
* L2C
* L1D
* DTLB
* LLC

The LLC result from the validation run was:

```text
cpu0->LLC DEAD BLOCKS: 19800 VALID EVICTIONS: 29184 DEAD BLOCK %: 67.85
```

This confirmed that the implemented counter was active during simulation and that the output formatting worked correctly.

## Repository Rule

Raw trace files and raw run logs should not be committed.

Only summarized results should be saved in the `results/` directory.

