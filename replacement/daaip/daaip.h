#ifndef REPLACEMENT_DAAIP_H
#define REPLACEMENT_DAAIP_H

#include <cstdint>
#include <vector>

#include "cache.h"
#include "modules.h"

// One reuse/owner record per cache way, mirroring the layout of drrip's flat
// [set * NUM_WAY + way] indexed vectors.
struct daaip_block_meta {
  bool reuse_bit = false;      // false until the block receives a hit after fill
  uint32_t owner_cpu = 0;      // core that installed this block (log2(NUM_CPUS) bits worth of info)
};

struct daaip : public champsim::modules::replacement {
private:
  unsigned& get_rrpv(long set, long way);
  daaip_block_meta& get_meta(long set, long way);

public:
  // --- RRIP mechanics (shared with SRRIP) ---
  static constexpr unsigned maxRRPV = 3;

  // --- DAAIP tuning parameters (Section III-C / V-C of the paper) ---
  static constexpr uint32_t PHASE_LEN = 1U << 16;      // 2^16 LLC misses per phase (16-bit InsertedBlockCounter)
  static constexpr uint32_t COUNTER_MASK = PHASE_LEN - 1;
  static constexpr uint32_t THRESHOLD_PERCENT = 90;    // deadblock % threshold T

  enum class insertion_mode : uint8_t { LIVELY, LESS_LIVELY };

  long NUM_SET, NUM_WAY;

  std::vector<unsigned> rrpv;                  // flat [set * NUM_WAY + way], RRPV values
  std::vector<daaip_block_meta> block_meta;    // flat [set * NUM_WAY + way], reuse bit + owner core

  // Per-core state (Algorithm 1). Sized to NUM_CPUS in the constructor.
  std::vector<uint32_t> new_insertion_ctr;     // NewInsertionCtr: counts misses (installs) since last phase boundary
  std::vector<uint32_t> deadblock_ctr;         // DeadBlockCtr: counts dead evictions since last phase boundary
  std::vector<insertion_mode> mode;            // current insertion mode per core

  explicit daaip(CACHE* cache);

  // void initialize_replacement() {}
  long find_victim(uint32_t triggering_cpu, uint64_t instr_id, long set, const champsim::cache_block* current_set, champsim::address ip,
                   champsim::address full_addr, access_type type);

  // Called on every cache hit (see note in daaip.cc about why the miss branch
  // here is effectively dead code given CACHE::handle_fill's call order).
  void update_replacement_state(uint32_t triggering_cpu, long set, long way, champsim::address full_addr, champsim::address ip, champsim::address victim_addr,
                                access_type type, uint8_t hit);

  // Called on every cache fill, BEFORE the victim way is overwritten.
  // This is where DAAIP's Algorithm 1 (deadblock feedback + phase tracking +
  // adaptive insertion) actually lives.
  void replacement_cache_fill(uint32_t triggering_cpu, long set, long way, champsim::address full_addr, champsim::address ip, champsim::address victim_addr,
                              access_type type);

  // use this function to print out your own stats at the end of simulation
  // void replacement_final_stats() {}
};

#endif