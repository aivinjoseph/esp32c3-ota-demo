# ESP32‑C3 OTA Demo

Minimal project that **builds, tests (optional), publishes, and over‑the‑air updates** firmware on an **ESP32‑C3** using GitHub Actions.

* Continuous Delivery: `.github/workflows/esp32.yml`
* Dual‑slot OTA partition table with rollback (`partitions.csv`)
* Wi‑Fi + HTTPS OTA logic in `main/ota_example.c`
* Placeholder `secrets.h.template` — copy to `secrets.h` and fill in your Wi‑Fi credentials **before building**.

## Quick start

1. Copy `secrets.h.template` ➜ `main/secrets.h` and set `WIFI_SSID`, `WIFI_PASS`, and `OTA_URL`.
2. Push the repo to GitHub. The **ESP32-Build** workflow compiles and uploads `firmware.bin` as an artefact.
3. **Factory‑flash** the board once:

   ```powershell
   esptool.py --chip esp32c3 --port COM3 --baud 460800 ^
     write_flash -z ^
       0x0      bootloader.bin ^
       0x8000   partition-table.bin ^
       0x10000  firmware.bin
   ```

4. The device boots, joins Wi‑Fi, checks `OTA_URL`.  
   If a newer image is present, it downloads into the inactive slot and reboots.

---  
*Public‑domain example – 2025-06-27*
