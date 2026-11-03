// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <cstdint>
#ifndef VK_NO_PROTOTYPES
#define VK_NO_PROTOTYPES
#endif
// Match rex/ui/vulkan/api.h even when this helper is included first. Defining
// beta extensions after vulkan_core.h was already parsed leaves Vulkan-Hpp
// referring to extension constants that were not declared by the C headers.
#ifndef VK_ENABLE_BETA_EXTENSIONS
#define VK_ENABLE_BETA_EXTENSIONS
#endif
// Only include core types here. The SDK API wrapper defines the platform
// surface macros before including vulkan.h; consuming that umbrella header
// here would prevent its later VI_NN declarations from being loaded.
#include <vulkan/vulkan_core.h>

namespace cod6::native {

// Consecutive-binding writes require identical stageFlags. The system, vertex,
// pixel, loop and fetch constant bindings do not all have the same visibility,
// so describe each binding separately (VUID-VkWriteDescriptorSet-descriptorCount-00317).
inline void MakeConstantDescriptorWrites(VkDescriptorSet set,
                                         const VkDescriptorBufferInfo* infos,
                                         uint32_t count, VkWriteDescriptorSet* writes) {
  for (uint32_t i = 0; i < count; ++i) {
    writes[i] = {};
    writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[i].dstSet = set;
    writes[i].dstBinding = i;
    writes[i].descriptorCount = 1;
    writes[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    writes[i].pBufferInfo = &infos[i];
  }
}

}  // namespace cod6::native
