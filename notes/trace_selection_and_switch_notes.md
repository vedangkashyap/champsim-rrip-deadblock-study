# Trace Selection and Switch Notes

## Purpose

This note explains why different traces were tried during the dead block counter experiments and why the project eventually switched to the CRC-2 `bzip2_259B` trace for stronger final results.

The project goal is to compare LRU, SRRIP, and DRRIP using dead block behavior and performance metrics. For this, the trace must create enough LLC activity and LLC valid evictions. If the LLC does not evict valid blocks, then LLC replacement policies cannot be meaningfully compared.

## Main Requirement for a Useful Trace

A trace is useful for this project only if it produces:

```text
LLC valid evictions > 0
```

This is important because LRU, SRRIP, and DRRIP mainly differ when the cache must choose an eviction victim.

If there are no LLC valid evictions, then there is no LLC victim selection. In that case, the dead block counter cannot compare LLC replacement behavior.

## Trace 1: 450.soplex-92B.champsimtrace.xz

The first useful trace was:

```text
450.soplex-92B.champsimtrace.xz
```

It was run with:

```text
Warmup: 1,000,000 instructions
Simulation: 1,000,000 instructions
```

This trace was useful because it produced LLC valid evictions even with a short 1M simulation run.

Main LLC results:

| Policy | LLC Dead Blocks | LLC Valid Evictions | LLC Dead Block % |    IPC |
| ------ | --------------: | ------------------: | ---------------: | -----: |
| LRU    |          19,800 |              29,184 |            67.85 | 0.2692 |
| SRRIP  |          47,952 |              47,952 |              100 | 0.2598 |
| DRRIP  |          46,250 |              48,466 |            95.43 | 0.2565 |

### What this trace showed

The `450.soplex` trace showed that SRRIP and DRRIP selected dead LLC blocks as victims much more often than LRU.

However, LRU had better overall IPC for this short run. Therefore, the result was useful for validating the dead block counter, but it did not show RRIP-style policies improving performance.

### Sanity Check

The `450.soplex` trace was rerun later to confirm repeatability.

The LRU rerun matched the original LRU result, and the SRRIP rerun matched the original SRRIP result. This confirmed that the experiment flow was deterministic and the observed differences were not random.

## Trace 2: 429.mcf-217B.champsimtrace.xz

The second trace tried was:

```text
429.mcf-217B.champsimtrace.xz
```

It was chosen because `mcf` is a memory-sensitive benchmark and is relevant to cache replacement studies.

The short run used:

```text
Warmup: 1,000,000 instructions
Simulation: 1,000,000 instructions
```

However, the LLC showed:

```text
LLC valid evictions = 0
```

This meant that LRU, SRRIP, and DRRIP produced the same visible replacement-policy results for the short run.

### Why it was skipped

The trace was not kept as a main result because, at the tested run length, it did not create LLC valid evictions. Since LLC replacement policies only differ when the LLC must select victims, this short `mcf` run was not useful for final LLC replacement comparison.

A longer `mcf` run was considered, but the project shifted toward CRC-2 traces because CRC-2 is more directly aligned with cache replacement policy evaluation.

## Trace 3: CRC-2 bzip2_259B.trace.xz

The final trace selected for stronger experiments was:

```text
bzip2_259B.trace.xz
```

This trace was downloaded from the CRC-2 trace set and stored locally in:

```text
traces_crc2/bzip2_259B.trace.xz
```

CRC-2 traces were preferred because CRC-2 was designed around cache replacement policy evaluation. This makes the trace source more suitable for a project comparing LRU, SRRIP, and DRRIP.

## Why bzip2_259B Was Chosen

The `bzip2_259B` trace was chosen because:

```text
Benchmark: bzip2
Source: CRC-2 trace set
Size: manageable compared with larger mcf traces
Relevance: useful for cache replacement experiments
```

The `mcf` CRC-2 traces were also relevant, but the visible options were much larger. The `bzip2_259B` trace was a practical first CRC-2 trace because it was smaller and still benchmark-relevant.

## Short CRC-2 bzip2 Run

The first CRC-2 bzip2 run used:

```text
Warmup: 1,000,000 instructions
Simulation: 1,000,000 instructions
```

This short run again showed:

```text
LLC valid evictions = 0
```

Therefore, it was not useful for LLC replacement comparison.

## Longer CRC-2 bzip2 Runs

The run length was increased to create meaningful LLC replacement behavior.

Two final run lengths were used:

```text
Warmup: 10,000,000 instructions
Simulation: 50,000,000 instructions
```

and:

```text
Warmup: 10,000,000 instructions
Simulation: 100,000,000 instructions
```

These longer runs produced LLC valid evictions and clear differences between LRU, SRRIP, and DRRIP.

## Best Final Trace

The strongest final trace result came from:

```text
CRC-2 bzip2_259B.trace.xz
Warmup: 10M
Simulation: 100M
```

This run showed both:

```text
SRRIP/DRRIP selected dead LLC victims much more effectively than LRU.
SRRIP/DRRIP improved IPC compared with LRU.
```

## Overall Trace Lesson

The experiments showed that the number of instructions alone does not guarantee useful LLC replacement behavior.

A trace can execute millions of instructions and still produce zero LLC valid evictions. In that case, LLC replacement policy comparison is weak.

For this project, the key trace-selection rule became:

```text
A trace is useful only if it creates LLC valid evictions.
```

The `450.soplex` trace was useful for quick validation. The CRC-2 `bzip2_259B` trace with longer simulation length became the strongest final comparison trace. The short `429.mcf` run was skipped because it did not create useful LLC eviction behavior at the tested length.

