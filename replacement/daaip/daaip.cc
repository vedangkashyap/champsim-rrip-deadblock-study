#include "daaip.h"

#include <algorithm>
#include <cassert>

#include "champsim.h"

daaip::daaip(CACHE* cache)
    : replacement(cache), NUM_SET(cache->NUM_SET), NUM_WAY(cache->NUM_WAY),
      rrpv(static_cast<std::size_t>(NUM_SET * NUM_WAY), maxRRPV),
      block_meta(static_cast<std::size_t>(NUM_SET * NUM_WAY)),
      new_insertion_ctr(NUM_CPUS, 0),
      deadblock_ctr(NUM_CPUS, 0),
      mode(NUM_CPUS, insertion_mode::LIVELY) // start lively: paper installs at LRU-1 until a phase proves otherwise
{
}

unsigned& daaip::get_rrpv(long set, long way) { return rrpv.at(static_cast<std::size_t>(set * NUM_WAY + way)); }

daaip_block_meta& daaip::get_meta(long set, long way) { return block_meta.at(static_cast<std::size_t>(set * NUM_WAY + way)); }

// find replacement victim -- identical to SRRIP: age everyone toward maxRRPV,
// evict the maxRRPV line. DAAIP never touches victim SELECTION, only
// insertion, per the paper (Section III-C).
long daaip::find_victim(uint32_t triggering_cpu, uint64_t instr_id, long set, const champsim::cache_block* current_set, champsim::address ip,
                        champsim::address full_addr, access_type type)
{
  auto begin = std::next(std::begin(rrpv), set * NUM_WAY);
  auto end = std::next(begin, NUM_WAY);

  auto victim = std::max_element(begin, end);
  if (auto diff = maxRRPV - *victim; diff != 0) {
    for (auto it = begin; it != end; ++it) {
      *it += diff;
    }
  }

  assert(begin <= victim);
  assert(victim < end);
  return std::distance(begin, victim);
}

// Called only on hits in this ChampSim build (CACHE::try_hit is the only
// caller of impl_update_replacement_state -- see CACHE::handle_fill, which
// calls impl_replacement_cache_fill instead on the miss path). The miss
// branch below is kept as a defensive fallback in case that call graph ever
// changes, but it should not fire in normal operation.
void daaip::update_replacement_state(uint32_t triggering_cpu, long set, long way, champsim::address full_addr, champsim::address ip,
                                     champsim::address victim_addr, access_type type, uint8_t hit)
{
  if (way < 0 || way >= NUM_WAY) {
    return;
  }

  if (hit) {
    get_rrpv(set, way) = 0;                 // promote to MRU on re-reference
    get_meta(set, way).reuse_bit = true;     // block has now been re-used at least once
    return;
  }

  // Defensive fallback only -- see comment above. Mirrors the LESS_LIVELY
  // (LRU) insertion point without touching phase/deadblock accounting, since
  // that accounting is owned by replacement_cache_fill.
  get_rrpv(set, way) = maxRRPV;
}

// Called on every cache fill, before *way is overwritten with the incoming
// block (CACHE::handle_fill, line ordering: replacement_cache_fill() ->
// [dead-block bookkeeping in cache.cc] -> *way = fill_block(...)).
//
// This is where Algorithm 1 from the paper lives in full:
//   1. Feedback: was the block being evicted from this way ever reused?
//      -> attribute a dead/live outcome to the core that installed it.
//   2. Phase tracking: has this core crossed a phase boundary (PHASE_LEN
//      misses)? If so, compute % deadblocks, compare to threshold, set mode,
//      halve both counters.
//   3. Insertion: write the new block's RRPV according to current mode, and
//      reset its reuse bit / owner tag.
void daaip::replacement_cache_fill(uint32_t triggering_cpu, long set, long way, champsim::address full_addr, champsim::address ip,
                                   champsim::address victim_addr, access_type type)
{
  // CACHE::handle_fill calls impl_replacement_cache_fill() unconditionally,
  // even when way == set_end (a bypass -- no free way was found/selected,
  // way_idx == NUM_WAY). There is no real cache_block slot in that case, so
  // rrpv/block_meta must not be touched for it. However, the request itself
  // is still a genuine LLC miss from triggering_cpu's point of view, and
  // Algorithm 1 (Section III-C) defines a phase in terms of "misses to the
  // LLC" -- so a bypass still needs to count toward NewInsertionCtr/mode
  // tracking, it just cannot carry deadblock feedback (no prior occupant
  // metadata exists for a slot that was never written) or receive an
  // insertion RRPV write (no slot to write into).
  const bool is_bypass = (way < 0 || way >= NUM_WAY);

  // --- Step 1: feedback from the block being evicted out of this way ---
  // victim_addr is default-constructed/empty when there was no valid
  // occupant (see CACHE::handle_fill: evicting_address stays {} unless
  // way->valid was true). We use that as the "was there really a victim"
  // signal, consistent with how cache.cc itself gates total_valid_evictions.
  if (!is_bypass && victim_addr != champsim::address{}) {
    auto& meta = get_meta(set, way);
    auto owner = meta.owner_cpu;
    if (owner < NUM_CPUS && !meta.reuse_bit) {
      deadblock_ctr[owner]++;
    }
  }

  // --- Step 2: this fill is itself a new install/miss for triggering_cpu ---
  // Counts even on bypass: a bypass is still a genuine LLC miss.
  auto core = triggering_cpu;
  if (core < NUM_CPUS) {
    new_insertion_ctr[core]++;

    if (new_insertion_ctr[core] >= PHASE_LEN) {
      uint32_t pct_dead = (new_insertion_ctr[core] == 0) ? 0 : (deadblock_ctr[core] * 100U) / new_insertion_ctr[core];

      mode[core] = (pct_dead > THRESHOLD_PERCENT) ? insertion_mode::LESS_LIVELY : insertion_mode::LIVELY;

      new_insertion_ctr[core] /= 2;
      deadblock_ctr[core] /= 2;
    }
  }

  // --- Step 3: adaptive insertion -- only if there is a real way to write into ---
  if (!is_bypass) {
    auto& meta = get_meta(set, way);
    get_rrpv(set, way) = (mode[core < NUM_CPUS ? core : 0] == insertion_mode::LESS_LIVELY) ? maxRRPV : (maxRRPV - 1);
    meta.reuse_bit = false;
    meta.owner_cpu = triggering_cpu;
  }
}