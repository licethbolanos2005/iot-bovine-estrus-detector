import serial
import json
import time
import sys
import requests
from datetime import datetime, timezone

# ─── Configuración ──────────────────────────────────────────────────────────
SERIAL_PORT   = "COM3"
BAUD_RATE     = 115200
INFLUX_URL    = "http://localhost:8086"
INFLUX_TOKEN  = "mi-token-secreto"
INFLUX_ORG    = "vacas_iot"
INFLUX_BUCKET = "sensores_bovinos"

HEADERS = {
    "Authorization": f"Token {INFLUX_TOKEN}",
    "Content-Type": "text/plain"
}

NIVEL_CELO_MAP = {
    0: "No detectado",
    1: "Posible",
    2: "Probable",
    3: "Confirmado"
}

# ─── Escribir en InfluxDB via HTTP ──────────────────────────────────────────
def escribir_influx(line_protocol: str):
    url = f"{INFLUX_URL}/api/v2/write?org={INFLUX_ORG}&bucket={INFLUX_BUCKET}&precision=s"
    response = requests.post(url, headers=HEADERS, data=line_protocol)
    if response.status_code != 204:
        print(f"[ERROR InfluxDB] {response.status_code}: {response.text}")
        return False
    return True

# ─── Construir line protocol desde JSON ─────────────────────────────────────
def json_a_line_protocol(raw_json: str):
    data = json.loads(raw_json)

    device_id = data.get("id", "desconocido")
    s         = data.get("s", {})
    c         = data.get("c", {})

    # Sensores
    temperatura  = s.get("t",   0)
    fsr_voltaje  = s.get("fsr", 0)
    corriente_mA = s.get("mA",  0)
    voltaje_V    = s.get("V",   0)

    ax = s.get("a", {}).get("x", 0)
    ay = s.get("a", {}).get("y", 0)
    az = s.get("a", {}).get("z", 0)
    magnitud_accel = (ax**2 + ay**2 + az**2) ** 0.5

    gx = s.get("g", {}).get("x", 0)
    gy = s.get("g", {}).get("y", 0)
    gz = s.get("g", {}).get("z", 0)

    # Celo
    nivel_num = int(c.get("n",   0))
    nivel_str = NIVEL_CELO_MAP.get(nivel_num, "Desconocido")
    actividad = c.get("act", 0)
    temp_alta = 1 if c.get("ta", False) else 0
    monta     = 1 if c.get("m",  False) else 0
    rpm       = int(s.get("rpm", 0))

    timestamp = int(datetime.now(timezone.utc).timestamp())

    line = (
        f"lecturas_bovino,device_id={device_id} "
        f"temperatura={temperatura},"
        f"accel_x={ax},"
        f"accel_y={ay},"
        f"accel_z={az},"
        f"magnitud_accel={magnitud_accel:.4f},"
        f"giro_x={gx},"
        f"giro_y={gy},"
        f"giro_z={gz},"
        f"fsr_voltaje={fsr_voltaje},"
        f"corriente_mA={corriente_mA},"
        f"voltaje_V={voltaje_V},"
        f"celo_nivel_num={nivel_num},"
        f"celo_actividad={actividad},"
        f"celo_temp_alta={temp_alta},"
        f"celo_monta={monta},"
        f"rpm={rpm} "
        f"{timestamp}"
    )

    return line, device_id, nivel_str

# ─── Bucle principal ─────────────────────────────────────────────────────────
def main():
    print(f"[BRIDGE] Iniciando en puerto {SERIAL_PORT} a {BAUD_RATE} baud...")

    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=2)
        print(f"[SERIAL] Puerto {SERIAL_PORT} abierto OK")
    except serial.SerialException as e:
        print(f"[ERROR] No se pudo abrir {SERIAL_PORT}: {e}")
        sys.exit(1)

    buffer = ""
    print("[BRIDGE] Esperando datos del gateway...\n")

    while True:
        try:
            linea = ser.readline().decode("utf-8", errors="ignore").strip()

            if not linea:
                continue

            if linea.startswith("{"):
                buffer = linea
            elif buffer and linea.endswith("}"):
                buffer += linea
            else:
                print(f"[GATEWAY LOG] {linea}")
                continue

            try:
                line_protocol, device_id, nivel = json_a_line_protocol(buffer)
                ok = escribir_influx(line_protocol)

                if ok:
                    ts = datetime.now().strftime("%H:%M:%S")
                    print(f"[{ts}] ✓ Escrito InfluxDB | {device_id} | Celo: {nivel}")

                buffer = ""

            except json.JSONDecodeError:
                pass

        except KeyboardInterrupt:
            print("\n[BRIDGE] Detenido por el usuario")
            break
        except Exception as e:
            print(f"[ERROR] {e}")
            time.sleep(1)

    ser.close()

if __name__ == "__main__":
    main()