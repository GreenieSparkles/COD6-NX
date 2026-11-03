/* COD6_PC_PORT: stubs de las funciones que en Switch aporta la SDK.
 *
 * El codigo del juego (parcheado con el registro de "funcion de invitado") y el
 * nuestro (instrumentacion COLA / PREP4 / FENCE) llaman a RexSwitch*; en Switch
 * las define sdk/src/core/exception_handler_switch.cpp, que no compila fuera de
 * libnx. Aqui van como no-ops para que el PC enlace.
 */
#if !defined(__SWITCH__)

#include <cstdint>
#include <cstddef>

extern "C" {

void RexSwitchGuestFnSet(const char* nombre) { (void)nombre; }
void RexSwitchDrawTick(void) {}
void RexSwitchPresentTick(void) {}
uint32_t RexSwitchFallosTotal(void) { return 0; }

/* La SDK de Switch tambien exporta estas; por si el codigo comun las pide. */
void RexSwitchPerfCount(unsigned id) { (void)id; }
void RexSwitchPerfAdd(unsigned id, uint64_t valor) { (void)id; (void)valor; }
void RexSwitchPerfMax(unsigned id, uint64_t valor) { (void)id; (void)valor; }
int RexSwitchPerfSkip(unsigned bit) { (void)bit; return 0; }
const char* RexSwitchLogDir(void) { return "./logs/"; }

} /* extern "C" */

/* El log extendido (write a un fd extra) solo existe en la SDK de Switch. */
int g_rex_log_extra_fd = -1;

#endif /* !__SWITCH__ */
