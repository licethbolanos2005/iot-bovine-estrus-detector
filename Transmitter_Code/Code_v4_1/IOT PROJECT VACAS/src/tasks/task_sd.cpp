#include "queues_globales.h"
#include "task_sd.h"
#include "../config.h"
#include <SD.h>
#include <SPI.h>

static SPIClass spi_sd(HSPI);
static bool sd_disponible = false;

#define ARCHIVO_LOG    "/registro.csv"
#define ENCABEZADO_CSV "fecha,hora,temperatura,accel_x,accel_y,accel_z,magnitud,voltaje_V,corriente_mA,fsr_V,nivel_celo,monta\n"

// ─── Inicializar SD ───────────────────────────────────
static bool sd_init() {
    spi_sd.begin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI, PIN_SD_CS);

    if (!SD.begin(PIN_SD_CS, spi_sd)) {
        Serial.println("[TASK_SD] MicroSD no encontrada");
        spi_sd.end();
        return false;
    }

    uint8_t tipo = SD.cardType();
    if (tipo == CARD_NONE) {
        Serial.println("[TASK_SD] No hay tarjeta insertada");
        return false;
    }

    const char *tipoStr;
    switch (tipo) {
        case CARD_MMC:  tipoStr = "MMC";     break;
        case CARD_SD:   tipoStr = "SDSC";    break;
        case CARD_SDHC: tipoStr = "SDHC";    break;
        default:        tipoStr = "Desconocido";
    }

    uint64_t tamanoMB = SD.cardSize() / (1024 * 1024);
    Serial.printf("[TASK_SD] MicroSD OK — Tipo: %s | Tamano: %llu MB\n",
        tipoStr, tamanoMB);

    // ── Crear archivo con encabezados si no existe ──
    if (!SD.exists(ARCHIVO_LOG)) {
        File archivo = SD.open(ARCHIVO_LOG, FILE_WRITE);
        if (archivo) {
            archivo.print(ENCABEZADO_CSV);
            archivo.close();
            Serial.println("[TASK_SD] Archivo registro.csv creado con encabezados");
        }
    } else {
        Serial.println("[TASK_SD] Archivo registro.csv existente — modo append");
    }

    return true;
}

// ─── Guardar lectura en CSV ───────────────────────────
static void sd_guardar(const DatosNodo &datos) {
    File archivo = SD.open(ARCHIVO_LOG, FILE_APPEND);
    if (!archivo) {
        Serial.println("[TASK_SD] Error abriendo archivo para escritura");
        return;
    }

    archivo.printf(
        "%04d-%02d-%02d,%02d:%02d:%02d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.3f,%d,%d\n",
        datos.anio, datos.mes, datos.dia,
        datos.hora, datos.minuto, datos.segundo,
        datos.temperatura,
        datos.accel_x, datos.accel_y, datos.accel_z,
        datos.magnitud_accel,
        datos.voltaje_V,
        datos.corriente_mA,
        datos.fsr_voltaje,
        (int)datos.nivel_celo,
        (int)datos.monta
    );

    archivo.close();
}

// ─── Task principal ───────────────────────────────────
void task_sd(void *pvParameters) {
    Serial.println("[TASK_SD] Iniciada");

    // Pequeno delay para no chocar con otras inicializaciones
    vTaskDelay(pdMS_TO_TICKS(2000));

    sd_disponible = sd_init();
    if (!sd_disponible) {
        Serial.println("[TASK_SD] Funcionando sin almacenamiento local");
    }

    DatosNodo datos;
    uint32_t escrituras_ok    = 0;
    uint32_t escrituras_error = 0;

    for (;;) {
        if (sd_disponible) {
            if (xSemaphoreTake(mutex_datos, pdMS_TO_TICKS(500)) == pdTRUE) {
                datos = datos_compartidos;
                xSemaphoreGive(mutex_datos);

                if (datos.datos_validos) {
                    sd_guardar(datos);
                    escrituras_ok++;

                    if (escrituras_ok % 10 == 0) {
                        Serial.printf("[TASK_SD] %lu registros guardados | %lu errores\n",
                            escrituras_ok, escrituras_error);
                    }
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(INTERVALO_SD));
    }
}