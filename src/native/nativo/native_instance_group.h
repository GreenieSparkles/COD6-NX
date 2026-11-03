#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>

namespace cod6::native {

// Holds at most one adjacent instance group. Signature equality is supplied by
// the renderer and must be exact; this class never hashes, reorders or scans
// past an incompatible draw. The emitter owns buffer allocation and command
// submission so those platform-specific lifetimes stay outside this helper.
template <typename Command, typename Signature, typename Record,
          std::size_t MaxInstances>
class AdjacentInstanceGroup {
 public:
  static_assert(MaxInstances >= 2);

  template <typename Emit>
  void Offer(const Command& command, const Signature& signature,
             const Record& record, Emit&& emit) {
    if (!count_) {
      Start(command, signature, record);
      return;
    }
    if (signature == signature_ && count_ < MaxInstances) {
      records_[count_++] = record;
      return;
    }
    Flush(std::forward<Emit>(emit));
    Start(command, signature, record);
  }

  template <typename Emit>
  void Flush(Emit&& emit) {
    if (!count_) return;
    emit(command_, records_.data(), count_);
    count_ = 0;
  }

  bool pending() const { return count_ != 0; }
  std::uint32_t count() const { return count_; }

 private:
  void Start(const Command& command, const Signature& signature,
             const Record& record) {
    command_ = command;
    signature_ = signature;
    records_[0] = record;
    count_ = 1;
  }

  Command command_{};
  Signature signature_{};
  std::array<Record, MaxInstances> records_{};
  std::uint32_t count_ = 0;
};

}  // namespace cod6::native
