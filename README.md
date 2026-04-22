# DoorUnlock — Mongoose OS Firmware

A WiFi-enabled door unlock controller built on [Mongoose OS](https://mongoose-os.com/). It runs on an **ESP8266** or **ESP32** microcontroller, drives a relay to pulse-unlock a door, and exposes a browser-based setup UI plus remote RPC commands.

---

## Table of Contents

- [How It Works](#how-it-works)
- [Hardware Requirements](#hardware-requirements)
- [Wiring](#wiring)
- [Project Structure](#project-structure)
- [Configuration](#configuration)
- [Prerequisites](#prerequisites)
- [Build & Flash](#build--flash)
  - [Build Locally with Docker](#build-locally-with-docker)
  - [Build & Flash via mos CLI](#build--flash-via-mos-cli)
- [First-Time Setup (Web UI)](#first-time-setup-web-ui)
- [Unlocking the Door](#unlocking-the-door)
  - [From the Web UI](#from-the-web-ui)
  - [Via RPC over HTTP](#via-rpc-over-http)
  - [Via Mongoose Dash (Cloud)](#via-mongoose-dash-cloud)
- [RPC Reference](#rpc-reference)
- [Factory Reset](#factory-reset)
- [Cloud / Dash Integration](#cloud--dash-integration)
- [Troubleshooting](#troubleshooting)

---

## How It Works

```
[ Browser / App / Cloud ]
         │
         │  HTTP RPC  (e.g. POST /rpc/Wifi.Unlock)
         ▼
  ┌─────────────────────────────┐
  │   ESP8266 / ESP32           │
  │                             │
  │   Mongoose OS firmware      │
  │   ├── init.js  (MJS logic)  │
  │   └── index.html (Web UI)   │
  │                             │
  │   GPIO pin 14 ──────────────┼──► Relay ──► Door lock
  └─────────────────────────────┘
```

1. On boot the device starts a **WiFi access point** (`Bruno_??????`, password `432FACC99A`).
2. You connect to that AP and open `http://192.168.4.1` to configure your home WiFi and pulse duration.
3. After saving, the device joins your network and listens for RPC commands.
4. An "unlock" command tells the firmware to energise the relay for `pulseTm` milliseconds, then de-energise it — momentarily releasing the door latch.

---

## Hardware Requirements

| Component | Notes |
|-----------|-------|
| ESP8266 (Wemos D1 mini recommended) **or** ESP32 | Any Mongoose OS–compatible board works |
| 5 V relay module (single channel) | Make sure it is logic-level compatible with 3.3 V GPIO |
| Door electric strike / magnetic lock | Must match relay output (12 V / 24 V supply usually needed) |
| USB cable | For flashing; also powers the MCU during development |
| External power supply | For the lock coil (do **not** power it from the MCU 3.3 V rail) |

---

## Wiring

```
Wemos D1 mini (ESP8266)      Relay module
─────────────────────────    ──────────────
D5  (GPIO 14)  ───────────►  IN  (signal)
3V3            ───────────►  VCC
GND            ───────────►  GND

Relay NO/COM contacts ──► Lock supply circuit
```

> **Default GPIO is 14** (D5 on Wemos D1 mini). You can change this in the web UI or by editing `hardware.relayPin` in `mos.yml`.

---

## Project Structure

```
DoorUnlock/
├── src/
│   └── main.c          # Minimal C bootstrap — just calls mgos_app_init
├── fs/
│   ├── init.js         # All runtime logic (relay, RPC handlers, cloud events)
│   ├── index.html      # Browser-based WiFi + hardware setup UI
│   └── zepto.min.js    # Lightweight jQuery-compatible AJAX library
├── mos.yml             # Mongoose OS manifest (config schema, libs, build settings)
├── LICENSE             # Apache 2.0
└── README.md
```

### Key files explained

**`mos.yml`** — The build manifest. Declares:
- Custom config keys under `hardware.*` (relay pin, pulse time, etc.)
- Which Mongoose OS libraries to pull in (WiFi, HTTP server, RPC, OTA, Dash …)
- Default network settings (AP SSID, IP)
- A build condition that increases flash slot size for ESP32

**`fs/init.js`** — The entire device behaviour in ~60 lines of MJS (embedded JavaScript):
- Reads config on boot and sets up the relay GPIO
- Defines the `unlock()` function — toggles GPIO for `pulseTm` ms
- Registers two RPC handlers: `Wifi.Unlock` and `Factory.reset`
- Listens to cloud connect/disconnect events and schedules a self-reboot if the cloud stays unreachable for 5 minutes

**`fs/index.html`** — A single-page web app served directly from the device:
- Scan & select WiFi networks
- Set STA credentials and pulse duration
- Trigger an unlock from the browser

---

## Configuration

All runtime config lives under the `hardware` namespace defined in `mos.yml`:

| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `hardware.relayPin` | int | `14` | GPIO pin connected to the relay signal wire |
| `hardware.pulseTm` | int | `100` | How long (ms) the relay stays energised per unlock |
| `hardware.initialRelayStat` | bool | `false` | GPIO idle level — set to `true` if your relay is active-LOW |
| `hardware.unlock` | bool | `false` | Set to `true` + reboot to trigger unlock on next boot |
| `wifi.ap.ssid` | string | `Bruno_??????` | AP network name (`??????` is filled with MAC bytes) |
| `wifi.ap.pass` | string | `432FACC99A` | AP password — **change this before deploying** |
| `dash.token` | string | `""` | Your Mongoose Dash device token (leave empty to disable cloud) |

You can change any value through the web UI, via `mos config-set`, or by editing `mos.yml` defaults before building.

---

## Prerequisites

Install the **Mongoose OS `mos` CLI** ([official guide](https://mongoose-os.com/docs/mongoose-os/quickstart/setup.md)):

```bash
# macOS
brew tap cesanta/mos
brew install mos

# or via the universal installer
curl -fsSL https://mongoose-os.com/downloads/mos/install.sh | /bin/bash
```

Verify the install:

```bash
mos --version
```

**Docker** is required only for local offline builds (see below). Install [Docker Desktop](https://docs.docker.com/desktop/).

---

## Build & Flash

### Build Locally with Docker

Use this if you are offline or want a fully reproducible build without the Mongoose OS cloud builder:

```bash
mos build --local
```

This spins up the Mongoose OS Docker build container, compiles the firmware, and places the output in `build/fw.zip`.

### Build & Flash via mos CLI

> Connect your ESP8266/ESP32 board via USB before running these commands.

```bash
# 1. Build the firmware (uses cloud builder by default)
mos build --platform esp8266

# For ESP32:
mos build --platform esp32

# 2. Flash to the connected device
mos flash

# 3. (Optional) Open the serial console to see boot logs
mos console
```

To build and flash in one step:

```bash
mos build --platform esp8266 && mos flash
```

---

## First-Time Setup (Web UI)

1. After flashing, the device boots into AP mode.
2. On your phone or laptop, connect to the WiFi network **`Bruno_xxxxxx`** (password: `432FACC99A`).
3. Open a browser and navigate to **`http://192.168.4.1`**.
4. The setup page lets you:
   - **Scan** for nearby networks and pick yours.
   - Enter your WiFi **password**.
   - Set the **pulse duration** (ms) — how long the relay fires.
   - Press **Save** — the device saves the config and reboots into station mode.
5. Once rebooted, the device joins your home network. Find its IP from your router's DHCP table (or run `mos call Sys.GetInfo`).

---

## Unlocking the Door

### From the Web UI

While connected to the same network as the device, open `http://<device-ip>` and click **Unlock Door**.

> During initial setup the web UI is reachable at `http://192.168.4.1` while still on the AP.

### Via RPC over HTTP

Send a POST request to the device's IP:

```bash
curl -X POST http://<device-ip>/rpc/Wifi.Unlock
```

Expected response:

```json
{"success": true, "message": "Door Opened"}
```

### Via Mongoose Dash (Cloud)

If `dash.token` is configured, you can call the RPC from anywhere:

```bash
mos --port wss://dash.mongoose-os.com/api/v2/rpc call Wifi.Unlock
```

---

## RPC Reference

| Method | Payload | Description |
|--------|---------|-------------|
| `Wifi.Unlock` | none | Pulses the relay to unlock the door |
| `Factory.reset` | none | Wipes all config and reboots the device |
| `Config.Set` | `{"config": {...}}` | Update any config key at runtime |
| `Config.Save` | `{"reboot": true}` | Persist config changes and optionally reboot |
| `Wifi.Scan` | none | Return list of visible SSIDs |
| `Sys.GetInfo` | none | Return firmware version, uptime, free RAM, IP address |

Standard Mongoose OS RPC methods (`Config.*`, `Wifi.*`, `Sys.*`) are provided by the bundled libraries — see the [Mongoose OS RPC docs](https://mongoose-os.com/docs/mongoose-os/api/rpc/).

---

## Factory Reset

**Via RPC:**

```bash
curl -X POST http://<device-ip>/rpc/Factory.reset
```

**Manually:** Hold the flash button on the board for ~5 seconds while it boots (hardware-specific; check your board's docs). The device will wipe config and restart in AP mode.

---

## Cloud / Dash Integration

[Mongoose Dash](https://mongoose-os.com/docs/mongoose-os/cloud/dash.md) is a free cloud dashboard for Mongoose OS devices.

1. Create an account at [dash.mongoose-os.com](https://dash.mongoose-os.com).
2. Add a new device and copy the token.
3. Set the token on the device:

```bash
mos config-set dash.token=<your-token>
```

4. The device will connect to Dash on next boot. You can then call RPCs, view logs, and push OTA updates from the dashboard.

> The firmware implements a self-healing behaviour: if the cloud disconnects unexpectedly after having been connected, the device schedules a reboot in **5 minutes**. If the cloud reconnects before the timer fires, the reboot is cancelled. This keeps stuck devices from staying offline indefinitely.

---

## Troubleshooting

| Symptom | Fix |
|---------|-----|
| Can't see the AP after flashing | Check serial console with `mos console`; confirm WiFi AP libs are present in `mos.yml` |
| Door doesn't unlock | Verify GPIO pin number matches your wiring (`hardware.relayPin`). Try increasing `pulseTm` to 500 ms |
| Relay clicks but lock doesn't open | Check external power supply to the lock coil; relay COM/NO vs COM/NC orientation |
| Device keeps rebooting | Could be a brown-out from the relay coil. Add a flyback diode across the lock coil; use a separate power supply |
| `mos flash` can't find port | Run `mos ports` to list available serial ports, then `mos flash --port /dev/tty.usbserial-XXXX` |
| Web UI not loading | Confirm you are on the same network as the device; try `http://192.168.4.1` while on the AP |
| Cloud token not working | Re-copy the token from Dash; set with `mos config-set dash.token=<token>` and reboot |

---

## License

Apache 2.0 — see [LICENSE](LICENSE).
