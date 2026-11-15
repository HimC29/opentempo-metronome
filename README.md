# OpenTempo Metronome

An open-source digital metronome built on the ESP32-C3, styled after the classic pyramid metronome.

This is an early-stage hobby project — the hardware and firmware both work, but most of the planned feature set isn't built yet. See the roadmap below for what's done and what's not.

---

## Status

Working right now:
- Rotary encoder sets BPM (40–208 range, ±1 per detent)
- OLED shows current BPM and on/off state with a clean centered layout and large fonts
- Push-button toggles the metronome on/off
- A single tick sample plays through the I2S amp at the set tempo

Not built yet:
- Accented beat on beat 1 / time signature support
- Switchable sound packs (woodblock, click, rimshot, beep)
- Tap tempo
- Schematics (coming soon)
- Enclosure (will be posted on MakerWorld once it's done)

---

## Hardware

| Part | Notes |
|---|---|
| ESP32-C3 Super Mini | 160MHz RISC-V, 4MB flash, USB-C. No PCNT peripheral — encoder is read via GPIO interrupts, not a hardware pulse counter. |
| MAX98357A | I2S class-D mono amp |
| 4Ω 3W speaker | Mini full-range driver, 40mm diameter, round frame |
| SSD1306 OLED | 0.96", I2C |
| Rotary encoder (KY040-style) | With push switch |

A 100–470µF cap across the MAX98357A's VIN/GND is recommended to stop brownouts when the amp draws current for a tick.

### Pinout

| Component | Pin | ESP32-C3 GPIO |
|---|---|---|
| MAX98357A | BCLK | 0 |
| MAX98357A | LRC | 1 |
| MAX98357A | DIN | 3 |
| OLED | SDA | 4 |
| OLED | SCL | 5 |
| Encoder | CLK | 6 |
| Encoder | DT | 7 |
| Encoder | SW | 10 |

GPIO 2, 8, and 9 are avoided on purpose since they're boot-strapping pins on the C3 — wiring anything to them risks the board failing to boot correctly.

---

## Building

This is a PlatformIO project.

```bash
git clone https://github.com/HimC29/opentempo-metronome.git
cd opentempo-metronome
pio run --target upload
```

The board should auto-detect over USB-C. If it doesn't, check the port with `pio device list` and pass it explicitly:

```bash
pio run --target upload --upload-port /dev/ttyACM0
```

If the board doesn't reset into bootloader mode on its own, hold BOOT, tap RESET, then release BOOT right before flashing.

### Dependencies

Pulled automatically by PlatformIO from `platformio.ini`:

- [U8g2](https://github.com/olikraus/u8g2) — OLED display
- [OneButton](https://github.com/mathertel/OneButton) — debounced button handling
- `ESP_I2S` and `Wire` ship with the Arduino-ESP32 core, no separate install needed

The rotary encoder is read with a hand-rolled interrupt-driven quadrature decoder (`main.cpp`), not a library — the usual `ESP32Encoder` library depends on PCNT hardware the C3 doesn't have.

---

## Project structure

```text
opentempo-metronome/
├── src/
│   ├── main.cpp         — firmware
│   └── tick_data.h      — tick sample as a raw PCM byte array
├── platformio.ini
└── README.md
```

---

## Changelog

### v0.2.0
- Upgraded OLED UI with large centered fonts (`logisoso32_tn` for BPM and `logisoso16_tr` for state)

### v0.1.0
- Rotary encoder input with BPM clamped to 40–208
- OLED display of BPM and running state
- Button toggles running state on/off
- Single tick sample plays via I2S at the current tempo

---

## License

MIT

---

Bug reports and suggestions welcome — open an issue.
