// cod6 - ReXGlue Recompiled Project
//
// Customize your app by overriding virtual hooks from rex::ReXApp.

#pragma once

#include <filesystem>
#include <cerrno>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdio>

#include <rex/filesystem.h>

#include <rex/logging.h>
#include <rex/rex_app.h>

#include "cod6_renderizador.h"

#if defined(__SWITCH__)
#include <switch.h>
#endif

extern "C" void RexSwitchWatchdogStart(void);



#if defined(__SWITCH__)
/*
 * RED_INIT
 *
 * En la tanda 57 el autotest dio:
 *   [net] NETSELFTEST socket(UDP) = -1 (errno=2)   // ENOENT
 * y ni este proyecto ni nfsmw-nx llaman nunca a socketInitializeDefault(), así
 * que el servicio bsd:u no está arrancado y socket() no puede crear nada. Eso es
 * justo lo que el juego reporta como
 *   "Error during initialization: WARNING: Couldn't allocate IP port".
 * En PC no hace falta inicializar nada, y por eso allí arranca sin problema.
 */
/*
 * COD6_MODO_APPLET
 *
 * appletGetAppletType(): 6 = aplicacion (title override o NSP), 3/4 = applet
 * (album / overlay). En modo applet el proceso solo consigue ~485 MB en vez de
 * ~920 para la memoria del invitado y el juego se queda sin dibujar.
 */
extern "C" size_t RexGmCommittedBytes(void);
extern "C" size_t RexGmMappedBytes(void);

static void RexSwitchLogModo(void) {
  const AppletType tipo = appletGetAppletType();
  const char* nombre = "desconocido";
  switch (tipo) {
    case AppletType_Application: nombre = "aplicacion (memoria completa)"; break;
    case AppletType_SystemApplication: nombre = "aplicacion de sistema"; break;
    case AppletType_LibraryApplet: nombre = "LibraryApplet (album) MEMORIA INSUFICIENTE"; break;
    case AppletType_OverlayApplet: nombre = "OverlayApplet MEMORIA INSUFICIENTE"; break;
    case AppletType_SystemApplet: nombre = "SystemApplet"; break;
    default: break;
  }
  REX_SWITCH_LOG("[cod6] MODO applet=%d (%s)  invitado=%zu/%zu MB\n", (int)tipo, nombre,
                 size_t(RexGmCommittedBytes() >> 20), size_t(RexGmMappedBytes() >> 20));
  if (tipo != AppletType_Application && tipo != AppletType_SystemApplication) {
    REX_SWITCH_LOG(
        "[cod6] MODO AVISO: lanzado como applet. Para memoria completa hay que abrir hbmenu con "
        "title override (mantener R y pulsar A sobre un juego).\n");
  }
}

static void RexSwitchInitNetwork() {
  Result rc = socketInitializeDefault();
  REX_SWITCH_LOG("[net] socketInitializeDefault() = 0x%08X%s\n", (unsigned)rc,
                 (rc == 0) ? "" : "  <-- FALLO");

  Result rcN = nifmInitialize(NifmServiceType_User);
  REX_SWITCH_LOG("[net] nifmInitialize() = 0x%08X%s\n", (unsigned)rcN,
                 (rcN == 0) ? "" : "  <-- FALLO");
}
#endif

/*
 * NETSELFTEST
 *
 * En PC el juego arranca y se puede jugar; en Switch sale al segundo con
 *   "Error during initialization: WARNING: Couldn't allocate IP port"
 * pero no llamó a socket(), bind() ni getsockname() (0 registros en las
 * tandas 49-56). Así que antes de seguir suponiendo, se comprueba la capacidad
 * real del sistema: crear un socket UDP, atarlo al puerto 0 y preguntar qué
 * puerto tocó. Si esto falla, la causa está en la capa de red; si funciona, el
 * mensaje del juego viene de otro sitio y hay que dejar de mirar la red.
 */
static void RexSwitchNetSelfTest() {
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  REX_SWITCH_LOG("[net] NETSELFTEST socket(UDP) = %d (errno=%d)\n", fd, errno);
  if (fd < 0) {
    return;
  }

  struct sockaddr_in dir{};
  dir.sin_family = AF_INET;
  dir.sin_addr.s_addr = htonl(INADDR_ANY);
  dir.sin_port = 0;

  int r = bind(fd, reinterpret_cast<struct sockaddr*>(&dir), sizeof(dir));
  REX_SWITCH_LOG("[net] NETSELFTEST bind(0) = %d (errno=%d)\n", r, errno);

  struct sockaddr_in salida{};
  socklen_t len = sizeof(salida);
  int g = getsockname(fd, reinterpret_cast<struct sockaddr*>(&salida), &len);
  REX_SWITCH_LOG("[net] NETSELFTEST getsockname = %d (errno=%d) puerto=%u\n", g, errno,
                 (unsigned)ntohs(salida.sin_port));

  close(fd);
  REX_SWITCH_LOG("[net] NETSELFTEST fin\n");
}

