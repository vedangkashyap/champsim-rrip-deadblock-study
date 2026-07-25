#include "cache_stats.h"

cache_stats operator-(cache_stats lhs, cache_stats rhs)
{
  cache_stats result;
  result.pf_requested = lhs.pf_requested - rhs.pf_requested;
  result.pf_issued = lhs.pf_issued - rhs.pf_issued;
  result.pf_useful = lhs.pf_useful - rhs.pf_useful;
  result.pf_useless = lhs.pf_useless - rhs.pf_useless;
  result.pf_fill = lhs.pf_fill - rhs.pf_fill;


  result.hits = lhs.hits - rhs.hits;

  // ---------------------------------------------------------------
  // Name: VEDANGK
  // Reason: Subtraction updated for the renamed *_global fields and
  // the two new *_percpu event_counters, so warmup-vs-ROI phase
  // subtraction keeps working correctly for the new per-core stats.
  // ---------------------------------------------------------------
  result.dead_block_count_global = lhs.dead_block_count_global - rhs.dead_block_count_global;
  result.total_valid_evictions_global = lhs.total_valid_evictions_global - rhs.total_valid_evictions_global;
  result.dead_block_count_percpu = lhs.dead_block_count_percpu - rhs.dead_block_count_percpu;
  result.total_valid_evictions_percpu = lhs.total_valid_evictions_percpu - rhs.total_valid_evictions_percpu;
  // ---------------------------------------------------------------

  result.misses = lhs.misses - rhs.misses;

  result.total_miss_latency_cycles = lhs.total_miss_latency_cycles - rhs.total_miss_latency_cycles;
  return result;
}