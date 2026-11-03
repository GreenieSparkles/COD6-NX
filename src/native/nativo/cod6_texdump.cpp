// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * COD6_TEXDUMP
 *
 * Llevamos horas mirando cvars. Esto vuelca a disco los bytes CRUDOS que el
 * invitado tiene en la memoria de una textura DXT grande, para decodificarlos
 * fuera de la consola y ver de una vez si el problema es:
 *
 *   a) los bytes del invitado ya son basura  -> leemos la direccion equivocada,
 *      o el juego aun no los ha escrito;
 *   b) los bytes del invitado son DXT validos -> el fallo esta en nuestro
 *      untiling / subida / muestreo.
 *
 * Se vuelca la cabecera (direccion, formato, tamano, tiled, pitch) y los bytes.
 * Formato del fichero: una linea de texto por textura y luego los bytes crudos.
 */
#include <rex/cvar.h>
#include <rex/logging.h>
#include <rex/memory/utils.h>
#include <rex/ppc/function.h>
#include <cstdio>
#include <cstring>

REXCVAR_DEFINE_UINT32(cod6_texdump_count, 0, "COD6",
    "Volcar los bytes crudos de esta many texturas DXT grandes (0 = apagado)")
    .lifecycle(rex::cvar::Lifecycle::kInitOnly);

namespace {

std::atomic<uint32_t> g_dumped{0};

}  // namespace

namespace cod6::texdump {

/* La llama el cache de texturas cuando tiene una textura DXT >= 256 de ancho. */
void Dump(uint32_t base_address, uint32_t format, uint32_t width, uint32_t height,
          uint32_t pitch, bool tiled, uint32_t packed_mips, const uint8_t* base,
          uint32_t base_bytes, const uint8_t* mips, uint32_t mip_bytes) {
  const uint32_t limite = REXCVAR_GET(cod6_texdump_count);
  if (!limite) return;
  const uint32_t n = g_dumped.fetch_add(1, std::memory_order_relaxed);
  if (n >= limite) return;
  if (!base || !base_bytes) return;
  char nombre[96];
  std::snprintf(nombre, sizeof(nombre), "/switch/cod6-nx/texdump/t%02u.txt", unsigned(n));
  FILE* f = std::fopen(nombre, "wb");
  if (!f) {
    REXLOG_WARN("[texdump] no se pudo abrir {}", nombre);
    return;
  }
  std::fprintf(f,
               "addr=%08X format=%u %ux%u pitch=%u tiled=%d packed_mips=%u\n"
               "base_bytes=%u mip_bytes=%u mips_present=%d\n",
               unsigned(base_address), unsigned(format), unsigned(width), unsigned(height),
               unsigned(pitch), tiled ? 1 : 0, unsigned(packed_mips), unsigned(base_bytes),
               unsigned(mip_bytes), mips ? 1 : 0);
  std::fwrite(base, 1, base_bytes, f);
  if (mips && mip_bytes) {
    std::fprintf(f, "\n---MIPS---\n");
    std::fwrite(mips, 1, mip_bytes, f);
  }
  std::fclose(f);
  REXLOG_INFO("[texdump] {} addr={:08X} fmt={} {}x{} {} bytes", nombre, unsigned(base_address),
              unsigned(format), unsigned(width), unsigned(height), unsigned(base_bytes));
}

}  // namespace cod6::texdump
