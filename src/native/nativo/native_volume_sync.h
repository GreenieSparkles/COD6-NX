// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>

namespace cod6::native {
struct VolumeUploadState {bool initialized=false;uint64_t content=0,checked_frame=0;};
enum class VolumeUpdate {kCurrent,kUpdated,kRetry};
// Capture owns the source before Prepare is allowed to submit/wait/recycle
// guest storage. Hash and Record must both use that same captured snapshot.
// Only successful recording commits the GPU image's content identity.
template<class Capture,class Hash,class Prepare,class Record>
VolumeUpdate UpdateVolumeSnapshot(VolumeUploadState& state,uint64_t frame,uint64_t interval,
                                  Capture capture,Hash hash,Prepare prepare,Record record) {
  if(state.initialized && frame>=state.checked_frame && frame-state.checked_frame<interval)
    return VolumeUpdate::kCurrent;
  if(!capture())return VolumeUpdate::kRetry;
  const uint64_t observed=hash();
  if(state.initialized && state.content==observed) {
    state.checked_frame=frame;return VolumeUpdate::kCurrent;
  }
  if(!prepare() || !record(state.initialized))return VolumeUpdate::kRetry;
  state={true,observed,frame};
  return VolumeUpdate::kUpdated;
}
} // namespace cod6::native
