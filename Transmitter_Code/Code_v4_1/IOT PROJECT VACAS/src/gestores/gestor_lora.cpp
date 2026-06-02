#include "gestor_lora.h"
#include "../config.h"
#include <RadioLib.h>
#include <SPI.h>

static SPIClass *spi_lora = nullptr;
static SX1262   *radio    = nullptr;

bool gestor_lora_init() {
    spi_lora = new SPIClass(FSPI);  // FSPI=1, SD usa bus 2 — no hay conflicto
    if (!spi_lora) {
        Serial.println("[LORA] Error: no se pudo crear SPIClass");
        return false;
    }
    spi_lora->begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);
    delay(100);

    Module *mod = new Module(LORA_CS, LORA_IRQ, LORA_RST, LORA_BUSY, *spi_lora);
    radio = new SX1262(mod);

    int state = radio->begin(
        LORA_FREQ,
        LORA_BW,
        LORA_SF,
        LORA_CR,
        LORA_SW,
        LORA_PWR,
        LORA_PREAMBLE
    );

    if (state != RADIOLIB_ERR_NONE) {
        Serial.printf("[LORA] Error init: %d\n", state);
        return false;
    }

    Serial.println("[LORA] SX1262 OK");
    return true;
}

bool gestor_lora_transmitir(const char *payload) {
    if (!radio) {
        Serial.println("[LORA] Error: radio null");
        return false;
    }

    int state = radio->transmit((uint8_t *)payload, strlen(payload));

    if (state != RADIOLIB_ERR_NONE) {
        Serial.printf("[LORA] Error TX: %d\n", state);
        return false;
    }
    return true;
}