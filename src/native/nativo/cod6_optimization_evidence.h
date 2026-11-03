// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <rex/logging.h>
#include <rex/cvar.h>
#include <cstdint>
#include "cod6_diagnostics.h"
#include "cod6_file_writes.h"
#include "cod6_wait_diagnostics.h"

REXCVAR_DECLARE(bool, cod6_cpu_skinning);
REXCVAR_DECLARE(bool, cod6_cpu_skinning_verify_only);
REXCVAR_DECLARE(bool, cod6_lightgrid_pack);
REXCVAR_DECLARE(bool, cod6_lightgrid_pack_verify_only);
REXCVAR_DECLARE(bool, cod6_native_zero_fill);
REXCVAR_DECLARE(bool, cod6_native_zero_fill_verify_only);
REXCVAR_DECLARE(bool, cod6_worker_idle_wait);
REXCVAR_DECLARE(bool, cod6_worker_stalled_wait);
REXCVAR_DECLARE(uint32_t, cod6_model_lod_rigid);
REXCVAR_DECLARE(uint32_t, cod6_model_lod_skinned);
REXCVAR_DECLARE(bool, cod6_native_vertex_by_writes);
REXCVAR_DECLARE(bool, cod6_native_vertex_by_writes_verify_only);
REXCVAR_DECLARE(bool, cod6_native_record_thread);
REXCVAR_DECLARE(bool, cod6_native_record_batch_publish);
REXCVAR_DECLARE(bool, cod6_native_vertex_sync_cache);
REXCVAR_DECLARE(bool, cod6_ppc_thread_local_msr);
#if defined(COD6_SWITCH_ZERO_WAIT_OVERLAY)
REXCVAR_DECLARE(bool, cod6_switch_zero_wait_fast_poll);
#endif

// native_textures.cpp defines this cvar inside cod6::native, not globally.
namespace cod6::native {
REXCVAR_DECLARE(bool, cod6_native_volume_copy_plan);
inline bool VolumeCopyPlanEnabledForEvidence() {
  return REXCVAR_GET(cod6_native_volume_copy_plan);
}
}

#ifndef COD6_BUILD_REVISION
#define COD6_BUILD_REVISION "unknown"
#endif
#ifndef COD6_BUILD_DIRECT_CALLS
#define COD6_BUILD_DIRECT_CALLS -1
#endif
#ifndef COD6_BUILD_FIXED_FLUSH
#define COD6_BUILD_FIXED_FLUSH 0
#endif

