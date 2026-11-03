// SPDX-License-Identifier: GPL-3.0-or-later
#include "cod6sp_pch.h"
// COD4-inspired policy, re-derived for the locked COD5 image/call chain.
#include "cod6_drawsurf_policy.h"
#include "cod6_render_hooks.h"
#include "cod6_lod_policy.h"

#include <rex/cvar.h>
#include <rex/memory/utils.h>

REXCVAR_DEFINE_BOOL(cod6_game_skip_depth_prepass, false, "COD5",
                    "Experimental fixed-XEX type-zero draw-list suppression; depth consumers require hardware validation")
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);
REXCVAR_DEFINE_BOOL(cod6_game_drawsurfs_diagnostics, false, "COD5",
                    "Count fixed-XEX draw lists by technique before selecting a prepass optimization")
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);
REXCVAR_DEFINE_UINT32(cod6_model_lod_rigid, 0, "COD5",
                      "Fixed-XEX rigid model LOD scale: 0 keeps the game's value; 1-4 selects less detail at distance "
                      "as the value increases. Quality/performance experiment; does not change saved settings")
    .allowed({"0","1","2","3","4"}).lifecycle(rex::cvar::Lifecycle::kInitOnly);
REXCVAR_DEFINE_UINT32(cod6_model_lod_skinned, 0, "COD5",
                      "Fixed-XEX skinned model LOD scale: 0 keeps the game's value; 1-4 selects less detail at distance "
                      "as the value increases. Quality/performance experiment; does not change saved settings")
    .allowed({"0","1","2","3","4"}).lifecycle(rex::cvar::Lifecycle::kInitOnly);

namespace {
bool skip_depth_prepass = false;
bool count_drawsurfs = false;
std::atomic<uint64_t> lists{0}, surfaces{0}, suppressed_lists{0}, suppressed_surfaces{0};
std::array<std::atomic<uint64_t>, 32> lists_by_technique{};
std::array<std::atomic<uint64_t>, 32> surfaces_by_technique{};
}

// Independent LOD experiment state; draw-list suppression does not depend on it.
namespace {
uint32_t lod_rigid=0,lod_skinned=0;
std::atomic<uint64_t> lod_calls{0},lod_rigid_changed{0},lod_skinned_changed{0},lod_invalid{0},lod_unsupported{0};
}

void cod6::game::ConfigureRenderHooks() {
  skip_depth_prepass = REXCVAR_GET(cod6_game_skip_depth_prepass);
  count_drawsurfs = REXCVAR_GET(cod6_game_drawsurfs_diagnostics);
  lod_rigid=REXCVAR_GET(cod6_model_lod_rigid);
  lod_skinned=REXCVAR_GET(cod6_model_lod_skinned);
  REXLOG_INFO("[game-opt] COD5 draw-surfs: type-zero suppression {}, list diagnostics {}",
              skip_depth_prepass, count_drawsurfs);
  REXLOG_INFO("[game-opt] COD5 model LOD scale: rigid {}, skinned {} (0 = original game setting; 1-4 = quality override)",
              lod_rigid,lod_skinned);
}

cod6::game::LodCounters cod6::game::ReadLodCounters() {
  return {lod_calls.load(std::memory_order_relaxed),lod_rigid_changed.load(std::memory_order_relaxed),
          lod_skinned_changed.load(std::memory_order_relaxed),lod_invalid.load(std::memory_order_relaxed),
          lod_unsupported.load(std::memory_order_relaxed)};
}

