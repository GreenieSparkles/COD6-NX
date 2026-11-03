// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace cod6::native {

// A queued conversion must not borrow guest memory. The ring can acknowledge
// consumption before the worker runs, allowing the game to recycle that memory.
// Capture on the recording thread and keep only owned bytes in the worker job.
class TextureUploadSource {
 public:
  void Capture(const uint8_t* base, size_t base_size,
               const uint8_t* mips, size_t mip_size) {
    if (base && base_size) base_.assign(base, base + base_size);
    else base_.clear();
    if (mips && mip_size) mips_.assign(mips, mips + mip_size);
    else mips_.clear();
  }

  const uint8_t* base() const { return base_.empty() ? nullptr : base_.data(); }
  const uint8_t* mips() const { return mips_.empty() ? nullptr : mips_.data(); }
  size_t size() const { return base_.size() + mips_.size(); }

  // Conversion is complete; GPU copies use staging, not these source bytes.
  void Release() {
    std::vector<uint8_t>().swap(base_);
    std::vector<uint8_t>().swap(mips_);
  }

 private:
  std::vector<uint8_t> base_, mips_;
};

}  // namespace cod6::native
