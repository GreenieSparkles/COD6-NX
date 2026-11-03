#version 450
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copies a rectangle of a depth buffer into the R32_SFLOAT image the
// translated shaders sample a depth resolve (k_24_8, k_24_8_FLOAT) as, in one
// pass. Reading the depth through a buffer with two copies cost about 1 ms of
// GPU time per full-screen resolve on this GPU; this is a small fraction.
//
// Rebuild depth_to_float_cs.h after editing:
//   glslangValidator -S comp -V --target-env vulkan1.1
//     --vn kDepthToFloatCs -o depth_to_float_cs.h depth_to_float.cs.glsl
#extension GL_EXT_samplerless_texture_functions : require
layout(local_size_x = 8, local_size_y = 8) in;
layout(set = 0, binding = 0) uniform texture2D depth_buffer;
layout(set = 0, binding = 1, r32f) uniform writeonly image2D dest;
layout(push_constant) uniform Params {
  ivec2 source_origin;
  ivec2 dest_origin;
  uvec2 source_size;
  uvec2 dest_size;
};
void main() {
  uvec2 p = gl_GlobalInvocationID.xy;
  if (p.x >= dest_size.x || p.y >= dest_size.y) {
    return;
  }
  // Pixel-center nearest mapping for the only ratios accepted by the host
  // cvar. Do not use the generic source/destination integer division here:
  // cod5sp_008 measured it at roughly 8-13x the old depth-resolve cost on
  // Tegra X1 / NVK. The exact mappings are:
  //   75%: floor((p + 0.5) * 3 / 4) == (3*p + 1) >> 2
  //   50%: floor((p + 0.5) * 1 / 2) == p >> 1
  // Identity copies still map p to p exactly.
  uvec2 source_p = p;
  if (all(equal(source_size * 4u, dest_size * 3u))) {
    source_p = (p * 3u + 1u) >> 2u;
  } else if (all(equal(source_size * 2u, dest_size))) {
    source_p = p >> 1u;
  }
  source_p = min(source_p, source_size - uvec2(1u));
  ivec2 source_c = source_origin + ivec2(source_p);
  ivec2 dest_c = dest_origin + ivec2(p);
  imageStore(dest, dest_c, vec4(texelFetch(depth_buffer, source_c, 0).r));
}
