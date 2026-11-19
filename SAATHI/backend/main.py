import json
import os
import threading
from datetime import datetime, timezone

import serial
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware

app = FastAPI(title="SAATHI simulation API")
app.add_middleware(
    CORSMiddleware,
    allow_origins=["http://localhost:5173"],
    allow_methods=["GET"],
    allow_headers=["*"],
)

latest = None
lock = threading.Lock()


def read_serial():
    global latest
    port = os.environ.get("SAATHI_SERIAL_PORT")
    if not port:
        print("Set SAATHI_SERIAL_PORT to start reading the ESP32.")
        return

    while True:
        try:
            with serial.Serial(port, 115200, timeout=2) as connection:
                print(f"Reading SAATHI simulation data from {port}")
                while True:
                    line = connection.readline().decode("utf-8", errors="replace").strip()
                    if not line.startswith("{"):
                        continue
                    try:
                        data = json.loads(line)
                    except json.JSONDecodeError:
                        continue

                    # Never treat an unmarked device message as a clinical reading.
                    if data.get("device") != "SAATHI" or data.get("simulated") is not True:
                        continue

                    with lock:
                        latest = {
                            "temperature_c": data.get("temperature_c"),
                            "pulse_bpm": data.get("pulse_bpm"),
                            "spo2_percent": data.get("spo2_percent"),
                            "uptime_ms": data.get("uptime_ms"),
                            "simulated": True,
                            "received_at": datetime.now(timezone.utc).isoformat(),
                        }
        except serial.SerialException as exc:
            print(f"Serial connection unavailable: {exc}")
            threading.Event().wait(3)


@app.on_event("startup")
def start_reader():
    threading.Thread(target=read_serial, daemon=True).start()


@app.get("/api/readings")
def readings():
    with lock:
        return {"reading": latest}
