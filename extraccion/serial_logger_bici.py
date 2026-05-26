import serial
from pathlib import Path
from datetime import datetime

# =======================
# CONFIGURA AQUÍ TU PUERTO
# =======================
# Cambia COM13 por el puerto real de tu Arduino.
# Míralo en Arduino IDE: Tools > Port
SERIAL_PORT = "COM13"

BAUDRATE = 230400

OUT_DIR = Path("dataset_raw")
OUT_DIR.mkdir(exist_ok=True)

current_file = None
current_filename = None


def clean_label(label: str) -> str:
    return label.strip().replace(" ", "_").replace("-", "_")


def open_new_file(label: str, run_id: str):
    global current_file, current_filename

    if current_file is not None:
        current_file.close()

    label = clean_label(label)
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")

    try:
        run_number = int(run_id)
    except ValueError:
        run_number = 0

    current_filename = OUT_DIR / f"{label}_{timestamp}_run{run_number:03d}.csv"

    current_file = open(current_filename, "w", encoding="utf-8", newline="")
    current_file.write("timestamp,accX,accY,accZ,gyrX,gyrY,gyrZ\n")
    current_file.flush()

    print(f"\n[GRABANDO] {current_filename}")


def close_current_file():
    global current_file, current_filename

    if current_file is not None:
        current_file.flush()
        current_file.close()
        print(f"[GUARDADO] {current_filename}\n")

    current_file = None
    current_filename = None


def main():
    print(f"Abriendo puerto {SERIAL_PORT} a {BAUDRATE} baudios...")
    print("IMPORTANTE: cierra el Serial Monitor del Arduino IDE antes de ejecutar Python.")

    with serial.Serial(SERIAL_PORT, BAUDRATE, timeout=2) as ser:
        print("Puerto abierto. Esperando datos...")
        print("Pulsa Ctrl+C para salir.\n")

        while True:
            raw = ser.readline()

            if not raw:
                continue

            line = raw.decode("utf-8", errors="ignore").strip()

            if not line:
                continue

            if line.startswith("#START,"):
                # Formato: #START,label,contador
                parts = line.split(",")
                if len(parts) >= 3:
                    label = parts[1]
                    run_id = parts[2]
                    open_new_file(label, run_id)
                else:
                    print("[AVISO] START mal formado:", line)

            elif line.startswith("#END,"):
                close_current_file()

            elif line.startswith("#CLASS,"):
                print("[CLASE ACTUAL]", line.split(",", 1)[1])

            elif line.startswith("#"):
                print(line)

            elif line.startswith("DATA,"):
                # Formato Arduino:
                # DATA,timestamp_ms,accX,accY,accZ,gyrX,gyrY,gyrZ
                if current_file is not None:
                    data_without_prefix = line[5:]  # quita "DATA,"
                    current_file.write(data_without_prefix + "\n")
                else:
                    # Datos recibidos sin grabación abierta: se ignoran
                    pass

            else:
                print("[IGNORADO]", line)


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\nCerrando...")
        close_current_file()