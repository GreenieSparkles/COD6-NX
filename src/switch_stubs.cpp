/* Switch build stubs for symbols libnx does not provide.
 *
 * The GPU plugin is linked for real now (GPU_PLUGINS xenos + NVK), so
 * rex_gpu_create / rex_gpu_abi_version come from src/graphics/plugin_main.cpp
 * and must NOT be stubbed here.
 */
#include <cstddef>
#include <cstdint>
#include <cstdlib>

extern "C" {

/* FFmpeg's av_cpu_count asks for the online CPU count; libnx has no sysconf. */
#ifndef _SC_NPROCESSORS_ONLN
#define _SC_NPROCESSORS_ONLN 84
#endif
long sysconf(int name) {
    if (name == _SC_NPROCESSORS_ONLN) return 4;
    return -1;
}

} /* extern "C" */
