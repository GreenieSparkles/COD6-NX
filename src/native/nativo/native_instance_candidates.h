// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <algorithm>
#include <cstdint>

namespace cod6::native {

// Record-only classifier for the COD4-style rigid/static instancing path.
//
// A real merge will need an instanced shader variant and an instance-constant
// buffer.  Until those exist, this class only counts adjacent draws for which
// every observed dependency is equal except vertex float constants c4-c8.  The
// renderer uses existing same-submission upload offsets for large immutable
// blocks and hashes only the split VS constants. Different upload offsets may
// conservatively hide a candidate, but an allocation cannot manufacture one.
// Hash collisions can only affect diagnostics; no rendering decision is made
// from this class.
struct InstanceCandidateSignature {
  uint64_t frame = 0;
  uint64_t submission = 0;
  uint64_t pass = 0;
  uint64_t pipeline = 0;
  uint64_t layout = 0;
  uint64_t targets = 0;
  uint64_t geometry = 0;
  uint64_t textures = 0;
  uint64_t fixed_constants = 0;
  uint64_t dynamic_state = 0;
  uint64_t instance_constants = 0;
  bool uses_instance_constants = false;
};

struct InstanceCandidateCounters {
  uint64_t draws = 0;
  uint64_t eligible = 0;
  uint64_t groups = 0;
  uint64_t reducible = 0;
  uint64_t instance_changed = 0;
  uint64_t instance_same = 0;
  uint64_t break_context = 0;
  uint64_t break_no_instance = 0;
  uint64_t break_pipeline_state = 0;
  uint64_t break_geometry = 0;
  uint64_t break_textures = 0;
  uint64_t break_fixed_constants = 0;
  uint64_t break_dynamic_state = 0;
  uint64_t max_group = 0;
};

class InstanceCandidateTracker {
 public:
  void Observe(const InstanceCandidateSignature& current) {
    ++counters_.draws;
    if (!current.uses_instance_constants) {
      ++counters_.break_no_instance;
      have_previous_ = false;
      group_size_ = 0;
      return;
    }
    ++counters_.eligible;
    if (!have_previous_) {
      previous_ = current;
      have_previous_ = true;
      group_size_ = 1;
      return;
    }

    if (current.frame != previous_.frame ||
        current.submission != previous_.submission || current.pass != previous_.pass) {
      ++counters_.break_context;
      Restart(current);
      return;
    }
    if (current.pipeline != previous_.pipeline || current.layout != previous_.layout ||
        current.targets != previous_.targets) {
      ++counters_.break_pipeline_state;
      Restart(current);
      return;
    }
    if (current.geometry != previous_.geometry) {
      ++counters_.break_geometry;
      Restart(current);
      return;
    }
    if (current.textures != previous_.textures) {
      ++counters_.break_textures;
      Restart(current);
      return;
    }
    if (current.fixed_constants != previous_.fixed_constants) {
      ++counters_.break_fixed_constants;
      Restart(current);
      return;
    }
    if (current.dynamic_state != previous_.dynamic_state) {
      ++counters_.break_dynamic_state;
      Restart(current);
      return;
    }

    if (group_size_ == 1) ++counters_.groups;
    ++group_size_;
    ++counters_.reducible;
    counters_.max_group = std::max<uint64_t>(counters_.max_group, group_size_);
    if (current.instance_constants == previous_.instance_constants) {
      ++counters_.instance_same;
    } else {
      ++counters_.instance_changed;
    }
    previous_ = current;
  }

  const InstanceCandidateCounters& counters() const { return counters_; }

 private:
  void Restart(const InstanceCandidateSignature& current) {
    previous_ = current;
    group_size_ = 1;
  }

  InstanceCandidateSignature previous_{};
  InstanceCandidateCounters counters_{};
  uint64_t group_size_ = 0;
  bool have_previous_ = false;
};

}  // namespace cod6::native