namespace cod6::evidence {
inline void Begin() {
  if constexpr (!cod6::diagnostics::kEnabled) return;
  REXLOG_INFO("[opt-evidence] begin schema=1 revision={} direct_calls={} fixed_flush={} "
              "xex=72378BCD351E56532AD0F565A0E192DA8F5AD314B0844AB0ED67B3A6B68771E0",
              COD6_BUILD_REVISION, COD6_BUILD_DIRECT_CALLS, COD6_BUILD_FIXED_FLUSH);
  REXLOG_INFO("[fixed-flush] schema=1 enabled={} policy=arm-fz-fz16-always-on "
              "scalar_subnormal_semantics=changed hardware_acceptance=unverified",
              COD6_BUILD_FIXED_FLUSH);
  REXLOG_INFO("[opt-evidence] policy counters=cumulative cadence_seconds=10 "
              "comparison=sampled visual_acceptance=external performance_acceptance=external");
  REXLOG_INFO("[opt-evidence] static id=ppc_interrupt thread_local_requested={} "
              "call_count=not_instrumented runtime_acceptance=unverified",
              REXCVAR_GET(cod6_ppc_thread_local_msr));
}

// Called only by the asynchronous periodic report consumer. No per-draw logging
// or new hot-path atomics. Values are immutable captured counter snapshots;
// in-flight calls can temporarily leave compared+fast below calls.
template<class Skin, class Light, class Fill, class Worker, class Lod, class Render,
         class File, class Wait>
void Report(uint64_t sample, uint64_t frames, const Skin& skin, const Light& light,
            const Fill& fill, const Worker& worker, const Lod& lod, const Render& r,
            bool write_healthy, const File& file, const Wait& wait) {
  if constexpr (!cod6::diagnostics::kEnabled) return;
  REXLOG_INFO("[opt-evidence] sample={} frames={} counters=cumulative",sample,frames);
  REXLOG_INFO("[opt-evidence] id=skinning enabled={} verify={} entries=82446A98,82446C28,82446E18,82447048 "
              "calls={} compared={} fast={} fallback={} mismatch={} unit=vertices",
              REXCVAR_GET(cod6_cpu_skinning),REXCVAR_GET(cod6_cpu_skinning_verify_only),
              skin.calls,skin.compared_vertices,skin.fast_vertices,skin.fallbacks,skin.mismatches);
  REXLOG_INFO("[opt-evidence] id=lightgrid enabled={} verify={} entries=82429DE0 "
              "calls={} compared={} fast={} fallback={} mismatch={} unit=calls coverage=partial",
              REXCVAR_GET(cod6_lightgrid_pack),REXCVAR_GET(cod6_lightgrid_pack_verify_only),
              light.calls,light.compared,light.fast,light.fallbacks,light.mismatches);
  REXLOG_INFO("[opt-evidence] id=zero_fill enabled={} verify={} entries=8237AB98 "
              "calls={} compared={} fast={} fallback={} mismatch={} unit=calls",
              REXCVAR_GET(cod6_native_zero_fill),REXCVAR_GET(cod6_native_zero_fill_verify_only),
              fill.calls,fill.compared,fill.fast,fill.fallbacks,fill.mismatches);
  REXLOG_INFO("[opt-evidence] id=worker_idle enabled={} entries=8242CEA0,8242CF40 "
              "calls={} fast={} unsupported={} errors={} comparison=none",
              worker.active,worker.scans,worker.parks,worker.unsupported,worker.errors);
  REXLOG_INFO("[opt-evidence] id=worker_stalled enabled={} entries=8242CEA0,8242CF40 "
              "calls={} fast={} errors={} comparison=none",
              worker.active&&REXCVAR_GET(cod6_worker_stalled_wait),
              worker.scans,worker.stalled_parks,worker.errors);
  REXLOG_INFO("[opt-evidence] id=lod enabled={} entries=82405078 calls={} fast={} "
              "invalid={} unsupported={} comparison=none quality_change=true",
              REXCVAR_GET(cod6_model_lod_rigid)>0 || REXCVAR_GET(cod6_model_lod_skinned)>0,
              lod.calls,lod.rigid_changed+lod.skinned_changed,lod.invalid,lod.unsupported);
  REXLOG_INFO("[opt-evidence] id=write_cache enabled={} verify={} calls={} compared={} fast={} "
              "mismatch={} busy={} healthy={} coverage=partial",
              REXCVAR_GET(cod6_native_vertex_by_writes),REXCVAR_GET(cod6_native_vertex_by_writes_verify_only),
              r.vertex_write_compared+r.vertex_write_hits+r.vertex_write_busy,
              r.vertex_write_compared,r.vertex_write_hits,r.vertex_write_mismatches,r.vertex_write_busy,write_healthy);
  REXLOG_INFO("[opt-evidence] id=recorder enabled={} batched={} calls={} fast={} "
              "published={} batches={} capacity_waits={} comparison=none",
              REXCVAR_GET(cod6_native_record_thread),REXCVAR_GET(cod6_native_record_batch_publish),
              r.record_queued,r.record_executed,r.record_publications,r.record_batches,r.record_capacity_waits);
  REXLOG_INFO("[opt-evidence] id=volume_plan enabled={} calls={} retries={} comparison=none",
              cod6::native::VolumeCopyPlanEnabledForEvidence(),r.volume_uploads,r.volume_retries);
  REXLOG_INFO("[opt-evidence] id=file_reads enabled={} entries=SDK_ReadInternal calls={} "
              "bytes={} invalid={} comparison=none coverage=physical_reads",
              REXCVAR_GET(cod6_native_vertex_by_writes),file.calls,file.bytes,file.invalid);
  REXLOG_INFO("[opt-evidence] id=vertex_sync enabled={} calls={} rechecks={} comparison=none",
              REXCVAR_GET(cod6_native_vertex_sync_cache),r.guest_cache_syncs,r.vertex_sync_rechecks);
#if defined(COD6_SWITCH_ZERO_WAIT_OVERLAY)
  REXLOG_INFO("[opt-evidence] id=zero_wait enabled={} entries=SDK_PosixConditionBase_Wait "
              "calls={} fast={} fallback={} ready={} contended={} comparison=none",
              REXCVAR_GET(cod6_switch_zero_wait_fast_poll),wait.fast_empty+wait.legacy_empty+wait.ready+wait.contended,
              wait.fast_empty,wait.legacy_empty,wait.ready,wait.contended);
#else
  (void)wait;
#endif
}
} // namespace cod6::evidence