class Cod6App : public rex::ReXApp {
 public:
  using rex::ReXApp::ReXApp;

  static std::unique_ptr<rex::ui::WindowedApp> Create(
      rex::ui::WindowedAppContext& ctx) {
    return std::unique_ptr<Cod6App>(new Cod6App(ctx, "cod6",
        PPCImageConfig));
  }

  // On the Switch the title lives on the SD card with the game data beside it,
  // so nothing is typed on a command line:
  //
  //   sdmc:/switch/cod6-nx/cod6.nro
  //   sdmc:/switch/cod6-nx/game_root/   <- your own extracted game files
  //   sdmc:/switch/cod6-nx/user/        <- saves and settings
  //   sdmc:/switch/cod6-nx/cache/       <- shader cache
  //
  // A cvar from cod6.toml still wins: the cvars are read before this hook runs,
  // and only empty values are filled in here.
  void OnConfigurePaths(rex::PathConfig& paths) override {
#if defined(__SWITCH__)
    // Emulators start NROs with an empty argv[0], so GetExecutableFolder() can
    // come back empty and every path below would be relative -- the game data
    // root then "does not exist" even though it is right where it belongs.
    // hbloader on the console always passes the real NRO path, so this only
    // matters under emulation.
    std::filesystem::path exe_dir = rex::filesystem::GetExecutableFolder();
    if (exe_dir.empty() || !std::filesystem::exists(exe_dir / "cod6.nro")) {
      static const std::filesystem::path kFallbackInstallDir = "sdmc:/switch/cod6-nx";
      if (std::filesystem::exists(kFallbackInstallDir / "cod6.nro")) {
        exe_dir = kFallbackInstallDir;
      }
    }
    /*
     * Set unconditionally, not only when empty.
     *
     * The SDK fills user_data_root and cache_root with its own default (based on the
     * app name) before this runs, so an empty() check never fires and the two
     * environments end up reading different folders: under Ryujinx the log showed
     * user=/switch/cod6, on the console /switch/cod6-nx/cod6. The package layout is
     * game_root/user/cache next to the NRO, so that is what gets used and both agree.
     */
    paths.game_data_root = exe_dir / "game_root";
    paths.user_data_root = exe_dir / "user";
    paths.cache_root = exe_dir / "cache";
    /*
     * Create user/ and cache/ instead of relying on the package shipping them.
     *
     * The renderer keeps its translated SPIR-V and its driver pipeline cache in
     * cache/, and the game writes profiles into user/. Measured: the periodic report
     * always said "N shaders (0 cached)", i.e. nothing was ever read back - a cache
     * directory that does not exist fails silently. BO2 solves the same problem by
     * shipping a precompiled shader library (bo2zm_shaders.spvpack); we cannot
     * precompile, so at least the cache has to persist between runs.
     */
    {
      std::error_code ec;
      std::filesystem::create_directories(paths.user_data_root, ec);
      std::filesystem::create_directories(paths.cache_root, ec);
    }
    /*
     * The title update device.
     *
     * Measured on hardware: the game opens update:\patch.ff while booting. With no
     * update: device registered the log said
     *     VFS: 'update:\' -> [no device]
     *     [NtCreateFile] FAILED: path='update:\patch.ff' -> 0xc000000f
     * and the game took its dirty-disc path: sub_8236B630 (imports
     * XamShowDirtyDiscErrorUI) -> sub_8236B030 -> XamLoaderLaunchTitle, i.e. it
     * showed an error dialog and restarted the title, which the runtime turns into
     * TerminateTitle. That is the black screen: one frame drawn, then exit.
     *
     * A console without a title update still HAS the device and only lacks the
     * file, and those are different failures to the game. So point update: at a
     * real (empty) folder and let patch.ff be missing, exactly like a retail unit.
     */
    paths.update_data_root = exe_dir / "update";
    /*
     * Create it instead of shipping an empty folder.
     *
     * Measured on hardware: with the path set but the folder absent the log said
     *     Update data: /switch/cod6-nx/update
     *     VFS: 'update:\' -> [no device]
     * because the runtime only registers the update: device when the directory
     * exists. An empty folder in the package is easy to lose when copying, so the
     * program makes it itself.
     */
    {
      std::error_code ec;
      std::filesystem::create_directories(paths.update_data_root, ec);
    }
    if (paths.config_path.empty()) paths.config_path = exe_dir / "cod6.toml";
#else
    /* COD6_PC_PORT: the title update device the comment above describes is needed
     * on every platform, not only on the console. Measured on the Windows build:
     * without it the log says
     *     VFS: 'update:\' -> [no device]
     *     [NtCreateFile] FAILED: path='update:\patch.ff' -> 0xc000000f
     * and the game takes the dirty-disc path: one frame, then exit. That is the
     * black window. Point update: at a real (empty) folder so patch.ff is merely
     * missing, exactly like a retail console without a title update.
     */
    if (paths.update_data_root.empty()) {
      paths.update_data_root = rex::filesystem::GetExecutableFolder() / "update";
      std::error_code ec;
      std::filesystem::create_directories(paths.update_data_root, ec);
    }
    if (paths.config_path.empty()) {
      paths.config_path = rex::filesystem::GetExecutableFolder() / "cod6.toml";
    }
#endif
    // stderr already goes to logs/rex/rex_stderr.log through the SDK's crash
    // hook, so this shows up even when the log file sink is not up yet.
    std::fprintf(stderr, "[cod6] paths: game=%s user=%s cache=%s\n",
                 paths.game_data_root.string().c_str(),
                 paths.user_data_root.string().c_str(),
                 paths.cache_root.string().c_str());
    std::fflush(stderr);
  }