extern "C" REX_FUNC(__imp__sub_82405078);
extern "C" REX_FUNC(sub_82405078) {
  const uint32_t output=ctx.r4.u32;
  __imp__sub_82405078(ctx,base);
  if(!lod_rigid && !lod_skinned && !count_drawsurfs) return;
  static const bool supported=[&] {
    const bool ok=cod6::game::MatchesLodSetup(base+cod6::game::kLodSetupAddress);
    if(!ok) REXLOG_ERROR("[game-opt] COD5 LOD setup fingerprint mismatch; retaining original output");
    else REXLOG_INFO("[game-opt] COD5 82405078 LOD setup verified; output in entry r4; scale/base offsets C/14, 18/20");
    return ok;
  }();
  const uint64_t number=lod_calls.fetch_add(1,std::memory_order_relaxed);
  if(!supported) {lod_unsupported.fetch_add(1,std::memory_order_relaxed);return;}
  const auto load=[base](uint32_t address) {return rex::memory::load_and_swap<uint32_t>(base+address);};
  const auto store=[base](uint32_t address,uint32_t value) {rex::memory::store_and_swap<uint32_t>(base+address,value);};
  const auto result=cod6::game::ApplyLodScale(output,lod_rigid,lod_skinned,load,store);
  if(result.changed&1) lod_rigid_changed.fetch_add(1,std::memory_order_relaxed);
  if(result.changed&2) lod_skinned_changed.fetch_add(1,std::memory_order_relaxed);
  if(result.invalid) lod_invalid.fetch_add(1,std::memory_order_relaxed);
  if(number<8 && output>=0x10000 && uint64_t(output)+0x38<=UINT64_C(0x100000000)) {
    REXLOG_INFO("[game-opt] LOD output {:08X}: rigid {:.4f}, skinned {:.4f}, changed mask {}, invalid mask {}",
                output,std::bit_cast<float>(load(output+0xC)),std::bit_cast<float>(load(output+0x18)),
                result.changed,result.invalid);
  }
}

cod6::game::DrawSurfCounters cod6::game::ReadDrawSurfCounters() {
  DrawSurfCounters result;
  result.lists = lists.load(std::memory_order_relaxed);
  result.surfaces = surfaces.load(std::memory_order_relaxed);
  result.suppressed_lists = suppressed_lists.load(std::memory_order_relaxed);
  result.suppressed_surfaces = suppressed_surfaces.load(std::memory_order_relaxed);
  for (size_t i=0; i<result.lists_by_technique.size(); ++i) {
    result.lists_by_technique[i] = lists_by_technique[i].load(std::memory_order_relaxed);
    result.surfaces_by_technique[i] = surfaces_by_technique[i].load(std::memory_order_relaxed);
  }
  return result;
}

extern "C" REX_FUNC(__imp__sub_82422508);

// Override the Switch weak public thunk. __imp__ remains the original full
// generated body. No codegen/PCH mutation or guest-register substitution.
extern "C" REX_FUNC(sub_82422508) {
  if (!skip_depth_prepass && !count_drawsurfs) {
    __imp__sub_82422508(ctx, base);
    return;
  }
  static const bool supported = [&] {
    const bool ok = cod6::game::MatchesDrawSurfsPrologue(base + cod6::game::kDrawSurfsAddress);
    if (!ok) {
      REXLOG_ERROR("[game-opt] COD5 draw-surfs prologue mismatch; using the original path without suppression");
    } else {
      REXLOG_INFO("[game-opt] COD5 82422508 prologue verified; count +4, technique +8, list in r5");
    }
    return ok;
  }();
  if (!supported) {
    __imp__sub_82422508(ctx, base);
    return;
  }
  const auto load = [base](uint32_t address) {
    return rex::memory::load_and_swap<uint32_t>(base + address);
  };
  const auto store = [base](uint32_t address, uint32_t value) {
    rex::memory::store_and_swap<uint32_t>(base + address, value);
  };
  const uint32_t list = ctx.r5.u32;
  auto result = cod6::game::RunDrawSurfList(list, skip_depth_prepass, load, store,
                                         [&] { __imp__sub_82422508(ctx, base); });
  const auto number = lists.fetch_add(1, std::memory_order_relaxed);
  surfaces.fetch_add(result.surfaces, std::memory_order_relaxed);
  if (result.technique < lists_by_technique.size()) {
    lists_by_technique[result.technique].fetch_add(1, std::memory_order_relaxed);
    surfaces_by_technique[result.technique].fetch_add(result.surfaces, std::memory_order_relaxed);
  }
  if (result.suppressed) {
    suppressed_lists.fetch_add(1, std::memory_order_relaxed);
    suppressed_surfaces.fetch_add(result.surfaces, std::memory_order_relaxed);
  }
  if (number < 12) {
    REXLOG_INFO("[game-opt] list {:08X}: {} surfaces, technique {}, suppressed {}",
                list, result.surfaces, result.technique, result.suppressed);
  }
}
