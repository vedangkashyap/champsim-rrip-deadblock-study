#ifndef CACHE_STATS_H
#define CACHE_STATS_H

#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>

#include "channel.h"
#include "event_counter.h"

struct cache_stats {
  std::string name;
  // prefetch stats
  uint64_t pf_requested = 0;
  uint64_t pf_issued = 0;
  uint64_t pf_useful = 0;
  uint64_t pf_useless = 0;
  uint64_t pf_fill = 0;
  
  // ---------------------------------------------------------------
  // Name: VEDANGK
  // Reason: Renamed the old cache-wide dead_block_count/
  // total_valid_evictions to *_global (unambiguous name), and added
  // matching *_percpu event_counters (keyed the same way as hits/
  // misses below) so shared structures like the LLC can report a
  // dead-block percentage per core instead of one blended number.
  // ---------------------------------------------------------------
  uint64_t dead_block_count_global = 0;
  uint64_t total_valid_evictions_global = 0;

  champsim::stats::event_counter<std::remove_cv_t<decltype(NUM_CPUS)>> dead_block_count_percpu = {};
  champsim::stats::event_counter<std::remove_cv_t<decltype(NUM_CPUS)>> total_valid_evictions_percpu = {};
  // ---------------------------------------------------------------

  champsim::stats::event_counter<std::pair<access_type, std::remove_cv_t<decltype(NUM_CPUS)>>> hits = {};
  champsim::stats::event_counter<std::pair<access_type, std::remove_cv_t<decltype(NUM_CPUS)>>> misses = {};
  champsim::stats::event_counter<std::pair<access_type, std::remove_cv_t<decltype(NUM_CPUS)>>> mshr_merge = {};
  champsim::stats::event_counter<std::pair<access_type, std::remove_cv_t<decltype(NUM_CPUS)>>> mshr_return = {};

  long total_miss_latency_cycles{};
};

cache_stats operator-(cache_stats lhs, cache_stats rhs);

#endif