  // --- boot phase markers -------------------------------------------------
  // A black screen can mean a dozen different things; these lines say exactly
  // how far the app got before it stopped. Grep the log for "[cod6]".

  void OnPostInitLogging() override {
    REXLOG_INFO("[cod6] phase 1/9: logging ready");
  }

  void OnPreSetup(rex::RuntimeConfig& config) override {
    /*
     * Aqui se elige el renderizador. Si RuntimeConfig::graphics queda vacio, el
     * SDK carga el plugin que diga gpu_plugin ("xenos" = emulacion). Si lo
     * llenamos nosotros, se usa el nuestro y nadie carga el plugin. Es la misma
     * puerta que usa nfsmw-nx para sustituir la emulacion por su renderizador.
     */
    if (cod6::Cod6QuiereRenderizadorNativo()) {
      config.graphics = cod6::Cod6CrearGraphicsSystem();
      REX_SWITCH_LOG("[cod6] RENDERIZADOR nativo pedido en cod6.toml\n");
    } else {
      /*
       * COD6_PLUGIN_POR_DEFECTO
       *
       * Si el toml no se pudo leer (se corrompio en la tarjeta) no hay ningun cvar
       * aplicado y gpu_plugin queda vacio: el SDK no carga la emulacion de GPU,
       * no hay Vulkan ni hilos de GPU, y el juego se queda en negro a los 5 s sin
       * decir por que. Pasó en las tandas 156-159.
       */
      if (config.gpu_plugin.empty()) {
        config.gpu_plugin = "xenos";
        REX_SWITCH_LOG(
            "[cod6] AVISO: gpu_plugin vacio (cod6.toml ilegible?) -> se usa 'xenos' por defecto\n");
      }
      REX_SWITCH_LOG("[cod6] RENDERIZADOR xenos (emulacion, por defecto)\n");
    }
    REXLOG_INFO("[cod6] phase 2/9: creating runtime (guest memory, GPU init)");
  }

  void OnLoadXexImage(std::string& xex_image) override {
    REXLOG_INFO("[cod6] phase 3/9: loading guest image ({} bytes)", xex_image.size());
  }

  void OnPostLoadXexImage() override {
    REXLOG_INFO("[cod6] phase 4/9: guest image loaded");
  }

  void OnPostSetup() override {
    REXLOG_INFO("[cod6] phase 5/9: runtime ready (VFS mounted, GPU up)");
#if defined(__SWITCH__)
    RexSwitchInitNetwork();
    RexSwitchLogModo();  /* COD6_MODO_APPLET */
#endif
    RexSwitchNetSelfTest();
    /*
     * WATCHDOG_FIX: el vigilante de fotogramas y atascos se lanza AQUI, en el
     * hilo principal y antes de que arranque el invitado. Antes se lanzaba desde
     * el prologo de la primera funcion de invitado (__imp__xstart) y crear alli
     * un std::thread lanzaba std::system_error, que terminaba el proceso: esa era
     * la pantalla negra de las tandas 73 y 74.
     */
#if defined(__SWITCH__)  /* COD6_PC_PORT: solo existe en la SDK de Switch */
    RexSwitchWatchdogStart();
#endif
  }

  void OnPreLaunchModule() override {
    REXLOG_INFO("[cod6] phase 6/9: launching guest thread");
  }

  void OnPostLaunchModule(rex::system::XThread* thread) override {
    (void)thread;
    REXLOG_INFO("[cod6] phase 7/9: guest thread running");
  }

  void OnCreateDialogs(rex::ui::ImGuiDrawer* drawer) override {
    (void)drawer;
    REXLOG_INFO("[cod6] phase 8/9: ImGui drawer up (first frame is being drawn)");
  }

  bool OnWindowCloseRequested() override {
    REXLOG_INFO("[cod6] phase 9/9: window close requested");
    return true;
  }

  void OnShutdown() override {
    REXLOG_INFO("[cod6] shutdown");
  }
};
