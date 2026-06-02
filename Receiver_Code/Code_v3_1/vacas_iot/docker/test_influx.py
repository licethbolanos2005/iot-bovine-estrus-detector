import influxdb_client
from influxdb_client import InfluxDBClient, Point
from influxdb_client.client.write_api import SYNCHRONOUS
from datetime import datetime, timezone
import time

INFLUX_URL    = "http://localhost:8086"
INFLUX_TOKEN  = "mi-token-secreto"
INFLUX_ORG    = "vacas_iot"
INFLUX_BUCKET = "sensores_bovinos"

with InfluxDBClient(url=INFLUX_URL, token=INFLUX_TOKEN, org=INFLUX_ORG) as client:
    write_api = client.write_api(write_options=SYNCHRONOUS)
    
    punto = (
        Point("lecturas_bovino")
        .tag("device_id", "collar_001")
        .field("temperatura", 38.5)
        .field("celo_nivel_num", 0)
        .field("celo_nivel_str", "No detectado")
    )
    
    write_api.write(bucket=INFLUX_BUCKET, org=INFLUX_ORG, record=punto)
    time.sleep(1)
    print("✓ Punto escrito correctamente")