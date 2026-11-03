#pragma once

#include <cstdint>

#include <rex/ppc/context.h>

// Check-only generated sources compare vector values by their two exact
// 64-bit lanes. Fail closed if the SDK representation changes.
static_assert(sizeof(PPCVRegister) == 16);

extern "C" void cod6_local_reg_check_fail(
    const char* function,
    const char* callee,
    uint32_t site,
    const char* register_name);

void cod6_local_reg_check_startup();
void cod6_local_reg_check_shutdown();
