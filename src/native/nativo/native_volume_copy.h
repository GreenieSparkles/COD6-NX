// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <limits>
#include <vector>

namespace cod6::native {
// Exact runs of adjacent source bytes in linear output order. Guest tiling is
// evaluated once for this immutable fetch layout, never approximated by a
// hardcoded assumption about Xenos microtiles.
class VolumeCopyPlan {
 public:
  struct Run {uint32_t source,bytes;};
  template<class Offset> bool Build(uint32_t width,uint32_t height,uint32_t depth,uint32_t bpb,
                                   uint64_t source_bytes,Offset offset) {
    runs_.clear();bpb_=0;bytes_=0;
    if(!width || !height || !depth || (bpb!=1 && bpb!=2 && bpb!=4 && bpb!=8))return false;
    const uint64_t pixels=uint64_t(width)*height;
    if(pixels>UINT32_MAX/uint64_t(depth)/bpb)return false;
    const uint64_t bytes=pixels*depth*bpb;
    if(bytes>source_bytes || source_bytes>UINT32_MAX)return false;
    for(uint32_t z=0;z<depth;++z)for(uint32_t y=0;y<height;++y)for(uint32_t x=0;x<width;++x) {
      const uint64_t source=uint64_t(offset(x,y,z));
      if(source>source_bytes || bpb>source_bytes-source) {runs_.clear();return false;}
      if(!runs_.empty() && uint64_t(runs_.back().source)+runs_.back().bytes==source) {
        runs_.back().bytes+=bpb;
      } else runs_.push_back({uint32_t(source),bpb});
    }
    bpb_=bpb;bytes_=uint32_t(bytes);return true;
  }
  template<class Swap16,class Swap32>
  void Copy(const uint8_t* source,uint8_t* destination,Swap16 swap16,Swap32 swap32)const {
    uint8_t* cursor=destination;
    for(const auto& run:runs_) {
      const uint8_t* in=source+run.source;uint8_t* out=cursor;cursor+=run.bytes;
      if(bpb_==1) {std::memcpy(out,in,run.bytes);continue;}
      if(bpb_==2) {
        for(uint32_t i=0;i<run.bytes;i+=2) {uint16_t v;std::memcpy(&v,in+i,2);v=swap16(v);std::memcpy(out+i,&v,2);}
      } else {
        for(uint32_t i=0;i<run.bytes;i+=4) {uint32_t v;std::memcpy(&v,in+i,4);v=swap32(v);std::memcpy(out+i,&v,4);}
      }
    }
  }
  uint32_t bytes()const{return bytes_;}
  std::size_t run_count()const{return runs_.size();}
  std::size_t metadata_bytes()const{return runs_.size()*sizeof(Run);}
 private:
  std::vector<Run> runs_;
  uint32_t bpb_=0,bytes_=0;
};
} // namespace cod6::native
