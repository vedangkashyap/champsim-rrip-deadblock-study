# Dead Block Counter Validation: 450.soplex with LRU

## Purpose

This file records the first successful validation run of the dead block counter.

The goal of this run was not to compare replacement policies yet. The goal was to confirm that the implemented dead block counter works during a real ChampSim simulation and that the new statistics appear in the normal text output.

This run should be treated as the first LRU baseline validation because the binary used here was built from the default `champsim_config.json`, where the LLC replacement policy was `lru`.

## Trace Used

```text
450.soplex-92B.champsimtrace.xz
```

This is the workload trace used for the validation run.

The trace was stored locally in:

```text
traces/450.soplex-92B.champsimtrace.xz
```

This is the local path used while running ChampSim.

Trace files are not committed to GitHub because they are large and are ignored by `.gitignore`.

## Replacement Policy Used

This validation run used the default `bin/champsim` binary built from:

```text
champsim_config.json
```

This was the baseline configuration file used before creating separate LRU and SRRIP experiment configs.

At the time of this run, the LLC replacement policy in `champsim_config.json` was:

```text
replacement: lru
```

Therefore, these results are LRU baseline validation results, not SRRIP results.

## Run Length

| Phase      | Instructions |
| ---------- | -----------: |
| Warmup     |    1,000,000 |
| Simulation |    1,000,000 |

The warmup phase prepares the simulator state before measurement. The simulation phase is the measured region where final statistics are collected.

## Run Command

```bash
./bin/champsim --warmup-instructions 1000000 --simulation-instructions 1000000 traces/450.soplex-92B.champsimtrace.xz | tee run_soplex_deadblock.log
```

Runs the default ChampSim binary on the `450.soplex` trace.

The `--warmup-instructions` option sets the number of warmup instructions.

The `--simulation-instructions` option sets the number of measured simulation instructions.

The `tee` command prints the output on the terminal and also saves the full output into `run_soplex_deadblock.log`.

## Extract Command

```bash
grep -n "DEAD BLOCKS\|VALID EVICTIONS\|DEAD BLOCK %" run_soplex_deadblock.log
```

Extracts only the dead block statistic lines from the full ChampSim output log.

This is useful because the complete ChampSim output is long, while the dead block lines are the specific validation target for this project.

## Extracted Output

```text
cpu0->cpu0_STLB DEAD BLOCKS: 666   VALID EVICTIONS: 907    DEAD BLOCK %: 73.43
cpu0->cpu0_L2C  DEAD BLOCKS: 22240 VALID EVICTIONS: 53997  DEAD BLOCK %: 41.19
cpu0->cpu0_L1I  DEAD BLOCKS: 0     VALID EVICTIONS: 0      DEAD BLOCK %: -
cpu0->cpu0_L1D  DEAD BLOCKS: 20754 VALID EVICTIONS: 65382  DEAD BLOCK %: 31.74
cpu0->cpu0_ITLB DEAD BLOCKS: 0     VALID EVICTIONS: 0      DEAD BLOCK %: -
cpu0->cpu0_DTLB DEAD BLOCKS: 295   VALID EVICTIONS: 6085   DEAD BLOCK %: 4.848
cpu0->LLC       DEAD BLOCKS: 19800 VALID EVICTIONS: 29184  DEAD BLOCK %: 67.85
```

## Summary Table

| Structure | Dead Blocks | Valid Evictions | Dead Block % |
| --------- | ----------: | --------------: | -----------: |
| STLB      |         666 |             907 |        73.43 |
| L2C       |       22240 |           53997 |        41.19 |
| L1I       |           0 |               0 |            - |
| L1D       |       20754 |           65382 |        31.74 |
| ITLB      |           0 |               0 |            - |
| DTLB      |         295 |            6085 |        4.848 |
| LLC       |       19800 |           29184 |        67.85 |

## Interpretation

The output confirms that the dead block counter is active and visible in the normal ChampSim output.

The LLC result was:

```text
cpu0->LLC DEAD BLOCKS: 19800 VALID EVICTIONS: 29184 DEAD BLOCK %: 67.85
```

This means that during the measured simulation region:

* 29,184 valid LLC blocks were evicted.
* 19,800 of those evicted LLC blocks had not received a hit after being filled.
* The LLC dead block percentage was 67.85%.

For this trace, run length, and LRU replacement configuration, the LLC dead block percentage was high. This suggests that many LLC insertions did not provide reuse before eviction.

## Validation Conclusion

This run validates the basic functionality of the counter:

* blocks are marked as not reused on fill
* blocks are marked as reused on hit
* valid evictions are counted
* blocks evicted without reuse are counted as dead
* dead block statistics are printed in the final output

Raw log files are kept local and ignored by Git. Only this summarized result is committed.

