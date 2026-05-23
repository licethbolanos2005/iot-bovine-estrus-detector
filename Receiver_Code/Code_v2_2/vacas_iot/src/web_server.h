#pragma once
#include <Arduino.h>
#include <WebServer.h>

// Inicia el servidor HTTP en el puerto 80.
void startWebServer();

// Debe llamarse en el loop() principal para atender peticiones.
void handleWebServer();