#ifndef GESTOR_JSON_H
#define GESTOR_JSON_H

#include "../tipos.h"

// ─── Tabla de claves abreviadas ───────────────────────
// "id"  = device_id
// "ts"  = timestamp ISO 8601
// "s"   = sensores
//   "t"   = temperatura °C
//   "fsr" = voltaje FSR V
//   "mA"  = corriente mA
//   "V"   = voltaje bus V
//   "a"   = acelerometro {x,y,z} m/s²
//   "g"   = giroscopio {x,y,z} rad/s
// "c"   = celo
//   "n"   = nivel (0=no,1=posible,2=probable,3=confirmado)
//   "act" = magnitud aceleracion m/s²
//   "ta"  = temperatura alta bool
//   "m"   = monta bool

#define JSON_BUFFER_SIZE 220

void gestor_json_serializar(const DatosNodo &datos,
                             char *buffer,
                             size_t buffer_size);

#endif